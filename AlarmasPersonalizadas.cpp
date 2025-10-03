/**
 * @file AlarmasPersonalizadas.cpp
 * @brief Implementación del módulo de alarmas personalizables
 * 
 * @author Julian Salas Bartolomé
 * @date 2025-01-02
 * @version 1.0
 */

#include "AlarmasPersonalizadas.h"
#include "Alarmas.h"
#include "Acciones.h"

// Definiciones de constantes
namespace Config {
    namespace AlarmasPersonalizadas {
        const char* ARCHIVO_JSON = "/alarmas_personalizadas.json";
        const char* VERSION_JSON = "1.0";
    }
}

// Instancia global
AlarmasPersonalizadas AlarmasWeb;

// ========== CONSTRUCTOR Y DESTRUCTOR ==========

/**
 * @brief Constructor de la clase AlarmasPersonalizadas
 * 
 * Inicializa una nueva instancia de AlarmasPersonalizadas con valores por defecto.
 * El ID inicial se establece en 1, el estado de inicialización en falso y
 * se limpia el vector de alarmas.
 */
AlarmasPersonalizadas::AlarmasPersonalizadas() 
    : _siguienteId(1), _inicializado(false) {
    _alarmas.clear();
    DBG_ALARMS("🔔 AlarmasPersonalizadas: Constructor");
}

/**
 * @brief Destructor de la clase AlarmasPersonalizadas
 * 
 * Libera los recursos utilizados por la instancia de AlarmasPersonalizadas.
 * Muestra un mensaje de depuración indicando que se está ejecutando el destructor.
 */
AlarmasPersonalizadas::~AlarmasPersonalizadas() {
    DBG_ALARMS("🔔 AlarmasPersonalizadas: Destructor");
}

// ========== GESTIÓN DE PERSISTENCIA ==========

/**
 * @brief Inicializa el módulo de alarmas personalizadas
 * 
 * Este método realiza las siguientes operaciones:
 * - Verifica si el módulo ya está inicializado
 * - Monta el sistema de archivos SPIFFS
 * - Carga las alarmas existentes desde el archivo
 * - Si no hay alarmas, crea las alarmas por defecto
 * 
 * @return true si la inicialización fue exitosa
 * @return false si hubo error al montar SPIFFS
 * 
 * @note Si el módulo ya está inicializado, retorna true sin realizar ninguna operación
 */
bool AlarmasPersonalizadas::inicializar() {
    DBG_ALARMS("🔔 AlarmasPersonalizadas: Inicializando módulo...");
    
    if (_inicializado) {
        DBG_ALARMS("⚠️ Módulo ya inicializado");
        return true;
    }
    
    // Verificar que SPIFFS esté montado
    if (!SPIFFS.begin(true)) {
        DBG_ALARMS("❌ Error: No se pudo montar SPIFFS");
        return false;
    }
    
    // Intentar cargar alarmas existentes
    if (!cargarDesdeArchivo()) {
        DBG_ALARMS("⚠️ No se pudieron cargar alarmas, creando archivo inicial...");
        this->_inicializarAlarmasDefecto();
        this->guardarEnArchivo();
    }
    
    this->_inicializado = true;
    DBG_ALARMS_PRINTF("✅ Módulo inicializado. Alarmas cargadas: %d", _alarmas.size());
    return true;
}

/**
 * @brief Carga las alarmas personalizadas desde un archivo JSON almacenado en SPIFFS
 * 
 * Este método lee el archivo JSON de alarmas almacenado en SPIFFS, deserializa su contenido
 * y carga las alarmas en la lista interna _alarmas. Durante el proceso:
 * - Verifica la existencia del archivo
 * - Lee y parsea el contenido JSON
 * - Limpia las alarmas existentes
 * - Valida y carga cada alarma del archivo
 * - Actualiza el ID para nuevas alarmas
 * 
 * @return true si las alarmas se cargaron correctamente
 * @return false si hubo algún error (archivo no existe, error de lectura o JSON inválido)
 * 
 * @note Las alarmas inválidas son ignoradas durante la carga
 * @note Actualiza _siguienteId basándose en el ID más alto encontrado
 * 
 * @see validarAlarma()
 * @see Config::AlarmasPersonalizadas::ARCHIVO_JSON
 */
