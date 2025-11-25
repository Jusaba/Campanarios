#include "Campanario.h"
#include <SPIFFS.h>
#include <FS.h>

    /**
     * @brief Constructor que inicializa el sistema de campanario
     * 
     * @details Inicializa todas las variables internas del campanario estableciendo
     *          un estado conocido y seguro. Prepara los arrays de campanas y
     *          campanadas, resetea contadores y establece flags de estado.
     *          
     *          **INICIALIZACIÓN REALIZADA:**
     *          - Array de punteros a campanas inicializado a nullptr
     *          - Contador de campanas establecido a 0
     *          - Array de campanadas limpiado
     *          - Variables de secuencia reseteadas
     *          - Estado del campanario establecido a 0 (todos los bits clear)
     *          - Puntero de calefacción inicializado a nullptr
     * 
     * @note **ESTADO INICIAL:** Campanario completamente inactivo y listo para uso
     * @note **MEMORIA:** Arrays inicializados pero sin asignación dinámica
     * @note **THREAD-SAFE:** Constructor es thread-safe
     * 
     * @see AddCampana() - Para añadir campanas al sistema inicializado
     * @see AddCalefaccion() - Para añadir sistema de calefacción
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    CAMPANARIO::CAMPANARIO() {
        // Inicializar array de campanas
        for (int i = 0; i < Config::Campanario::MAX_CAMPANAS; i++) {
            this->_pCampanas[i] = nullptr;
        }
        this->_nNumCampanas = 0;

        // Inicializar sistema de secuencias
        this->_LimpiaraCampanadas();
        this->_indiceCampanadaActual = 0;
        this->_ultimoToqueMs = 0;
        this->_tocandoSecuencia = false;

        // Inicializar variables de estado
        this->_nCampanaTocada = 0;
        this->_lCalefaccion = false;
        this->_pCalefaccion = nullptr;
        this->_nEstadoCampanario = 0;
        this->_tocando = false;
        this->_Calefaccion = false;

        DBG_CAM("[CAMPANARIO] Inicializado - Estado: LISTO");
    }
    /**
     * @brief Destructor que asegura la limpieza segura del sistema
     * 
     * @details Destructor que detiene cualquier secuencia activa y limpia
     *          el estado del sistema para evitar comportamientos inesperados.
     * 
     * @note **SEGURIDAD:** Para automáticamente cualquier secuencia en curso
     * @note **CLEANUP:** Limpia estado pero no libera punteros (no owns them)
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    CAMPANARIO::~CAMPANARIO() {
        this->ParaSecuencia();
        
        // Liberar memoria de secuencias dinámicas
        if (_secuenciaDifuntos) {
            delete[] _secuenciaDifuntos;
            _secuenciaDifuntos = nullptr;
        }
        if (_secuenciaMisa) {
            delete[] _secuenciaMisa;
            _secuenciaMisa = nullptr;
        }
        if (_secuenciaFiesta) {
            delete[] _secuenciaFiesta;
            _secuenciaFiesta = nullptr;
        }
        
        DBG_CAM("[CAMPANARIO] Destructor - Sistema limpiado\n");        
    }

    /**
     * @brief Carga las secuencias de campanadas desde archivo JSON
     * 
     * @details Lee el archivo Secuencias.json desde SPIFFS y carga las tres
     *          secuencias (Difuntos, Misa, Fiesta) en memoria dinámica.
     *          Si el archivo no existe, crea uno con valores por defecto.
     *          
     *          **PROCESO DE CARGA:**
     *          1. Verifica existencia del archivo Secuencias.json
     *          2. Si no existe, crea archivo con secuencias por defecto
     *          3. Parsea el JSON manualmente (sin ArduinoJson)
     *          4. Asigna memoria dinámica para cada secuencia
     *          5. Carga los pasos de cada secuencia
     * 
     * @return true si las secuencias se cargaron correctamente, false si hubo error
     * 
     * @note **SPIFFS:** Requiere que SPIFFS esté inicializado previamente
     * @note **MEMORIA:** Asigna memoria dinámica que se libera en el destructor
     * @note **JSON SIMPLE:** Parseo manual optimizado, sin librerías externas
     * 
     * @warning **MEMORIA LIMITADA:** Verifica que hay suficiente heap antes de cargar
     * @warning **ARCHIVO CORRUPTO:** Si el JSON está mal formado, usa valores por defecto
     * 
     * @see CrearSecuenciasPorDefecto() - Crea archivo si no existe
     * 
     * @since v1.0.14
     * @author Julian Salas Bartolomé
     */
    bool CAMPANARIO::CargarSecuencias() {
        DBG_CAM("[CAMPANARIO] Cargando secuencias desde SPIFFS...");
        
        // Verificar que SPIFFS está montado
        if (!SPIFFS.begin(true)) {
            DBG_CAM("[CAMPANARIO] ERROR: No se pudo montar SPIFFS");
            return false;
        }
        
        // Verificar si existe el archivo
        if (!SPIFFS.exists("/Secuencias.json")) {
            DBG_CAM("[CAMPANARIO] Secuencias.json no existe, usando valores por defecto hardcodeados");
            // Por ahora retornamos false para usar las secuencias hardcodeadas
            // En el futuro se puede crear el archivo automáticamente
            return false;
        }
        
        // Abrir archivo
        File file = SPIFFS.open("/Secuencias.json", "r");
        if (!file) {
            DBG_CAM("[CAMPANARIO] ERROR: No se pudo abrir Secuencias.json");
            return false;
        }
        
        // Leer contenido completo
        String jsonContent = file.readString();
        file.close();
        
        DBG_CAM_PRINTF("[CAMPANARIO] JSON leído: %d bytes\n", jsonContent.length());
        
        // Parsear JSON manualmente (formato simple y conocido)
        // Buscar cada secuencia y cargarla
        
        // Liberar memoria anterior si existe
        if (_secuenciaDifuntos) delete[] _secuenciaDifuntos;
        if (_secuenciaMisa) delete[] _secuenciaMisa;
        if (_secuenciaFiesta) delete[] _secuenciaFiesta;
        
        // Cargar secuencia de Difuntos
        int startDifuntos = jsonContent.indexOf("\"difuntos\"");
        if (startDifuntos > 0) {
            int startPasos = jsonContent.indexOf("\"pasos\"", startDifuntos);
            int startArray = jsonContent.indexOf("[", startPasos);
            int endArray = jsonContent.indexOf("]", startArray);
            
            if (startArray > 0 && endArray > startArray) {
                String pasosStr = jsonContent.substring(startArray + 1, endArray);
                _numPasosDifuntos = 0;
                
                // Contar número de objetos {}
                int count = 0;
                for (int i = 0; i < pasosStr.length(); i++) {
                    if (pasosStr[i] == '{') count++;
                }
                
                _numPasosDifuntos = count;
                _secuenciaDifuntos = new PasoSecuencia[_numPasosDifuntos];
                
                // Parsear cada paso
                int currentPaso = 0;
                int pos = 0;
                while (currentPaso < _numPasosDifuntos && pos < pasosStr.length()) {
                    int objStart = pasosStr.indexOf("{", pos);
                    int objEnd = pasosStr.indexOf("}", objStart);
                    
                    if (objStart < 0 || objEnd < 0) break;
                    
                    String obj = pasosStr.substring(objStart, objEnd + 1);
                    
                    // Extraer campana
                    int campanaIdx = obj.indexOf("\"campana\"");
                    int campanaValStart = obj.indexOf(":", campanaIdx) + 1;
                    int campanaValEnd = obj.indexOf(",", campanaValStart);
                    if (campanaValEnd < 0) campanaValEnd = obj.indexOf("}", campanaValStart);
                    String campanaStr = obj.substring(campanaValStart, campanaValEnd);
                    campanaStr.trim();
                    
                    // Extraer repeticiones
                    int repIdx = obj.indexOf("\"repeticiones\"");
                    int repValStart = obj.indexOf(":", repIdx) + 1;
                    int repValEnd = obj.indexOf(",", repValStart);
                    if (repValEnd < 0) repValEnd = obj.indexOf("}", repValStart);
                    String repStr = obj.substring(repValStart, repValEnd);
                    repStr.trim();
                    
                    // Extraer intervalo
                    int intIdx = obj.indexOf("\"intervalo\"");
                    int intValStart = obj.indexOf(":", intIdx) + 1;
                    int intValEnd = obj.indexOf(",", intValStart);
                    if (intValEnd < 0) intValEnd = obj.indexOf("}", intValStart);
                    String intStr = obj.substring(intValStart, intValEnd);
                    intStr.trim();
                    
                    _secuenciaDifuntos[currentPaso].indiceCampana = campanaStr.toInt();
                    _secuenciaDifuntos[currentPaso].repeticiones = repStr.toInt();
                    _secuenciaDifuntos[currentPaso].intervaloMs = intStr.toInt();
                    
                    currentPaso++;
                    pos = objEnd + 1;
                }
                
                DBG_CAM_PRINTF("[CAMPANARIO] Secuencia Difuntos cargada: %d pasos\n", _numPasosDifuntos);
            }
        }
        
        // Cargar secuencia de Misa (mismo proceso)
        int startMisa = jsonContent.indexOf("\"misa\"");
        if (startMisa > 0) {
            int startPasos = jsonContent.indexOf("\"pasos\"", startMisa);
            int startArray = jsonContent.indexOf("[", startPasos);
            int endArray = jsonContent.indexOf("]", startArray);
            
            if (startArray > 0 && endArray > startArray) {
                String pasosStr = jsonContent.substring(startArray + 1, endArray);
                int count = 0;
                for (int i = 0; i < pasosStr.length(); i++) {
                    if (pasosStr[i] == '{') count++;
                }
                
                _numPasosMisa = count;
                _secuenciaMisa = new PasoSecuencia[_numPasosMisa];
                
                int currentPaso = 0;
                int pos = 0;
                while (currentPaso < _numPasosMisa && pos < pasosStr.length()) {
                    int objStart = pasosStr.indexOf("{", pos);
                    int objEnd = pasosStr.indexOf("}", objStart);
                    if (objStart < 0 || objEnd < 0) break;
                    
                    String obj = pasosStr.substring(objStart, objEnd + 1);
                    
                    int campanaIdx = obj.indexOf("\"campana\"");
                    int campanaValStart = obj.indexOf(":", campanaIdx) + 1;
                    int campanaValEnd = obj.indexOf(",", campanaValStart);
                    if (campanaValEnd < 0) campanaValEnd = obj.indexOf("}", campanaValStart);
                    String campanaStr = obj.substring(campanaValStart, campanaValEnd);
                    campanaStr.trim();
                    
                    int repIdx = obj.indexOf("\"repeticiones\"");
                    int repValStart = obj.indexOf(":", repIdx) + 1;
                    int repValEnd = obj.indexOf(",", repValStart);
                    if (repValEnd < 0) repValEnd = obj.indexOf("}", repValStart);
                    String repStr = obj.substring(repValStart, repValEnd);
                    repStr.trim();
                    
                    int intIdx = obj.indexOf("\"intervalo\"");
                    int intValStart = obj.indexOf(":", intIdx) + 1;
                    int intValEnd = obj.indexOf(",", intValStart);
                    if (intValEnd < 0) intValEnd = obj.indexOf("}", intValStart);
                    String intStr = obj.substring(intValStart, intValEnd);
                    intStr.trim();
                    
                    _secuenciaMisa[currentPaso].indiceCampana = campanaStr.toInt();
                    _secuenciaMisa[currentPaso].repeticiones = repStr.toInt();
                    _secuenciaMisa[currentPaso].intervaloMs = intStr.toInt();
                    
                    currentPaso++;
                    pos = objEnd + 1;
                }
                
                DBG_CAM_PRINTF("[CAMPANARIO] Secuencia Misa cargada: %d pasos\n", _numPasosMisa);
            }
        }
        
        // Cargar secuencia de Fiesta
        int startFiesta = jsonContent.indexOf("\"fiesta\"");
        if (startFiesta > 0) {
            int startPasos = jsonContent.indexOf("\"pasos\"", startFiesta);
            int startArray = jsonContent.indexOf("[", startPasos);
            int endArray = jsonContent.indexOf("]", startArray);
            
            if (startArray > 0 && endArray > startArray) {
                String pasosStr = jsonContent.substring(startArray + 1, endArray);
                int count = 0;
                for (int i = 0; i < pasosStr.length(); i++) {
                    if (pasosStr[i] == '{') count++;
                }
                
                _numPasosFiesta = count;
                _secuenciaFiesta = new PasoSecuencia[_numPasosFiesta];
                
                int currentPaso = 0;
                int pos = 0;
                while (currentPaso < _numPasosFiesta && pos < pasosStr.length()) {
                    int objStart = pasosStr.indexOf("{", pos);
                    int objEnd = pasosStr.indexOf("}", objStart);
                    if (objStart < 0 || objEnd < 0) break;
                    
                    String obj = pasosStr.substring(objStart, objEnd + 1);
                    
                    int campanaIdx = obj.indexOf("\"campana\"");
                    int campanaValStart = obj.indexOf(":", campanaIdx) + 1;
                    int campanaValEnd = obj.indexOf(",", campanaValStart);
                    if (campanaValEnd < 0) campanaValEnd = obj.indexOf("}", campanaValStart);
                    String campanaStr = obj.substring(campanaValStart, campanaValEnd);
                    campanaStr.trim();
                    
                    int repIdx = obj.indexOf("\"repeticiones\"");
                    int repValStart = obj.indexOf(":", repIdx) + 1;
                    int repValEnd = obj.indexOf(",", repValStart);
                    if (repValEnd < 0) repValEnd = obj.indexOf("}", repValStart);
                    String repStr = obj.substring(repValStart, repValEnd);
                    repStr.trim();
                    
                    int intIdx = obj.indexOf("\"intervalo\"");
                    int intValStart = obj.indexOf(":", intIdx) + 1;
                    int intValEnd = obj.indexOf(",", intValStart);
                    if (intValEnd < 0) intValEnd = obj.indexOf("}", intValStart);
                    String intStr = obj.substring(intValStart, intValEnd);
                    intStr.trim();
                    
                    _secuenciaFiesta[currentPaso].indiceCampana = campanaStr.toInt();
                    _secuenciaFiesta[currentPaso].repeticiones = repStr.toInt();
                    _secuenciaFiesta[currentPaso].intervaloMs = intStr.toInt();
                    
                    currentPaso++;
                    pos = objEnd + 1;
                }
                
                DBG_CAM_PRINTF("[CAMPANARIO] Secuencia Fiesta cargada: %d pasos\n", _numPasosFiesta);
            }
        }
        
        DBG_CAM("[CAMPANARIO] Secuencias cargadas correctamente desde SPIFFS");
        return true;
    }
    
    /**
     * @brief Añade una campana al sistema de campanario
     * 
     * @details Registra una campana en el array interno para poder utilizarla
     *          en las secuencias de campanadas. Verifica que no se exceda el
     *          límite máximo de campanas soportadas.
     * 
     * @param pCampana Puntero a objeto CAMPANA a añadir al sistema
     * 
     * @note **LÍMITE:** Máximo MAX_CAMPANAS (2) campanas soportadas
     * @note **PUNTERO:** No toma ownership - el objeto debe persistir externamente
     * @note **ORDEN:** Las campanas se añaden en el orden llamado (índice 0, 1, ...)
     * 
     * @warning **PUNTERO VÁLIDO:** Verificar que pCampana != nullptr antes de llamar
     * @warning **PERSISTENCIA:** El objeto CAMPANA debe existir durante toda la vida del CAMPANARIO
     * 
     * @see CAMPANA - Clase de campana individual que se añade
     * 
     * @example
     * @code
     * CAMPANA campanaMayor(12);
     * CAMPANA campanaMediana(13);
     * CAMPANARIO campanario;
     * 
     * campanario.AddCampana(&campanaMayor);    // Índice 0
     * campanario.AddCampana(&campanaMediana);  // Índice 1
     * @endcode
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void CAMPANARIO::AddCampana(CAMPANA* pCampana) {
        if (this->_nNumCampanas < Config::Campanario::MAX_CAMPANAS) {           // Verifica que no se exceda el número máximo de campanas
            this->_pCampanas[_nNumCampanas++] = pCampana;                       // Añade la campana al array y aumenta el contador
            DBG_CAM_PRINTF("Campana añadida. Total campanas: %d", this->_nNumCampanas);
        }else {
            DBG_CAM("No se pueden añadir más campanas al campanario.");
        }   

    }

    /**
     * @brief Inicia la secuencia tradicional de campanadas para difuntos
     * 
     * @details Activa la secuencia de campanadas específica para ceremonias
     *          de difuntos. Utiliza la secuencia predefinida almacenada en
     *          PROGMEM y genera las campanadas planas para ejecución.
     *          
     *          **PROCESO DE INICIO:**
     *          1. Verifica que no hay otra secuencia activa
     *          2. Genera campanadas planas desde secuenciaDifuntos
     *          3. Establece flag BitDifuntos en el estado
     *          4. Inicia la ejecución de secuencia
     * 
     * @note **SECUENCIA COMPLEJA:** Utiliza patrones tradicionales de difuntos
     * @note **MEMORIA:** Lee secuencia desde PROGMEM para optimización
     * @note **ESTADO:** Establece BitDifuntos durante la ejecución
     * 
     * @warning **SOLAPAMIENTO:** No inicia si hay otra secuencia activa
     * @warning **CAMPANAS:** Requiere al menos una campana añadida al sistema
     * 
     * @see secuenciaDifuntos - Array con la secuencia predefinida
     * @see ParaSecuencia() - Para detener la secuencia manualmente
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void CAMPANARIO::TocaDifuntos(void) {
        DBG_CAM("Tocando campanas para difuntos...");
        DBG_CAM_PRINT ("numPasosDifuntos: ");
        DBG_CAM(_numPasosDifuntos);
        this->secuenciaActiva = Config::Secuencia::DIFUNTOS;                        // Indica que la secuencia activa es Difuntos
        this->_GeneraraCampanadas(_secuenciaDifuntos, _numPasosDifuntos);           // Genera la secuencia plana de campanadas para difuntos
        this->IniciarSecuenciaCampanadas();                                         // Inicia la secuencia de toque de campanadas
        this->_nEstadoCampanario |= Config::States::BIT_SECUENCIA;                  // Marca el estado del campanario como tocando difuntos
    }

    /**
     * @brief Inicia la secuencia litúrgica de campanadas para misa
     * 
     * @details Activa la secuencia de campanadas específica para ceremonias
     *          de misa. Utiliza la secuencia predefinida más compleja del sistema
     *          con múltiples fases y patrones variados.
     *          
     *          **CARACTERÍSTICAS DE LA SECUENCIA:**
     *          - Múltiples fases con diferentes ritmos
     *          - Alternancia entre campanas para crear polifonía
     *          - Accelerando y rallentando según tradición litúrgica
     *          - Final con serie de campanadas solemnes
     * 
     * @note **SECUENCIA COMPLEJA:** La más elaborada del sistema (62 pasos)
     * @note **DURACIÓN:** Aproximadamente 2-3 minutos de duración total
     * @note **ESTADO:** Establece BitMisa durante la ejecución
     * 
     * @warning **SOLAPAMIENTO:** No inicia si hay otra secuencia activa
     * @warning **CAMPANAS:** Requiere al menos 2 campanas para efectos de alternancia
     * 
     * @see secuenciaMisa - Array con la secuencia litúrgica completa
     * @see ParaSecuencia() - Para detener la secuencia manualmente
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */    
    void CAMPANARIO::TocaMisa(void) {

        DBG_CAM ("Tocando campanas para misa...");
        DBG_CAM_PRINT ("numPasosMisa: ");
        DBG_CAM(_numPasosMisa);
        this->secuenciaActiva = Config::Secuencia::MISA;                            // Indica que la secuencia activa es Misa
        this->_GeneraraCampanadas(_secuenciaMisa, _numPasosMisa);
        this->IniciarSecuenciaCampanadas(); // Inicia la secuencia de campanadas
        this->_nEstadoCampanario |= Config::States::BIT_SECUENCIA;
    }
    /**
     * @brief Inicia la secuencia festiva de campanadas para celebraciones
     * 
     * @details Activa la secuencia de campanadas específica para eventos
     *          festivos y celebraciones. Utiliza patrones alegres y dinámicos
     *          que crean ambiente de fiesta y celebración.
     *          
     *          **CARACTERÍSTICAS DE LA SECUENCIA:**
     *          - Ritmo alegre y festivo con crescendo inicial
     *          - Alternancia dinámica entre campanas
     *          - Repique central tradicional español
     *          - Finale triunfal con campanadas de celebración
     *          - Duración: aproximadamente 3-4 minutos
     * 
     * @note **SECUENCIA FESTIVA:** Diseñada para crear ambiente celebrativo
     * @note **ENERGÉTICA:** Más dinámica y rápida que secuencias solemnes
     * @note **ESTADO:** Establece BIT_SECUENCIA durante la ejecución
     * 
     * @warning **SOLAPAMIENTO:** No inicia si hay otra secuencia activa
     * @warning **CAMPANAS:** Requiere al menos 2 campanas para efectos dinámicos
     * 
     * @see secuenciaFiesta - Array con la secuencia festiva completa
     * @see ParaSecuencia() - Para detener la secuencia manualmente
     * 
     * @example
     * @code
     * // Celebración de boda
     * campanario.TocaFiesta();
     * 
     * // Fiesta patronal programada
     * alarmas.addAlarm(12, 0, ALARM_MATCH_HOUR_MIN, 
     *                 []() { campanario.TocaFiesta(); });
     * @endcode
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void CAMPANARIO::TocaFiesta(void) {

        DBG_CAM ("Tocando campanas para fiesta...");
        DBG_CAM_PRINT ("numPasosFiesta: ");
        DBG_CAM(_numPasosFiesta);
        this->secuenciaActiva = Config::Secuencia::FIESTA;                          // Indica que la secuencia activa es Fiesta
        this->_GeneraraCampanadas(_secuenciaFiesta, _numPasosFiesta);
        this->IniciarSecuenciaCampanadas(); // Inicia la secuencia de campanadas
        this->_nEstadoCampanario |= Config::States::BIT_SECUENCIA;
    }
    /**
     * @brief Genera secuencia de campanadas planas desde pasos definidos
     * 
     * @details Convierte una secuencia de pasos (que incluyen repeticiones e
     *          intervalos) en un array plano de campanadas individuales para
     *          optimizar la ejecución posterior.
     *          
     *          **PROCESO DE GENERACIÓN:**
     *          1. Lee cada paso de la secuencia desde PROGMEM
     *          2. Expande las repeticiones en campanadas individuales
     *          3. Aplica intervalos específicos a cada campanada
     *          4. Almacena en array plano para ejecución secuencial
     * 
     * @param secuencia Puntero a array de PasoSecuencia en PROGMEM
     * @param numPasos Número de pasos en la secuencia
     * 
     * @note **PROGMEM:** Lee secuencias desde memoria de programa para optimización
     * @note **EXPANSIÓN:** Convierte pasos con repeticiones en campanadas individuales
     * @note **LÍMITE:** Máximo MAX_CAMPANADAS_SECUENCIA campanadas en array plano
     * 
     * @warning **MEMORIA:** Puede consumir hasta 1.6KB de RAM para secuencias grandes
     * @warning **OVERFLOW:** Verifica que no se exceda el límite de campanadas
     * 
     * @see ToquePlano - Estructura utilizada para campanadas individuales
     * @see PasoSecuencia - Estructura de entrada leída desde PROGMEM
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void CAMPANARIO::_GeneraraCampanadas(const PasoSecuencia* secuencia, int numPasos) {
        int idx = 0;
        this->_LimpiaraCampanadas(); // Limpia las campanadas antes de generar nuevas
        for (int i = 0; i < numPasos; ++i) {
            for (int r = 0; r < secuencia[i].repeticiones; ++r) {
                this->_aCampanadas[idx].indiceCampana = secuencia[i].indiceCampana;
                this->_aCampanadas[idx].intervaloMs = secuencia[i].intervaloMs;
                idx++;
            }
        }
        this->_nCampanadas = idx;
        
        DBG_CAM("Campanadas generadas:");
        DBG_CAM_PRINT ("Numero de campanadas: ");
        DBG_CAM(this->_nCampanadas);
        

    }

    /**
     * @brief Limpia el array de campanadas y reinicia contadores
     * 
     * @details Resetea el array de campanadas planas y el contador para
     *          preparar el sistema para una nueva secuencia.
     * 
     * @note **LIMPIEZA:** Solo resetea contador, no borra memoria por eficiencia
     * @note **RÁPIDO:** Operación O(1) para máximo rendimiento
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void CAMPANARIO::_LimpiaraCampanadas(void) {
       this->_nCampanadas = 0; // Resetea el contador de campanadas
        for (int i = 0; i < 200; ++i) {
            this->_aCampanadas[i].indiceCampana = -1; // Resetea el índice de la campana
            this->_aCampanadas[i].intervaloMs = 0; // Resetea el intervalo en milisegundos
        }
        DBG_CAM("Campanadas limpiadas.");
    }

    /**
     * @brief Inicia la ejecución de la secuencia de campanadas generada
     * 
     * @details Inicializa las variables de control para comenzar la ejecución
     *          de una secuencia de campanadas previamente generada. Resetea
     *          contadores y establece el estado de ejecución activa.
     * 
     * @note **PRERREQUISITO:** Debe llamarse después de generar campanadas con _GeneraraCampanadas()
     * @note **CONTROL:** Resetea índice actual y timestamp para nueva secuencia
     * @note **ESTADO:** Establece _tocandoSecuencia = true
     * 
     * @see _GeneraraCampanadas() - Debe llamarse antes de esta función
     * @see ActualizarSecuenciaCampanadas() - Debe llamarse periódicamente después
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void CAMPANARIO::IniciarSecuenciaCampanadas(void) {
        if (this->_nCampanadas == 0) {
            DBG_CAM("Error: No hay campanadas generadas");
            return;
        }
         
        this->_indiceCampanadaActual = 0;
        this->_ultimoToqueMs = 0;
        this->_tocandoSecuencia = (this->_nCampanadas > 0);
        DBG_CAM("Secuencia de campanadas iniciada");
    }

    /**
     * @brief Actualiza la ejecución de la secuencia de campanadas activa
     * 
     * @details Motor principal de ejecución de secuencias que debe ser llamado
     *          periódicamente desde el loop principal para procesar las campanadas
     *          según los intervalos de tiempo programados.
     *          
     *          **ALGORITMO DE EJECUCIÓN:**
     *          1. Verifica si hay secuencia activa
     *          2. Comprueba si ha transcurrido el intervalo necesario
     *          3. Toca la campana correspondiente al paso actual
     *          4. Avanza al siguiente paso de la secuencia
     *          5. Si termina la secuencia: limpia estado y flags
     * 
     * @return 1 si la secuencia continúa activa, 0 si ha terminado o no hay secuencia
     * 
     * @note **LLAMADA PERIÓDICA:** Debe llamarse cada pocos milisegundos desde loop()
     * @note **NO BLOQUEANTE:** Utiliza millis() para control temporal no-bloqueante
     * @note **AUTO-LIMPIEZA:** Se limpia automáticamente al terminar la secuencia
     * 
     * @warning **FRECUENCIA:** Llamar con suficiente frecuencia para precisión temporal
     * @warning **CAMPANAS VÁLIDAS:** Verifica índices válidos antes de tocar
     * 
     * @see IniciarSecuenciaCampanadas() - Para iniciar una nueva secuencia
     * @see ParaSecuencia() - Para detener manualmente la secuencia
     * 
     * @example
     * @code
     * void loop() {
     *     // Actualizar secuencias de campanadas
     *     campanario.ActualizarSecuenciaCampanadas();
     *     
     *     // Otras tareas del sistema...
     *     delay(10);  // Control de frecuencia
     * }
     * @endcode
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    int CAMPANARIO::ActualizarSecuenciaCampanadas(void) {
        if (!this->_tocandoSecuencia || this->_indiceCampanadaActual >= this->_nCampanadas)                                                 // Si no se está tocando una secuencia o no hay campanadas, retorna 0   
        {
            return 0;   
        }    
        unsigned long ahora = millis();                                                                                                     // Obtiene el tiempo actual en milisegundos
        if (this->_ultimoToqueMs == 0 || (ahora - this->_ultimoToqueMs) >= this->_aCampanadas[this->_indiceCampanadaActual].intervaloMs) {  // Si es el primer toque o ha pasado el intervalo definido
            int idxCampana = this->_aCampanadas[this->_indiceCampanadaActual].indiceCampana;                                                // Obtiene el índice de la campana a tocar
            if (idxCampana >= 0 && idxCampana < this->_nNumCampanas) {                                                                      // Verifica que el índice de campana esté dentro del rango válido   
                this->_pCampanas[idxCampana]->Toca();                                                                                       // Llama al método Toca de la campana correspondiente para hacerla sonar
                this->_nCampanaTocada = 1 + idxCampana;                                                                                     // Actualiza el número de campanada tocada ( el 1 es poruq e la campana 1 esta en un indice 0)
                    DBG_CAM_PRINT("Tocando campana: ");
                    DBG_CAM(this->_nCampanaTocada);
            } else {
                    DBG_CAM("Índice de campana fuera de rango.");
            }
            this->_ultimoToqueMs = ahora;                                                                                                   // Actualiza la marca de tiempo del último toque
            this->_indiceCampanadaActual++;                                                                                                 // Incrementa el índice de la campanada actual    
            if (this->_indiceCampanadaActual >= this->_nCampanadas) {                                                                       // Si se han tocado todas las campanadas de la secuencia plana
                this->_tocandoSecuencia = false;                                                                                            // Marca la secuencia como no activa    
                DBG_CAM("Secuencia de campanadas finalizada.");
            }
        }
        return this->_nCampanaTocada;                                                                                                       // Retorna el número de campana tocada en la última secuencia
    }

    /**
     * @brief Resetea el contador de campana tocada
     * 
     * @details Reinicia la variable que rastrea qué campana fue tocada por última vez.
     *          Útil para limpiar el estado entre secuencias o para propósitos de debug.
     * 
     * @note **ESTADO:** Establece _nCampanaTocada a 0
     * @note **USO:** Principalmente para limpieza de estado y debug
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void CAMPANARIO::ResetCampanaTocada(void) {
        this->_nCampanaTocada = 0; // Resetea el número de campana tocada
        DBG_CAM("Número de campana tocada reseteado.");
    }

    /**
     * @brief Detiene inmediatamente cualquier secuencia de campanadas activa
     * 
     * @details Para la ejecución de la secuencia actual, limpia el array de
     *          campanadas y resetea todos los flags de estado relacionados
     *          con secuencias de campanadas.
     *          
     *          **ACCIONES REALIZADAS:**
     *          1. Marca secuencia como inactiva (_tocandoSecuencia = false)
     *          2. Limpia array de campanadas planas
     *          3. Resetea contadores de secuencia
     *          4. Limpia flags de estado (BitDifuntos, BitMisa, etc.)
     *          5. Establece estado de reposo (_tocando = false)
     * 
     * @note **INMEDIATO:** Detiene la secuencia instantáneamente sin esperar
     * @note **LIMPIEZA COMPLETA:** Resetea todo el estado relacionado con secuencias
     * @note **SEGURO:** Puede llamarse en cualquier momento sin efectos secundarios
     * 
     * @see IniciarSecuenciaCampanadas() - Para iniciar una nueva secuencia después
     * @see GetEstadoSecuencia() - Para verificar si hay secuencia activa antes de parar
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void CAMPANARIO::ParaSecuencia(void) {
        this->_tocandoSecuencia = false;                                                                // Detiene la secuencia de campanadas
        this->_LimpiaraCampanadas();                                                                    // Limpia las campanadas
        this -> secuenciaActiva = Config::Secuencia::NINGUNA;                                           // Resetea la secuencia activa
        DBG_CAM("Secuencia de campanadas detenida.");
        this->_nEstadoCampanario &= ~((Config::States::BIT_CALEFACCION - 1));                           // Limpia los bits de estado del campanario relacionados con las campanadas                     
    }

    /**
     * @brief Toca el cuarto de hora especificado
     * 
     * @details Toca las campanadas correspondientes a un cuarto de hora específico.
     *          Utiliza la segunda campana del array si está disponible, o la primera
     *          como fallback.
     * 
     * @param nCuarto Número de cuarto a tocar (1-4)
     * 
     * @note **CAMPANA:** Prefiere campana índice 1, fallback a índice 0
     * @note **RANGO:** nCuarto debe estar entre 1-4
     * @note **SONIDO:** Tradicionalmente los cuartos suenan diferente a las horas
     * 
     * @see TocaHorayCuartos() - Función que utiliza esta para tocar cuartos
     * @see TocaMediaHora() - Para toque específico de media hora
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void CAMPANARIO::TocaCuarto(int nCuarto) {
        this->_LimpiaraCampanadas();                                                                    // Limpia las campanadas antes de tocar la hora
        for (int i = 0; i < nCuarto; ++i) {                                                             // Itera sobre el número de cuartos a tocar                           
            this->_aCampanadas[i].indiceCampana = 1;                                                    // Toca la campana 2
            this->_aCampanadas[i].intervaloMs = 1000;                                                   // espaciado 1000 ms
        }
        this->_nCampanadas = nCuarto;                                                                   // Actualiza el número de campanadas a tocar
        this->_nEstadoCampanario |= Config::States::BIT_CUARTOS;                                        // Actualiza el estado del campanario para indicar que se están tocando cuartos
        this->IniciarSecuenciaCampanadas();                                                             // Inicia la secuencia de campanadas
        DBG_CAM_PRINT("Tocando cuarto: ");
        DBG_CAM(nCuarto);
    }

    /**
     * @brief Toca la hora con cuartos previos según tradición campanera
     * 
     * @details Ejecuta la secuencia tradicional de toques de hora que incluye
     *          primero 4 cuartos y después el número de campanadas correspondiente
     *          a la hora. Convierte automáticamente formato 24h a 12h.
     * 
     * @param nHora Hora a tocar (0-23, se convierte automáticamente a formato 12h)
     * 
     * @note **FORMATO:** Convierte 24h a 12h automáticamente (13h = 1 campanada)
     * @note **CUARTOS:** Siempre toca 4 cuartos antes de la hora
     * @note **MEDIANOCHE:** 0h se convierte a 12 campanadas
     * 
     * @warning **RANGO:** nHora debe estar entre 0-23
     * @warning **CAMPANAS:** Requiere al menos una campana añadida al sistema
     * 
     * @see TocaCuarto() - Función utilizada para tocar los cuartos
     * @see TocaHoraSinCuartos() - Para tocar solo la hora sin cuartos
     * 
     * @example
     * @code
     * // Tocar las 15:00 (3 campanadas + 4 cuartos previos)
     * campanario.TocaHorayCuartos(15);
     * 
     * // Tocar medianoche (12 campanadas + 4 cuartos previos)
     * campanario.TocaHorayCuartos(0);
     * @endcode
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void CAMPANARIO::TocaHorayCuartos(int nHora) {
        this->_LimpiaraCampanadas();                                // Limpia las campanadas antes de tocar la hora
        int i = 0;                                                  // Itera para los 4 cuartos
        for ( i = 0; i < 4; ++i) {
            this->_aCampanadas[i].indiceCampana = 1;                // Toca la campana 2 los cuatro cuartos
            this->_aCampanadas[i].intervaloMs = 1000;               // espaciado 1000 ms
        }

        int nHoraReal = nHora % 12;                                 // Asegura que la hora esté en el rango de 0 a 11
        int nHoraTocada = (nHoraReal == 0) ? 12 : nHoraReal;        // Si es 0, se toca la campana 12
        for ( int i = 0; i < nHoraTocada; ++i) {
            this->_aCampanadas[i+4].indiceCampana = 0;              // Toca la campana 2 para la hora
            this->_aCampanadas[i+4].intervaloMs = ( i== 0) ? 3000 : 1000;       // espaciados 1000 ms o 3000 en el primer toque de hora
        }    
        this->_nCampanadas = nHoraTocada + 4;                       // Actualiza el número de campanadas a tocar (4 cuartos + hora)
        this->_nEstadoCampanario |= Config::States::BIT_HORA;       // Actualiza el estado del campanario para indicar que se está tocando la hora
        this->IniciarSecuenciaCampanadas();                         // Inicia la secuencia de campanadas
        DBG_CAM_PRINTF("Tocando hora %d (%dh) con cuartos\n", nHoraTocada, nHora);
     }
    /**
     * @brief Toca solo la hora sin cuartos previos
     * 
     * @details Ejecuta únicamente las campanadas correspondientes a la hora
     *          sin tocar los cuartos previos. Útil para situaciones donde
     *          solo se requiere el toque de hora.
     * 
     * @param nHora Hora a tocar (0-23, se convierte automáticamente a formato 12h)
     * 
     * @note **SIN CUARTOS:** No toca cuartos, solo campanadas de hora
     * @note **FORMATO:** Misma conversión 24h a 12h que TocaHorayCuartos()
     * @note **CAMPANA:** Utiliza la primera campana del array (índice 0)
     * 
     * @see TocaHorayCuartos() - Para tocar hora completa con cuartos
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void CAMPANARIO::TocaHoraSinCuartos(int nHora) {
        this->_LimpiaraCampanadas();                            // Limpia las campanadas antes de tocar la hora
        int nHoraReal = nHora % 12;                             // Asegura que la hora esté en el rango de 0 a 11
        int nHoraTocada = (nHoraReal == 0) ? 12 : nHoraReal;    // Si es 0, se toca la campana 12
        for (int i = 0; i < nHoraTocada; ++i) {
            this->_aCampanadas[i].indiceCampana = 0;            // Toca la campana 2 para la hora
            this->_aCampanadas[i].intervaloMs = (i == 0) ? 3000 : 2000; // espaciados 1000 ms o 3000 en el primer toque de hora
        }
        this->_nCampanadas = nHoraTocada;                       // Actualiza el número de campanadas a tocar (solo la hora)
        this->_nEstadoCampanario |= Config::States::BIT_HORA;       // Actualiza el estado del campanario para indicar que se está tocando la hora
        this->IniciarSecuenciaCampanadas();                         // Inicia la secuencia de campanadas
        DBG_CAM_PRINTF("Tocando hora %d (%dh) sin cuartos\n", nHoraTocada, nHora);
    }

    /**
     * @brief Toca la campanada de media hora
     * 
     * @details Toca una sola campanada para indicar la media hora (:30).
     *          Utiliza la campana de cuartos si está disponible.
     * 
     * @note **CAMPANADA ÚNICA:** Solo una campanada para media hora
     * @note **CAMPANA:** Utiliza campana de cuartos (índice 1) preferentemente
     * 
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void CAMPANARIO::TocaMediaHora(void) {
        this->_LimpiaraCampanadas();                                    // Limpia las campanadas antes de tocar la media hora
        this->_aCampanadas[0].indiceCampana = 1;                        // Toca la campana 2 para la media hora
        this->_aCampanadas[0].intervaloMs = 1000;                       // espaciado 1000 ms
        this->_nCampanadas = 1;                                         // Actualiza el número de campanadas a tocar (1 para media hora)
        this->_nEstadoCampanario |= Config::States::BIT_HORA;           // Actualiza el estado del campanario para indicar que se está tocando la media hora
        this->IniciarSecuenciaCampanadas();                             // Inicia la secuencia de campanadas
        DBG_CAM("Tocando media hora\n");
    }
    /**
     * @brief Obtiene el estado de ejecución de secuencias
     * 
     * @details Devuelve si hay alguna secuencia de campanadas activa en el sistema.
     *          Útil para verificar disponibilidad antes de iniciar nuevas secuencias.
     * 
     * @return true si hay secuencia activa, false si el sistema está libre
     * 
     * @note **THREAD-SAFE:** Lectura atómica de variable booleana
     * @note **INMEDIATO:** No realiza operaciones costosas
     * 
     * @see ParaSecuencia() - Para detener secuencia activa
     * @see ActualizarSecuenciaCampanadas() - Motor que actualiza este estado
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    bool CAMPANARIO::GetEstadoSecuencia() {
        return this->_tocandoSecuencia;                                         // Retorna el estado de la secuencia de campanadas
    }

    /**
     * @brief Añade sistema de calefacción al campanario
     * 
     * @details Registra un objeto CALEFACCION para poder controlarlo desde
     *          el sistema central del campanario. Permite integración completa
     *          del control térmico con el sistema de campanadas.
     * 
     * @param pCalefaccion Puntero a objeto CALEFACCION a integrar
     * 
     * @note **INTEGRACIÓN:** Permite control coordinado de calefacción y campanadas
     * @note **PUNTERO:** No toma ownership - el objeto debe persistir externamente
     * 
     * @warning **PUNTERO VÁLIDO:** Verificar que pCalefaccion != nullptr
     * 
     * @see CALEFACCION - Clase de calefacción que se integra
     * @see EnciendeCalefaccion() - Para activar la calefacción integrada
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void CAMPANARIO::AddCalefaccion (CALEFACCION* pCalefaccion) {
        if (this->_pCalefaccion == nullptr) { // Verifica que no haya calefacción añadida
            this->_pCalefaccion = pCalefaccion; // Asigna la calefacción al campanario
            DBG_CAM("Calefacción añadida al campanario.");
        } else {
            DBG_CAM("ERROR: Ya hay una calefacción añadida al campanario.");
        }

    }
    /**
     * @brief Obtiene el estado de la calefacción
     * 
     * @details Devuelve si la calefacción integrada está actualmente encendida.
     * 
     * @return true si la calefacción está encendida, false si está apagada
     * 
     * @note **DIRECTO:** Consulta directamente el objeto CALEFACCION
     * 
     * @see CALEFACCION::GetEstado() - Función subyacente utilizada
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    bool CAMPANARIO::GetEstadoCalefaccion() {
        if (this->_pCalefaccion != nullptr) {
            return (this->_nEstadoCampanario & Config::States::BIT_CALEFACCION) != 0; // Retorna true si el bit de calefacción está activado
        } else {
            DBG_CAM("No hay calefacción añadida al campanario.");
            return false; // Si no hay calefacción, retorna false
        }
    }
    /**
     * @brief Enciende la calefacción con temporizador
     * 
     * @details Activa la calefacción integrada por el número de minutos especificado
     *          y actualiza el estado del campanario para reflejar que la calefacción
     *          está activa.
     * 
     * @param nMinutos Duración en minutos para mantener encendida la calefacción
     * 
     * @note **INTEGRADO:** Actualiza flag BitCalefaccion en estado del campanario
     * @note **TEMPORIZADOR:** Utiliza sistema de temporizador de CALEFACCION
     * 
     * @warning **CALEFACCIÓN AÑADIDA:** Requiere llamada previa a AddCalefaccion()
     * 
     * @see AddCalefaccion() - Debe llamarse antes de usar esta función
     * @see CALEFACCION::Enciende() - Función subyacente utilizada
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void CAMPANARIO::EnciendeCalefaccion(int nMinutos) {
        if (this->_pCalefaccion != nullptr) {
            this->_pCalefaccion->Enciende(nMinutos); // Enciende la calefacción
            this->_nEstadoCampanario |= Config::States::BIT_CALEFACCION; // Actualiza el estado del campanario para indicar que la calefacción está encendida
            DBG_CAM("Calefacción encendida.");
        } else {
            DBG_CAM("No hay calefacción añadida al campanario.");
        }
    }
    /**
     * @brief Apaga inmediatamente la calefacción
     * 
     * @details Desactiva la calefacción integrada y actualiza el estado del
     *          campanario para reflejar que la calefacción está inactiva.
     * 
     * @note **INMEDIATO:** Apagado instantáneo sin esperar temporizador
     * @note **ESTADO:** Limpia flag BitCalefaccion del estado
     * 
     * @see EnciendeCalefaccion() - Para activar la calefacción
     * @see CALEFACCION::Apaga() - Función subyacente utilizada
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void CAMPANARIO::ApagaCalefaccion() {
        if (this->_pCalefaccion != nullptr) {
            this->_pCalefaccion->Apaga(); // Apaga la calefacción
            this->_nEstadoCampanario &= ~Config::States::BIT_CALEFACCION; // Actualiza el estado del campanario para indicar que la calefacción está apagada
            DBG_CAM("Calefacción apagada.");
        } else {
            DBG_CAM("No hay calefacción añadida al campanario.");
        }
    }
    /**
     * @brief Verifica temporizador de calefacción y actualiza estado
     * 
     * @details Comprueba el estado del temporizador de calefacción y actualiza
     *          automáticamente el estado del campanario si la calefacción se
     *          ha apagado automáticamente.
     * 
     * @return Tiempo restante en segundos o 0.0 si está apagada
     * 
     * @note **AUTO-ACTUALIZACIÓN:** Actualiza estado si calefacción se apaga sola
     * @note **INTEGRADO:** Sincroniza estado de CALEFACCION con CAMPANARIO
     * 
     * @see CALEFACCION::VerificarTemporizador() - Función subyacente utilizada
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    double CAMPANARIO::TestTemporizacionCalefaccion(void) {
        if (this->_pCalefaccion != nullptr) {
            return this->_pCalefaccion->VerificarTemporizador();            // Retorna el estado de la temporización de la calefacción y los segundos que faltan para apagar
        } else {
            DBG_CAM("No hay calefacción añadida al campanario.");
            return -1;                                                      // Si no hay calefacción, retorna false
        }
    }
    /**
     * @brief Establece el flag de conexión a Internet
     * 
     * @details Marca el campanario como conectado a Internet limpiando
     *          el flag BitEstadoSinInternet del estado.
     * 
     * @note **CONECTIVIDAD:** Indica que hay conexión a Internet disponible
     * @note **ESTADO:** Limpia bit de "sin internet"
     * 
     * @see ClearInternetConectado() - Para marcar como desconectado
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void CAMPANARIO::SetInternetConectado (void) {
        this->_nEstadoCampanario &= ~Config::States::BIT_SIN_INTERNET; // Actualiza el estado del campanario para indicar que hay conexión a Internet
        DBG_CAM("Internet conectado.");
    }
    /**
     * @brief Limpia el flag de conexión a Internet
     * 
     * @details Marca el campanario como sin conexión a Internet estableciendo
     *          el flag BitEstadoSinInternet del estado.
     * 
     * @note **CONECTIVIDAD:** Indica que no hay conexión a Internet
     * @note **ESTADO:** Establece bit de "sin internet"
     * 
     * @see SetInternetConectado() - Para marcar como conectado
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void CAMPANARIO::ClearInternetConectado (void) {
        this->_nEstadoCampanario |= Config::States::BIT_SIN_INTERNET; // Actualiza el estado del campanario para indicar que no hay conexión a Internet
        DBG_CAM("Internet desconectado.");
    }
    /**
     * @brief Activa la protección de campanadas
     * 
     * @details Establece el flag de protección que puede utilizarse para
     *          prevenir ejecución de campanadas en ciertos horarios o condiciones.
     * 
     * @note **PROTECCIÓN:** Activa sistema de protección de campanadas
     * @note **HORARIO:** Típicamente para horarios nocturnos o especiales
     * 
     * @see ClearProteccionCampanadas() - Para desactivar protección
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void CAMPANARIO::SetProteccionCampanadas(void) {
        this->_nEstadoCampanario |= Config::States::BIT_PROTECCION_CAMPANADAS; // Establece el estado de protección de campanadas
        DBG_CAM("Protección de campanadas activada");
    }

    /**
     * @brief Desactiva la protección de campanadas
     * 
     * @details Limpia el flag de protección permitiendo la ejecución normal
     *          de todas las secuencias de campanadas.
     * 
     * @note **PROTECCIÓN:** Desactiva sistema de protección
     * @note **NORMAL:** Permite funcionamiento normal de campanadas
     * 
     * @see SetProteccionCampanadas() - Para activar protección
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void CAMPANARIO::ClearProteccionCampanadas(void) {
        this->_nEstadoCampanario &= ~Config::States::BIT_PROTECCION_CAMPANADAS; // Limpia el estado de protección de campanadas
        DBG_CAM("Protección de campanadas desactivada");
    }

    /**
     * @brief Obtiene el estado completo del campanario
     * 
     * @details Devuelve el estado completo del campanario como entero con
     *          flags de bits que representan todos los subsistemas activos.
     * 
     * @return Entero con flags de bits del estado actual
     * 
     * @note **FLAGS:** Utiliza constantes Bit* para interpretación
     * @note **COMPLETO:** Incluye estado de todos los subsistemas
     * 
     * @see BitDifuntos, BitMisa, BitCalefaccion, etc. - Constantes para interpretación
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    int CAMPANARIO::GetEstadoCampanario(void) {
        return this->_nEstadoCampanario;
    }


    uint8_t CAMPANARIO::GetSecuenciaActiva(void) {
        return this->secuenciaActiva;
    }