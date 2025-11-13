/**
 * @file TelegramServicio.cpp
 * @brief Implementación del servicio de comunicación con Telegram
 */

#include "TelegramServicio.h"
#include "Campanario.h"
#include "RTC.h"
#include "Acciones.h"
#include "Auxiliar.h"

// Función helper para URL encoding
String urlEncode(const String& str) {
    String encoded = "";
    char c;
    char code0;
    char code1;
    for (int i = 0; i < str.length(); i++) {
        c = str.charAt(i);
        if (c == ' ') {
            encoded += '+';
        } else if (isalnum(c)) {
            encoded += c;
        } else {
            code1 = (c & 0xf) + '0';
            if ((c & 0xf) > 9) {
                code1 = (c & 0xf) - 10 + 'A';
            }
            c = (c >> 4) & 0xf;
            code0 = c + '0';
            if (c > 9) {
                code0 = c - 10 + 'A';
            }
            encoded += '%';
            encoded += code0;
            encoded += code1;
        }
    }
    return encoded;
}

// Variables externas
extern bool lConexionInternet;

// Instancia global
TelegramServicio telegramBot;

// Constantes de configuración
namespace TelegramConfig {
    const unsigned long CHECK_INTERVAL_MS = 5000;      // Verificar mensajes cada 5 segundos (menos frecuente)
    const unsigned long NOTIFICATION_COOLDOWN_MS = 30000; // Cooldown de 30 segundos entre notificaciones del mismo tipo
    const int MAX_MESSAGE_LENGTH = 4096;               // Longitud máxima de mensaje de Telegram
    const String API_BASE_URL = "https://api.telegram.org/bot";
    const int MAX_RETRIES = 2;                         // Máximo número de reintentos
    const unsigned long RETRY_DELAY_MS = 1000;         // Delay entre reintentos
    
    // Configuración de modo de operación
    const bool SEND_ONLY_MODE = false;                 // true = solo envío, false = envío y recepción
}

TelegramServicio::TelegramServicio() {
    lastUpdateId = 0;
    lastCheckTime = 0;
    notificationCooldown = 0;
    serviceEnabled = false;
}

bool TelegramServicio::begin(const String& token, const String& authorizedChatId) {
    if (token.length() == 0 || authorizedChatId.length() == 0) {
        DBG_TELEGRAM("❌ Token o Chat ID no válidos");
        return false;
    }
    
    botToken = token;
    chatId = authorizedChatId;
    serviceEnabled = true;
    
    // Inicialización simple sin limpieza para evitar conflictos
    DBG_TELEGRAM("🔄 Inicializando servicio Telegram...");
    delay(2000); // Esperar 2 segundos antes de empezar
    
    // Verificar conectividad con Telegram
    String response = sendRequest("getMe", "");
    if (response.length() > 0) {
        DynamicJsonDocument doc(1024);
        if (deserializeJson(doc, response) == DeserializationError::Ok && doc["ok"]) {
            String botName = doc["result"]["first_name"];
            DBG_TELEGRAM_PRINTF("✅ Bot conectado: %s", botName.c_str());
            
            // Enviar mensaje de inicio específico del campanario
            String nombreUpper = Config::Telegram::CAMPANARIO_NOMBRE;
            nombreUpper.toUpperCase();
            String inicioMsg = "🔔 <b>MÓDULO " + nombreUpper + " INICIADO</b>\n\n";
            inicioMsg += "🏛️ <b>Campanario:</b> " + Config::Telegram::CAMPANARIO_NOMBRE + "\n";
            inicioMsg += "📍 <b>Ubicación:</b> " + Config::Telegram::CAMPANARIO_UBICACION + "\n";
            inicioMsg += "🆔 <b>ID:</b> <code>" + Config::Telegram::CAMPANARIO_ID + "</code>\n";
            inicioMsg += "🌐 <b>IP:</b> " + WiFi.localIP().toString() + "\n\n";
            inicioMsg += "✅ Sistema operativo y listo para recibir comandos\n";
            inicioMsg += "📱 <b>Modo bidireccional activo</b> - Puedes enviar comandos\n";
            inicioMsg += "💡 Usa <code>/help_" + Config::Telegram::CAMPANARIO_ID + "</code> para ver comandos disponibles";
            
            // Pequeña pausa antes de empezar a verificar mensajes
            delay(3000);
            sendMessage(inicioMsg, false);
            return true;
        }
    }
    
    DBG_TELEGRAM("❌ Error al conectar con Telegram");
    serviceEnabled = false;
    return false;
}

