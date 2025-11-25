#include "OTAServicio.h"

// Compatibilidad con diferentes versiones del framework ESP32
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 0, 0)
    // ESP-IDF 4.x y superior: esp_task_wdt_init requiere esp_task_wdt_config_t
    #define INIT_WATCHDOG(timeout_s, panic) \
        do { \
            esp_task_wdt_config_t wdt_config = { \
                .timeout_ms = (timeout_s) * 1000, \
                .idle_core_mask = 0, \
                .trigger_panic = (panic) \
            }; \
            esp_task_wdt_init(&wdt_config); \
        } while(0)
#else
    // ESP-IDF 3.x: esp_task_wdt_init toma timeout y panic directamente
    #define INIT_WATCHDOG(timeout_s, panic) esp_task_wdt_init(timeout_s, panic)
#endif

OTAServicio OTA;

OTAServicio::OTAServicio() 
    : _state(OTA_IDLE)
    , _progress(0)
    , _autoUpdateEnabled(false)
    , _lastUpdateCheck(0)
    , _progressCallback(nullptr)
    , _errorCallback(nullptr)
    , _successCallback(nullptr)
{
}

void OTAServicio::begin() {
    DBG_OTA("Servicio OTA iniciado");
    DBG_OTA_PRINTF("Version actual: %s", Config::OTA::FIRMWARE_VERSION.c_str());
    setState(OTA_IDLE);
}

void OTAServicio::enableAutoUpdate(bool enable) {
    _autoUpdateEnabled = enable;
    Config::OTA::AUTO_UPDATE_ENABLED = enable;  // Sincronizar con config
    if (enable) {
        DBG_OTA("Auto-update habilitado");
        _lastUpdateCheck = millis();
    } else {
        DBG_OTA("Auto-update deshabilitado");
    }
}

void OTAServicio::checkAutoUpdate() {
    if (!_autoUpdateEnabled) return;
    
    unsigned long now = millis();
    if (now - _lastUpdateCheck < Config::OTA::AUTO_UPDATE_INTERVAL_HOURS * 3600000UL) {
        return; // Aún no es hora de comprobar
    }
    
    _lastUpdateCheck = now;
    DBG_OTA("Comprobacion automatica de actualizaciones...");
    
    VersionInfo info = checkForUpdates();
    if (info.newVersionAvailable) {
        DBG_OTA_PRINTF("Nueva version disponible: %s", info.latestVersion.c_str());
        // Aquí podrías enviar notificación Telegram
        // Por ahora solo registramos el evento
    }
}

VersionInfo OTAServicio::checkForUpdates() {
    VersionInfo info;
    info.newVersionAvailable = false;
    info.currentVersion = Config::OTA::FIRMWARE_VERSION;
    
    setState(OTA_CHECKING);
    setProgress(10, "Conectando a GitHub...");
    
    // Usar WiFiClientSecure sin validación de certificado para evitar problemas
    WiFiClientSecure client;
    client.setInsecure(); // No validar certificado SSL
    
    HTTPClient http;
    http.setTimeout(15000); // 15 segundos timeout
    
    String apiUrl = Config::OTA::getGitHubApiUrl();
    DBG_OTA_PRINTF("Consultando: %s", apiUrl.c_str());
    
    if (!http.begin(client, apiUrl)) {
        setError("Error al iniciar conexion HTTPS");
        setState(OTA_ERROR);
        return info;
    }
    
    // Headers requeridos por GitHub API
    http.addHeader("Accept", "application/vnd.github+json");
    http.addHeader("User-Agent", "ESP32-OTA-Client");
    
    setProgress(30, "Consultando ultima version...");
    
    int httpCode = http.GET();
    
    DBG_OTA_PRINTF("Codigo HTTP: %d", httpCode);
    
    if (httpCode != HTTP_CODE_OK) {
        if (httpCode > 0) {
            setError(("Error HTTP: " + String(httpCode)).c_str());
        } else {
            setError(("Error de conexion: " + String(httpCode)).c_str());
        }
        http.end();
        setState(OTA_ERROR);
        return info;
    }
    
    setProgress(50, "Procesando respuesta...");
    
    String payload = http.getString();
    http.end();
    
    DBG_OTA_PRINTF("Respuesta recibida: %d bytes", payload.length());
    
    // Parsear JSON
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
        setError(("Error parseando JSON: " + String(error.c_str())).c_str());
        setState(OTA_ERROR);
        return info;
    }
    
    // Extraer información de la release
    const char* tagName = doc["tag_name"];
    if (!tagName) {
        setError("No se encontro tag_name en la respuesta");
        setState(OTA_ERROR);
        return info;
    }
    
    // Quitar la 'v' o 'V' del principio si existe (v1.0.0 -> 1.0.0)
    String version = String(tagName);
    if (version.startsWith("v") || version.startsWith("V")) {
        version = version.substring(1);
    }
    
    info.latestVersion = version;
    info.releaseNotes = doc["body"] | "";
    
    // Buscar assets (archivos .bin)
    JsonArray assets = doc["assets"];
    for (JsonObject asset : assets) {
        String name = asset["name"].as<String>();
        String downloadUrl = asset["browser_download_url"].as<String>();
        int size = asset["size"];
        
        if (name.indexOf("firmware") >= 0 && name.endsWith(".bin")) {
            info.firmwareUrl = downloadUrl;
            info.firmwareSize = size;
        } else if (name.indexOf("spiffs") >= 0 && name.endsWith(".bin")) {
            info.spiffsUrl = downloadUrl;
            info.spiffsSize = size;
        }
    }
    
    // Verificar si hay nueva versión
    info.newVersionAvailable = isNewVersionAvailable(version);
    
    setProgress(100, "Comprobacion completada");
    setState(OTA_IDLE);
    
    DBG_OTA_PRINTF("Version actual: %s, Ultima: %s, Nueva disponible: %s", 
                  info.currentVersion.c_str(), 
                  info.latestVersion.c_str(),
                  info.newVersionAvailable ? "SI" : "NO");
    
    return info;
}