bool AlarmasPersonalizadas::cargarDesdeArchivo() {
    DBG_ALARMS("📂 Cargando alarmas desde archivo JSON...");
    
    if (!SPIFFS.exists(Config::AlarmasPersonalizadas::ARCHIVO_JSON)) {                      //Si no existe el fichero json con las alarmas
        DBG_ALARMS("📄 Archivo de alarmas no existe");
        return false;
    }
    
    File archivo = SPIFFS.open(Config::AlarmasPersonalizadas::ARCHIVO_JSON, "r");           //Abrimos el archivo en modo lectura
    if (!archivo) {                                                                         //Si no se puede abrir
        DBG_ALARMS("❌ Error al abrir archivo de alarmas");
        return false;
    }
    
    // Leer contenido del archivo
    String contenido = archivo.readString();                                                //Leemos el contenido del archivo
    archivo.close();                                                                        //Cerramos el archivo                
    
    DBG_ALARMS_PRINTF("📄 Contenido archivo (%d bytes): %s", contenido.length(), contenido.c_str());
    
    // Parsear JSON
    JsonDocument doc;                                                                       //Crear documento JSON        
    DeserializationError error = deserializeJson(doc, contenido);                           //Parsear el contenido del archivo 
    
    if (error) {                                                                            //Si hay error al parsear
        DBG_ALARMS_PRINTF("❌ Error parseando JSON: %s", error.c_str());
        return false;
    }
    
    // Limpiar alarmas actuales
    this->_alarmas.clear();                                                                       //Limpiamos las alarmas actuales                          
    
    // Cargar alarmas del JSON
    JsonArray alarmasArray = doc["alarmas"];                                                      //Obtenemos el array de alarmas del JSON
    int alarmascargadas = 0;
    
    for (JsonObject alarmaObj : alarmasArray) {                                                   //Recorremos cada objeto alarma del array   
        AlarmaPersonalizada alarma;                                                               //Creamos una instancia de AlarmaPersonalizada
        
        alarma.id = alarmaObj["id"] | 0;                                                          //Cargamos los datos de la alarma desde el JSON   
        alarma.nombre = alarmaObj["nombre"] | "";
        alarma.dia = alarmaObj["dia"] | 0;
        alarma.hora = alarmaObj["hora"] | 0;
        alarma.minuto = alarmaObj["minuto"] | 0;
        alarma.segundo = alarmaObj["segundo"] | 0;
        alarma.accion = alarmaObj["accion"] | "MISA";
        alarma.parametro = alarmaObj["parametro"] | 0;
        alarma.habilitada = alarmaObj["habilitada"] | true;
        alarma.descripcion = alarmaObj["descripcion"] | "";
        
        if (this->validarAlarma(alarma)) {                                                        //Validamos la alarma
            this->_alarmas.push_back(alarma);                                                     //Si es válida, la añadimos al vector de alarmas
            alarmascargadas++;                                                                    //Incrementamos contador de alarmas cargadas
            
            // Actualizar siguiente ID
            if (alarma.id >= this->_siguienteId) {
                this->_siguienteId = alarma.id + 1;
            }
        } else {
            DBG_ALARMS_PRINTF("⚠️ Alarma inválida ignorada: ID=%d", alarma.id);
        }
    }
    
    DBG_ALARMS_PRINTF("✅ Alarmas cargadas correctamente: %d", alarmascargadas);
    return true;
}

/**
 * @brief Guarda todas las alarmas personalizadas en un archivo JSON.
 * 
 * Este método serializa todas las alarmas almacenadas en la clase en formato JSON
 * y las guarda en el sistema de archivos SPIFFS. El JSON generado incluye:
 * - Versión del formato JSON
 * - Timestamp de la generación
 * - Número total de alarmas
 * - Array con todas las alarmas, donde cada alarma contiene:
 *   - id
 *   - nombre
 *   - día
 *   - hora
 *   - minuto
 *   - segundo
 *   - acción
 *   - parámetro
 *   - estado de habilitación
 *   - descripción
 * 
 * @return true Si el archivo se guardó correctamente
 * @return false Si hubo algún error al crear el archivo o escribir los datos
 */
