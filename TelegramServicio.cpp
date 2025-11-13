/**
 * @file TelegramServicio.cpp
 * @brief Implementación del servicio de comunicación con Telegram
 */

#include "TelegramServicio.h"
#include "Campanario.h"
#include "RTC.h"
#include "Acciones.h"
#include "Auxiliar.h"

// Variables externas
extern bool lConexionInternet;

// Instancia global
TelegramServicio telegramBot;

// Constantes de configuración
namespace TelegramConfig {
    const unsigned long CHECK_INTERVAL_MS = 2000;      // Verificar mensajes cada 2 segundos
    const unsigned long NOTIFICATION_COOLDOWN_MS = 30000; // Cooldown de 30 segundos entre notificaciones del mismo tipo
    const int MAX_MESSAGE_LENGTH = 4096;               // Longitud máxima de mensaje de Telegram
    const String API_BASE_URL = "https://api.telegram.org/bot";
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
    
    // Verificar conectividad con Telegram
    String response = sendRequest("getMe", "");
    if (response.length() > 0) {
        DynamicJsonDocument doc(1024);
        if (deserializeJson(doc, response) == DeserializationError::Ok && doc["ok"]) {
            String botName = doc["result"]["first_name"];
            DBG_TELEGRAM_PRINTF("✅ Bot conectado: %s", botName.c_str());
            
            // Enviar mensaje de inicio
            sendMessage("🔔 *Sistema Campanario Conectado*\n\n"
                       "Bot iniciado correctamente\\.\n"
                       "Usa /help para ver comandos disponibles\\.", false);
            return true;
        }
    }
    
    DBG_TELEGRAM("❌ Error al conectar con Telegram");
    serviceEnabled = false;
    return false;
}

void TelegramServicio::checkMessages() {
    if (!serviceEnabled || WiFi.status() != WL_CONNECTED) return;
    if (millis() - lastCheckTime < TelegramConfig::CHECK_INTERVAL_MS) return;
    
    lastCheckTime = millis();
    
    String parameters = "offset=" + String(lastUpdateId + 1) + "&limit=10&timeout=1";
    String response = sendRequest("getUpdates", parameters);
    
    if (response.length() == 0) return;
    
    DynamicJsonDocument doc(8192);
    if (deserializeJson(doc, response) != DeserializationError::Ok) {
        DBG_TELEGRAM("❌ Error parsing updates");
        return;
    }
    
    if (!doc["ok"]) return;
    
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
    
    if (cmd == "/start" || cmd == "/help") {
        sendHelpMenu();
    }
    else if (cmd == "/estado" || cmd == "/status") {
        String status = formatSystemStatus();
        sendMessage(status);
    }
    else if (cmd == "/alarmas") {
        String alarms = formatAlarmStatus();
        sendMessage(alarms);
    }
    else if (cmd == "/hora") {
        if (RTC::isNtpSync()) {
            String timeStr = "🕐 *Hora actual*: " + RTC::getTimeStr();
            timeStr += "\n📅 *Fecha*: " + RTC::getTimeStr();
            if (lConexionInternet) {
                timeStr += "\n🌐 *Sincronizado con NTP*";
            }
            sendMessage(timeStr);
        } else {
            sendMessage("❌ RTC no sincronizado");
        }
    }
    else if (cmd == "/calefaccion") {
        if (Campanario.GetEstadoCalefaccion()) {
            float remaining = Campanario.TestTemporizacionCalefaccion();
            String msg = "🔥 *Calefacción ACTIVA*\n";
            if (remaining > 0) {
                msg += "⏱️ Tiempo restante: " + String(remaining/60, 1) + " minutos";
            }
            sendMessage(msg);
        } else {
            sendMessage("❄️ Calefacción *DESACTIVADA*");
        }
    }
    else if (cmd.startsWith("/calefaccion_on")) {
        // Extraer tiempo si se especifica: /calefaccion_on 30
        int tiempo = 15; // Por defecto 15 minutos
        int spaceIndex = cmd.indexOf(' ');
        if (spaceIndex > 0) {
            tiempo = cmd.substring(spaceIndex + 1).toInt();
            if (tiempo <= 0 || tiempo > 120) tiempo = 15;
        }
        
        EjecutaSecuencia(Config::States::SET_TEMPORIZADOR, tiempo);
        sendMessage("🔥 Calefacción activada por " + String(tiempo) + " minutos");
    }
    else if (cmd == "/calefaccion_off") {
        EjecutaSecuencia(Config::States::CALEFACCION_OFF);
        sendMessage("❄️ Calefacción desactivada");
    }
    else if (cmd == "/angelus") {
        EjecutaSecuencia(Config::States::DIFUNTOS);  // Usando Difuntos como Angelus
        sendMessage("🔔 Tocando Angelus");
    }
    else if (cmd == "/misa") {
        EjecutaSecuencia(Config::States::I2CState::MISA);
        sendMessage("⛪ Tocando Misa");
    }
    else if (cmd == "/fiesta") {
        EjecutaSecuencia(Config::States::I2CState::FIESTA);
        sendMessage("🎉 Tocando Fiesta");
    }
    else if (cmd == "/difuntos") {
        EjecutaSecuencia(Config::States::I2CState::DIFUNTOS);
        sendMessage("🕊️ Tocando Difuntos");
    }
    else if (cmd == "/horas") {
        accionTocaHora();
        sendMessage("🕐 Tocando horas");
    }
    else if (cmd == "/test_campana1") {
        EjecutaSecuencia(Config::States::DIFUNTOS);  // Test campana 1
        sendMessage("🔔 Test Campana 1");
    }
    else if (cmd == "/test_campana2") {
        EjecutaSecuencia(Config::States::MISA);      // Test campana 2
        sendMessage("🔔 Test Campana 2");
    }
    else if (cmd == "/stop" || cmd == "/parar") {
        EjecutaSecuencia(Config::States::STOP);
        sendMessage("⏹️ Deteniendo secuencias");
    }
    else if (cmd == "/reiniciar") {
        sendMessage("🔄 Reiniciando sistema en 5 segundos...");
        delay(1000);
        ESP.restart();
    }
    else if (cmd == "/memoria") {
        String msg = "💾 *Estado de Memoria*\n";
        msg += "🔹 Heap libre: " + String(ESP.getFreeHeap() / 1024.0, 1) + " KB\n";
        msg += "🔹 Heap total: " + String(ESP.getHeapSize() / 1024.0, 1) + " KB\n";
        msg += "🔹 PSRAM libre: " + String(ESP.getFreePsram() / 1024.0, 1) + " KB";
        sendMessage(msg);
    }
    else {
        String msg = "❓ Comando no reconocido: `" + command + "`\n";
        msg += "Usa /help para ver comandos disponibles\\.";
        sendMessage(msg);
    }
}

