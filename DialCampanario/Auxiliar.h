//Tratamiento de iamgenes  ( redimensionado ) https://www.photopea.com/
#ifndef DIAL_H
    #define DIAL_H

    #define DEBUG
    #define DEBUGBOTON
    #define DEBUGAUXILIAR
    #define DEBUGI2CTX
    #define DEBUGI2CREQUEST
    #define DEBUGI2CRX
    #define DEBUGI2CREQUEST
    #define DEBUGMENU

    #include <Wire.h>
    #include "Display.h"
    #include <M5Dial.h>


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
           EstadoSinInternet
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

int nEstadoMenu = 0;



struct CampanarioEstado                                     //Estructura para almacenar el estado del campanario
{
    uint8_t nEstado;                                        // Estado del campanario
    uint8_t nHora;                                          // Hora actual
    uint8_t nMinutos;                                       // Minutos actuales
    uint8_t nSegundos;                                      // Segundos actuales
};
CampanarioEstado campanarioEstado;                          // Estructura para almacenar el estado del campanario

int nMenuItems = 4;                 // Cambia según el menú activo
int menuIndex = 0;                  // Índice del menú actual

long nMilisegundoTemporal = 0;                              // Variable para almacenar el tiempo temporal
int nContadorCiclosDisplaySleep = 0;                        // Contador de ciclos de sleep del display



    int nEstado = EstadoInicio;                             // Estado inicial del sistema
    uint8_t nEstadoAnterior = EstadoInicio;                 // Estado anterior del menú
    uint8_t nEstadoActual = EstadoInicio;                   // Estado actual del menú
    int nPosicionAneterior = 0;                             // Estado anterior del sistema
    int nPosicionActual = 0;                                // Estado actual del sistema
    bool lCambioEstado = false;                             // Indica si ha habido un cambio de estado
    bool lSleep = false;                                    // Indica si el display está en modo sleep
    bool lInternet = false;                                 // Indica si no hay conexión a Internet

    void InicioDisplay (void);
    void MostrarOpcion(int seleccionado);
    void EnviarEstado(int nEstadoMenu);
     void SolicitarEstadoCampanario(void);
    void SolicitarEstadoHora(void); // Solicita la hora al esclavo I2C
    void SeleccionaMenu(int nEstadoSeleccionado);
    
    void ReiniciarEstado(int nuevoEstado);
    void EnviarYReiniciar(int estado);
    void ManejarBotonA(void);
    void ManejarEncoder(void);
    void ActualizarDisplaySleep(void);



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
         */
        void MostrarOpcion(int seleccionado) 
        {
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
            }
        }

/**
 * @brief Envía el estado actual del menú a un dispositivo esclavo a través del bus I2C.
 *
 * Esta función inicia una transmisión I2C con la dirección definida por I2C_SLAVE_ADDR,
 * envía el valor del estado del menú especificado por nEstadoMenu y finaliza la transmisión.
 * Informa por el puerto serie si la transmisión fue exitosa o si ocurrió un error.
 *
 * @param nEstadoMenu Valor entero que representa el estado actual del menú a enviar al esclavo I2C.
 */