bool AlarmasPersonalizadas::guardarEnArchivo() {
    DBG_ALARMS("💾 Guardando alarmas en archivo JSON...");
    
    // Crear documento JSON
    JsonDocument doc;                                                                   //Crear documento JSON
    doc["version"] = Config::AlarmasPersonalizadas::VERSION_JSON;                       // Versión del formato JSON
    doc["timestamp"] = millis();                                                        // Timestamp de generación
    doc["total"] = this->_alarmas.size();                                               // Número total de alarmas

    JsonArray alarmasArray = doc.createNestedArray("alarmas");                          // Array de alarmas

    // Añadir cada alarma al JSON
    for (const auto& alarma : this->_alarmas) {                                         //Recorremos cada alarma del vector
        JsonObject alarmaObj = alarmasArray.createNestedObject();                       //Creamos un objeto JSON para la alarma
        
        alarmaObj["id"] = alarma.id;                                                    //Cargamos los datos de la alarma en el objeto JSON
        alarmaObj["nombre"] = alarma.nombre;
        alarmaObj["dia"] = alarma.dia;
        alarmaObj["hora"] = alarma.hora;
        alarmaObj["minuto"] = alarma.minuto;
        alarmaObj["segundo"] = alarma.segundo;
        alarmaObj["accion"] = alarma.accion;
        alarmaObj["parametro"] = alarma.parametro;
        alarmaObj["habilitada"] = alarma.habilitada;
        alarmaObj["descripcion"] = alarma.descripcion;
    }
    
    // Abrir archivo para escritura
    File archivo = SPIFFS.open(Config::AlarmasPersonalizadas::ARCHIVO_JSON, "w");       //Abrimos el archivo en modo escritura
    if (!archivo) {                                                                     //Si no se puede abrir
        DBG_ALARMS("❌ Error al crear archivo de alarmas");
        return false;
    }
    
    // Escribir JSON al archivo
    size_t bytesEscritos = serializeJson(doc, archivo);                                 //Escribimos el JSON en el archivo
    archivo.close();                                                                    //Cerramos el archivo

    if (bytesEscritos == 0) {                                                           //Si no se escribió nada
        DBG_ALARMS("❌ Error escribiendo JSON al archivo");
        return false;
    }
    
    DBG_ALARMS_PRINTF("✅ Archivo guardado correctamente (%d bytes)", bytesEscritos);
    return true;                                                                        //Guardado correcto
}

// ========== GESTIÓN CRUD ==========

/**
 * @brief Crea una nueva alarma personalizada en el sistema
 * 
 * @details Crea una nueva alarma personalizada con los datos proporcionados, asigna un ID único
 * y la almacena tanto en memoria como en el archivo de persistencia.
 * 
 * @param alarma Objeto AlarmaPersonalizada con los datos de la alarma a crear
 * 
 * @return int ID asignado a la alarma creada, o -1 si ocurre algún error:
 *         - Si se alcanza el número máximo de alarmas permitidas
 *         - Si los datos de la alarma son inválidos
 * 
 * @note Aunque falle el guardado en archivo, la alarma quedará creada en memoria
 *       si el resto del proceso fue exitoso
 */
int AlarmasPersonalizadas::crearAlarma(const AlarmaPersonalizada& alarma) {
    DBG_ALARMS_PRINTF("➕ Creando nueva alarma: %s", alarma.nombre.c_str());
    
    if (this->_alarmas.size() >= Config::AlarmasPersonalizadas::MAX_ALARMAS) {
        DBG_ALARMS("❌ Máximo número de alarmas alcanzado");
        return -1;
    }
    
    // Crear copia y asignar ID
    AlarmaPersonalizada nuevaAlarma = alarma;                                           //Crear copia de la alarma  
    nuevaAlarma.id = this->_generarNuevoId();                                           //Asignar nuevo ID único 
    
    if (!this->validarAlarma(nuevaAlarma)) {                                            //Validar datos de la alarma    
        DBG_ALARMS("❌ Datos de alarma inválidos");                                     //Si no es valida
        return -1;
    }
    
    // Añadir al vector
    this->_alarmas.push_back(nuevaAlarma);                                                    //Añadir la nueva alarma al vector de alarmas
    
    // Guardar en archivo
    if (!this->guardarEnArchivo()) {                                                          //Intentar guardar en archivo
        DBG_ALARMS("⚠️ Error guardando archivo, pero alarma creada en memoria");
    }
    
    DBG_ALARMS_PRINTF("✅ Alarma creada con ID: %d", nuevaAlarma.id);
    return nuevaAlarma.id;
}

