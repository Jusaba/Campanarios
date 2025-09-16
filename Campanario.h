/**
 * @file Campanario.h
 * @brief Implementación del sistema central de control del campanario automatizado
 * 
 * @details Este archivo implementa la clase CAMPANARIO, que es el núcleo central
 *          del sistema de campanario automatizado. Coordina y gestiona todas las
 *          funcionalidades del campanario incluyendo:
 *          
 *          **FUNCIONALIDADES PRINCIPALES:**
 *          - Gestión de múltiples campanas con control individual
 *          - Ejecución de secuencias complejas (Difuntos, Misa, Horas)
 *          - Sistema de estados con flags de bits para monitorización
 *          - Integración y control del sistema de calefacción
 *          - Protección anti-solapamiento de secuencias
 *          - Control temporal preciso con intervalos configurables
 *          
 *          **SECUENCIAS IMPLEMENTADAS:**
 *          - Difuntos: Secuencia tradicional con patrones específicos
 *          - Misa: Secuencia litúrgica compleja con múltiples fases
 *          - Horas: Toques de hora con cuartos opcionales
 *          - Cuartos: Toques de cuarto de hora independientes
 *          
 *          **SISTEMA DE ESTADOS:**
 *          - BitDifuntos: Secuencia de difuntos activa
 *          - BitMisa: Secuencia de misa activa  
 *          - BitHora: Secuencia de horas activa
 *          - BitCuartos: Secuencia de cuartos activa
 *          - BitCalefaccion: Sistema de calefacción activo
 *          - BitEstadoSinInternet: Sin conexión a internet
 *          - BitEstadoProteccionCampanadas: Protección activa
 *          
 *          **ARQUITECTURA MODULAR:**
 *          - Separación clara entre lógica de secuencias y ejecución
 *          - Sistema de generación de campanadas planas para optimización
 *          - Control independiente de cada subsistema
 *          - Debug modular activable por compilación
 * 
 * @note **MEMORIA:** Secuencias almacenadas en PROGMEM para optimización
 * @note **TIMING:** Utiliza millis() para control temporal no-bloqueante
 * @note **ESCALABILIDAD:** Diseñado para hasta MAX_CAMPANAS campanas
 * 
 * @warning **MEMORIA RAM:** Array de campanadas consume ~1.6KB (200 elementos)
 * @warning **CONCURRENCIA:** No thread-safe - usar desde un solo hilo
 * @warning **PUNTEROS:** Verificar validez de punteros antes de asignar
 * 
 * @author Julian Salas Bartolomé
 * @date 2025-09-16
 * @version 1.0
 * 
 * @see Campanario.h - Definiciones de la clase y constantes
 * @see Campana.h - Control de campanas individuales utilizadas
 * @see Calefaccion.h - Sistema de calefacción integrado
 * @see Alarmas.h - Sistema que programa las secuencias automáticas
 * 
 */
