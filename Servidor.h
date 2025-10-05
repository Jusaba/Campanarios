/**
 * @file Servidor.h
 * @brief Sistema de servidor web HTTP y WebSocket para control del proyecto Campanarios
 * 
 * @details Este header define el sistema completo de servidor web que permite:
 *          
 *          **FUNCIONALIDADES PRINCIPALES:**
 *          - Servidor HTTP asíncrono para control remoto del campanario
 *          - WebSocket para comunicación bidireccional en tiempo real
 *          - Sistema de archivos SPIFFS para contenido web estático
 *          - Verificación de conectividad a Internet
 *          - Control del campanario a través de la clase CAMPANARIO
 *          - Debug condicional del servidor con DEBUGSERVIDOR en Debug.h
 *          - Gestión de eventos de WebSocket (conexión, desconexión, mensajes)
 *          
 *          **ARQUITECTURA DEL SERVIDOR:**
 *          - AsyncWebServer: Servidor HTTP no bloqueante de alto rendimiento
 *          - AsyncWebSocket: Comunicación bidireccional en tiempo real
 *          - SPIFFS: Sistema de archivos para archivos HTML, CSS, JS
 *          - HTTPClient: Cliente para verificación de conectividad externa
 *          - Integración con clase CAMPANARIO: Control especializado del sistema
 *          - Sistema de debug condicional con macro DEBUGSERVIDOR en Debug.h
 * 
 * @note **INTEGRACIÓN CON SISTEMA:**
 *       - El servidor se integra con la clase CAMPANARIO para control completo
 *       - Controla el campanario a través de métodos de la clase especializada
 *       - Gestiona estados y secuencias mediante el objeto Campanario
 *       - Monitoreo de conectividad a Internet con hayInternet()
 *       - Sistema de archivos SPIFFS para interfaz web
 * 
 * 
 * 
 * @warning **DEPENDENCIAS CRÍTICAS:**
 *          - ESPAsyncWebServer.h: Biblioteca del servidor HTTP asíncrono
 *          - AsyncTCP.h: Comunicación TCP asíncrona requerida
 *          - WiFi.h: Conectividad de red fundamental
 *          - SPIFFS.h: Sistema de archivos para contenido web
 *          - HTTPClient.h: Cliente HTTP para verificación de Internet
 *          - Campanario.h: Clase principal del sistema de campanario
 *          - Configuracion.h: Configuración del sistema
 *          - Debug.h: Sistema de logging del proyecto
 * 
 * @warning **CONFIGURACIÓN:**
 *          - SPIFFS debe estar inicializado antes de usar el servidor
 *          - WiFi debe estar conectado antes de iniciar servidor
 *          - La clase CAMPANARIO debe estar inicializada
 *          - Definir DEBUGSERVIDOR en debug.h para habilitar logging
 * 
 * @author Julian Salas Bartolomé
 * @date 2025-09-14
 * @version 3.0
 * 
 * @since v1.0 - Servidor básico HTTP
 * @since v2.0 - WebSocket y integración con CAMPANARIO
 * @since v3.0 - SPIFFS, verificación Internet y debug avanzado
 * 
 * @see Servidor.cpp - Implementación de todas las funcionalidades
 * @see Campanario.h - Clase principal integrada
 * @see Configuracion.h - Configuración del sistema
 * @see Debug.h - Sistema de logging
 * 
 */

#ifndef SERVIDOR_H
	#define SERVIDOR_H
    #include <stdint.h>
    #include <Arduino.h>
    #include <WiFi.h>
    #include <AsyncTCP.h>
    #include <ESPAsyncWebServer.h>
    #include <SPIFFS.h>
    #include <HTTPClient.h>
    #include "Configuracion.h"
    #include "Debug.h"
    
    #include "Campanario.h"
    #include "AlarmasPersonalizadas.h"
    #include "Alarmas.h" 
    #include <ArduinoJson.h>



    extern bool servidorIniciado;
    extern int  nToque;
    extern AsyncWebSocket ws;
    extern AsyncWebServer server;
    extern WiFiClient client;

    extern AlarmScheduler Alarmas;

    extern CAMPANARIO Campanario;

    void ServidorOn(const char* usuario, const char* clave);                                                                        // Función para iniciar el servidor HTTP y WebSocket
    void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);    // Callback para manejar eventos del WebSocket
    void procesaMensajeWebSocket(void *arg, uint8_t *data, size_t len);                                                             // Procesa los mensajes recibidos por WebSocket
    void procesarComandoAlarma(AsyncWebSocketClient *client, const String& comando, const String& datos);                                                              // Procesa comandos relacionados con las alarmas personalizables
    uint8_t convertirDiaAMascara(int dia);
    String cargarIdiomaDesdeConfig(void);
    String obtenerConfiguracionJSON(void);
    
    bool guardarIdiomaEnConfig(const String& idioma);

    bool hayInternet(void);                                                                                                         // Comprueba si hay conexión a Internet

    // Funciones auxiliares
    // Estas funciones se utilizan para listar los archivos en SPIFFS y para imprimir el contenido de un archivo específico.
    // Se pueden utilizar para depurar o verificar el contenido de los archivos almacenados en el sistema de archivos SPIFFS.
    void listSPIFFS(void);
    void printFileContent(const char* filename);


  

#endif