void EnviarEstado(int nEstadoMenu) {
    Wire.beginTransmission(I2C_SLAVE_ADDR);
    Wire.write(nEstadoMenu); 
    uint8_t result = Wire.endTransmission();
    #ifdef DEBUGI2CTX
        if (result == 0) {
            Serial.printf("EnviarEstado->Estado %d enviado correctamente al esclavo I2C\n", nEstadoMenu);
        } else {
            Serial.printf("EnviarEstado->Error al enviar el estado %d al esclavo I2C, código de error: %d\n", nEstadoMenu, result);
        }
    #endif
}

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
    
    #ifdef DEBUGI2CREQUEST       
        Serial.println("SolicitarEstadoCampanario->Solicitando estado del campanario...");
    #endif 

    Wire.beginTransmission(I2C_SLAVE_ADDR);
    Wire.write(EstadoCampanario); 
    Wire.endTransmission();

    Wire.requestFrom(I2C_SLAVE_ADDR, 1); // Solicita 1 byte al esclavo
    if (Wire.available()) {
        nEstadoActual = Wire.read();    

        if (nEstadoActual != nEstadoAnterior) {
            #ifdef DEBUGI2CREQUEST
                Serial.printf("SolicitarEstadoCampanario->Cambio de estado detectado: %d -> %d\n", nEstadoAnterior, nEstadoActual); 
            #endif
            nEstadoAnterior = nEstadoActual;
            SeleccionaMenu(nEstadoActual); // Llama a la función para seleccionar el menú según el estado recibido
            nEstado = 0; // Reinicia el estado al primer elemento del menú
            lCambioEstado = true; // Marca que ha habido un cambio de estado
        } 
    } else {
        #ifdef DEBUGI2CREQUEST
            Serial.println("SolicitarEstadoCampanario->No se recibió respuesta del campanario");
        #endif
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
    #ifdef DEBUGI2CREQUEST 
        Serial.println("SolicitarEstadoHora->Solicitando hora del campanario...");
    #endif

    // Solicita la hora al esclavo I2C
    Wire.beginTransmission(I2C_SLAVE_ADDR);
    Wire.write(EstadoHora); 
    Wire.endTransmission();

    Wire.requestFrom(I2C_SLAVE_ADDR, 4); // Solicita 4 bytes al esclavo

    if (Wire.available() >= 4) {
        nEstadoActual = Wire.read();
        
        lInternet = (nEstadoActual & bitEstadoSinInternet) ? false : true; // Si el bit bitEstadoSinInternet está activo, no hay conexión a Internet
        campanarioEstado.nHora = Wire.read();
        campanarioEstado.nMinutos = Wire.read();
        campanarioEstado.nSegundos = Wire.read(); // Lee los segundos del esclavo I2C
        campanarioEstado.nEstado = nEstadoActual; // Actualiza el estado del campanario
        if (nEstadoActual != nEstadoAnterior) {
            #ifdef DEBUGI2CREQUEST
                Serial.printf("SolicitarEstadoHora->Cambio de estado detectado: %d -> %d\n", nEstadoAnterior, nEstadoActual); 
            #endif
            nEstadoAnterior = nEstadoActual;
            SeleccionaMenu(nEstadoActual); // Llama a la función para seleccionar el menú según el estado recibido
            nEstado = 0; // Reinicia el estado al primer elemento del menú
            lCambioEstado = true; // Marca que ha habido un cambio de estado
        } 
        #ifdef DEBUGI2CRX
            Serial.printf("SolicitarEstadoHora->Estado actual del campanario: %d\n", nEstadoActual);
            Serial.printf("SolicitarEstadoHora->Hora recibida: %02d:%02d:%02d\n", campanarioEstado.nHora, campanarioEstado.nMinutos, campanarioEstado.nSegundos);
        #endif        
    } else {
        #ifdef DEBUGI2CREQUEST
            Serial.println("SolicitarEstadoHora->No se recibió respuesta del campanario");
        #endif
    }            
    
}


/**
 * @brief Selecciona el estado del menú según el estado seleccionado.
 *
 * Esta función evalúa el valor de nEstadoSeleccionado utilizando máscaras de bits
 * (bitEstadoDifuntos, bitEstadoMisa y bitEstadoCalefaccionOn) para determinar el estado
 * actual del menú. Asigna un valor a nEstadoMenu según las siguientes condiciones:
 *   - Si está activo Difuntos o Misa y la calefacción está encendida, nEstadoMenu = 3.
 *   - Si está activo Difuntos o Misa y la calefacción está apagada, nEstadoMenu = 2.
 *   - Si no está activo Difuntos ni Misa pero la calefacción está encendida, nEstadoMenu = 1.
 *   - Si no está activo Difuntos ni Misa ni la calefacción, nEstadoMenu = 0.
 *
 * @param nEstadoSeleccionado Valor entero que representa el estado actual mediante bits.
 */
void SeleccionaMenu(int nEstadoSeleccionado) 
{
    bool difuntosMisa = nEstadoSeleccionado & (bitEstadoDifuntos | bitEstadoMisa);
    bool calefOn = nEstadoSeleccionado & bitEstadoCalefaccionOn;
    nEstadoMenu = difuntosMisa ? (calefOn ? 3 : 2) : (calefOn ? 1 : 0);
}