String TelegramServicio::sendRequest(const String& method, const String& parameters) {
    if (!serviceEnabled || WiFi.status() != WL_CONNECTED) return "";
    
    HTTPClient http;
    String url = TelegramConfig::API_BASE_URL + botToken + "/" + method;
    
    http.begin(url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
    int httpCode = http.POST(parameters);
    String response = "";
    
    if (httpCode == 200) {
        response = http.getString();
    } else {
        DBG_TELEGRAM_PRINTF("❌ HTTP Error: %d", httpCode);
    }
    
    http.end();
    return response;
}

bool TelegramServicio::sendMessage(const String& message, bool silent) {
    if (!serviceEnabled) return false;
    
    String params = "chat_id=" + chatId;
    params += "&text=" + message;
    params += "&parse_mode=MarkdownV2";
    if (silent) params += "&disable_notification=true";
    
    String response = sendRequest("sendMessage", params);
    return response.length() > 0;
}

void TelegramServicio::sendStatusNotification(const String& component, const String& status, const String& details) {
    if (!serviceEnabled) return;
    if (millis() - notificationCooldown < TelegramConfig::NOTIFICATION_COOLDOWN_MS) return;
    
    String message = "📊 *" + escapeMarkdown(component) + "*\n";
    message += "🔹 Estado: " + escapeMarkdown(status) + "\n";
    if (details.length() > 0) {
        message += "🔹 Detalles: " + escapeMarkdown(details) + "\n";
    }
    message += "🕐 " + RTC::getTimeStr();
    
    if (sendMessage(message, true)) {
        notificationCooldown = millis();
    }
}

void TelegramServicio::sendAlert(const String& alertType, const String& description) {
    if (!serviceEnabled) return;
    
    String message = "🚨 *ALERTA: " + escapeMarkdown(alertType) + "*\n\n";
    message += escapeMarkdown(description) + "\n\n";
    message += "🕐 " + RTC::getTimeStr();
    
    sendMessage(message);
}

void TelegramServicio::sendTypingAction(const String& chatId) {
    String params = "chat_id=" + chatId + "&action=typing";
    sendRequest("sendChatAction", params);
}

String TelegramServicio::formatSystemStatus() {
    String status = "📊 **ESTADO DEL SISTEMA**\n\n";
    
    // Conexión
    status += "🌐 *Conectividad*\n";
    status += "• WiFi: ";
    status += (WiFi.status() == WL_CONNECTED ? "✅ Conectado" : "❌ Desconectado");
    status += "\n";
    status += "• Internet: ";
    status += (lConexionInternet ? "✅ Disponible" : "❌ No disponible");
    status += "\n";
    status += "• IP: " + WiFi.localIP().toString() + "\n\n";
    
    // Hora
    status += "🕐 *Tiempo*\n";
    status += "• Hora actual: " + RTC::getTimeStr() + "\n";
    status += "• Fecha: " + RTC::getTimeStr() + "\n";
    status += "• NTP Sync: ";
    status += (RTC::isNtpSync() ? "✅" : "❌");
    status += "\n\n";
    
    // Calefacción
    status += "🔥 *Calefacción*\n";
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
    status += "🔔 *Campanas*\n";
    status += "• Secuencia activa: ";
    status += (Campanario.GetEstadoSecuencia() ? "✅ Sí" : "❌ No");
    status += "\n";
    
    // Memoria
    status += "\n💾 *Memoria*\n";
    status += "• Heap libre: " + String(ESP.getFreeHeap() / 1024.0, 1) + " KB\n";
    status += "• Uptime: " + String(millis() / 60000) + " min";
    
    return escapeMarkdown(status);
}

String TelegramServicio::formatAlarmStatus() {
    String status = "⏰ **ESTADO DE ALARMAS**\n\n";
    status += "🔹 Sistema: ";
    status += (RTC::isNtpSync() ? "✅ Activo" : "❌ Inactivo (sin NTP)");
    status += "\n";
    status += "🔹 Próxima verificación en: " + String((TelegramConfig::CHECK_INTERVAL_MS - (millis() - lastCheckTime))/1000) + "s\n\n";
    status += "📋 *Alarmas configuradas:*\n";
    status += "• Tocar horas: Cada hora en punto\n";
    status += "• Tocar medias: Cada 30 minutos\n";
    status += "• Sincronización NTP: 12:02 diario\n";
    status += "• Actualización DNS: Cada hora (min 10)";
    
    return escapeMarkdown(status);
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
    String help = "🔔 **COMANDOS DISPONIBLES**\n\n";
    
    help += "📊 *Información del Sistema:*\n";
    help += "• `/estado` \\- Estado completo del sistema\n";
    help += "• `/hora` \\- Hora y fecha actual\n";
    help += "• `/alarmas` \\- Estado de alarmas programadas\n";
    help += "• `/memoria` \\- Estado de memoria del sistema\n\n";
    
    help += "🔥 *Control de Calefacción:*\n";
    help += "• `/calefaccion` \\- Estado actual\n";
    help += "• `/calefaccion_on` \\- Activar \\(15 min\\)\n";
    help += "• `/calefaccion_on 30` \\- Activar por X minutos\n";
    help += "• `/calefaccion_off` \\- Desactivar\n\n";
    
    help += "🔔 *Control de Campanas:*\n";
    help += "• `/angelus` \\- Tocar Angelus\n";
    help += "• `/misa` \\- Tocar Misa\n";
    help += "• `/fiesta` \\- Tocar Fiesta\n";
    help += "• `/difuntos` \\- Tocar Difuntos\n";
    help += "• `/horas` \\- Tocar horas\n";
    help += "• `/test_campana1` \\- Test campana 1\n";
    help += "• `/test_campana2` \\- Test campana 2\n";
    help += "• `/stop` \\- Parar todas las secuencias\n\n";
    
    help += "⚙️ *Sistema:*\n";
    help += "• `/reiniciar` \\- Reiniciar ESP32\n";
    help += "• `/help` \\- Mostrar esta ayuda";
    
    sendMessage(help);
}