//Tratamiento de iamgenes  ( redimensionado ) https://www.photopea.com/
#ifndef DIAL_H
    #define DIAL_H

// Las opciones de depuración se controlan desde Config::Debug
    #include <Wire.h>
    #include "Display.h"
    #include <M5Dial.h>
    #include "Configuracion.h"


    #define I2C_SLAVE_ADDR 0x12

    
    //#define bitEstadoInicio            0x00
    #define bitEstadoDifuntos          0x01
    #define bitEstadoMisa              0x02
    //#define bitEstadoStop              0x04
    #define bitEstadoCalefaccionOn     0x10
    //#define bitEstadoCalefaccionOff    0x20
    //#define bitEstadoCampanario        0x40
    //#define bitEstadoHora              0x80
    #define bitEstadoSinInternet        0x20
    #define bitEstadoProteccionCampanadas 0x40



    #define SLAVE_ADDRESS 0x08

    #define nEstados                7

    #define nmsGetEstadoCampanario  500 // Tiempo en milisegundos para solicitar el estado del campanario 
    #define nCiclosDisplaySleep         30 // Tiempo en milisegundos para poner el display en modo sleep
    #define nCiclosDisplayHora           20 // Tiempo en milisegundos para solicitar la hora al esclavo I2C

        // Define los estados
        enum Estados {
           EstadoInicio = 0,
           EstadoDifuntos,
           EstadoMisa,
           EstadoStop,
           EstadoCalefaccionOn,
           EstadoCalefaccionOff,
           EstadoCampanario,
           EstadoHora,
           EstadoSinInternet,
           EstadoProteccionCampanadas,
           EstadoFechaHora,
           EstadoSetTemporizador,
           EstadoFin,
           EstadoFechaHoraoTemporizacion,
           EstadoTemporizacion
        };


    const int* menuActual = nullptr; 
    const int menu0[] = {EstadoDifuntos, EstadoMisa, EstadoCalefaccionOn};
    #define ItemsMenu0  3
    const int menu1[] = {EstadoDifuntos, EstadoMisa,  EstadoCalefaccionOff};
    #define ItemsMenu1  3
    const int menu2[] = { EstadoStop, EstadoCalefaccionOn};
    #define ItemsMenu2  2
    const int menu3[] = { EstadoStop, EstadoCalefaccionOff};
    #define ItemsMenu3  2
    const int menu4[] = { EstadoCalefaccionOn };
    #define ItemsMenu4  1
    const int menu5[] = { EstadoCalefaccionOff };
    #define ItemsMenu5  1

    int nMenu = 0;                                              // Variable para almacenar el numero menú actual

    struct CampanarioEstado                                     //Estructura para almacenar el estado del campanario
    {
        uint8_t nEstado;                                        // Estado del campanario
        uint8_t nHora;                                          // Hora actual
        uint8_t nMinutos;                                       // Minutos actuales
        uint8_t nSegundos;                                      // Segundos actuales
        uint8_t nDia;                                           // Día actual
        uint8_t nMes;                                           // Mes actual
        uint8_t nAno;                                           // Año actual (2 dígitos)
    };
    CampanarioEstado campanarioEstado;                          // Estructura para almacenar el estado del campanario

    int nMenuItems = 4;                                     // Cambia según el menú activo

    long nMilisegundoTemporal = 0;                          // Variable para almacenar el tiempo temporal
    int nContadorCiclosDisplaySleep = 0;                    // Contador de ciclos de sleep del display

    int nTemporizacion = 45;                                // Variable para almacenar la temporización de la calefacción

    int nEstado = EstadoInicio;                             // Estado inicial del sistema
    uint8_t nEstadoAnterior = EstadoInicio;                 // Estado anterior del menú
    uint8_t nEstadoActual = EstadoInicio;                   // Estado actual del menú
    int nPosicionAneterior = 0;                             // Estado anterior del sistema
    int nPosicionActual = 0;                                // Estado actual del sistema
    bool lCambioEstado = false;                             // Indica si ha habido un cambio de estado
    bool lSleep = false;                                    // Indica si el display está en modo sleep
    bool lInternet = false;                                 // Indica si no hay conexión a Internet

    void InicioDisplay (void);                              //!< Inicializa la pantalla del dispositivo M5Dial
    void AsignaMenu (void);                                 //!< Asigna el menú correspondiente según el estado actual
    void MostrarOpcion(int seleccionado);                   //!< Muestra un mensaje en pantalla según la opción seleccionada

    void EnviarEstado(int nEstadoMenu);                     //!< Envía el estado actual del menú a un dispositivo esclavo a través del bus I2C
    void EnviarTemporizacion(void);                         //!< Envía el estado de la calefacción encendida al esclavo I2C con la temporización
    void EnviarEstadoConParametro(int nEstadoTx, int nParametroTx); //!< Envía un estado y un parametro al dispositivo esclavo a través del bus I2C
    void SolicitarEstadoCampanario(void);                   //!< Solicita el estado del campanario al esclavo I2C
    void SolicitarEstadoHora(void);                         //!< Solicita el estado y la hora al esclavo I2C
    void SolicitarEstadoFechaHora(void);                    //!< Solicita la fecha y hora al esclavo I2C
    void SolicitarEstadoFechaHoraoTemporizacion(void);       //!< Solicita  la temporizacion de la calefaccion

    void SeleccionaMenu(int nEstadoSeleccionado);           //!< Selecciona el menú según el estado recibido

    void ReiniciarEstado(int nuevoEstado, int nPosicionMenu);  //!< Reinicia el estado del sistema al nuevo estado especificado y deja el menu en la posición indicada
    void EnviarYReiniciar(int estado, int nPosicionMenu);   //!< Envía el estado actual y reinicia el sistema al nuevo estado especificado
    void ManejarBotonA(void);                               //!< Maneja el evento del botón A del M5Dial
    void DespertarDisplay(void);                            //!< Despierta el display del modo sleep
    void ProcesarAccionEstado(int estadoActual);            //!< Procesa la acción correspondiente al estado seleccionado
    void SetTemporizacion(void);                            //!< Inicia la configuración del temporizador de calefacción
    void ManejarEncoder(void);                              //!< Maneja el evento del encoder del M5Dial
    void ActualizarDisplaySleep(void);                      //!< Actualiza el display en modo sleep



    /**
     * @brief Inicializa la pantalla del dispositivo M5Dial
     * 
     * Esta función configura e inicializa la pantalla del dispositivo M5Dial.
     * Utiliza la configuración predeterminada del M5 y establece los parámetros
     * iniciales necesarios para el funcionamiento de la pantalla.
     * 
     * @note Requiere que el objeto M5Dial esté correctamente definido
     * 
     * @return void No devuelve ningún valor
     */
        void InicioDisplay (void)
        {
            auto cfg = M5.config();
            M5Dial.begin(cfg, true, false);
        
            CreaSpritesMenu();
        }        

    /**
     * @brief Muestra un mensaje en pantalla según la opción seleccionada.
     *
     * Esta función recibe un valor entero que representa el estado o la opción seleccionada
     * y muestra el mensaje correspondiente en el display llamando a la función adecuada.
     *
     * @param seleccionado Valor entero que indica la opción seleccionada. Puede ser uno de los siguientes:
     *  - EstadoInicio:          Muestra el mensaje de inicio.
     *  - EstadoDifuntos:        Muestra el mensaje de difuntos.
     *  - EstadoMisa:            Muestra el mensaje de misa.
     *  - EstadoStop:            Muestra el mensaje de parada.
     *  - EstadoCalefaccionOn:   Muestra el mensaje de calefacción encendida.
     *  - EstadoCalefaccionOff:  Muestra el mensaje de calefacción apagada.
     *  - EstadoSetTemporizador:  Muestra el mensaje de temporización.
     */
        void MostrarOpcion(int seleccionado) 
        {
            if constexpr (Config::Debug::MENU_DEBUG) {
                Serial.printf("MostrarOpcion->Seleccionado: %d\n", seleccionado);
            }
            switch (seleccionado) {
                case EstadoInicio:          
                     MensajeInicio();
                    break;
                case EstadoDifuntos:        
                    MensajeDifuntos(); 
                    break;
                case EstadoMisa:            
                    MensajeMisa(); 
                    break;
                case EstadoStop:            
                    MensajeStop(); 
                    break;
                case EstadoCalefaccionOn:   
                    MensajeCalefaccionOn(); 
                    break;
                case EstadoCalefaccionOff:  
                    MensajeCalefaccionOff(); 
                    break;
                case EstadoSetTemporizador:
                    MensajeTemporizacion(nTemporizacion, true); 
                    break;
            }
        }

    /**
     * @brief Asigna el menú actual y muestra la opción correspondiente.
     *
     * Esta función selecciona el menú activo y el número de elementos del menú
     * según el valor de nMenu. Luego, muestra la opción actual del menú basada en nEstado.
     * Si el sistema está en modo de bajo consumo (lSleep es verdadero), restaura el brillo
     * de la pantalla al máximo utilizando BrilloFull(), siempre que el brillo actual sea menor a 100.
     */
        void AsignaMenu (void) 
        {
            switch(nMenu) {
                case 0: menuActual = menu0; nMenuItems = ItemsMenu0; break;
                case 1: menuActual = menu1; nMenuItems = ItemsMenu1; break;
                case 2: menuActual = menu2; nMenuItems = ItemsMenu2; break;
                case 3: menuActual = menu3; nMenuItems = ItemsMenu3; break;
                case 4: menuActual = menu4; nMenuItems = ItemsMenu4; break;
                case 5: menuActual = menu5; nMenuItems = ItemsMenu5; break;
            }
            MostrarOpcion(menuActual[nEstado]);
            if (lSleep) {
               lSleep = false;                           // Asegurarse de que el brillo esté al máximo
               if ( M5.Display.getBrightness() < 100) {
                    BrilloFull();                  // Asegurarse de que el brillo esté al máximo
                } 
            }  
        }

    /**
     * @brief Selecciona el estado del menú según el estado seleccionado.
     *
     * Esta función evalúa el valor de nEstadoSeleccionado utilizando máscaras de bits
     * para determinar el estado actual del menú. Si bitEstadoProteccionCampanadas está activo,
     * solo permite opciones de calefacción (menús 4 y 5). Si no, permite todas las opciones.
     * Asigna un valor a nEstadoMenu según las siguientes condiciones:
     *   - Si protección activa y calefacción ON: nEstadoMenu = 5 (solo calefacción OFF)
     *   - Si protección activa y calefacción OFF: nEstadoMenu = 4 (solo calefacción ON)
     *   - Si está activo Difuntos o Misa y la calefacción está encendida: nEstadoMenu = 3
     *   - Si está activo Difuntos o Misa y la calefacción está apagada: nEstadoMenu = 2
     *   - Si no está activo Difuntos ni Misa pero la calefacción está encendida: nEstadoMenu = 1
     *   - Si no está activo Difuntos ni Misa ni la calefacción: nEstadoMenu = 0
     *
     * @param nEstadoSeleccionado Valor entero que representa el estado actual mediante bits.
     */
        void SeleccionaMenu(int nEstadoSeleccionado) 
        {
            bool ProteccionToqueHoras = nEstadoSeleccionado & bitEstadoProteccionCampanadas;
            bool calefOn = nEstadoSeleccionado & bitEstadoCalefaccionOn;

            if (ProteccionToqueHoras) {
                // Si la protección está activa, solo permite opciones de calefacción
                nMenu = calefOn ? 5 : 4;  // Menu 5: solo CalefaccionOff, Menu 4: solo CalefaccionOn
            } else {
                // Comportamiento normal cuando no hay protección
                bool difuntosMisa = nEstadoSeleccionado & (bitEstadoDifuntos | bitEstadoMisa);
                nMenu = difuntosMisa ? (calefOn ? 3 : 2) : (calefOn ? 1 : 0);
            }
        }


    //======================================================================================================
    // Funciones de envío de ordenes al control del campanario I2C
    //======================================================================================================
    /**
     * @brief Envía el estado actual del menú a un dispositivo esclavo a través del bus I2C.
     *
     * Esta función inicia una transmisión I2C con la dirección definida por I2C_SLAVE_ADDR,
     * envía el valor del estado del menú especificado por nEstadoMenu y finaliza la transmisión.
     * Informa por el puerto serie si la transmisión fue exitosa o si ocurrió un error.
     *
     * @param nEstadoMenu Valor entero que representa el estado actual del menú a enviar al esclavo I2C.
     */
        void EnviarEstado (int nEstadoMenu ) 
        {
            Wire.beginTransmission(I2C_SLAVE_ADDR);
            Wire.write(nEstadoMenu); 
            uint8_t result = Wire.endTransmission();
            if constexpr (Config::Debug::I2C_TX_DEBUG) {
                if (result == 0) {
                    Serial.printf("EnviarEstado->Estado %d enviado correctamente al esclavo I2C\n", nEstadoMenu);
                } else {
                    Serial.printf("EnviarEstado->Error al enviar el estado %d al esclavo I2C, código de error: %d\n", nEstadoMenu, result);
                }
            }        
        }

    /**
     * @brief Envía un estado y un parámetro asociado al esclavo I2C.
     *
     * Esta función inicia una transmisión I2C al dispositivo esclavo definido por I2C_SLAVE_ADDR,
     * enviando primero el valor de estado (nEstadoTx) y luego el parámetro asociado (nParametroTx).
     * Al finalizar la transmisión, si está habilitada la depuración (DEBUGI2CTX), se imprime en el
     * puerto serie el resultado de la operación, indicando si fue exitosa o si ocurrió algún error.
     *
     * @param nEstadoTx Estado u orden a enviar al esclavo I2C.
     * @param nParametroTx Parámetro asociado al estado/orden enviado.
     */
        void EnviarEstadoConParametro(int nEstadoTx, int nParametroTx) 
        {
            Wire.beginTransmission(I2C_SLAVE_ADDR);
            Wire.write(nEstadoTx);                                              // Envía el estado/orden
            Wire.write(nParametroTx);                                           // Envía el parámetro asociado al estado/orden
            uint8_t result = Wire.endTransmission();
            if constexpr (Config::Debug::I2C_TX_DEBUG) {
                if (result == 0) {
                    Serial.printf("EnviarEstadoConParametro->Estado %d con parámetro %d enviado correctamente al esclavo I2C\n", nEstadoTx, nParametroTx);
                } else {
                    Serial.printf("EnviarEstadoConParametro->Error al enviar el estado %d con parámetro %d al esclavo I2C, código de error: %d\n", nEstadoTx, nParametroTx, result);
                }
            }
        }

    //======================================================================================================
    // Funciones de recepción de información del control del campanario I2C
    // SolicitarEstadoCampanario.- Solicita solo el estado del campanario
    // SolicitarEstadoHora.-  Solicita el estado y la hora del campanario
    // SolicitarEstadoFechaHora.- Solicita el estado, la hora y la fecha del campanario
    //======================================================================================================
    /**
     * @brief Solicita el estado actual del campanario a través del bus I2C.
     *
     * Esta función envía una solicitud al dispositivo esclavo I2C identificado por la dirección I2C_SLAVE_ADDR
     * para obtener el estado actual del campanario. Si se detecta un cambio de estado respecto al anterior,
     * se actualiza el estado, se selecciona el menú correspondiente y se marca que ha habido un cambio de estado.
     * 
     * Mensajes de depuración pueden ser impresos por el puerto serie si están habilitados los flags DEBUGI2CREQUEST o DEBUGI2CTX.
     *
     * Variables externas utilizadas:
     * - I2C_SLAVE_ADDR: Dirección del dispositivo esclavo I2C.
     * - EstadoCampanario: Comando para solicitar el estado.
     * - nEstadoActual: Variable donde se almacena el estado recibido.
     * - nEstadoAnterior: Variable que almacena el estado anterior para detectar cambios.
     * - SeleccionaMenu(int): Función que selecciona el menú según el estado recibido.
     * - nEstado: Variable que indica el elemento actual del menú.
     * - lCambioEstado: Bandera que indica si ha habido un cambio de estado.
     */
        void SolicitarEstadoCampanario() 
        {

            if constexpr (Config::Debug::I2C_REQUEST_DEBUG) {
                Serial.println("SolicitarEstadoCampanario->Solicitando estado del campanario...");
            } 
        
            Wire.beginTransmission(I2C_SLAVE_ADDR);                                     // Inicia la transmisión I2C al esclavo
            Wire.write(EstadoCampanario);                                               // Envía el comando para solicitar el estado del campanario                 
            Wire.endTransmission();                                                     // Termina la transmisión

            Wire.requestFrom(I2C_SLAVE_ADDR, 1);                                        // Solicita 1 byte al esclavo
            if (Wire.available()) {
                nEstadoActual = Wire.read();    
                campanarioEstado.nEstado = nEstadoActual;                               // Actualiza el estado del campanario            
                if (nEstadoActual != nEstadoAnterior) {
                    if constexpr (Config::Debug::I2C_REQUEST_DEBUG) {
                        Serial.printf("SolicitarEstadoCampanario->Cambio de estado detectado: %d -> %d\n", nEstadoAnterior, nEstadoActual); 
                    }
                    nEstadoAnterior = nEstadoActual;
                    SeleccionaMenu(nEstadoActual); // Llama a la función para seleccionar el menú según el estado recibido
                    nEstado = 0; // Reinicia el estado al primer elemento del menú
                    lCambioEstado = true; // Marca que ha habido un cambio de estado
                } 
            } else {
                if constexpr (Config::Debug::I2C_REQUEST_DEBUG) {
                    Serial.println("SolicitarEstadoCampanario->No se recibió respuesta del campanario");
                }
            }    
        }

    /**
     * @brief Solicita el estado y la hora actual al dispositivo esclavo a través del bus I2C.
     *
     * Esta función envía una solicitud al esclavo I2C identificado por la dirección I2C_SLAVE_ADDR
     * para obtener el estado actual y la hora (hora, minutos y segundos) del campanario.
     * Si la respuesta es válida (se reciben al menos 4 bytes), actualiza las variables globales
     * correspondientes con los valores recibidos. Si detecta un cambio de estado respecto al anterior,
     * actualiza el menú y marca que ha habido un cambio de estado.
     *
     * Mensajes de depuración se imprimen si están habilitados los flags DEBUGI2CREQUEST o DEBUGI2CRX.
     *
     * Variables globales modificadas:
     *  - nEstadoActual: Estado actual recibido del esclavo.
     *  - campanarioEstado.nHora: Hora recibida.
     *  - campanarioEstado.nMinutos: Minutos recibidos.
     *  - campanarioEstado.nSegundos: Segundos recibidos.
     *  - campanarioEstado.nEstado: Estado recibido.
     *  - nEstadoAnterior: Estado anterior para detectar cambios.
     *  - nEstado: Índice del menú, reiniciado si hay cambio de estado.
     *  - lCambioEstado: Bandera que indica si hubo cambio de estado.
     *
     * @note Requiere que la comunicación I2C esté inicializada previamente.
     */
        void SolicitarEstadoHora() {
            if constexpr (Config::Debug::I2C_REQUEST_DEBUG) {
                Serial.println("SolicitarEstadoHora->Solicitando hora del campanario...");
            }
        
            // Solicita la hora al esclavo I2C
            Wire.beginTransmission(I2C_SLAVE_ADDR);
            Wire.write(EstadoHora); 
            Wire.endTransmission();
        
            Wire.requestFrom(I2C_SLAVE_ADDR, 4); // Solicita 4 bytes al esclavo
        
            if (Wire.available() >= 4) {
                nEstadoActual = Wire.read();                                        // Lee el estado del esclavo I2C        
                lInternet = (nEstadoActual & bitEstadoSinInternet) ? false : true;  // Si el bit bitEstadoSinInternet está activo, no hay conexión a Internet
                campanarioEstado.nHora = Wire.read();                               // Lee la hora del esclavo I2C
                campanarioEstado.nMinutos = Wire.read();                            // Lee los minutos del esclavo I2C
                campanarioEstado.nSegundos = Wire.read();                           // Lee los segundos del esclavo I2C
                campanarioEstado.nEstado = nEstadoActual;                           // Actualiza el estado del campanario
                if (nEstadoActual != nEstadoAnterior) {
                    if constexpr (Config::Debug::I2C_REQUEST_DEBUG) {
                        Serial.printf("SolicitarEstadoHora->Cambio de estado detectado: %d -> %d\n", nEstadoAnterior, nEstadoActual); 
                    }
                    nEstadoAnterior = nEstadoActual;                                // Actualiza el estado anterior
                    SeleccionaMenu(nEstadoActual);                                  // Llama a la función para seleccionar el menú según el estado recibido
                    nEstado = 0;                                                    // Reinicia el estado al primer elemento del menú
                    lCambioEstado = true;                                           // Marca que ha habido un cambio de estado
                } 
                if constexpr (Config::Debug::I2C_RX_DEBUG) {
                    Serial.printf("SolicitarEstadoHora->Estado actual del campanario: %d\n", nEstadoActual);
                    Serial.printf("SolicitarEstadoHora->Hora recibida: %02d:%02d:%02d\n", campanarioEstado.nHora, campanarioEstado.nMinutos, campanarioEstado.nSegundos);
                }        
            } else {
                if constexpr (Config::Debug::I2C_REQUEST_DEBUG) {
                    Serial.println("SolicitarEstadoHora->No se recibió respuesta del campanario");
                }
            }            

        }

    /**
     * @brief Solicita la fecha y hora actual al dispositivo esclavo a través del bus I2C.
     *
     * Esta función envía una solicitud al esclavo I2C para obtener el estado actual, fecha y hora del campanario.
     * Lee 7 bytes de respuesta que incluyen el estado, día, mes, año, hora, minutos y segundos.
     * Si se detecta un cambio de estado respecto al anterior, actualiza el menú y marca el cambio de estado.
     * Incluye mensajes de depuración opcionales mediante macros de preprocesador.
     *
     * Requiere que las variables globales y estructuras relacionadas con el estado del campanario estén definidas.
     *
     * @note Utiliza las macros DEBUGI2CREQUEST y DEBUGI2CRX para imprimir información de depuración por Serial.
     */
        void SolicitarEstadoFechaHora() {
            if constexpr (Config::Debug::I2C_REQUEST_DEBUG) {
                Serial.println("SolicitarEstadoFechaHora->Solicitando fecha y hora del campanario...");
            }
        
            // Solicita la hora al esclavo I2C
            Wire.beginTransmission(I2C_SLAVE_ADDR);                                     //Hacemos Request I2C al esclavo para solicitar Fecha y Hora
            Wire.write(EstadoFechaHora); 
            Wire.endTransmission();
        
            Wire.requestFrom(I2C_SLAVE_ADDR, 7);                                        // Solicita 7 bytes al esclavo
        
            if (Wire.available() >= 7) {
                nEstadoActual = Wire.read();                                            // Lee el estado del esclavo I2C
                lInternet = (nEstadoActual & bitEstadoSinInternet) ? false : true;      // Si el bit bitEstadoSinInternet está activo, no hay conexión a Internet
                campanarioEstado.nDia = Wire.read();                                    // Lee el día del esclavo I2C
                campanarioEstado.nMes = Wire.read();                                    // Lee el mes del esclavo I2C
                campanarioEstado.nAno = Wire.read();                                    // Lee el año del esclavo I2C
                campanarioEstado.nHora = Wire.read();                                   // Lee la hora del esclavo I2C
                campanarioEstado.nMinutos = Wire.read();                                // Lee los minutos del esclavo I2C
                campanarioEstado.nSegundos = Wire.read();                               // Lee los segundos del esclavo I2C
                campanarioEstado.nEstado = nEstadoActual;                               // Actualiza el estado del campanario
                if (nEstadoActual != nEstadoAnterior) {                                 // Si hay un cambio de estado
                    if constexpr (Config::Debug::I2C_REQUEST_DEBUG) {
                        Serial.printf("SolicitarEstadoFechaHora->Cambio de estado detectado: %d -> %d\n", nEstadoAnterior, nEstadoActual);
                    }
                    nEstadoAnterior = nEstadoActual;
                    SeleccionaMenu(nEstadoActual);                                      // Llama a la función para seleccionar el menú según el estado recibido
                    nEstado = 0;                                                        // Reinicia el estado al primer elemento del menú
                    lCambioEstado = true;                                               // Marca que ha habido un cambio de estado
                }
                if constexpr (Config::Debug::I2C_RX_DEBUG) {
                    Serial.printf("SolicitarEstadoFechaHora->Estado actual del campanario: %d\n", nEstadoActual);
                    Serial.printf("SolicitarEstadoFechaHora->Hora recibida: %02d:%02d:%02d %02d/%02d/%02d\n", campanarioEstado.nHora, campanarioEstado.nMinutos, campanarioEstado.nSegundos, campanarioEstado.nDia, campanarioEstado.nMes, campanarioEstado.nAno);
                }        
            } else {
                if constexpr (Config::Debug::I2C_REQUEST_DEBUG) {
                    Serial.println("SolicitarEstadoFechaHora->No se recibió respuesta del campanario");
                }
            }            

        }

        void SolicitarEstadoFechaHoraoTemporizacion ()
        {
            if constexpr (Config::Debug::I2C_REQUEST_DEBUG) {
                Serial.println("SolicitarEstadoFechaHoraoTemporizacion->Solicitando estado de fecha/hora y temporización del campanario...");
            }
        
            // Solicita la hora al esclavo I2C
            Wire.beginTransmission(I2C_SLAVE_ADDR);                                     //Hacemos Request I2C al esclavo para solicitar Fecha y Hora
            Wire.write(EstadoFechaHoraoTemporizacion); 
            Wire.endTransmission();
        
            Wire.requestFrom(I2C_SLAVE_ADDR, 7);                                        // Solicita 7 bytes al esclavo
        
            if (Wire.available() >= 7) {
                nEstadoActual = Wire.read();                                            // Lee el estado del esclavo I2C
                lInternet = (nEstadoActual & bitEstadoSinInternet) ? false : true;      // Si el bit bitEstadoSinInternet está activo, no hay conexión a Internet
                campanarioEstado.nDia = Wire.read();                                    // Lee el día del esclavo I2C
                campanarioEstado.nMes = Wire.read();                                    // Lee el mes del esclavo I2C
                campanarioEstado.nAno = Wire.read();                                    // Lee el año del esclavo I2C
                campanarioEstado.nHora = Wire.read();                                   // Lee la hora del esclavo I2C
                campanarioEstado.nMinutos = Wire.read();                                // Lee los minutos del esclavo I2C
                campanarioEstado.nSegundos = Wire.read();                               // Lee los segundos del esclavo I2C
                campanarioEstado.nEstado = nEstadoActual;                               // Actualiza el estado del campanario
                if (nEstadoActual != nEstadoAnterior) {                                 // Si hay un cambio de estado
                    if constexpr (Config::Debug::I2C_REQUEST_DEBUG) {
                       Serial.printf("SolicitarEstadoFechaHoraoTemporizacion->Cambio de estado detectado: %d -> %d\n", nEstadoAnterior, nEstadoActual);
                    }
                    nEstadoAnterior = nEstadoActual;
                    SeleccionaMenu(nEstadoActual);                                      // Llama a la función para seleccionar el menú según el estado recibido
                    nEstado = 0;                                                        // Reinicia el estado al primer elemento del menú
                    lCambioEstado = true;                                               // Marca que ha habido un cambio de estado
                }
                if constexpr (Config::Debug::I2C_REQUEST_DEBUG) {
                    Serial.printf("SolicitarEstadoFechaHoraoTemporizacion->Estado actual del campanario: %d\n", nEstadoActual);
                    Serial.printf("SolicitarEstadoFechaHoraoTemporizacion->Hora recibida: %02d:%02d:%02d %02d/%02d/%02d\n", campanarioEstado.nHora, campanarioEstado.nMinutos, campanarioEstado.nSegundos, campanarioEstado.nDia, campanarioEstado.nMes, campanarioEstado.nAno);
                }        
            } else {
                if constexpr (Config::Debug::I2C_REQUEST_DEBUG) {
                    Serial.println("SolicitarEstadoFechaHoraoTemporizacion->No se recibió respuesta del campanario");
                }
            }                   
        }


    /**
     * @brief Reinicia el estado actual y asigna un nuevo estado anterior.
     * 
     * Esta función establece la variable nEstado a 0 y actualiza nEstadoAnterior
     * con el valor proporcionado en el parámetro nuevoEstado.
     * 
     * @param nuevoEstado Valor que se asignará a nEstadoAnterior.
     */
        void ReiniciarEstado(int nuevoEstado, int nPosicionMenu ) {
            nEstado = nPosicionMenu;
            nEstadoAnterior = nuevoEstado;
        }

    /**
     * @brief Envía orden al control del campanario y reinicia el estado.
     *
     * Esta función llama a EnviarEstado para enviar el estado proporcionado como argumento,
     * y posteriormente llama a ReiniciarEstado para reiniciar ese mismo estado.
     *
     * @param estado Valor del estado que se enviará y reiniciará.
     */
        void EnviarYReiniciar(int estado, int nPosicionMenu ) {
            EnviarEstado(estado);
            ReiniciarEstado(estado, nPosicionMenu);
        }
    //========================================================================================================
    //Manejador boton A
    //========================================================================================================
    /**
     * @brief Maneja la acción al presionar el botón A.
     *
     * Si está definido DEBUG, imprime un mensaje en el monitor serie indicando que el botón A fue presionado.
     * Si el sistema está en modo sleep, despierta el display y termina la función.
     * En caso contrario, procesa la acción correspondiente según el estado actual del menú.
     */
        void ManejarBotonA(void) {

            int estadoActual;

            if constexpr (Config::Debug::GENERAL_DEBUG) {
                Serial.println("Botón A presionado");
            }

            // Manejo del modo sleep
            if (lSleep) {
                DespertarDisplay();
                return;
            }

            // Procesar acción según el estado actual
            if constexpr (Config::Debug::BUTTON_DEBUG) {
                Serial.printf("ManejarBotonA->Estado Actual: %d\n", nEstado);
                Serial.print(menuActual[0]);
            }

            if (nEstado != EstadoSetTemporizador) {
                estadoActual = menuActual[nEstado];
            }else{
                estadoActual = EstadoSetTemporizador;                               // Si estamos en el estado de temporización, mantenemos ese estado
            }

            ProcesarAccionEstado(estadoActual);
        }

    /**
     * @brief Despierta el display del modo sleep
     */
        void DespertarDisplay(void) {
            lSleep = false;
            nContadorCiclosDisplaySleep = 0;
            if (M5.Display.getBrightness() < 100) BrilloFull();
            MostrarOpcion(menuActual[nEstado]);
        }

        /**
         * @brief Procesa la acción correspondiente según el estado actual del sistema.
         *
         * Esta función evalúa el estado actual recibido al pulsar el boton como parámetro y ejecuta la acción asociada,
         * como enviar órdenes específicas, cambiar banderas de estado, o configurar temporizadores.
         * Además, si está habilitado el modo de depuración (DEBUGBOTON), imprime información relevante
         * sobre el estado procesado a través del puerto serie.
         *
         * @param estadoActual Estado actual del sistema a procesar. Debe ser uno de los valores definidos
         *                     por las constantes de estado (por ejemplo, EstadoInicio, EstadoDifuntos, etc.).
         *
         * @note Algunas acciones pueden modificar la variable global lCambioEstado para indicar un cambio
         *       de estado relevante en el sistema.
         */
        void ProcesarAccionEstado(int estadoActual) 
        {
            if constexpr (Config::Debug::BUTTON_DEBUG) {
                Serial.printf("ProcesarAccionEstado->Estado Actual: %d\n", estadoActual);
            }

            switch (estadoActual) {
                case EstadoInicio:
                    if constexpr (Config::Debug::BUTTON_DEBUG) {
                        Serial.println("Estado Inicio");
                    }
                    break;

                case EstadoDifuntos:
                    if constexpr (Config::Debug::BUTTON_DEBUG) {
                        Serial.println("Estado Difuntos");
                    }
                    EnviarEstado(EstadoDifuntos);                       // Envía orden de Difuntos
                    lCambioEstado = true;
                    break;

                case EstadoMisa:
                    if constexpr (Config::Debug::BUTTON_DEBUG) {
                        Serial.println("Estado Misa");
                    }
                    EnviarEstado(EstadoMisa);                           // Envía orden de Misa
                    lCambioEstado = true;
                    break;

                case EstadoStop:
                    if constexpr (Config::Debug::BUTTON_DEBUG) {
                        Serial.println("Estado Stop");
                    }
                    EnviarEstado(EstadoStop);                           // Envía orden de Stop
                    lCambioEstado = true;
                    break;

                case EstadoCalefaccionOn:
                    if constexpr (Config::Debug::BUTTON_DEBUG) {
                        Serial.println("Estado Calefaccion On");
                    }
                    SetTemporizacion();
                    lCambioEstado = false;
                    break;

                case EstadoCalefaccionOff:
                    if constexpr (Config::Debug::BUTTON_DEBUG) {
                        Serial.println("Estado Calefaccion Off");
                    }
                    EnviarEstado(EstadoCalefaccionOff);                 // Envía orden de Calefacción Off
                    lCambioEstado = true;
                    break;

                case EstadoSetTemporizador:
                    //if constexpr (Config::Debug::BUTTON_DEBUG) {
                        Serial.println("Estado Set Temporizador");
                    //}
                    EnviarTemporizacion();
                    break;

                default:
                    lCambioEstado = true;
                    break;
            }
        }

    //========================================================================================================
    //Funciones de temporizacion de la calefacción
    //========================================================================================================

    /**
     * @brief Establece el estado de temporización del sistema.
     *
     * Esta función configura el sistema para iniciar el proceso de temporización,
     * mostrando la pantalla correspondiente y actualizando los estados internos.
     * Si está habilitado DEBUGBOTON, imprime información de depuración por el puerto serie.
     */
        void SetTemporizacion(void) 
        {
            MensajeTemporizacion(nTemporizacion, true);
            nEstado = EstadoSetTemporizador;
            nEstadoAnterior = EstadoSetTemporizador;
            lCambioEstado = false;

            if constexpr (Config::Debug::BUTTON_DEBUG) {
                Serial.println("SetTemporizacion->Estado Set Temporizador iniciado");
                Serial.printf("SetTemporizacion->nEstado: %d, nEstadoAnterior: %d\n", nEstado, nEstadoAnterior);
            }
        }

    /**
     * @brief Confirma la configuración del temporizador envia el estado Calefaccio On y la temporizacion.
     *
     * Envía el estado de temporización junto con el valor actual de temporización,
     * cambia el estado al de calefacción encendida y actualiza el estado anterior
     * al de configuración de temporizador.
     */
        void EnviarTemporizacion(void)
         {
            EnviarEstadoConParametro(EstadoSetTemporizador, nTemporizacion); // Envía el estado de temporización con el valor actual
            nEstado = EstadoCalefaccionOn; // Cambia al estado de calefacción encendida
            nEstadoAnterior = EstadoSetTemporizador; // Actualiza el estado anterior al de
            if constexpr (Config::Debug::BUTTON_DEBUG) {
                Serial.printf("EnviarTemporizacion->Estado Set Temporizador enviado con valor: %d\n", nTemporizacion);
                Serial.printf("EnviarTemporizacion->Nuevo Estado: %d, Estado Anterior: %d\n", nEstado, nEstadoAnterior);
            }
        }

        void ManejarEncoder(void) {
            nPosicionActual = M5Dial.Encoder.read();
            if (nPosicionActual != nPosicionAneterior) {
                if (nEstado == EstadoSetTemporizador) {                     // Manejo del encoder en modo de temporización
                M5Dial.Speaker.tone(8000, 20);
                if (nPosicionActual > nPosicionAneterior) {
                    nTemporizacion++;
                    if (nTemporizacion > 60) nTemporizacion = 60; // Limita el temporizador a 60 minutos
                } else {
                    nTemporizacion--;
                    if (nTemporizacion < 1) nTemporizacion = 1; // Limita el temporizador a 1 minuto
                }
                MensajeTemporizacion(nTemporizacion, false);
                lCambioEstado = false;
                nPosicionAneterior = nPosicionActual;
            }else{                                                          // Manejo del encoder en otros estados
                M5Dial.Speaker.tone(8000, 20);
                if (nPosicionActual > nPosicionAneterior) {
                    if (nEstado < nMenuItems - 1) {
                        nEstado++;
                    } else {
                        nEstado = nMenuItems - 1;
                    }
                } else {
                    if (nEstado > 0) {
                        nEstado--;
                    } else {
                        nEstado = 0;
                    }
                }
                if constexpr (Config::Debug::GENERAL_DEBUG) {
                    Serial.printf("Nueva Posicion Actual: %d, Nuevo Estado Actual: %d\n", nPosicionActual, nEstado);
                }
                lCambioEstado = true;
                nPosicionAneterior = nPosicionActual;
            }
        }
    }

    /**
     * @brief Actualiza el estado de brillo y sueño del display.
     *
     * Esta función gestiona el ciclo de sueño del display. Si el brillo está activo y el display no está en modo sueño,
     * incrementa el contador de ciclos y, al alcanzar el límite, reduce el brillo y activa el modo sueño.
     * Si el brillo no está activo, muestra la fecha y hora y aumenta el brillo. Si el brillo está activo,
     * actualiza la fecha y hora en el display.
     *
     * En modo depuración (DEBUG), imprime un mensaje indicando que el display está activo y mostrando la hora.
     */
    void ActualizarDisplaySleep(void) 
    {
        if (lBrillo && !lSleep) {
            nContadorCiclosDisplaySleep++;
            if (nContadorCiclosDisplaySleep >= nCiclosDisplaySleep) {
                BajaBrillo();
                nContadorCiclosDisplaySleep = 0;
                lSleep = true;
            }
        } else {
            if (!lBrillo) {
                if (nEstadoActual & (1 << 4)) {
                    MostrarCalefaccionTemporizada(campanarioEstado.nHora, campanarioEstado.nMinutos, campanarioEstado.nSegundos, true);
                }else{
                    MensajeFechaHora(campanarioEstado.nHora, campanarioEstado.nMinutos, campanarioEstado.nSegundos, campanarioEstado.nDia, campanarioEstado.nMes, campanarioEstado.nAno);
                }
                SubeBrillo(40);
            } else {
                if (nEstadoActual & (1 << 4)) {
                    MostrarCalefaccionTemporizada(campanarioEstado.nHora, campanarioEstado.nMinutos, campanarioEstado.nSegundos, false);
                }else{
                    MensajeFechaHora(campanarioEstado.nHora, campanarioEstado.nMinutos, campanarioEstado.nSegundos, campanarioEstado.nDia, campanarioEstado.nMes, campanarioEstado.nAno);
                }
            }
            if constexpr (Config::Debug::GENERAL_DEBUG) {
                Serial.println("Display activo, se presenta la hora");
            }
                
        }
    }
#endif

