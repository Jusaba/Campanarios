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

    
    #define bitEstadoInicio            0x00
    #define bitEstadoDifuntos          0x01
    #define bitEstadoMisa              0x02
    #define bitEstadoStop              0x04
    #define bitEstadoCalefaccionOn     0x10
    #define bitEstadoCalefaccionOff    0x20
    #define bitEstadoCampanario        0x40
    #define bitEstadoHora              0x80


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
           EstadoHora
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



struct CampanarioEstado
{
    uint8_t nEstado;   // Estado del campanario
    uint8_t nHora;     // Hora actual
    uint8_t nMinutos;  // Minutos actuales
    uint8_t nSegundos; // Segundos actuales
};
CampanarioEstado campanarioEstado; // Estructura para almacenar el estado del campanario

int nMenuItems = 4;                 // Cambia según el menú activo
int menuIndex = 0;                  // Índice del menú actual

long nMilisegundoTemporal = 0;      // Variable para almacenar el tiempo temporal
int nContadorCiclosDisplaySleep = 0;        // Contador de ciclos de sleep del display



    int nEstado = EstadoInicio;                 // Estado inicial del sistema
    uint8_t nEstadoAnterior = EstadoInicio; // Estado anterior del menú
    uint8_t nEstadoActual = EstadoInicio; // Estado actual del menú
    int nPosicionAneterior = 0;         // Estado anterior del sistema
    int nPosicionActual = 0;           // Estado actual del sistema
    bool lCambioEstado = false;  // Indica si ha habido un cambio de estado
    bool lSleep = false; // Indica si el display está en modo sleep

    void InicioDisplay (void);
    void MostrarOpcion(int seleccionado);
    void EnviarEstado(int nEstadoMenu);
    void SolicitarEstadoCampanario(void);
    void SolicitarEstadoHora(void); // Solicita la hora al esclavo I2C
    void SeleccionaMenu(int nEstadoSeleccionado);
    





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

void SolicitarEstadoCampanario() {
    
    #ifdef DEBUGI2CREQUEST       
        Serial.println("SolicitarEstadoCampanario->Solicitando estado del campanario...");
    #endif 

    Wire.beginTransmission(I2C_SLAVE_ADDR);
    Wire.write(EstadoCampanario); 
    Wire.endTransmission();
    #ifdef DEBUGI2CTX
        Serial.println("SolicitarEstadoCampanario->Solicitud de estado enviada al campanario");   
    #endif 

    Wire.requestFrom(I2C_SLAVE_ADDR, 1); // Solicita 1 byte al esclavo
    if (Wire.available()) {
        nEstadoActual = Wire.read();    
        #ifdef DEBUGI2CREQUEST
            Serial.printf("SolicitarEstadoCampanario->Estado actual del campanario: %d\n", nEstadoActual);
        #endif

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
void SolicitarEstadoHora() {
    #ifdef DEBUGI2CREQUEST 
        Serial.println("SolicitarEstadoHora->Solicitando hora del campanario...");
    #endif

    // Solicita la hora al esclavo I2C
    Wire.beginTransmission(I2C_SLAVE_ADDR);
    Wire.write(EstadoHora); 
    Wire.endTransmission();

    Wire.requestFrom(I2C_SLAVE_ADDR, 4); // Solicita 2 bytes: hora y minutos

    if (Wire.available() >= 4) {
        nEstadoActual = Wire.read();
        campanarioEstado.nHora = Wire.read();
        campanarioEstado.nMinutos = Wire.read();
        campanarioEstado.nSegundos = Wire.read(); // Lee los segundos del esclavo I2C
        campanarioEstado.nEstado = nEstadoActual; // Actualiza el estado del campanario
        if (nEstadoActual != nEstadoAnterior) {
            #ifdef DEBUGI2CREQUEST
                Serial.printf("SolicitarEstadoCampanario->Cambio de estado detectado: %d -> %d\n", nEstadoAnterior, nEstadoActual); 
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
            Serial.println("SolicitarEstadoCampanario->No se recibió respuesta del campanario");
        #endif
    }            


    
}

void SeleccionaMenu(int nEstadoSeleccionado) {
    #ifdef DEBUGMENU
        Serial.println("Seleccionando menú según el estado recibido...");
    #endif
    
    if ((nEstadoSeleccionado & (bitEstadoDifuntos | bitEstadoMisa)) && !(nEstadoSeleccionado & bitEstadoCalefaccionOn)) {
        #ifdef DEBUGMENU
            Serial.println("Menu2");
        #endif
        nEstadoMenu = 2; // Cambia al menú 2
    } else if ((nEstadoSeleccionado & (bitEstadoDifuntos | bitEstadoMisa)) && (nEstadoSeleccionado & bitEstadoCalefaccionOn)) {
        #ifdef DEBUGMENU
            Serial.println("Menu3");
        #endif
        nEstadoMenu = 3; // Cambia al menú 3
    } else if (!(nEstadoSeleccionado & (bitEstadoDifuntos | bitEstadoMisa)) && !(nEstadoSeleccionado & bitEstadoCalefaccionOn)) {
        #ifdef DEBUGMENU
            Serial.println("Menu0");
        #endif
        nEstadoMenu = 0; // Cambia al menú 0
    } else if (!(nEstadoSeleccionado & (bitEstadoDifuntos | bitEstadoMisa)) && (nEstadoSeleccionado & bitEstadoCalefaccionOn)) {
        #ifdef DEBUGMENU
            Serial.println("Menu1");
        #endif
        nEstadoMenu = 1; // Cambia al menú 1
    }
}


            
#endif