/**
 * @brief Modifica una alarma existente en el sistema
 * 
 * @details Busca una alarma por su ID y la actualiza con los nuevos datos proporcionados.
 * Mantiene el ID original y valida que los nuevos datos sean correctos antes de realizar
 * la modificación. Los cambios se persisten automáticamente en el archivo de almacenamiento.
 * 
 * @param id Identificador único de la alarma a modificar
 * @param alarma Nueva configuración de la alarma con los datos actualizados
 * 
 * @return true Si la alarma se modificó exitosamente
 * @return false Si la alarma no existe o los datos son inválidos
 * 
 * @see validarAlarma()
 * @see guardarEnArchivo()
 */
bool AlarmasPersonalizadas::modificarAlarma(int id, const AlarmaPersonalizada& alarma) {
    DBG_ALARMS_PRINTF("✏️ Modificando alarma ID: %d", id);
    
    int indice = this->_buscarIndice(id);                                                   //Buscar índice de la alarma por ID
    if (indice == -1) {                                                                     //Si no se encuentra
        DBG_ALARMS("❌ Alarma no encontrada");
        return false;
    }
    
    // Validar datos
    AlarmaPersonalizada alarmaTemp = alarma;                                               //Crear copia de la alarma
    alarmaTemp.id = id; // Mantener el ID original

    if (!this->validarAlarma(alarmaTemp)) {                                                //Validar datos de la alarma
        DBG_ALARMS("❌ Datos de alarma inválidos");                                        //Si no es valida
        return false;
    }
    
    // Actualizar alarma
    _alarmas[indice] = alarmaTemp;                                                         //Actualizar la alarma en el vector 
    
    // Guardar en archivo
    guardarEnArchivo();
    
    DBG_ALARMS("✅ Alarma modificada correctamente");
    return true;
}

/**
 * @brief Elimina una alarma personalizada por su ID
 * 
 * @details Busca una alarma por su ID en el vector de alarmas y la elimina.
 * Después de eliminar la alarma, guarda los cambios en el archivo de configuración.
 * 
 * @param id Identificador único de la alarma a eliminar
 * 
 * @return true Si la alarma fue encontrada y eliminada correctamente
 * @return false Si la alarma no fue encontrada
 * 
 * @note Esta función modifica el vector de alarmas y actualiza el archivo de configuración
 */
bool AlarmasPersonalizadas::eliminarAlarma(int id) {
    DBG_ALARMS_PRINTF("🗑️ Eliminando alarma ID: %d", id);
    
    int indice = this->_buscarIndice(id);                                            //Buscar índice de la alarma por ID                    
    if (indice == -1) {                                                              //Si no se encuentra    
        DBG_ALARMS("❌ Alarma no encontrada");
        return false;
    }
    
    // Eliminar del vector
    this->_alarmas.erase(this->_alarmas.begin() + indice);                           //Eliminar la alarma del vector de alarmas    
    
    // Guardar en archivo
    this->guardarEnArchivo();                                                        //Guardar cambios en el archivo

    DBG_ALARMS("✅ Alarma eliminada correctamente");
    return true;
}

bool AlarmasPersonalizadas::habilitarAlarma(int id, bool estado) {
    DBG_ALARMS_PRINTF("🔄 %s alarma ID: %d", estado ? "Habilitando" : "Deshabilitando", id);
    
    int indice = _buscarIndice(id);
    if (indice == -1) {
        DBG_ALARMS("❌ Alarma no encontrada");
        return false;
    }
    
    _alarmas[indice].habilitada = estado;
    
    // Guardar en archivo
    guardarEnArchivo();
    
    DBG_ALARMS_PRINTF("✅ Alarma %s correctamente", estado ? "habilitada" : "deshabilitada");
    return true;
}