void TelegramServicio::checkMessages() {
    if (!serviceEnabled || WiFi.status() != WL_CONNECTED) return;
    
    // Si está en modo solo-envío, no hacer getUpdates
    if (TelegramConfig::SEND_ONLY_MODE) {
        DBG_TELEGRAM("📤 Modo solo-envío activo - Sin getUpdates");
        return;
    }
    
    if (millis() - lastCheckTime < TelegramConfig::CHECK_INTERVAL_MS) return;
    
    lastCheckTime = millis();
    
    String parameters = "offset=" + String(lastUpdateId + 1) + "&limit=3&timeout=1";  // Pocos mensajes, timeout corto
    String response = sendRequest("getUpdates", parameters);
    
    if (response.length() == 0) {
        DBG_TELEGRAM("⚠️ Sin respuesta de getUpdates");
        return;
    }
    
    DynamicJsonDocument doc(8192);
    DeserializationError error = deserializeJson(doc, response);
    if (error) {
        DBG_TELEGRAM_PRINTF("❌ Error parsing JSON: %s", error.c_str());
        return;
    }
    
    if (!doc["ok"].as<bool>()) {
        DBG_TELEGRAM_PRINTF("❌ API Error: %s", doc["description"].as<const char*>());
        return;
    }
    
    JsonArray updates = doc["result"];
    for (JsonObject update : updates) {
        if (processUpdate(doc)) {
            lastUpdateId = update["update_id"].as<unsigned long>();
        }
    }
}

bool TelegramServicio::processUpdate(DynamicJsonDocument& update) {
    if (!update["result"][0]["message"]) return false;
    
    JsonObject message = update["result"][0]["message"];
    String messageText = message["text"];
    String fromChatId = String(message["chat"]["id"].as<long long>());
    String userName = message["from"]["first_name"];
    
    // Verificar autorización
    if (fromChatId != chatId) {
        DBG_TELEGRAM_PRINTF("❌ Mensaje no autorizado de chat ID: %s", fromChatId.c_str());
        return true;
    }
    
    DBG_TELEGRAM_PRINTF("📨 Comando recibido: %s (de %s)", messageText.c_str(), userName.c_str());
    
    // Mostrar indicador de escritura
    sendTypingAction(fromChatId);
    
    // Procesar comando
    handleCommand(messageText, fromChatId);
    
    return true;
}

void TelegramServicio::handleCommand(const String& command, const String& chatId) {
    String cmd = command;
    cmd.toLowerCase();
    
    // Debug: mostrar comando recibido
    DBG_TELEGRAM_PRINTF("🔍 Procesando comando: '%s'", cmd.c_str());
    
    // Crear sufijo para este campanario
    String miSufijo = "_" + Config::Telegram::CAMPANARIO_ID;
    DBG_TELEGRAM_PRINTF("🔍 Mi sufijo: '%s'", miSufijo.c_str());
    
    // ========== COMANDOS ESPECÍFICOS DEL CAMPANARIO ==========
    if (cmd.endsWith(miSufijo)) {
        DBG_TELEGRAM("✅ Comando específico detectado");
        // Quitar el sufijo para procesar el comando base
        String comandoBase = cmd.substring(0, cmd.length() - miSufijo.length());
        DBG_TELEGRAM_PRINTF("🔍 Comando base: '%s'", comandoBase.c_str());
        
        if (comandoBase == "/estado" || comandoBase == "/status") {
            String status = "🏛️ <b>" + Config::Telegram::CAMPANARIO_NOMBRE + " (" + Config::Telegram::CAMPANARIO_UBICACION + ")</b>\n\n";
            status += formatSystemStatus();
            sendMessage(status);
        }
        else if (comandoBase == "/reiniciar") {
            sendMessage("🏛️ <b>" + Config::Telegram::CAMPANARIO_NOMBRE + ":</b> 🔄 Reiniciando sistema en 5 segundos...");
            delay(1000);
            ESP.restart();
        }
        else if (comandoBase == "/memoria") {
            String msg = "🏛️ <b>" + Config::Telegram::CAMPANARIO_NOMBRE + "</b> - Estado de Memoria\n";
            msg += "🔹 Heap libre: " + String(ESP.getFreeHeap() / 1024.0, 1) + " KB\n";
            msg += "🔹 Heap total: " + String(ESP.getHeapSize() / 1024.0, 1) + " KB\n";
            msg += "🔹 PSRAM libre: " + String(ESP.getFreePsram() / 1024.0, 1) + " KB";
            sendMessage(msg);
        }
        else if (comandoBase == "/help" || comandoBase == "/start") {
            sendHelpMenu();
        }
        else {
            sendMessage("🏛️ <b>" + Config::Telegram::CAMPANARIO_NOMBRE + ":</b> ❓ Comando no reconocido: <code>" + comandoBase + "</code>");
        }
    }
    // Si el comando no es para este campanario, ignorar completamente
    // (no enviar mensajes de error para comandos de otros campanarios)
}

