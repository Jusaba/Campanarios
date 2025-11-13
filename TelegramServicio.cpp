/**
 * @file TelegramServicio.cpp
 * @brief Servicio de Telegram simple para campanarios
 * @author Julian Salas Bartolomé
 * @version 2.0
 */

#include "TelegramServicio.h"
#include "RTC.h"
#include "Auxiliar.h"

// Variables externas
extern bool lConexionInternet;

// Instancia global
TelegramServicio telegramBot;

// Configuración
const String API_BASE_URL = "https://api.telegram.org/bot";
const unsigned long CHECK_INTERVAL_MS = 3000;  // Verificar cada 3 segundos

TelegramServicio::TelegramServicio() {
    serviceEnabled = false;
}

bool TelegramServicio::begin(const String& token, const String& chatId, const String& campanarioId) {
    if (token.length() == 0 || chatId.length() == 0) {
        DBG_TELEGRAM("❌ Token o Chat ID no válidos");
        return false;
    }
    
    this->botToken = token;
    this->chatId = chatId;
    this->campanarioId = campanarioId;
    this->serviceEnabled = true;
    
    DBG_TELEGRAM("✅ Telegram inicializado (solo notificaciones)");
    return true;
}

// checkMessages eliminado - solo notificaciones de salida

// processCommand eliminado - solo notificaciones de salida

String TelegramServicio::sendRequest(const String& method, const String& parameters) {
    if (!serviceEnabled || WiFi.status() != WL_CONNECTED) return "";
    
    HTTPClient http;
    String url = API_BASE_URL + botToken + "/" + method;
    
    http.begin(url);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    http.setTimeout(10000);
    
    int httpCode = http.POST(parameters);
    String response = "";
    
    if (httpCode == 200) {
        response = http.getString();
    }
    
    http.end();
    return response;
}

void TelegramServicio::sendNotification(const String& message) {
    if (!serviceEnabled || WiFi.status() != WL_CONNECTED) {
        DBG_TELEGRAM("❌ Telegram no disponible");
        return;
    }
    
    DBG_TELEGRAM("⚡ Activando Telegram para notificación...");
    
    String params = "chat_id=" + chatId + "&text=" + urlEncode(message) + "&parse_mode=Markdown";
    String response = sendRequest("sendMessage", params);
    
    if (response.length() > 0) {
        DBG_TELEGRAM("✅ Notificación enviada - Telegram desactivado");
    } else {
        DBG_TELEGRAM("❌ Error enviando notificación");
    }
}

String TelegramServicio::urlEncode(const String& str) {
    String encoded = "";
    for (int i = 0; i < str.length(); i++) {
        char c = str.charAt(i);
        if (c == ' ') {
            encoded += '+';
        } else if (isalnum(c)) {
            encoded += c;
        } else {
            encoded += '%';
            encoded += String(c >> 4, HEX);
            encoded += String(c & 0xF, HEX);
        }
    }
    return encoded;
}

void TelegramServicio::sendStartupNotification() {
    if (!serviceEnabled) return;
    String campanarioUpper = campanarioId;
    campanarioUpper.toUpperCase();
    String msg = "🔔 *" + campanarioUpper + " INICIADO*\n\n";
    msg += "✅ Sistema operativo\n";
    msg += "📡 IP: " + WiFi.localIP().toString();
    sendNotification(msg);
}

void TelegramServicio::sendSequenceNotification(const String& sequenceName) {
    if (!serviceEnabled) return;
    String campanarioUpper = campanarioId;
    campanarioUpper.toUpperCase();
    String msg = "🔔 *" + campanarioUpper + "*\n";
    msg += "🎵 Tocando: " + sequenceName;
    sendNotification(msg);
}

void TelegramServicio::sendStopNotification() {
    if (!serviceEnabled) return;
    String campanarioUpper = campanarioId;
    campanarioUpper.toUpperCase();
    String msg = "🛑 *" + campanarioUpper + "*\n";
    msg += "⏹️ Secuencia detenida";
    sendNotification(msg);
}
void TelegramServicio::sendMsgNotification(const String& message) {
    if (!serviceEnabled) return;
    String campanarioUpper = campanarioId;
    campanarioUpper.toUpperCase();
    String msg = "📢 *" + campanarioUpper + "*\n";
    msg += message;
    sendNotification(msg);
}
// Funciones de control temporal eliminadas - solo notificaciones automáticas