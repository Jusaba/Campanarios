/**
 * @file Servidor.h
 * @brief Sistema de servidor web HTTP y WebSocket con gestión completa de alarmas personalizables
 * 
 * @details Este header define el sistema completo de servidor web que permite control
 *          total del campanario con funcionalidades avanzadas de gestión web:
 *          
 *          **FUNCIONALIDADES PRINCIPALES:**
 *          - Servidor HTTP asíncrono para control remoto del campanario
 *          - WebSocket para comunicación bidireccional en tiempo real
 *          - Sistema de archivos SPIFFS para contenido web estático
 *          - Verificación de conectividad a Internet
 *          - Control del campanario a través de la clase CAMPANARIO
 *          - Debug condicional del servidor con DEBUGSERVIDOR en Debug.h
 *          - Gestión de eventos de WebSocket (conexión, desconexión, mensajes)
 *          - **GESTIÓN DE ALARMAS:** Sistema completo de alarmas personalizables vía web
 *          - **CONFIGURACIÓN MULTIIDIOMA:** Soporte catalán/español con persistencia
 *          - **API WEBSOCKET:** Comandos especializados para alarmas y configuración
 *          
 *          **ARQUITECTURA DEL SERVIDOR:**
 *          - AsyncWebServer: Servidor HTTP no bloqueante de alto rendimiento
 *          - AsyncWebSocket: Comunicación bidireccional en tiempo real
 *          - SPIFFS: Sistema de archivos para archivos HTML, CSS, JS y configuración
 *          - HTTPClient: Cliente para verificación de conectividad externa
 *          - Integración con clase CAMPANARIO: Control especializado del sistema
 *          - Integración con AlarmScheduler: Gestión completa de alarmas
 *          - Sistema de debug condicional con macro DEBUGSERVIDOR en Debug.h
 *          - **PERSISTENCIA JSON:** Configuración y alarmas en SPIFFS
 *          
 *          **SISTEMA DE ALARMAS PERSONALIZABLES:**
 *          - Creación dinámica de alarmas desde interfaz web
 *          - Edición completa con preservación de callbacks
 *          - Eliminación segura con reorganización automática
 *          - Habilitación/deshabilitación individual por ID único
 *          - Persistencia automática en /alarmas_personalizadas.json
 *          - Tipos de acción: MISA, DIFUNTOS, FIESTA (extensible)
 *          - Configuración automática de callbacks según tipo
 *          - Exportación JSON para sincronización con interfaz web
 *          
 *          **SISTEMA DE CONFIGURACIÓN MULTIIDIOMA:**
 *          - Soporte para catalán ("ca") y español ("es")
 *          - Persistencia en /config.json en SPIFFS
 *          - Cambio dinámico desde interfaz web sin reinicio
 *          - Configuración por defecto catalana recuperable
 *          - Validación y fallback automático ante errores
 * 
 * @note **INTEGRACIÓN CON SISTEMA:**
 *       - El servidor se integra con la clase CAMPANARIO para control completo
 *       - Controla el campanario a través de métodos de la clase especializada
 *       - Gestiona estados y secuencias mediante el objeto Campanario
 *       - Monitoreo de conectividad a Internet con hayInternet()
 *       - Sistema de archivos SPIFFS para interfaz web y persistencia
 *       - **ALARMAS:** Integración completa con AlarmScheduler para gestión web
 *       - **CALLBACKS:** Configuración automática de accionSecuencia según tipo
 *       - **TIEMPO REAL:** Sincronización inmediata vía WebSocket de cambios
 * 
 * @note **COMANDOS WEBSOCKET IMPLEMENTADOS:**
 *       - **ALARMAS:** ADD_ALARMA_WEB, EDIT_ALARMA_WEB, DELETE_ALARMA_WEB
 *       - **CONTROL:** TOGGLE_ALARMA_WEB, GET_ALARMAS_WEB, GET_STATS_ALARMAS_WEB
 *       - **CONFIGURACIÓN:** SET_IDIOMA, GET_CONFIG, GET_IDIOMA
 *       - **CAMPANARIO:** Comandos de control directo del sistema (heredados)
 *       - **RESPUESTAS:** Confirmaciones automáticas a todos los clientes conectados
 * 
 * @note **ESTRUCTURA DE DATOS JSON:**
 *       - **Alarmas:** {"id", "nombre", "descripcion", "dia", "hora", "minuto", "accion", "habilitada"}
 *       - **Configuración:** {"version", "idioma", "configuracion": {...}}
 *       - **Estadísticas:** {"total", "habilitadas", "personalizables", "sistema", "maximas"}
 *       - **Respuestas:** Prefijos específicos (ALARMA_CREADA_WEB:, ERROR_ALARMA_WEB:, etc.)
 * 
 * @warning **DEPENDENCIAS CRÍTICAS:**
 *          - ESPAsyncWebServer.h: Biblioteca del servidor HTTP asíncrono
 *          - AsyncTCP.h: Comunicación TCP asíncrona requerida
 *          - WiFi.h: Conectividad de red fundamental
 *          - SPIFFS.h: Sistema de archivos para contenido web y persistencia
 *          - HTTPClient.h: Cliente HTTP para verificación de Internet
 *          - ArduinoJson.h: Serialización JSON para alarmas y configuración
 *          - Campanario.h: Clase principal del sistema de campanario
 *          - Alarmas.h: Sistema de alarmas personalizables integrado
 *          - Configuracion.h: Estados y configuración del sistema
 *          - Debug.h: Sistema de logging del proyecto
 * 
 * @warning **CONFIGURACIÓN REQUERIDA:**
 *          - SPIFFS debe estar inicializado antes de usar el servidor
 *          - WiFi debe estar conectado antes de iniciar servidor
 *          - La clase CAMPANARIO debe estar inicializada
 *          - AlarmScheduler debe estar configurado y con begin() ejecutado
 *          - Definir DEBUGSERVIDOR en debug.h para habilitar logging
 *          - **CALLBACKS:** accionSecuencia debe estar disponible para alarmas
 *          - **ESPACIO SPIFFS:** Suficiente para archivos web + JSON de configuración
 * 
 * @warning **LIMITACIONES DEL SISTEMA:**
 *          - Máximo clientes WebSocket simultáneos limitado por ESP32
 *          - Persistencia JSON requiere espacio SPIFFS disponible
 *          - Alarmas limitadas por MAX_ALARMAS del AlarmScheduler
 *          - Configuración no thread-safe (usar desde hilo principal)
 *          - **CALLBACKS:** No serializables, se configuran al crear alarmas
 *          - **IDS ÚNICOS:** No reutilizan números eliminados
 * 
 * @author Julian Salas Bartolomé
 * @date 2025-01-06
 * @version 3.1
 * 
 * @since v1.0 - Servidor básico HTTP
 * @since v2.0 - WebSocket y integración con CAMPANARIO
 * @since v3.0 - SPIFFS, verificación Internet y debug avanzado
 * @since v3.1 - Sistema completo de alarmas personalizables y configuración multiidioma
 * 
 * @see Servidor.cpp - Implementación de todas las funcionalidades
 * @see Alarmas.h - Sistema de alarmas personalizables integrado
 * @see Campanario.h - Clase principal integrada
 * @see Configuracion.h - Configuración del sistema
 * @see Debug.h - Sistema de logging
 * @see AlarmasPersonalizadas.h - Definiciones específicas de alarmas web
 * 
 * @todo Implementar autenticación de usuarios para comandos críticos
 * @todo Sistema de perfiles de configuración para diferentes usuarios
 * @todo Backup automático de configuración antes de cambios críticos
 * @todo Validación en tiempo real de parámetros desde interfaz web
 * @todo Sistema de notificaciones push para estados críticos
 * @todo Interfaz web responsive para dispositivos móviles
 * @todo API REST complementaria al sistema WebSocket existente
 * @todo Historial de cambios de configuración con timestamps
 * 
 * @example **EJEMPLO DE USO - INICIALIZACIÓN COMPLETA:**
 * @code
 * void setup() {
 *     // Inicializar dependencias
 *     SPIFFS.begin(true);
 *     WiFi.begin(ssid, password);
 *     
 *     // Inicializar sistema de alarmas
 *     Alarmas.begin(true);
 *     
 *     // Iniciar servidor con todas las funcionalidades
 *     ServidorOn("admin", "password");
 *     
 *     Serial.println("Sistema iniciado con alarmas personalizables");
 * }
 * @endcode
 * 
 * @example **EJEMPLO DE PROCESAMIENTO DE COMANDOS:**
 * @code
 * // El servidor procesa automáticamente comandos como:
 * // ADD_ALARMA_WEB:{"nombre":"Misa","dia":1,"hora":11,"minuto":5,"accion":"MISA"}
 * // Resultado: Alarma creada y guardada en SPIFFS automáticamente
 * 
 * // Cambio de idioma:
 * // SET_IDIOMA:es
 * // Resultado: Idioma cambiado y persistido en /config.json
 * @endcode
 * 
 * @example **EJEMPLO DE INTEGRACIÓN CON INTERFAZ WEB:**
 * @code
 * // JavaScript en la interfaz web:
 * ws.send('GET_ALARMAS_WEB');  // Solicitar lista de alarmas
 * ws.send('ADD_ALARMA_WEB:' + JSON.stringify(nuevaAlarma));  // Crear alarma
 * ws.send('SET_IDIOMA:es');    // Cambiar idioma
 * 
 * // El servidor responde automáticamente:
 * // ALARMAS_WEB:[{...}]      // Lista completa
 * // ALARMA_CREADA_WEB:5      // Confirmación creación
 * // IDIOMA_CAMBIADO:es       // Confirmación cambio idioma
 * @endcode
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