String TelegramServicio::sendRequest(const String& method, const String& parameters) {
    if (!serviceEnabled || WiFi.status() != WL_CONNECTED) {
        DBG_TELEGRAM("❌ Servicio deshabilitado o WiFi desconectado");
        return "";
    }
    
    HTTPClient http;
    String url = TelegramConfig::API_BASE_URL + botToken + "/" + method;
    
    http.begin(url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.setTimeout(15000);  // Timeout de 15 segundos
    http.setConnectTimeout(10000);  // Timeout de conexión de 10 segundos
    
    DBG_TELEGRAM_PRINTF("🔄 Enviando: %s", method.c_str());
    
    int httpCode = http.POST(parameters);
    String response = "";
    
    if (httpCode == 200) {
        response = http.getString();
        DBG_TELEGRAM("✅ Respuesta recibida correctamente");
    } else if (httpCode > 0) {
        String errorResponse = http.getString();
        if (httpCode == 409) {
            DBG_TELEGRAM("⚠️ Error 409: Conflicto de instancias. Pausando...");
            // Desactivar temporalmente el servicio para evitar spam
            if (errorResponse.indexOf("terminated by other getUpdates") >= 0) {
                DBG_TELEGRAM("🔄 Esperando 60 segundos antes de reintentar...");
                lastCheckTime = millis() + 60000; // Esperar 60 segundos extra para dar tiempo a otros bots
            }
        } else {
            DBG_TELEGRAM_PRINTF("❌ HTTP Error: %d", httpCode);
        }
        if (errorResponse.length() > 0) {
            DBG_TELEGRAM_PRINTF("Error details: %s", errorResponse.c_str());
        }
    } else {
        DBG_TELEGRAM_PRINTF("❌ Conexión Error: %d (Timeout o sin red)", httpCode);
    }
    
    http.end();
    return response;
}

bool TelegramServicio::sendMessage(const String& message, bool silent) {
    if (!serviceEnabled) return false;
    
    // Verificar longitud del mensaje
    if (message.length() > TelegramConfig::MAX_MESSAGE_LENGTH) {
        DBG_TELEGRAM("❌ Mensaje demasiado largo");
        return false;
    }
    
    String params = "chat_id=" + chatId;
    params += "&text=" + urlEncode(message);  // URL encode del mensaje
    params += "&parse_mode=HTML";
    if (silent) params += "&disable_notification=true";
    
    // Intentar envío con reintento
    for (int intento = 0; intento < 2; intento++) {
        String response = sendRequest("sendMessage", params);
        if (response.length() > 0) {
            DBG_TELEGRAM("✅ Mensaje enviado correctamente");
            return true;
        }
        
        if (intento == 0) {
            DBG_TELEGRAM("🔄 Reintentando envío...");
            delay(1000);  // Esperar 1 segundo antes del reintento
        }
    }
    
    DBG_TELEGRAM("❌ Falló el envío después de reintentos");
    return false;
}

void TelegramServicio::sendStatusNotification(const String& component, const String& status, const String& details) {
    if (!serviceEnabled) return;
    if (millis() - notificationCooldown < TelegramConfig::NOTIFICATION_COOLDOWN_MS) return;
    
    String message = "📊 <b>" + component + "</b>\n";
    message += "🔹 Estado: " + status + "\n";
    if (details.length() > 0) {
        message += "🔹 Detalles: " + details + "\n";
    }
    message += "🕐 " + RTC::getTimeStr();
    
    if (sendMessage(message, true)) {
        notificationCooldown = millis();
    }
}

void TelegramServicio::sendAlert(const String& alertType, const String& description) {
    if (!serviceEnabled) return;
    
    String message = "🚨 <b>ALERTA: " + alertType + "</b>\n\n";
    message += description + "\n\n";
    message += "🕐 " + RTC::getTimeStr();
    
    sendMessage(message);
}

void TelegramServicio::sendTypingAction(const String& chatId) {
    String params = "chat_id=" + chatId + "&action=typing";
    sendRequest("sendChatAction", params);
}

String TelegramServicio::formatSystemStatus() {
    String status = "📊 <b>ESTADO DEL SISTEMA</b>\n\n";
    
    // Conexión
    status += "🌐 <b>Conectividad</b>\n";
    status += "• WiFi: ";
    status += (WiFi.status() == WL_CONNECTED ? "✅ Conectado" : "❌ Desconectado");
    status += "\n";
    status += "• Internet: ";
    status += (lConexionInternet ? "✅ Disponible" : "❌ No disponible");
    status += "\n";
    status += "• IP: " + WiFi.localIP().toString() + "\n\n";
    
    // Hora
    status += "🕐 <b>Tiempo</b>\n";
    status += "• Hora actual: " + RTC::getTimeStr() + "\n";
    status += "• Fecha: " + RTC::getTimeStr() + "\n";
    status += "• NTP Sync: ";
    status += (RTC::isNtpSync() ? "✅" : "❌");
    status += "\n\n";
    
    // Calefacción
    status += "🔥 <b>Calefacción</b>\n";
    if (Campanario.GetEstadoCalefaccion()) {
        float remaining = Campanario.TestTemporizacionCalefaccion();
        status += "• Estado: 🔥 ACTIVA\n";
        if (remaining > 0) {
            status += "• Tiempo restante: " + String(remaining/60, 1) + " min\n";
        }
    } else {
        status += "• Estado: ❄️ DESACTIVADA\n";
    }
    status += "\n";
    
    // Campanas
    status += "🔔 <b>Campanas</b>\n";
    status += "• Secuencia activa: ";
    status += (Campanario.GetEstadoSecuencia() ? "✅ Sí" : "❌ No");
    status += "\n";
    
    // Memoria
    status += "\n💾 <b>Memoria</b>\n";
    status += "• Heap libre: " + String(ESP.getFreeHeap() / 1024.0, 1) + " KB\n";
    status += "• Uptime: " + String(millis() / 60000) + " min";
    
    return status;
}

String TelegramServicio::formatAlarmStatus() {
    String status = "⏰ <b>ESTADO DE ALARMAS</b>\n\n";
    status += "🔹 Sistema: ";
    status += (RTC::isNtpSync() ? "✅ Activo" : "❌ Inactivo (sin NTP)");
    status += "\n";
    status += "🔹 Próxima verificación en: " + String((TelegramConfig::CHECK_INTERVAL_MS - (millis() - lastCheckTime))/1000) + "s\n\n";
    status += "📋 <b>Alarmas configuradas:</b>\n";
    status += "• Tocar horas: Cada hora en punto\n";
    status += "• Tocar medias: Cada 30 minutos\n";
    status += "• Sincronización NTP: 12:02 diario\n";
    status += "• Actualización DNS: Cada hora (min 10)";
    
    return status;
}

String TelegramServicio::escapeMarkdown(const String& text) {
    String escaped = text;
    escaped.replace("_", "\\_");
    escaped.replace("*", "\\*");
    escaped.replace("[", "\\[");
    escaped.replace("]", "\\]");
    escaped.replace("(", "\\(");
    escaped.replace(")", "\\)");
    escaped.replace("~", "\\~");
    escaped.replace("`", "\\`");
    escaped.replace(">", "\\>");
    escaped.replace("#", "\\#");
    escaped.replace("+", "\\+");
    escaped.replace("-", "\\-");
    escaped.replace("=", "\\=");
    escaped.replace("|", "\\|");
    escaped.replace("{", "\\{");
    escaped.replace("}", "\\}");
    escaped.replace(".", "\\.");
    escaped.replace("!", "\\!");
    return escaped;
}

void TelegramServicio::sendHelpMenu() {
    String nombreUpper = Config::Telegram::CAMPANARIO_NOMBRE;
    nombreUpper.toUpperCase();
    String help = "🔔 <b>CAMPANARIO " + nombreUpper + "</b>\n\n";
    help += "🏛️ <b>Comandos disponibles:</b>\n\n";
    help += "• <code>/help_" + Config::Telegram::CAMPANARIO_ID + "</code> - Mostrar esta ayuda\n";
    help += "• <code>/estado_" + Config::Telegram::CAMPANARIO_ID + "</code> - Estado del sistema\n";
    help += "• <code>/memoria_" + Config::Telegram::CAMPANARIO_ID + "</code> - Estado de memoria\n";
    help += "• <code>/reiniciar_" + Config::Telegram::CAMPANARIO_ID + "</code> - Reiniciar sistema\n\n";
    
    help += "💡 <b>Ejemplos de uso:</b>\n";
    help += "• <code>/estado_" + Config::Telegram::CAMPANARIO_ID + "</code>\n";
    help += "• <code>/memoria_" + Config::Telegram::CAMPANARIO_ID + "</code>\n\n";
    
    help += "📍 <b>Ubicación:</b> " + Config::Telegram::CAMPANARIO_UBICACION + "\n";
    help += "🆔 <b>ID del campanario:</b> " + Config::Telegram::CAMPANARIO_ID;
    
    sendMessage(help);
}