// ========== MÉTODOS PRIVADOS ==========

int AlarmasPersonalizadas::_buscarIndice(int id) const {
    for (size_t i = 0; i < _alarmas.size(); i++) {
        if (_alarmas[i].id == id) {
            return i;
        }
    }
    return -1;
}

int AlarmasPersonalizadas::_generarNuevoId() {
    return _siguienteId++;
}

bool AlarmasPersonalizadas::validarAlarma(const AlarmaPersonalizada& alarma) {
    // Validar horario
    if (alarma.hora < 0 || alarma.hora > 23) return false;
    if (alarma.minuto < 0 || alarma.minuto > 59) return false;
    if (alarma.segundo < 0 || alarma.segundo > 59) return false;
    
    // Validar día
    if (alarma.dia < 0 || alarma.dia > 7) return false;
    
    // Validar acción
    if (alarma.accion != "MISA" && alarma.accion != "DIFUNTOS" && 
        alarma.accion != "FIESTA" && alarma.accion != "HORA") {
        return false;
    }
    
    // Validar nombre no vacío
    if (alarma.nombre.length() == 0) return false;
    
    return true;
}

void AlarmasPersonalizadas::_inicializarAlarmasDefecto() {
    DBG_ALARMS("🔄 Creando alarmas por defecto...");
    
    // Misa dominical 11:05
    AlarmaPersonalizada misa1;
    misa1.nombre = "Misa Domingo 11:05";
    misa1.dia = 1; // Domingo
    misa1.hora = 11;
    misa1.minuto = 5;
    misa1.accion = "MISA";
    misa1.habilitada = true;
    misa1.descripcion = "Primera llamada misa dominical";
    
    // Misa dominical 11:25
    AlarmaPersonalizada misa2;
    misa2.nombre = "Misa Domingo 11:25";
    misa2.dia = 1; // Domingo
    misa2.hora = 11;
    misa2.minuto = 25;
    misa2.accion = "MISA";
    misa2.habilitada = true;
    misa2.descripcion = "Segunda llamada misa dominical";
    
    // Añadir alarmas por defecto
    _alarmas.push_back(misa1);
    _alarmas.push_back(misa2);
    
    // Asignar IDs
    _alarmas[0].id = _generarNuevoId();
    _alarmas[1].id = _generarNuevoId();
    
    DBG_ALARMS("✅ Alarmas por defecto creadas");
}

// ========== CONSULTAS ==========

String AlarmasPersonalizadas::obtenerTodasJSON() {
    DBG_ALARMS("📋 Generando JSON con todas las alarmas...");
    
    JsonDocument doc;
    doc["version"] = Config::AlarmasPersonalizadas::VERSION_JSON;
    doc["total"] = _alarmas.size();
    doc["timestamp"] = millis();
    
    JsonArray alarmasArray = doc.createNestedArray("alarmas");
    
    for (const auto& alarma : _alarmas) {
        JsonObject alarmaObj = alarmasArray.createNestedObject();
        
        alarmaObj["id"] = alarma.id;
        alarmaObj["nombre"] = alarma.nombre;
        alarmaObj["dia"] = alarma.dia;
        alarmaObj["diaNombre"] = _diaToString(alarma.dia);
        alarmaObj["hora"] = alarma.hora;
        alarmaObj["minuto"] = alarma.minuto;
        alarmaObj["segundo"] = alarma.segundo;
        alarmaObj["accion"] = alarma.accion;
        alarmaObj["parametro"] = alarma.parametro;
        alarmaObj["habilitada"] = alarma.habilitada;
        alarmaObj["descripcion"] = alarma.descripcion;
        
        // Formatear hora para mostrar (ej: "11:05")
        char horaFormateada[8];
        sprintf(horaFormateada, "%02d:%02d", alarma.hora, alarma.minuto);
        alarmaObj["horaTexto"] = horaFormateada;
    }
    
    String resultado;
    serializeJson(doc, resultado);
    
    DBG_ALARMS_PRINTF("📋 JSON generado (%d chars): %s", resultado.length(), resultado.c_str());
    return resultado;
}

