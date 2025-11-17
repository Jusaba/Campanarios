/**
 * @file TelegramServicio.h
 * @brief Servicio de Telegram simple para campanarios
 * @author Julian Salas Bartolomé
 * @version 2.0
 */

#ifndef TELEGRAM_SERVICIO_H
#define TELEGRAM_SERVICIO_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include "Configuracion.h"
#include "Debug.h"

class TelegramServicio {
private:
    String botToken;
    String chatId;
    String campanarioId;
    bool serviceEnabled;
    
    // Funciones privadas
    String sendRequest(const String& method, const String& parameters);
    String urlEncode(const String& str);
    
public:
    TelegramServicio();
    
    // Funciones públicas principales
    bool begin(const String& token, const String& chatId, const String& campanarioId);
    void sendNotification(const String& message);  // Habilita, envía y deshabilita automáticamente
    void sendStartupNotification();
    void SendReconexionNotification();
    void sendStopNotification(int nMetodo);
    void sendCalefaccionOnNotification(int nMetodo);
    void sendCalefaccionOffNotification(int nMetodo);
    void sendHoraNotification(const String& horaStr);
    void sendMediaHoraNotification(const String& horaStr);
    void sendSequenceNotification(const String& sequenceName, int nMetodo);
    void sendMsgNotification(const String& message);  // Habilita, envía y deshabilita automáticamente
    bool isEnabled() const { return serviceEnabled; }
};

// Instancia global
extern TelegramServicio telegramBot;

#endif // TELEGRAM_SERVICIO_H