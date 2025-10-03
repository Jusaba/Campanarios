/**
 * @file AlarmasPersonalizadas.h
 * @brief Módulo independiente para gestión de alarmas personalizables vía web
 * 
 * @details Módulo reutilizable que extiende el sistema de alarmas existente
 *          añadiendo funcionalidades de persistencia JSON y gestión web.
 *          Diseñado para integrarse sin modificar el código base existente.
 * 
 * @features
 *          - Persistencia en SPIFFS (JSON)
 *          - API REST/WebSocket compatible
 *          - Gestión CRUD completa (Create, Read, Update, Delete)
 *          - Estados habilitado/deshabilitado
 *          - Integración transparente con AlarmScheduler existente
 * 
 * @author Julian Salas Bartolomé
 * @date 2025-01-02
 * @version 1.0
 */

#ifndef ALARMAS_PERSONALIZADAS_H
#define ALARMAS_PERSONALIZADAS_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <vector>
#include "Configuracion.h"
#include "Debug.h"

// Forward declaration para evitar dependencias circulares
class AlarmScheduler;

namespace Config {
    namespace AlarmasPersonalizadas {
        extern const char* ARCHIVO_JSON;
        extern const char* VERSION_JSON;
        constexpr int MAX_ALARMAS = 50;
        constexpr size_t JSON_BUFFER_SIZE = 4096;
    }
}

/**
 * @brief Estructura de datos para alarma personalizable
 */
struct AlarmaPersonalizada {
    int id = 0;                         // ID único autogenerado
    String nombre = "";                 // Nombre descriptivo (ej: "Misa Domingo")
    int dia = 0;                        // 0=todos, 1=domingo, 2=lunes... 7=sábado
    int hora = 0;                       // 0-23
    int minuto = 0;                     // 0-59  
    int segundo = 0;                    // 0-59
    String accion = "MISA";             // Tipo: "MISA", "DIFUNTOS", "FIESTA", "HORA"
    int parametro = 0;                  // Parámetro adicional según acción
    bool habilitada = true;             // Estado activo/inactivo
    String descripcion = "";            // Descripción opcional
    
    // Constructor por defecto
    AlarmaPersonalizada() = default;
    
    // Constructor con parámetros básicos
    AlarmaPersonalizada(const String& nom, int d, int h, int m, const String& acc) 
        : nombre(nom), dia(d), hora(h), minuto(m), accion(acc) {}
};

/**
 * @brief Clase gestora de alarmas personalizables
 * 
 * @details Módulo independiente que gestiona alarmas configurables desde web.
 *          Se integra con AlarmScheduler existente mediante callbacks.
 */
class AlarmasPersonalizadas {
public:
    AlarmasPersonalizadas();
    ~AlarmasPersonalizadas();
    
    // === GESTIÓN DE PERSISTENCIA ===
    bool inicializar();                                                             // Inicializa módulo y carga desde SPIFFS
    bool cargarDesdeArchivo();                                                      // Carga alarmas desde JSON
    bool guardarEnArchivo();                                                        // Guarda alarmas en JSON
    
    // === GESTIÓN CRUD ===
    int crearAlarma(const AlarmaPersonalizada& alarma);                             // Retorna ID asignado, -1 si error
    bool modificarAlarma(int id, const AlarmaPersonalizada& alarma);                // Modifica alarma existente por ID
    bool eliminarAlarma(int id);                                                    // Elimina alarma por ID
    bool habilitarAlarma(int id, bool estado);                                      // Habilita/deshabilita alarma por ID   
    
    // === CONSULTAS ===
    String obtenerTodasJSON();                                                      // Todas las alarmas en formato JSON
    String obtenerAlarmaJSON(int id);                                               // Una alarma específica en JSON
    AlarmaPersonalizada* obtenerAlarma(int id);                                     // Puntero a alarma o nullptr
    std::vector<AlarmaPersonalizada> obtenerHabilitadas();                          // Solo alarmas activas
    int contarAlarmas() const { return _alarmas.size(); }                           // Número total de alarmas
    bool existeAlarma(int id) const;                                                // Comprueba existencia por ID   
    
    // === INTEGRACIÓN CON SISTEMA DE ALARMAS ===
    void registrarEnSistemaAlarmas(AlarmScheduler* scheduler);                      // Registra todas las alarmas habilitadas
    void actualizarSistemaAlarmas(AlarmScheduler* scheduler);                       // Actualiza alarmas en sistema
    
    // === UTILIDADES ===
    void limpiarTodas();                                                            // Elimina todas las alarmas
    bool validarAlarma(const AlarmaPersonalizada& alarma);                          // Valida datos de alarma
    String obtenerEstadisticas();                                                   // Info del módulo en JSON

    // === CALLBACKS PARA INTEGRACIÓN ===
    typedef void (*CallbackAccion)(uint16_t parametro);                             // Tipo de función callback
    void setCallbackMisa(CallbackAccion callback) { _callbackMisa = callback; }
    void setCallbackDifuntos(CallbackAccion callback) { _callbackDifuntos = callback; }
    void setCallbackFiesta(CallbackAccion callback) { _callbackFiesta = callback; }
    
private:
    std::vector<AlarmaPersonalizada> _alarmas;                                      // Vector de alarmas
    int _siguienteId;                                                               // Contador para IDs únicos
    bool _inicializado;                                                             // Estado de inicialización
    
    // Callbacks para acciones
    CallbackAccion _callbackMisa = nullptr;
    CallbackAccion _callbackDifuntos = nullptr;
    CallbackAccion _callbackFiesta = nullptr;
    
    // === MÉTODOS PRIVADOS ===
    int _buscarIndice(int id) const;                                                // Busca índice por ID
    int _generarNuevoId();                                                          // Genera ID único
    bool _crearArchivoVacio();                                                      // Crea archivo JSON inicial
    void _inicializarAlarmasDefecto();                                              // Crea alarmas por defecto
    JsonDocument _crearJsonVacio();                                                 // Plantilla JSON vacía
    String _accionToString(const String& accion);                                   // Valida y normaliza acciones
    String _diaToString(int dia);                                                   // Convierte día a nombre
};

// === INSTANCIA GLOBAL ===
extern AlarmasPersonalizadas AlarmasWeb;                                            // Instancia global para uso en el sistema

#endif // ALARMAS_PERSONALIZADAS_H