bool OTAServicio::isNewVersionAvailable(const String& latestVersion) {
    return compareVersions(Config::OTA::FIRMWARE_VERSION, latestVersion) < 0;
}

int OTAServicio::compareVersions(const String& v1, const String& v2) {
    // Comparar versiones semánticas (1.2.3)
    int v1_major = 0, v1_minor = 0, v1_patch = 0;
    int v2_major = 0, v2_minor = 0, v2_patch = 0;
    
    sscanf(v1.c_str(), "%d.%d.%d", &v1_major, &v1_minor, &v1_patch);
    sscanf(v2.c_str(), "%d.%d.%d", &v2_major, &v2_minor, &v2_patch);
    
    if (v1_major != v2_major) return v1_major - v2_major;
    if (v1_minor != v2_minor) return v1_minor - v2_minor;
    return v1_patch - v2_patch;
}

bool OTAServicio::performFullUpdate(const VersionInfo& versionInfo) {
    if (!versionInfo.newVersionAvailable) {
        setError("No hay actualizacion disponible");
        return false;
    }
    
    DBG_OTA("Iniciando actualizacion completa...");
    
    // Aumentar timeout del watchdog durante la actualización (60 segundos)
    INIT_WATCHDOG(60, true);
    DBG_OTA("Watchdog timeout aumentado a 60s");
    
    // 1. Actualizar firmware
    if (!versionInfo.firmwareUrl.isEmpty()) {
        if (!updateFirmware(versionInfo.firmwareUrl, versionInfo.firmwareSize)) {
            setError("Error actualizando firmware");
            // Restaurar watchdog normal antes de salir
            INIT_WATCHDOG(30, true);
            return false;
        }
    }
    
    // 2. Actualizar SPIFFS (opcional)
    if (!versionInfo.spiffsUrl.isEmpty()) {
        if (!updateSPIFFS(versionInfo.spiffsUrl, versionInfo.spiffsSize)) {
            DBG_OTA("Advertencia: Error actualizando SPIFFS (no critico)");
            // No es crítico, continuamos
        }
    }
    
    setState(OTA_SUCCESS);
    if (_successCallback) {
        _successCallback(versionInfo.latestVersion.c_str());
    }
    
    DBG_OTA("Actualizacion completada. Reiniciando en 5 segundos...");
    
    // Dar tiempo para que se envíen los mensajes WebSocket
    for (int i = 5; i > 0; i--) {
        DBG_OTA_PRINTF("Reiniciando en %d...", i);
        delay(1000);
    }
    
    ESP.restart();
    
    return true;
}

