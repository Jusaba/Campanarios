/**
 * @file Servidor.h
 * @brief Definiciones y funciones para el servidor web y WebSocket del sistema de campanario.
 *
 * Este archivo contiene las definiciones, variables globales y funciones necesarias
 * para la gestión del servidor HTTP y WebSocket en el sistema de domótica del campanario.
 * Permite la interacción remota con el sistema mediante una interfaz web, el control
 * de las campanas y la calefacción, así como la gestión de archivos en SPIFFS.
 *
 * Funcionalidades principales:
 * - Inicialización y configuración del servidor HTTP y WebSocket.
 * - Autenticación básica de usuarios para el acceso web.
 * - Procesamiento de mensajes recibidos por WebSocket para controlar el campanario y la calefacción.
 * - Notificación en tiempo real a los clientes web mediante WebSocket.
 * Como funciones auxiliares:
 * - Listado e impresión de archivos almacenados en SPIFFS.
 *
 * @author [Tu Nombre]
 * @date [Fecha de creación o última modificación]
 * @version 1.0
 */
#ifndef SERVIDOR_H
	#define SERVIDOR_H
    #include <WiFi.h>
    #include <AsyncTCP.h>
    #include <ESPAsyncWebServer.h>
    #include <SPIFFS.h>
    #include <HTTPClient.h>
    #include "Configuracion.h"
    #include "Debug.h"
    
    #include "Campanario.h"
    
    // Definiciones de estados del sistema
    // Estos estados se utilizan para controlar la secuencia de toques de las campanas
    // y el estado de la calefacción.
    /*
    #define EstadoInicio            0
    #define EstadoDifuntos          1
    #define EstadoMisa              2
    #define EstadoStop              3
    #define EstadoCalefaccionOn     4
    #define EstadoCalefaccionOff    5
    */


    #define DEBUGSERVIDOR                                               // Descomentar para activar el modo de depuración del servidor

    extern bool servidorIniciado;
    extern int  nToque;
    extern AsyncWebSocket ws;
    extern AsyncWebServer server;
    extern WiFiClient client;

    // Pines de los relés para controlar las campanas
    //const int Rele0 = 26;                           
    //const int Rele1 = 27;
    //const int Rele2 = 25;


    // Usuario y contraseña
    //const char* http_username = "usuario";
    //const char* http_password = "clave";


    extern CAMPANARIO Campanario;

    void ServidorOn(const char* usuario, const char* clave);            // Función para iniciar el servidor HTTP y WebSocket
    void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);  // Callback para manejar eventos del WebSocket
    void procesaMensajeWebSocket(void *arg, uint8_t *data, size_t len); // Procesa los mensajes recibidos por WebSocket
    bool hayInternet(void);                              // Comprueba si hay conexión a Internet

    // Funciones auxiliares
    // Estas funciones se utilizan para listar los archivos en SPIFFS y para imprimir el contenido de un archivo específico.
    // Se pueden utilizar para depurar o verificar el contenido de los archivos almacenados en el sistema de archivos SPIFFS.
    void listSPIFFS(void);
    void printFileContent(const char* filename);


  

#endif