#ifndef CAMPANARIO_H
	#define CAMPANARIO_H

    #include  "CAMPANA.h"
    #include "Calefaccion.h"
    #include "Debug.h"
    #include "Configuracion.h"


    

    struct PasoSecuencia {                                      // Estructura que representa un paso en una secuencia de campanadas
        int indiceCampana;                                      //!< Índice de la campana a tocar (0 para la primera, 1 para la segunda, etc.)    
        int repeticiones;                                       //!< Número de veces que se repite el toque de la campana en este paso
        int intervaloMs;                                        //!< Intervalo en milisegundos entre toques
    };

    struct ToquePlano {                                         // Estructura que representa un toque de campana en una secuencia
        int indiceCampana;                                      //!< Índice de la campana a tocar
        int intervaloMs;                                        //!< Intervalo en milisegundos entre toques
    };

   const PasoSecuencia secuenciaDifuntos[] PROGMEM = {
        {1, 3, 2500},
        {1, 1, 2500},
        {0, 1, 2300},
        {1, 2, 2200},
        {1, 1, 1800},
        {0, 1, 2700},
        {1, 2, 2700},
        {0, 1, 2900},
        {1, 2, 2500},
        {1, 1, 2000},
        {0, 1, 2000},
        {1, 3, 2500},
        {1, 1, 2500},
        {0, 1, 2300},
        {1, 2, 2200},
        {1, 1, 1800},
        {0, 1, 2700},
        {1, 2, 2700},
        {0, 1, 2900},
        {1, 2, 2500},
        {1, 1, 2000},
        {0, 1, 2000}
    };
    const int numPasosDifuntos = sizeof(secuenciaDifuntos) / sizeof(PasoSecuencia);
    const PasoSecuencia secuenciaMisa[] PROGMEM = {
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 600},
        {1, 1, 600},
        {0, 1, 600},
        {1, 1, 600},
        {0, 1, 600},
        {1, 1, 600},
        {0, 1, 600},
        {1, 1, 600},
        {0, 1, 600},
        {1, 1, 600},
        {0, 1, 600},
        {1, 1, 600},
        {0, 1, 500},
        {1, 1, 500},
        {0, 1, 480},
        {1, 1, 480},
        {0, 1, 480},
        {1, 1, 480},
        {0, 1, 480},
        {1, 1, 480},
        {0, 1, 480},
        {1, 1, 600}, 
        {0, 2, 1500},
        {0, 1, 2000},
        {1, 1, 2000},
        {0, 1, 900},
        {1, 1, 600},
        {0, 1, 500},
        {1, 1, 500},
        {0, 1, 500},
        {1, 1, 500},
        {0, 1, 500},
        {1, 1, 500},
        {0, 1, 500},
        {1, 1, 500},
        {0, 1, 500},
        {1, 1, 500},
        {0, 1, 500},
        {1, 1, 500},
        {0, 1, 500},
        {1, 1, 500},
        {0, 1, 500},
        {1, 1, 500},
        {0, 1, 500},
        {1, 1, 500},
        {0, 1, 500},
        {1, 1, 500}, 
        {0, 7, 1200},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {0, 1, 1000},
        {1, 1, 1000},
        {1, 1, 2000} 
    };    
    const int numPasosMisa = sizeof(secuenciaMisa) / sizeof(PasoSecuencia);

    class CAMPANARIO 
    {
        public:

            CAMPANARIO();                                               //!< Constructor con pin 
            ~CAMPANARIO();
            void AddCampana (CAMPANA* pCampana);                        //!< Añade una campana al campanario
            void TocaDifuntos (void);                                   //!< Toca la secuencia de campanadas para difuntos   
            void TocaMisa (void);                                       //!< Toca la secuencia de campanadas para misa   
            void IniciarSecuenciaCampanadas(void);                      //!< Inicia la secuencia de campanadas
            int ActualizarSecuenciaCampanadas(void);                    //!< Actualiza la secuencia de campanadas, tocando las campanas según el intervalo definido
            void ResetCampanaTocada(void);                              //!< Resetea el número de campana tocada
            void ParaSecuencia(void);                                   //!< Detiene la secuencia de campanadas
            void TocaHorayCuartos(int nHora);                           //!< Toca la campana 4 cuartos y la hora
            void TocaHoraSinCuartos(int nHora);                         //!< Toca la campana de la hora sin cuartos
            void TocaCuarto(int nCuarto);                               //!< Toca la campana del cuarto correspondiente
            void TocaMediaHora(void);                                   //!< Toca la campana de la media hora
            bool GetEstadoSecuencia (void);                             //!< Devuelve el estado de la secuencia de campanadas (true si está activa, false si no lo está)
            void AddCalefaccion(CALEFACCION* pCalefaccion);             //!< Añade la calefacción al campanario
            void EnciendeCalefaccion(int nMinutos);                     //!< Enciende la calefacción del campanario
            void ApagaCalefaccion(void);                                //!< Apaga la calefacción del campanario
            double TestTemporizacionCalefaccion(void);                  //!< Testea la temporización de la calefacción del campanario
            bool GetEstadoCalefaccion(void);                            //!< Devuelve el estado de la calefacción del campanario (true si está encendida, false si está apagada)
            int GetEstadoCampanario(void);                              //!< Devuelve el estado del campanario
            void SetInternetConectado(void);                            //!< Establece el estado de conexión a Internet del campanario
            void ClearInternetConectado(void);                          //!< Limpia el estado de conexión a Internet del campanario
            void SetProteccionCampanadas(void);                         //!< Establece el estado de protección de campanadas
            void ClearProteccionCampanadas(void);                       //!< Limpia el estado de protección de campanadas
        
        private:

            CAMPANA* _pCampanas[Config::Campanario::MAX_CAMPANAS];      //!< Array de punteros a las campanas del campanario
            int _nNumCampanas = 0;                                      //!< Número de campanas en el campanario
            
            ToquePlano _aCampanadas[200];                               //!< Array de campanadas a tocar en formato plano, un elemento por campanada, con un máximo de 200 elementos
            int _nCampanadas = 0;                                       //!< Número de campanadas a tocar en la secuencia actual   
            void _GeneraraCampanadas(const PasoSecuencia* secuencia, int numPasos); // Genera una secuencia de campanadas planas a partir de una secuencia de pasos definida
            void _LimpiaraCampanadas(void) ;                            //!< Limpia el array de campanadas y reinicia el contador.    
    
            int _indiceCampanadaActual = 0;                             //!< Índice de la campanada actual en la secuencia
            unsigned long _ultimoToqueMs = 0;                           //!< Marca de tiempo del último toque de campana en milisegundos
            bool _tocandoSecuencia = false;                             //!< Indica si se está tocando una secuencia de campanadas

            int _nCampanaTocada = 0;                                    //!< Número de campana tocada en la última secuencia
            bool _lCalefaccion = false;                                 //!< Estado de la calefacción del campanario
            
            CALEFACCION* _pCalefaccion = nullptr;                       //!< Puntero a la calefacción del campanario

            int _nEstadoCampanario = 0;                                 //!< Estado del campanario, representado por un entero donde cada bit indica un estado específico

            bool _tocando = false;                                      //!< Flag que indica si las campanas estan en reposo o tocando (1/0)
            bool _Calefaccion = false;                                  //!< Flag que indica si la calefacción está encendida o apagada (1/0)

        };
#endif


    