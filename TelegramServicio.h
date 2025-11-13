/**
 * @file TelegramServicio.h
 * @brief Servicio de comunicación bidireccional con Telegram para el sistema de campanario
 * 
 * Este servicio permite:
 * - Recibir comandos desde Telegram para controlar el campanario
 * - Enviar notificaciones y estados del sistema
 * - Consultar información en tiempo real
 * - Recibir alertas de eventos importantes
 * 
 * @author Julian Salas Bartolome
 * @date 12/11/2025
 * @version 1.0
 */

#ifndef TELEGRAM_SERVICIO_H
#define TELEGRAM_SERVICIO_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "Debug.h"
#include "Configuracion.h"

/**
 * @class TelegramServicio
 * @brief Maneja la comunicación bidireccional con Telegram Bot API
 */
class TelegramServicio {
private:
    String botToken;                    // Token del bot de Telegram
    String chatId;                      // ID del chat autorizado
    unsigned long lastUpdateId;        // ID del último mensaje procesado
    unsigned long lastCheckTime;       // Última vez que se verificaron mensajes
    unsigned long notificationCooldown; // Cooldown para evitar spam de notificaciones
    bool serviceEnabled;               // Estado del servicio
    
    // Métodos privados
    String sendRequest(const String& method, const String& parameters);
    bool processUpdate(DynamicJsonDocument& update);
    void handleCommand(const String& command, const String& chatId);
    void sendTypingAction(const String& chatId);
    String formatSystemStatus();
    String formatAlarmStatus();
    String escapeMarkdown(const String& text);

public:
    /**
     * @brief Constructor de la clase TelegramServicio
     */
    TelegramServicio();
    
    /**
     * @brief Inicializa el servicio de Telegram
     * @param token Token del bot de Telegram
     * @param authorizedChatId ID del chat autorizado
     * @return true si la inicialización es exitosa
     */
    bool begin(const String& token, const String& authorizedChatId);
    
    /**
     * @brief Verifica mensajes nuevos y los procesa
     */
    void checkMessages();
    
    /**
     * @brief Envía un mensaje de texto
     * @param message Mensaje a enviar
     * @param silent Envío silencioso (sin notificación)
     * @return true si el envío es exitoso
     */
    bool sendMessage(const String& message, bool silent = false);
    
    /**
     * @brief Envía una notificación de estado del sistema
     * @param component Componente que reporta (ej: "Campana", "Calefacción")
     * @param status Estado del componente
     * @param details Detalles adicionales
     */
    void sendStatusNotification(const String& component, const String& status, const String& details = "");
    
    /**
     * @brief Envía una alerta de emergencia
     * @param alertType Tipo de alerta
     * @param description Descripción de la alerta
     */
    void sendAlert(const String& alertType, const String& description);
    
    /**
     * @brief Verifica si el servicio está habilitado
     * @return true si está habilitado
     */
    bool isEnabled() const { return serviceEnabled; }
    
    /**
     * @brief Habilita o deshabilita el servicio
     * @param enabled Estado deseado
     */
    void setEnabled(bool enabled) { serviceEnabled = enabled; }
    
    /**
     * @brief Envía el menú de comandos disponibles
     */
    void sendHelpMenu();
};

// Instancia global del servicio
extern TelegramServicio telegramBot;

#endif // TELEGRAM_SERVICIO_H