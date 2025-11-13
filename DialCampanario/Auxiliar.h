
/**
 * @file Auxiliar.h
 * @brief Módulo principal de gestión del sistema DialCampanario
 * 
 * @details Este archivo contiene todas las funciones de alto nivel para la gestión
 *          del M5Dial como interfaz de control del campanario. Incluye:
 *          - Gestión de menús dinámicos según estado del campanario
 *          - Comunicación I2C con ESP32 del campanario
 *          - Control de encoder y botones del M5Dial
 *          - Gestión inteligente de display y modo sleep
 *          - Sistema de temporizador para calefacción
 * 
 * @author Julian Salas Bartolomé
 * @date 2025-06-15
 * @version 1.0
 * 
 * @copyright Copyright (c) 2024
 * 
 * @see Display.h, I2CServicio.h, Configuracion.h
 * 
 * @note Requiere M5Dial y ESP32 con I2C configurado
 * @warning Llamar InicializarVariablesGlobales() antes de usar
 */

#ifndef AUXILIAR_H
    #define AUXILIAR_H


    #include <Wire.h>
    #include "Display.h"
    #include <M5Dial.h>
    #include "I2CServicio.h"
    #include "Configuracion.h"
    #include "Debug.h"

    // ============================================================================
    // VARIABLES
    // ============================================================================
    boolean lComandoRecienEnviado = false;                          // Indica si se acaba de enviar un comando I2C (para evitar solapamientos)
    long nMilisegundoTemporal = 0;                                  // Variable para almacenar el tiempo temporal


    // ============================================================================
    // MENÚS - Definición de los menús disponibles
    // ============================================================================
    const int* menuActual = nullptr; 
    const int menu0[] = {Config::States::I2CState::DIFUNTOS, Config::States::I2CState::MISA, Config::States::I2CState::FIESTA, Config::States::I2CState::CALEFACCION_ON};
    #define ItemsMenu0  4
    const int menu1[] = {Config::States::I2CState::DIFUNTOS, Config::States::I2CState::MISA, Config::States::I2CState::FIESTA, Config::States::I2CState::CALEFACCION_OFF};
    #define ItemsMenu1  4
    const int menu2[] = { Config::States::I2CState::STOP, Config::States::I2CState::CALEFACCION_ON};
    #define ItemsMenu2  2
    const int menu3[] = { Config::States::I2CState::STOP, Config::States::I2CState::CALEFACCION_OFF};
    #define ItemsMenu3  2
    const int menu4[] = { Config::States::I2CState::CALEFACCION_ON };
    #define ItemsMenu4  1
    const int menu5[] = { Config::States::I2CState::CALEFACCION_OFF };
    #define ItemsMenu5  1

    // ============================================================================
    // ESTRUCTURAS DE DATOS
    // ============================================================================

    struct CampanarioEstado                                     //Estructura para almacenar el estado del campanario
    {
        uint8_t estadoActual;                                   // Estado del campanario
        uint8_t estadoAnterior;                                 // Estado anterior (para detectar cambios) 
        uint8_t nHora;                                          // Hora actual
        uint8_t nMinutos;                                       // Minutos actuales
        uint8_t nSegundos;                                      // Segundos actuales
        uint8_t nDia;                                           // Día actual
        uint8_t nMes;                                           // Mes actual
        uint8_t nAno;                                           // Año actual (2 dígitos)
        bool tieneInternet;                                     // Indica si hay conexión a Internet
    };
    CampanarioEstado campanarioEstado;                          // Estructura para almacenar el estado del campanario

    struct MenuState                                            // Estructura para almacenar el estado del menú
    {
        int indiceMenuActivo;                                   // Índice del menú activo (0-5)
        int posicionSeleccionada;                               // Posición seleccionada en el menú actual
        int numeroDeOpciones;                                   // Número de opciones del menú actual
        const int* opcionesDisponibles;                         // Puntero al array de opciones del menú
        bool hayQueMostrarCambio;                               // Indica si hay que actualizar el display
    };
    MenuState menu;

    struct EncoderState                                         // Estructura para almacenar el estado del encoder
    {
        int posicionAnterior;                                   // Posición anterior del encoder
        int posicionActual;                                     // Posición actual del encoder
        bool hayCambio;                                         // Indica si hubo cambio en el encoder
    };
    EncoderState encoder;    
    
    struct DisplayState                                         // Estructura para almacenar el estado del display
    {
        bool estaEnModoSleep;                                   // Indica si está en modo sleep
        int contadorCiclosSleep;                                // Contador para activar sleep
        long tiempoUltimaActividad;                             // Timestamp de última actividad
    };
    DisplayState display;
    
    struct TemporizadorState                                   // Estructura para almacenar el estado del temporizador de calefacción
    {
        int minutosConfiguracion;                              // Minutos configurados para calefacción
        bool estaConfigurandose;                               // Indica si está en modo configuración
    };
    TemporizadorState temporizador;
    
    // ============================================================================
    // TABLA DE OPCIONES
    // ============================================================================

    struct OpcionConfig                                         // Estructura para definir una opción del menú
    {
        int estado;                                             //Estado correspondiente a la opcion de menu
        void (*funcionMostrar)();                               //Funcion a ejecutar para mostrar la opcion
        bool esComando;                                         // true = envía comando I2C, false = acción especial
    };

    
    const OpcionConfig OPCIONES[] =                             // Tabla con TODAS las opciones 
    {
        {Config::States::I2CState::INICIO,              MensajeInicio,          false},
        {Config::States::I2CState::DIFUNTOS,            MensajeDifuntos,        true},     
        {Config::States::I2CState::MISA,                MensajeMisa,            true},    
        {Config::States::I2CState::FIESTA,              MensajeFiesta,          true},     
        {Config::States::I2CState::STOP,                MensajeStop,            true},     
        {Config::States::I2CState::CALEFACCION_ON,      MensajeCalefaccionOn,   false},    // Acción especial (temporizador)
        {Config::States::I2CState::CALEFACCION_OFF,     MensajeCalefaccionOff,  true},     
        {Config::States::I2CState::SET_TEMPORIZADOR,    nullptr,                false}     // Acción especial
    };

    #define NUM_OPCIONES (sizeof(OPCIONES)/sizeof(OpcionConfig))




    // ============================================================================
    // FUNCIONES DE DISPLAY
    // ============================================================================
    void DespertarDisplay(void);                                    //!< Despierta el display del modo sleep
    void ActualizarDisplaySleep(void);                              //!< Actualiza el display en modo sleep 

    // ============================================================================
    // FUNCIONES DE MENÚ Y NAVEGACIÓN
    // ============================================================================
    void MostrarOpcion(int seleccionado);                           //!< Muestra un mensaje en pantalla según la opción seleccionada
    void AsignaMenu (void);                                         //!< Asigna el menú correspondiente según el estado actual
    void SeleccionaMenu(void);                                      //!< Selecciona el menú según el estado recibido

    // ============================================================================
    // FUNCIONES DE CONTROL DE ENTRADA
    // ============================================================================
    void ManejarBotonA(void);                                       //!< Maneja el evento del botón A del M5Dial
    void ManejarEncoder(void);                                      //!< Maneja el evento del encoder del M5Dial

    // ============================================================================
    // FUNCIONES DE PROCESAMIENTO DE ESTADOS
    // ============================================================================
    void ProcesarAccionEstado(int estadoActual);                    //!< Procesa la acción correspondiente al estado seleccionado

    // ============================================================================
    // FUNCIONES DE TEMPORIZACIÓN DE CALEFACCIÓN
    // ============================================================================
    void SetTemporizacion(void);                                    //!< Inicia la configuración del temporizador de calefacción

    // ============================================================================
    // FUNCIONES DE COMUNICACIÓN I2C
    // ============================================================================
    //ENVIO COMANDOS I2C
    void EnviarEstado(int nEstadoMenu);                             //!< Envía el estado actual del menú a un dispositivo esclavo a través del bus I2C
    void EnviarTemporizacion(void);                                 //!< Envía el estado de la calefacción encendida al esclavo I2C con la temporización
    void EnviarEstadoConParametro(int nEstadoTx, int nParametroTx); //!< Envía un estado y un parametro al dispositivo esclavo a través del bus I2C
    
    //SOLICITUDES I2C
    void SolicitarEstadoCampanario(void);                           //!< Solicita el estado del campanario al esclavo I2C
    void SolicitarEstadoHora(void);                                 //!< Solicita el estado y la hora al esclavo I2C
    void SolicitarEstadoFechaHora(bool incluirTemporizacion = false); //!< Solicita la fecha y hora al esclavo I2C

    // ============================================================================
    // FUNCIÓN DE INICIALIZACIÓN
    // ============================================================================
    void InicializarVariablesGlobales(void);                        //!< Inicializa las variables globales del sistema




    // ============================================================================
    // FUNCIONES DE DISPLAY
    // ============================================================================



    /**
     * @brief Despierta el display del modo sleep y restaura interfaz
     * 
     * @details Desactiva el modo sleep, reinicia contadores de ciclos, restaura
     *          el brillo máximo de la pantalla y actualiza la visualización con
     *          la opción actualmente seleccionada en el menú.
     * 
     * @note Función automática llamada cuando se detecta actividad del usuario
     * @warning Solo debe llamarse cuando display.estaEnModoSleep es true
     * 
     * @see ActualizarDisplaySleep(), BrilloFull(), MostrarOpcion()
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
       void DespertarDisplay(void) {
            DBG_DISPLAY("DespertarDisplay->Despertando display del modo sleep");
            display.estaEnModoSleep = false;                                    // Indicamos que no estamos en modo sleep                          
            display.contadorCiclosSleep = 0;                                    // Reiniciamos el contador de ciclos de sleep
            if (M5.Display.getBrightness() < 100) BrilloFull();                 // Asegurarse de que el brillo esté al máximo
            MostrarOpcion(menu.opcionesDisponibles[menu.posicionSeleccionada]); // Mostramos la opción actual
        }
    /**
     * @brief Gestiona el modo sleep automático del display y muestra información temporal
     * 
     * @details Controla los ciclos de brillo y la transición automática a modo sleep.
     *          En modo activo, muestra fecha/hora normal o información de calefacción
     *          temporizada según el bit 4 del estado del sistema. Gestiona transiciones
     *          de brillo suaves y temporización automática de sleep.
     * 
     * @note Usa bit 4 del campanarioEstado.estadoActual para detectar calefacción temporizada
     * @note Se llama automáticamente desde el loop principal cada 500ms
     * 
     * @see BajaBrillo(), SubeBrillo(), MensajeFechaHora()
     * @see MostrarCalefaccionTemporizada(), Config::Display::CICLOS_PARA_SLEEP
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */        
        void ActualizarDisplaySleep(void) 
        {
            DBG_DISPLAY("ActualizarDisplaySleep->Actualizando display y gestionando sleep");
            if (lBrillo && !display.estaEnModoSleep) {  // Si hay brillo y no estamos en modo sleep                                         // Si hay brillo y no estamos en modo sleep
                display.contadorCiclosSleep++;                                                                                              // Incrementamos el contador de ciclos de sleep
                if (display.contadorCiclosSleep >= Config::Display::CICLOS_PARA_SLEEP) {                                                    // Si se alcanza el límite de ciclos para sleep                     
                    BajaBrillo();                                                                                                           // Bajamos el brillo de la pantalla            
                    display.contadorCiclosSleep = 0;                                                                                        // Reiniciamos el contador de ciclos de sleep
                    display.estaEnModoSleep = true;                                                                                         // Indicamos que estamos en modo sleep
                }
            } else {    
                if (campanarioEstado.estadoActual & Config::States::BIT_CALEFACCION ) {                                                 // Si el bit 4 está activo (calefacción temporizada)                                                                       
                    MostrarCalefaccionTemporizada(campanarioEstado.nHora, campanarioEstado.nMinutos, campanarioEstado.nSegundos, true); // Mostramos la información de calefacción temporizada
                }else{                                                                                                                  // Si el bit 4 no está activo (hora normal)                 
                    MensajeFechaHora(campanarioEstado.nHora, campanarioEstado.nMinutos, campanarioEstado.nSegundos, campanarioEstado.nDia, campanarioEstado.nMes, campanarioEstado.nAno, campanarioEstado.estadoActual & Config::States::BIT_SIN_INTERNET ); // Mostramos la fecha y hora normal
                }
                                                                                                                       // Si no hay brillo o estamos en modo sleep
                if (!lBrillo) {
                    SubeBrillo(40);                                                                                                         // Subimos el brillo de la pantalla                    
                } 

            }
        }
    // ============================================================================
    // FUNCIONES DE MENÚ Y NAVEGACIÓN
    // ============================================================================
    
    /**
     * @brief Muestra mensaje en pantalla según la opción seleccionada usando tabla OPCIONES
     *
     * @details Busca automáticamente en la tabla OPCIONES el estado correspondiente
     *          y ejecuta la función de mostrar asociada. Si la función es nullptr,
     *          maneja casos especiales como el temporizador. Sistema robusto con
     *          advertencias de debug para estados no encontrados.
     *
     * @param seleccionado Valor entero que indica el estado/opción a mostrar
     * 
     * @note Utiliza la tabla OPCIONES[] para mapeo automático estado->función
     * @warning Si el estado no existe en OPCIONES, muestra advertencia de debug
     * 
     * @see OPCIONES[], MensajeTemporizacion(), Config::States::SET_TEMPORIZADOR
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
        void MostrarOpcion(int seleccionado) 
        {
            DBG_DISPLAY_PRINTF("MostrarOpcion->Seleccionado: %d\n", seleccionado);
            // Buscar en tabla automáticamente:
            for (int i = 0; i < NUM_OPCIONES; i++) {                                                                        //Recorremos el array de OPCIONES
                if (OPCIONES[i].estado == seleccionado) {                                                                   //Si el estado coincide
                    if (OPCIONES[i].funcionMostrar != nullptr) {                                                            //Si la función no es nula
                        OPCIONES[i].funcionMostrar();                                                                       // Llamar función automáticamente
                        DBG_MENU_PRINTF("MostrarOpcion->Función encontrada y ejecutada para estado: %d\n", seleccionado);
                    } else {                                                                                                // Si la función es nula, manejar casos especiales
                        if (seleccionado == Config::States::SET_TEMPORIZADOR) {                                             // Caso especial para temporizador
                            MensajeTemporizacion(temporizador.minutosConfiguracion, true);                                  // Mostrar mensaje de temporizador
                            DBG_MENU("MostrarOpcion->Caso especial temporizador ejecutado\n");
                        }
                    }
                    return;
                }
            }
            // Si llegamos aquí, no se encontró el estado en la tabla
            DBG_MENU_PRINTF("MostrarOpcion->ADVERTENCIA: Estado %d no encontrado en tabla OPCIONES\n", seleccionado);
        }        

    /**
     * @brief Asigna el menú correspondiente según índice activo y muestra opción actual
     *
     * @details Selecciona el array de opciones correspondiente según menu.indiceMenuActivo,
     *          actualiza el número de opciones disponibles y muestra la opción seleccionada.
     *          Gestiona automáticamente la salida del modo sleep si es necesario,
     *          restaurando el brillo completo de la pantalla.
     *
     * @note Se llama automáticamente cuando menu.hayQueMostrarCambio es true
     * @note Maneja 6 menús diferentes (menu0-menu5) según el contexto del campanario
     * 
     * @see menu0-menu5[], MostrarOpcion(), BrilloFull()
     * @see menu.indiceMenuActivo, menu.numeroDeOpciones
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void AsignaMenu (void) 
    {
        DBG_MENU_PRINTF("AsignaMenu->Asignando menú %d\n", menu.indiceMenuActivo);
        switch(menu.indiceMenuActivo) {                     
            case 0: menu.opcionesDisponibles = menu0; menu.numeroDeOpciones = ItemsMenu0; break;
            case 1: menu.opcionesDisponibles = menu1; menu.numeroDeOpciones = ItemsMenu1; break;
            case 2: menu.opcionesDisponibles = menu2; menu.numeroDeOpciones = ItemsMenu2; break;
            case 3: menu.opcionesDisponibles = menu3; menu.numeroDeOpciones = ItemsMenu3; break;
            case 4: menu.opcionesDisponibles = menu4; menu.numeroDeOpciones = ItemsMenu4; break;
            case 5: menu.opcionesDisponibles = menu5; menu.numeroDeOpciones = ItemsMenu5; break;
        }

        MostrarOpcion(menu.opcionesDisponibles[menu.posicionSeleccionada]); 

        if (display.estaEnModoSleep) {                      
           display.estaEnModoSleep = false;                 
           if (M5.Display.getBrightness() < 100) {
                BrilloFull();
            } 
        }  
    }

    /**
     * @brief Selecciona el menú activo según el estado actual del campanario
     *
     * @details Implementa la lógica de selección de menús basada en máscaras de bits
     *          del estado del campanario. Utiliza operadores bitwise para evaluar
     *          múltiples condiciones simultáneamente: protección de campanadas,
     *          estados de difuntos/misa y estado de calefacción. Función crítica
     *          para la navegación, ya que determina qué opciones están disponibles.
     *
     * Lógica de selección:
     * - Si protección campanadas activa: menú 4 o 5 (según calefacción)
     * - Si difuntos/misa activos: menú 2 o 3 (según calefacción)
     * - Estado normal: menú 0 o 1 (según calefacción)
     *
     * @note Utiliza campanarioEstado.estadoActual como entrada
     * @warning Modifica directamente menu.indiceMenuActivo. Llamar AsignaMenu() después
     * 
     * @see AsignaMenu(), Config::States::BIT_PROTECCION_CAMPANADAS
     * @see Config::States::BIT_CALEFACCION, Config::States::BIT_SECUENCIA
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void SeleccionaMenu(void)
    {
        DBG_MENU_PRINTF("SeleccionaMenu->Estado seleccionado: %d\n", campanarioEstado.estadoActual);
        bool ProteccionToqueHoras = campanarioEstado.estadoActual & Config::States::BIT_PROTECCION_CAMPANADAS;              // Protección de campanadas activa
        bool calefOn = campanarioEstado.estadoActual & Config::States::BIT_CALEFACCION;                                     // Calefacción encendida

        if (ProteccionToqueHoras) {                                                                                         // Si la protección de campanadas está activa                   
            menu.indiceMenuActivo = calefOn ? 5 : 4;                                                                        // Menú 5 si calefacción ON, menú 4 si OFF             
        } else {                                                                                                            // Si la protección de campanadas NO está activa  
            bool secuencia = campanarioEstado.estadoActual & (Config::States::BIT_SECUENCIA );                           // Una secuencia activa
            menu.indiceMenuActivo = secuencia ? (calefOn ? 3 : 2) : (calefOn ? 1 : 0);                                   // Menú 3 o 2 si secuencia, menú 1 o 0 si normal  
        }
    }

    // ============================================================================
    // FUNCIONES DE CONTROL DE ENTRADA
    // ============================================================================
    /**
     * @brief Maneja la pulsación del botón A del M5Dial con lógica de contexto
     * 
     * @details Si el display está en modo sleep, lo despierta y termina. Si no,
     *          procesa la acción correspondiente al estado actual. Distingue entre
     *          modo normal (navegación de menú) y modo configuración de temporizador.
     *          Función principal de interacción del usuario con el sistema.
     * 
     * @note Función de entrada principal del usuario al sistema
     * @note Gestiona automáticamente el despertar del display si está en sleep
     * 
     * @see DespertarDisplay(), ProcesarAccionEstado()
     * @see temporizador.estaConfigurandose, menu.opcionesDisponibles
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
        void ManejarBotonA(void) {

            int estadoActual;

            DBG_BUTTONS("Botón A presionado");

            
            if (display.estaEnModoSleep) {                                                      //Si M5Dial esta en sueño
                DespertarDisplay();                                                             //Despierta el display
                return;                                                                         //Y salimos de la función
            }

            // Procesar acción según el estado actual
            DBG_BUTTONS_PRINTF("ManejarBotonA->Estado Actual: %d\n", menu.posicionSeleccionada);
            DBG_BUTTONS(menu.opcionesDisponibles[0]);

            if (temporizador.estaConfigurandose) {                                                  // Si no estamos en el estado de temporización, 
                DBG_BUTTONS("ManejarBotonA->Procesando estado de temporización");
                estadoActual = Config::States::I2CState::SET_TEMPORIZADOR;                          // Si estamos en el estado de temporización, mantenemos ese estado
            }else{
                DBG_BUTTONS("ManejarBotonA->Procesando estado normal");
                estadoActual = menu.opcionesDisponibles[menu.posicionSeleccionada];                 // procesamos el estado seleccionado
            }
     
            ProcesarAccionEstado(estadoActual);
        }

    /**
     * @brief Procesa la entrada del encoder rotatorio con modo dual de operación
     * 
     * @details Función dual que maneja dos modos de operación distintos:
     *          
     *          **Modo configuración temporizador:**
     *          - Ajusta minutos entre 1-60 con límites automáticos
     *          - Actualiza display en tiempo real con MensajeTemporizacion()
     *          - No marca cambio de menú (menu.hayQueMostrarCambio = false)
     *          
     *          **Modo navegación normal:**
     *          - Navega por opciones del menú actual con límites dinámicos
     *          - Marca actualización pendiente del display
     *          - Límites basados en menu.numeroDeOpciones
     * 
     * @note Reproduce tono de confirmación en cada cambio (8000Hz, 20ms)
     * @note Actualiza display.tiempoUltimaActividad para control de sleep automático
     * @warning Solo procesa cambios si encoder.posicionActual != encoder.posicionAnterior
     * 
     * @see SetTemporizacion(), MensajeTemporizacion()
     * @see ActualizarDisplaySleep(), menu.numeroDeOpciones
     * @see temporizador.estaConfigurandose, M5Dial.Speaker.tone()
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */        
        void ManejarEncoder(void) {
            DBG_AUXILIAR("ManejarEncoder->Manejando entrada del encoder");
            encoder.posicionActual = M5Dial.Encoder.read();
            if (encoder.posicionActual != encoder.posicionAnterior) {
                if (temporizador.estaConfigurandose) {                  // Manejo del encoder en modo de temporización
                    M5Dial.Speaker.tone(8000, 20);
                    if (encoder.posicionActual > encoder.posicionAnterior) {
                         temporizador.minutosConfiguracion = min(60, temporizador.minutosConfiguracion + 1);
                    } else {
                        temporizador.minutosConfiguracion = max(1, temporizador.minutosConfiguracion - 1);
                    }
                    MensajeTemporizacion(temporizador.minutosConfiguracion, false);
                    menu.hayQueMostrarCambio = false; 
                //nPosicionAneterior = nPosicionActual;
                }else{                                                          // Manejo del encoder en otros estados
                    if (encoder.posicionActual > encoder.posicionAnterior) {
                         menu.posicionSeleccionada = min(menu.numeroDeOpciones - 1, menu.posicionSeleccionada + 1);
                    } else {
                         menu.posicionSeleccionada = max(0, menu.posicionSeleccionada - 1);
                    }
                    menu.hayQueMostrarCambio = true;
                }
                DBG_ENCODER_PRINTF("ManejarEncoder->Posición Actual: %d, Posición Anterior: %d, Estado: %d, Posición Menú: %d\n", 
                                   encoder.posicionActual, encoder.posicionAnterior, campanarioEstado.estadoActual, menu.posicionSeleccionada);
                encoder.posicionAnterior = encoder.posicionActual;
                display.tiempoUltimaActividad = millis();
            }
        }

    // ============================================================================
    // FUNCIONES DE PROCESAMIENTO DE ESTADOS
    // ============================================================================
    /**
     * @brief Procesa acciones según estado actual usando tabla de configuración OPCIONES
     *
     * @details Sistema automático que busca en la tabla OPCIONES el estado correspondiente
     *          y ejecuta la acción asociada. Distingue entre:
     *          
     *          **Comandos I2C (esComando = true):**
     *          - Envío automático al ESP32 del campanario
     *          - Logging automático de éxito/error
     *          - Marca actualización de display
     *          
     *          **Acciones especiales (esComando = false):**
     *          - INICIO: Sin acción
     *          - CALEFACCION_ON: Inicia configuración temporizador
     *          - SET_TEMPORIZADOR: Confirma y envía temporización
     *
     * @param estadoActual Estado del sistema a procesar
     * 
     * @note Utiliza tabla OPCIONES[] para mapeo automático estado->acción
     * @warning Si el estado no existe en OPCIONES, muestra advertencia y marca cambio
     * 
     * @see OPCIONES[], I2C_EnviarComando(), SetTemporizacion()
     * @see EnviarTemporizacion(), Config::States::I2CState
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
        void ProcesarAccionEstado(int estadoActual) 
        {
            DBG_BUTTONS_PRINTF("ProcesarAccionEstado->Estado Actual: %d\n", estadoActual);
        
            // Buscar en tabla automáticamente:
            for (int i = 0; i < NUM_OPCIONES; i++) {
                if (OPCIONES[i].estado == estadoActual) {
                    if (OPCIONES[i].esComando) {
                        // Comando I2C automático:
                        DBG_BUTTONS_PRINTF("ProcesarAccionEstado->Enviando comando I2C: %d\n", estadoActual);
                        if (I2C_EnviarComando(estadoActual)) {
                            DBG_BUTTONS_PRINTF("ProcesarAccionEstado->Estado %d enviado correctamente", estadoActual);
                        } else {
                            DBG_BUTTONS_PRINTF("ProcesarAccionEstado->Error al enviar estado %d", estadoActual);
                        }
                        //EnviarEstado(estadoActual);
                       menu.hayQueMostrarCambio = true;
                    } else {
                        // Acciones especiales según el estado:
                        switch (estadoActual) {
                            case Config::States::I2CState::INICIO:
                                DBG_BUTTONS("ProcesarAccionEstado->Estado Inicio (sin acción)");
                                break;
                            case Config::States::I2CState::CALEFACCION_ON:
                                DBG_BUTTONS("ProcesarAccionEstado->Estado Calefaccion On -> SetTemporizacion");
                                SetTemporizacion();
                                menu.hayQueMostrarCambio = false;
                                break;
                            case Config::States::I2CState::SET_TEMPORIZADOR:
                                DBG_BUTTONS("ProcesarAccionEstado->Estado Set Temporizador -> EnviarTemporizacion");
                                EnviarTemporizacion();
                                campanarioEstado.estadoActual = Config::States::I2CState::CALEFACCION_ON;             // Cambia al estado de calefacción encendida
                                campanarioEstado.estadoAnterior = Config::States::I2CState::SET_TEMPORIZADOR;         // Actualiza el estado anterior al de configuración de temporizador
                                temporizador.estaConfigurandose = false;                                              // Indica que ya no estamos en modo configuración de temporizador
                                menu.hayQueMostrarCambio = true;                                                       // Indica que hay que actualizar el
                                break;
                        
                            default:
                                DBG_BUTTONS_PRINTF("ProcesarAccionEstado->Acción especial no definida para estado: %d\n", estadoActual);
                                menu.hayQueMostrarCambio = true;
                                break;
                        }
                    }
                    return;
                }
            }
        
            // Si llegamos aquí, no se encontró el estado en la tabla
            DBG_BUTTONS_PRINTF("ProcesarAccionEstado->ADVERTENCIA: Estado %d no encontrado en tabla OPCIONES\n", estadoActual);
            menu.hayQueMostrarCambio = true;  // Fallback seguro
        }

    // ============================================================================
    // FUNCIONES DE TEMPORIZACIÓN DE CALEFACCIÓN
    // ============================================================================
        /**
         * @brief Inicia el modo de configuración del temporizador de calefacción
         * 
         * @details Configura el sistema para iniciar el proceso de temporización de
         *          calefacción. Muestra la pantalla de configuración, actualiza estados
         *          internos y prepara el sistema para recibir entrada del encoder.
         *          El usuario podrá ajustar los minutos usando el encoder rotatorio.
         * 
         * @note Establece temporizador.estaConfigurandose = true para modo encoder
         * @note Después de llamar esta función, el encoder controla la temporización
         * @note No marca cambio de menú (menu.hayQueMostrarCambio = false)
         * 
         * @see EnviarTemporizacion(), ManejarEncoder()
         * @see MensajeTemporizacion(), temporizador.minutosConfiguracion
         * 
         * @since v1.0
         * @author Julian Salas Bartolomé
         */
        void SetTemporizacion(void) 
        {
            DBG_AUXILIAR("SetTemporizacion->Iniciando configuración de temporización");
            MensajeTemporizacion(temporizador.minutosConfiguracion, true);                  // Muestra el mensaje de temporización   
            //menu.posicionSeleccionada = Config::States::I2CState::SET_TEMPORIZADOR;       // Actualiza la posición seleccionada en el menú
            campanarioEstado.estadoActual = Config::States::I2CState::SET_TEMPORIZADOR;     // Actualiza el estado actual
            campanarioEstado.estadoAnterior = Config::States::I2CState::SET_TEMPORIZADOR;   // Actualiza el estado anterior
            menu.hayQueMostrarCambio = false;                                                // Indica que no hay que actualizar el display
            temporizador.estaConfigurandose = true;                                        // Indica que no estamos en modo configuración de temporizador
            DBG_BUTTONS("SetTemporizacion->Estado Set Temporizador iniciado");
        }

    // ============================================================================
    // FUNCIONES DE COMUNICACIÓN I2C
    // ============================================================================
    //----------------------------------------------------------------------------
    // ENVIO COMANDOS
    // ---------------------------------------------------------------------------

    /**
     * @brief Envía comando simple al campanario vía I2C con logging automático
     * 
     * @details Wrapper que encapsula el envío de comandos I2C simples con logging
     *          automático de debug. Proporciona retroalimentación clara del éxito
     *          o fallo de la operación. Función base para comandos sin parámetros.
     * 
     * @param nEstadoMenu Estado/comando a enviar al campanario
     * 
     * @note Wrapper con logging de debug automático para I2C_EnviarComando()
     * @note Proporciona retroalimentación clara en logs de debug
     * 
     * @see I2C_EnviarComando(), EnviarEstadoConParametro()
     * @see ProcesarAccionEstado() para uso automático
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
        void EnviarEstado(int nEstadoMenu) {
            DBG_AUXILIAR_PRINTF("EnviarEstado->Enviando estado %d al campanario...", nEstadoMenu);
            if (I2C_EnviarComando(nEstadoMenu)) {
                DBG_I2C_PRINTF("EnviarEstado->Estado %d enviado correctamente", nEstadoMenu);
            } else {
                DBG_I2C_PRINTF("EnviarEstado->Error al enviar estado %d", nEstadoMenu);
            }
        }

    /**
     * @brief Envía comando con parámetro al campanario vía I2C
     * 
     * @details Wrapper para comandos I2C que requieren parámetros adicionales.
     *          Usado principalmente para envío de temporizaciones de calefacción.
     *          Proporciona logging detallado del comando y parámetro enviados.
     * 
     * @param nEstadoTx Estado/comando a enviar al campanario
     * @param nParametroTx Parámetro asociado al comando (ej: minutos temporizador)
     * 
     * @note Usado principalmente para temporizaciones con EnviarTemporizacion()
     * @note Proporciona logging detallado de comando y parámetro
     * 
     * @see EnviarEstado(), I2C_EnviarComandoConParametro()
     * @see EnviarTemporizacion() para uso específico de temporizador
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
    void EnviarEstadoConParametro(int nEstadoTx, int nParametroTx) {
        DBG_AUXILIAR_PRINTF("EnviarEstadoConParametro->Enviando estado %d con parámetro %d al campanario...", nEstadoTx, nParametroTx);
        if (I2C_EnviarComandoConParametro(nEstadoTx, nParametroTx)) {
            DBG_I2C_PRINTF("EnviarEstadoConParametro->Estado %d con parámetro %d enviado", nEstadoTx, nParametroTx);
        } else {
            DBG_I2C_PRINTF("EnviarEstadoConParametro->Error al enviar estado %d con parámetro %d", nEstadoTx, nParametroTx);
        }
    }

    /**
     * @brief Confirma y envía la configuración del temporizador al campanario
     * 
     * @details Envía comando I2C con los minutos configurados, actualiza estados
     *          del sistema y establece el flag de comando recién enviado para
     *          evitar colisiones I2C. Función final del proceso de temporización.
     * 
     * @note Establece lComandoRecienEnviado = true para control de timing I2C
     * @note Resetea nMilisegundoTemporal para evitar peticiones inmediatas
     * @warning Debe llamarse solo después de SetTemporizacion()
     * 
     * @see SetTemporizacion(), I2C_EnviarComandoConParametro()
     * @see temporizador.minutosConfiguracion, lComandoRecienEnviado
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */        
        void EnviarTemporizacion(void)
        {
            DBG_AUXILIAR_PRINTF("EnviarTemporizacion->Enviando temporización: %d minutos\n", temporizador.minutosConfiguracion);
            if (I2C_EnviarComandoConParametro(Config::States::I2CState::SET_TEMPORIZADOR, temporizador.minutosConfiguracion)) {
                DBG_I2C_PRINTF("EnviarTemporizacion->Estado %d con parámetro %d enviado", Config::States::I2CState::SET_TEMPORIZADOR, temporizador.minutosConfiguracion);
            } else {
                DBG_I2C_PRINTF("EnviarTemporizacion->Error al enviar estado %d con parámetro %d", Config::States::I2CState::SET_TEMPORIZADOR, temporizador.minutosConfiguracion);
            }
            //EnviarEstadoConParametro(EstadoSetTemporizador, nTemporizacion);                      // Envía el estado de temporización con el valor actual
            lComandoRecienEnviado = true;                                                        // Indica que se acaba de enviar un comando
            nMilisegundoTemporal = millis();                                                   // Reinicia el temporizador para la próxima solicitud I2C
        }


    // ---------------------------------------------------------------------------
    // SOLICITUDES I2C
    // ---------------------------------------------------------------------------
    /**
     * @brief Solicita el estado básico del campanario vía I2C con gestión de menús
     * 
     * @details Wrapper que encapsula la comunicación I2C básica y maneja la lógica
     *          de cambio automático de menú. Detecta cambios de estado y actualiza
     *          la interfaz automáticamente. Función más simple para consultas
     *          de estado sin información temporal.
     * 
     * @note Actualiza campanarioEstado.estadoActual con respuesta recibida
     * @note Gestiona cambios de menú automáticamente si hay cambio de estado
     * 
     * @see I2C_SolicitarEstado(), SeleccionaMenu()
     * @see SolicitarEstadoHora(), SolicitarEstadoFechaHora()
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */    
        void SolicitarEstadoCampanario() {
            DBG_AUXILIAR("SolicitarEstadoCampanario->Iniciando solicitud de estado del campanario...");
            I2CResponse response = I2C_SolicitarEstado();

            if (response.success) {
                //uint8_t nuevoEstado = response.data[0];
                campanarioEstado.estadoActual = response.data[0];                    //Actualizamos el estado actual    
                DBG_I2C_PRINTF("SolicitarEstadoCampanario->Estado actual: %d", campanarioEstado.estadoActual);

                if (campanarioEstado.estadoActual != campanarioEstado.estadoAnterior) { //Si hay cambio de estado
                    DBG_I2C_PRINTF("SolicitarEstadoCampanario->Cambio de estado detectado: %d -> %d", campanarioEstado.estadoAnterior, campanarioEstado.estadoActual);
                    campanarioEstado.estadoAnterior = campanarioEstado.estadoActual;    //Actualizamos el estado anterior
                    SeleccionaMenu();                                                   //Seleccionamos el menu en funcion del estado recibido
                    menu.posicionSeleccionada = 0;                                      //Inicializamos la posicion del menu
                    menu.hayQueMostrarCambio = true;                                    //Mostramos el cambio en el display
                }
            }
        }

    /**
     * @brief Solicita estado y hora actual del campanario vía I2C
     * 
     * @details Obtiene estado del campanario junto con hora, minutos y segundos
     *          actuales. Actualiza estado de conectividad a Internet usando
     *          bit BIT_SIN_INTERNET y gestiona cambios de menú automáticamente.
     *          Función intermedia entre estado básico y fecha/hora completa.
     * 
     * @note Detecta conexión a Internet usando Config::States::BIT_SIN_INTERNET
     * @note Actualiza campanarioEstado.tieneInternet automáticamente
     * @note Requiere respuesta mínima de 4 bytes (estado + hora + min + seg)
     * 
     * @see I2C_SolicitarEstadoHora(), SolicitarEstadoFechaHora()
     * @see Config::States::BIT_SIN_INTERNET, campanarioEstado.tieneInternet
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */    
        void SolicitarEstadoHora() {
            DBG_AUXILIAR("SolicitarEstadoHora->Iniciando solicitud de hora del campanario...");

            I2CResponse response = I2C_SolicitarEstadoHora();                           // Solicitar estado y hora al campanario

            if (response.success && response.length >= 4) {                             // Verificar éxito de la recepcion y longitud mínima
                campanarioEstado.estadoActual = response.data[0];
                campanarioEstado.nHora = response.data[1];
                campanarioEstado.nMinutos = response.data[2];
                campanarioEstado.nSegundos = response.data[3];

                campanarioEstado.tieneInternet = !(campanarioEstado.estadoActual & Config::States::BIT_SIN_INTERNET);   // Actualizar estado de Internet

                if (campanarioEstado.estadoActual != campanarioEstado.estadoAnterior) { // Si hay cambio de estado
                    DBG_I2C_PRINTF("SolicitarEstadoHora->Cambio de estado detectado: %d -> %d", campanarioEstado.estadoAnterior, campanarioEstado.estadoActual);
                    campanarioEstado.estadoAnterior = campanarioEstado.estadoActual;    // Actualizar estado del campanario
                    SeleccionaMenu();                                                    // Seleccionar menú según el nuevo estado
                    menu.posicionSeleccionada = 0;                                      // Inicializar posición del menú
                    menu.hayQueMostrarCambio = true;                                    // Indicar que hay que mostrar el cambio en el display
                }
            }
        }

    /**
     * @brief Solicita estado, fecha y hora completa del campanario (versión unificada)
     * 
     * @details Función unificada que reemplaza funciones duplicadas anteriores.
     *          Puede incluir datos de temporización según el parámetro. Obtiene
     *          información temporal completa: estado, día, mes, año, hora, minutos
     *          y segundos. Evita duplicación de código y centraliza lógica común.
     * 
     * @param incluirTemporizacion true para incluir datos de temporización en petición,
     *                            false para solicitud básica de fecha/hora
     * 
     * @note Función unificada que reemplaza funciones duplicadas anteriores
     * @note Requiere respuesta mínima de 7 bytes para información completa
     * @note Actualiza toda la estructura campanarioEstado con información temporal
     * 
     * @see I2C_SolicitarEstadoFechaHora(), SolicitarEstadoHora()
     * @see campanarioEstado estructura completa
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
        void SolicitarEstadoFechaHora(bool incluirTemporizacion) {

            DBG_AUXILIAR("SolicitarEstadoFechaHora->Iniciando solicitud de fecha y hora del campanario...");
            // Seleccionar función I2C según parámetro:
            I2CResponse response = I2C_SolicitarEstadoFechaHora(incluirTemporizacion);                                  // Solicitar estado y fecha/hora (y opcionalmente temporización)

            const char* operacion = incluirTemporizacion ? 
                                   "SolicitarEstadoFechaHoraoTemporizacion" : 
                                   "SolicitarEstadoFechaHora";

            DBG_I2C_PRINTF("SolicitarEstadoFechaHora-> %s->Solicitando fecha y hora del campanario...", operacion);
            if (response.success && response.length >= 7) {                                                             // Verificar éxito de la recepción y longitud mínima                     
                // LÓGICA COMÚN (antes duplicada):
                campanarioEstado.estadoActual = response.data[0];                                                       // Actualizar estado actual
                campanarioEstado.tieneInternet = !(campanarioEstado.estadoActual & Config::States::BIT_SIN_INTERNET);   // Actualizar estado de Internet
                // Actualizar estructura campanario:
                campanarioEstado.nDia = response.data[1];
                campanarioEstado.nMes = response.data[2];
                campanarioEstado.nAno = response.data[3];
                campanarioEstado.nHora = response.data[4];
                campanarioEstado.nMinutos = response.data[5];
                campanarioEstado.nSegundos = response.data[6];


                if (campanarioEstado.estadoActual != campanarioEstado.estadoAnterior) {                                 // Si hay cambio de estado  
                    DBG_I2C_PRINTF("%s->Cambio de estado detectado: %d -> %d", 
                                  operacion, campanarioEstado.estadoAnterior, campanarioEstado.estadoActual);
                    campanarioEstado.estadoAnterior = campanarioEstado.estadoActual;                                    // Actualizar estado del campanario    
                    SeleccionaMenu();                                                                                   // Seleccionar menú según el nuevo estado
                    menu.posicionSeleccionada = 0;                                                                      // Inicializar posición del menú
                    menu.hayQueMostrarCambio = true;                                                                    // Indicar que hay que mostrar el cambio en el display
                }
                DBG_I2C_PRINTF("%s->Estado actual del campanario: %d", operacion, campanarioEstado.estadoActual);
                DBG_I2C_PRINTF("%s->Hora recibida: %02d:%02d:%02d %02d/%02d/%02d", 
                              operacion, campanarioEstado.nHora, campanarioEstado.nMinutos, 
                              campanarioEstado.nSegundos, campanarioEstado.nDia, 
                              campanarioEstado.nMes, campanarioEstado.nAno);
            } else {
                DBG_I2C_PRINTF("%s->No se recibió respuesta del campanario", operacion);
            }
        }

    // ============================================================================
    // FUNCIÓN DE INICIALIZACIÓN
    // ============================================================================

    /**
     * @brief Inicializa todas las estructuras y variables globales del sistema
     * 
     * @details Establece valores iniciales seguros para todas las estructuras de datos:
     *          
     *          **MenuState:** menú 0, posición 0, sin cambios pendientes
     *          **EncoderState:** posiciones en 0, sin cambios detectados
     *          **CampanarioEstado:** estado INICIO, sin Internet, fecha/hora en valores por defecto
     *          **DisplayState:** no sleep, contadores en 0, timestamp actual
     *          **TemporizadorState:** 45 minutos por defecto, no configurando
     *          
     *          Función crítica que debe llamarse antes de cualquier operación del sistema.
     * 
     * @note DEBE llamarse en setup() antes de usar cualquier otra función del sistema
     * @warning No llamar durante funcionamiento normal, solo en inicialización
     * @warning Resetea todos los estados a valores por defecto
     * 
     * @see setup() en DialCampanario.ino
     * @see MenuState, EncoderState, CampanarioEstado, DisplayState, TemporizadorState
     * 
     * @since v1.0
     * @author Julian Salas Bartolomé
     */
        void InicializarVariablesGlobales() {
            // Inicializar estado del menú:
            DBG_AUXILIAR("InicializarVariablesGlobales->Inicializando variables globales...");
            menu.indiceMenuActivo = 0;
            menu.posicionSeleccionada = 0;
            menu.numeroDeOpciones = 0;
            menu.opcionesDisponibles = nullptr;
            menu.hayQueMostrarCambio = false;

            // Inicializar encoder:
            encoder.posicionAnterior = 0;
            encoder.posicionActual = 0;
            encoder.hayCambio = false;

            // Inicializar campanario (mejorar estructura existente):
            campanarioEstado.estadoActual = Config::States::I2CState::INICIO;
            campanarioEstado.estadoAnterior = Config::States::I2CState::INICIO;
            campanarioEstado.tieneInternet = false;
            campanarioEstado.nHora = 0;
            campanarioEstado.nMinutos = 0;
            campanarioEstado.nSegundos = 0;
            campanarioEstado.nDia = 1;
            campanarioEstado.nMes = 1;
            campanarioEstado.nAno = 24;

            // Inicializar display:
            display.estaEnModoSleep = false;
            display.contadorCiclosSleep = 0;
            display.tiempoUltimaActividad = millis();

            // Inicializar temporizador:
            temporizador.minutosConfiguracion = 45;
            temporizador.estaConfigurandose = false;
        }


#endif