String AlarmasPersonalizadas::obtenerAlarmaJSON(int id) {
    DBG_ALARMS_PRINTF("🔍 Obteniendo JSON para alarma ID: %d", id);
    
    int indice = _buscarIndice(id);
    if (indice == -1) {
        DBG_ALARMS("❌ Alarma no encontrada");
        return "{}";
    }
    
    const auto& alarma = _alarmas[indice];
    
    JsonDocument doc;
    doc["id"] = alarma.id;
    doc["nombre"] = alarma.nombre;
    doc["dia"] = alarma.dia;
    doc["diaNombre"] = _diaToString(alarma.dia);
    doc["hora"] = alarma.hora;
    doc["minuto"] = alarma.minuto;
    doc["segundo"] = alarma.segundo;
    doc["accion"] = alarma.accion;
    doc["parametro"] = alarma.parametro;
    doc["habilitada"] = alarma.habilitada;
    doc["descripcion"] = alarma.descripcion;
    
    String resultado;
    serializeJson(doc, resultado);
    return resultado;
}

AlarmaPersonalizada* AlarmasPersonalizadas::obtenerAlarma(int id) {
    int indice = _buscarIndice(id);
    if (indice == -1) {
        return nullptr;
    }
    return &_alarmas[indice];
}

std::vector<AlarmaPersonalizada> AlarmasPersonalizadas::obtenerHabilitadas() {
    std::vector<AlarmaPersonalizada> habilitadas;
    
    for (const auto& alarma : _alarmas) {
        if (alarma.habilitada) {
            habilitadas.push_back(alarma);
        }
    }
    
    DBG_ALARMS_PRINTF("🔍 Alarmas habilitadas: %d de %d", habilitadas.size(), _alarmas.size());
    return habilitadas;
}

bool AlarmasPersonalizadas::existeAlarma(int id) const {
    return _buscarIndice(id) != -1;
}

// ========== INTEGRACIÓN CON SISTEMA DE ALARMAS ==========

void AlarmasPersonalizadas::registrarEnSistemaAlarmas(AlarmScheduler* scheduler) {
    if (!scheduler) {
        DBG_ALARMS("❌ Scheduler nulo");
        return;
    }
    
    DBG_ALARMS("🔗 Registrando alarmas personalizadas en sistema...");
    
    auto alarmasHabilitadas = obtenerHabilitadas();
    int registradas = 0;
    
    for (const auto& alarma : alarmasHabilitadas) {
        // Determinar callback según acción
        if (alarma.accion == "MISA" && _callbackMisa) {
            scheduler->addExternal(alarma.dia, alarma.hora, alarma.minuto, 0, 
                                 _callbackMisa, (uint16_t)Config::States::I2CState::MISA);
            registradas++;
            DBG_ALARMS_PRINTF("✅ Registrada MISA: %s (%s %02d:%02d)", 
                            alarma.nombre.c_str(), _diaToString(alarma.dia).c_str(), 
                            alarma.hora, alarma.minuto);
        }
        else if (alarma.accion == "DIFUNTOS" && _callbackDifuntos) {
            scheduler->addExternal(alarma.dia, alarma.hora, alarma.minuto, 0, 
                                 _callbackDifuntos, Config::States::I2CState::DIFUNTOS);
            registradas++;
            DBG_ALARMS_PRINTF("✅ Registrada DIFUNTOS: %s (%s %02d:%02d)", 
                            alarma.nombre.c_str(), _diaToString(alarma.dia).c_str(), 
                            alarma.hora, alarma.minuto);
        }
        else if (alarma.accion == "FIESTA" && _callbackFiesta) {
            scheduler->addExternal(alarma.dia, alarma.hora, alarma.minuto, 0, 
                                 _callbackFiesta, alarma.parametro);
            registradas++;
            DBG_ALARMS_PRINTF("✅ Registrada FIESTA: %s (%s %02d:%02d)", 
                            alarma.nombre.c_str(), _diaToString(alarma.dia).c_str(), 
                            alarma.hora, alarma.minuto);
        }
        else {
            DBG_ALARMS_PRINTF("⚠️ Alarma sin callback válido: %s (%s)", 
                            alarma.nombre.c_str(), alarma.accion.c_str());
        }
    }
    
    DBG_ALARMS_PRINTF("🔗 Alarmas registradas en sistema: %d de %d", registradas, alarmasHabilitadas.size());
}