/**
 * @brief Reinicia el estado actual y asigna un nuevo estado anterior.
 * 
 * Esta función establece la variable nEstado a 0 y actualiza nEstadoAnterior
 * con el valor proporcionado en el parámetro nuevoEstado.
 * 
 * @param nuevoEstado Valor que se asignará a nEstadoAnterior.
 */
void ReiniciarEstado(int nuevoEstado) {
    nEstado = 0;
    nEstadoAnterior = nuevoEstado;
}

/**
 * @brief Envía el estado especificado y luego reinicia dicho estado.
 *
 * Esta función llama a EnviarEstado para enviar el estado proporcionado como argumento,
 * y posteriormente llama a ReiniciarEstado para reiniciar ese mismo estado.
 *
 * @param estado Valor del estado que se enviará y reiniciará.
 */
void EnviarYReiniciar(int estado) {
    EnviarEstado(estado);
    ReiniciarEstado(estado);
}

void ManejarBotonA(void) {
    #ifdef DEBUG
        Serial.println("Botón A presionado");
    #endif
    if (lSleep) {
        lSleep = false;
        nContadorCiclosDisplaySleep = 0;
        if (M5.Display.getBrightness() < 100) BrilloFull();
        MostrarOpcion(menuActual[nEstado]);
    } else {
        int estadoActual = menuActual[nEstado];
        #ifdef DEBUGBOTON
            Serial.printf("Estado Actual: %d\n", estadoActual);
        #endif
        switch (estadoActual) {
            case EstadoInicio:
                #ifdef DEBUGBOTON
                    Serial.println("Estado Inicio");
                #endif
                break;
            case EstadoDifuntos:
                #ifdef DEBUGBOTON
                    Serial.println("Estado Difuntos");
                #endif
                EnviarYReiniciar(EstadoDifuntos);
                break;
            case EstadoMisa:
                #ifdef DEBUGBOTON
                    Serial.println("Estado Misa");
                #endif
                EnviarYReiniciar(EstadoMisa);
                break;
            case EstadoStop:
                #ifdef DEBUGBOTON
                    Serial.println("Estado Stop");
                #endif
                EnviarEstado(EstadoStop);
                nEstado = (nEstadoAnterior > 0) ? nEstadoAnterior - 1 : 0;
                break;
            case EstadoCalefaccionOn:
                #ifdef DEBUGBOTON
                    Serial.println("Estado Calefaccion On");
                #endif
                EnviarYReiniciar(EstadoCalefaccionOn);
                break;
            case EstadoCalefaccionOff:
                #ifdef DEBUGBOTON
                    Serial.println("Estado Calefaccion Off");
                #endif
                EnviarYReiniciar(EstadoCalefaccionOff);
                break;
            default:
                break;
        }
        lCambioEstado = true;
    }
}
void ManejarEncoder(void) {
    nPosicionActual = M5Dial.Encoder.read();
    if (nPosicionActual != nPosicionAneterior) {
        M5Dial.Speaker.tone(8000, 20);
        if (nPosicionActual > nPosicionAneterior) {
            if (nEstado < nMenuItems - 1) nEstado++;
            else nEstado = nMenuItems - 1;
        } else {
            if (nEstado > 0) nEstado--;
            else nEstado = 0;
        }
        #ifdef DEBUG
            Serial.printf("Nueva Posicion Actual: %d, Nuevo Estado Actual: %d\n", nPosicionActual, nEstado);
        #endif
        lCambioEstado = true;
        nPosicionAneterior = nPosicionActual;
    }
}
void ActualizarDisplaySleep(void) {
    if (lBrillo && !lSleep) {
        nContadorCiclosDisplaySleep++;
        if (nContadorCiclosDisplaySleep >= nCiclosDisplaySleep) {
            BajaBrillo();
            nContadorCiclosDisplaySleep = 0;
            lSleep = true;
        }
    } else {
        if (!lBrillo) {
            MensajeHora(campanarioEstado.nHora, campanarioEstado.nMinutos, campanarioEstado.nSegundos);
            SubeBrillo(40);
        } else {
            EscribeHora(campanarioEstado.nHora, campanarioEstado.nMinutos, campanarioEstado.nSegundos);
        }
        #ifdef DEBUG
            Serial.println("Display activo, se presenta la hora");
        #endif
        
    }
}
#endif

