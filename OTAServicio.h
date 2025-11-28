#ifndef OTASERVICIO_H
#define OTASERVICIO_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Update.h>
#include <SPIFFS.h>
#include <esp_task_wdt.h>
#include "Debug.h"
#include "Configuracion.h"

// Estados de actualización
enum OTAState {
    OTA_IDLE,
    OTA_CHECKING,
    OTA_DOWNLOADING_FIRMWARE,
    OTA_DOWNLOADING_SPIFFS,
    OTA_INSTALLING,
    OTA_SUCCESS,
    OTA_ERROR
};

// Resultado de comprobación de versión
struct VersionInfo {
    bool newVersionAvailable;
    String latestVersion;
    String currentVersion;
    String firmwareUrl;
    String spiffsUrl;
    size_t firmwareSize;
    size_t spiffsSize;
    String releaseNotes;
};

class OTAServicio {
public:
    OTAServicio();
    
    // Inicialización
    void begin();
    
    // Comprobación de actualizaciones
    VersionInfo checkForUpdates();
    bool isNewVersionAvailable(const String& latestVersion);
    
    // Descarga e instalación
    bool updateFirmware(const String& url, size_t expectedSize);
    bool updateSPIFFS(const String& url, size_t expectedSize);
    bool performFullUpdate(const VersionInfo& versionInfo);
    
    // Actualización asíncrona (no bloquea el WebSocket)
    void performFullUpdateAsync(const VersionInfo& versionInfo);
    
    // Auto-update
    void enableAutoUpdate(bool enable);
    void checkAutoUpdate();
    
    // Getters de estado
    OTAState getState() const { return _state; }
    int getProgress() const { return _progress; }
    String getLastError() const { return _lastError; }
    String getCurrentVersion() const { return Config::OTA::FIRMWARE_VERSION; }
    
    // Callbacks para notificaciones
    typedef void (*ProgressCallback)(int progress, const char* message);
    typedef void (*ErrorCallback)(const char* error);
    typedef void (*SuccessCallback)(const char* version);
    
    void setProgressCallback(ProgressCallback cb) { _progressCallback = cb; }
    void setErrorCallback(ErrorCallback cb) { _errorCallback = cb; }
    void setSuccessCallback(SuccessCallback cb) { _successCallback = cb; }

private:
    OTAState _state;
    int _progress;
    String _lastError;
    bool _autoUpdateEnabled;
    unsigned long _lastUpdateCheck;
    TaskHandle_t _otaTaskHandle;
    
    // Callbacks
    ProgressCallback _progressCallback;
    ErrorCallback _errorCallback;
    SuccessCallback _successCallback;
    
    // Cliente HTTPS
    WiFiClientSecure _httpsClient;
    
    // Métodos internos
    bool downloadFile(const String& url, uint8_t* buffer, size_t bufferSize, size_t expectedSize);
    bool verifyUpdate();
    void setState(OTAState state);
    void setProgress(int progress, const char* message = nullptr);
    void setError(const char* error);
    String parseGitHubResponse(const String& json, VersionInfo& info);
    int compareVersions(const String& v1, const String& v2);
};

extern OTAServicio OTA;

// Parámetros para tarea OTA asíncrona
struct OTATaskParams {
    OTAServicio* otaService;
    VersionInfo versionInfo;
};

#endif // OTASERVICIO_H