void AlarmasPersonalizadas::actualizarSistemaAlarmas(AlarmScheduler* scheduler) {
    DBG_ALARMS("🔄 Actualizando sistema de alarmas...");
    
    // TODO: Implementar limpieza de alarmas personalizadas existentes
    // Por ahora, simplemente registramos todas de nuevo
    // En una versión futura, se podría implementar un sistema más sofisticado
    
    registrarEnSistemaAlarmas(scheduler);
}

// ========== UTILIDADES ==========

void AlarmasPersonalizadas::limpiarTodas() {
    DBG_ALARMS("🧹 Limpiando todas las alarmas...");
    
    _alarmas.clear();
    _siguienteId = 1;
    
    guardarEnArchivo();
    
    DBG_ALARMS("✅ Todas las alarmas eliminadas");
}

String AlarmasPersonalizadas::obtenerEstadisticas() {
    JsonDocument doc;
    
    doc["modulo"] = "AlarmasPersonalizadas";
    doc["version"] = Config::AlarmasPersonalizadas::VERSION_JSON;
    doc["inicializado"] = _inicializado;
    doc["totalAlarmas"] = _alarmas.size();
    doc["siguienteId"] = _siguienteId;
    doc["maxAlarmas"] = Config::AlarmasPersonalizadas::MAX_ALARMAS;
    doc["espacioLibre"] = Config::AlarmasPersonalizadas::MAX_ALARMAS - _alarmas.size();
    
    // Contar alarmas habilitadas/deshabilitadas
    int habilitadas = 0, deshabilitadas = 0;
    for (const auto& alarma : _alarmas) {
        if (alarma.habilitada) habilitadas++;
        else deshabilitadas++;
    }
    
    doc["habilitadas"] = habilitadas;
    doc["deshabilitadas"] = deshabilitadas;
    
    // Estado de callbacks
    doc["callbackMisa"] = (_callbackMisa != nullptr);
    doc["callbackDifuntos"] = (_callbackDifuntos != nullptr);
    doc["callbackFiesta"] = (_callbackFiesta != nullptr);
    
    // Info de archivo
    doc["archivo"] = Config::AlarmasPersonalizadas::ARCHIVO_JSON;
    doc["archivoExiste"] = SPIFFS.exists(Config::AlarmasPersonalizadas::ARCHIVO_JSON);
    
    String resultado;
    serializeJson(doc, resultado);
    return resultado;
}

// ========== MÉTODOS PRIVADOS ADICIONALES ==========

String AlarmasPersonalizadas::_diaToString(int dia) {
    switch (dia) {
        case 0: return "Todos los días";
        case 1: return "Domingo";
        case 2: return "Lunes";
        case 3: return "Martes";
        case 4: return "Miércoles";
        case 5: return "Jueves";
        case 6: return "Viernes";
        case 7: return "Sábado";
        default: return "Día inválido";
    }
}

String AlarmasPersonalizadas::_accionToString(const String& accion) {
    String accionUpper = accion;
    accionUpper.toUpperCase();
    
    if (accionUpper == "MISA" || accionUpper == "DIFUNTOS" || 
        accionUpper == "FIESTA" || accionUpper == "HORA") {
        return accionUpper;
    }
    
    return "MISA"; // Por defecto
}

JsonDocument AlarmasPersonalizadas::_crearJsonVacio() {
    JsonDocument doc;
    doc["version"] = Config::AlarmasPersonalizadas::VERSION_JSON;
    doc["total"] = 0;
    doc["alarmas"] = JsonArray();
    return doc;
}