bool OTAServicio::updateFirmware(const String& url, size_t expectedSize) {
    setState(OTA_DOWNLOADING_FIRMWARE);
    setProgress(0, "Descargando firmware...");
    
    DBG_OTA_PRINTF("Descargando firmware desde: %s", url.c_str());
    
    WiFiClientSecure client;
    client.setInsecure();
    
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    
    if (!http.begin(client, url)) {
        setError("Error al conectar para descargar firmware");
        return false;
    }
    
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        setError(("Error HTTP descargando firmware: " + String(httpCode)).c_str());
        http.end();
        return false;
    }
    
    size_t contentLength = http.getSize();
    DBG_OTA_PRINTF("Tamano firmware: %d bytes", contentLength);
    
    if (contentLength <= 0 || contentLength > Config::OTA::MAX_FIRMWARE_SIZE) {
        setError("Tamano de firmware invalido");
        http.end();
        return false;
    }
    
    setState(OTA_INSTALLING);
    
    if (!Update.begin(contentLength, U_FLASH)) {
        setError(("Error iniciando Update: " + String(Update.errorString())).c_str());
        http.end();
        return false;
    }
    
    WiFiClient* stream = http.getStreamPtr();
    size_t written = 0;
    uint8_t buffer[2048]; // Buffer más grande para descargas más rápidas
    
    while (http.connected() && written < contentLength) {
        size_t available = stream->available();
        if (available) {
            int bytesRead = stream->readBytes(buffer, min(available, sizeof(buffer)));
            if (Update.write(buffer, bytesRead) != bytesRead) {
                setError("Error escribiendo firmware");
                Update.abort();
                http.end();
                return false;
            }
            written += bytesRead;
            
            int progress = (written * 100) / contentLength;
            setProgress(progress, "Instalando firmware...");
            
            if (progress % 10 == 0) {
                DBG_OTA_PRINTF("Progreso firmware: %d%%", progress);
            }
        }
        esp_task_wdt_reset(); // Reset watchdog
        yield(); // Dar tiempo a otras tareas
        vTaskDelay(1); // Delay de 1 tick para permitir que otras tareas se ejecuten
    }
    
    http.end();
    
    if (!Update.end(true)) {
        setError(("Error finalizando update: " + String(Update.errorString())).c_str());
        return false;
    }
    
    DBG_OTA("Firmware actualizado correctamente");
    setProgress(100, "Firmware instalado");
    return true;
}

bool OTAServicio::updateSPIFFS(const String& url, size_t expectedSize) {
    setState(OTA_DOWNLOADING_SPIFFS);
    setProgress(0, "Descargando SPIFFS...");
    
    DBG_OTA_PRINTF("Descargando SPIFFS desde: %s", url.c_str());
    
    WiFiClientSecure client;
    client.setInsecure();
    
    HTTPClient http;
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    
    if (!http.begin(client, url)) {
        setError("Error al conectar para descargar SPIFFS");
        return false;
    }
    
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        setError(("Error HTTP descargando SPIFFS: " + String(httpCode)).c_str());
        http.end();
        return false;
    }
    
    size_t contentLength = http.getSize();
    DBG_OTA_PRINTF("Tamano SPIFFS: %d bytes", contentLength);
    
    if (contentLength <= 0 || contentLength > Config::OTA::MAX_SPIFFS_SIZE) {
        setError("Tamano de SPIFFS invalido");
        http.end();
        return false;
    }
    
    if (!Update.begin(contentLength, U_SPIFFS)) {
        setError(("Error iniciando Update SPIFFS: " + String(Update.errorString())).c_str());
        http.end();
        return false;
    }
    
    WiFiClient* stream = http.getStreamPtr();
    size_t written = 0;
    uint8_t buffer[2048]; // Buffer más grande para descargas más rápidas
    
    while (http.connected() && written < contentLength) {
        size_t available = stream->available();
        if (available) {
            int bytesRead = stream->readBytes(buffer, min(available, sizeof(buffer)));
            if (Update.write(buffer, bytesRead) != bytesRead) {
                setError("Error escribiendo SPIFFS");
                Update.abort();
                http.end();
                return false;
            }
            written += bytesRead;
            
            int progress = (written * 100) / contentLength;
            setProgress(progress, "Instalando SPIFFS...");
            
            if (progress % 10 == 0) {
                DBG_OTA_PRINTF("Progreso SPIFFS: %d%%", progress);
            }
        }
        esp_task_wdt_reset(); // Reset watchdog
        yield(); // Dar tiempo a otras tareas
        vTaskDelay(1); // Delay de 1 tick
    }
    
    http.end();
    
    if (!Update.end(true)) {
        setError(("Error finalizando update SPIFFS: " + String(Update.errorString())).c_str());
        return false;
    }
    
    DBG_OTA("SPIFFS actualizado correctamente");
    setProgress(100, "SPIFFS instalado");
    return true;
}

void OTAServicio::setState(OTAState state) {
    _state = state;
}

void OTAServicio::setProgress(int progress, const char* message) {
    _progress = progress;
    if (_progressCallback && message) {
        _progressCallback(progress, message);
    }
}

void OTAServicio::setError(const char* error) {
    _lastError = String(error);
    DBG_OTA_PRINTF("ERROR: %s", error);
    setState(OTA_ERROR);
    if (_errorCallback) {
        _errorCallback(error);
    }
}
