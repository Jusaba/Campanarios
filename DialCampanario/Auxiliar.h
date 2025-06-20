#ifndef DIAL_H
    #define DIAL_H

    #include <Wire.h>
    #include <M5Dial.h>

    #define I2C_SLAVE_ADDR 0x12

    #define Color_Fondo         0x0000
    #define Color_Info          0xffff
    #define Color_Voltios       0xffff
    #define Color_Seleccion     TFT_ORANGE

    #define bitEstadoInicio            0x00
    #define bitEstadoDifuntos          0x01
    #define bitEstadoMisa              0x02
    #define bitEstadoStop              0x04
    #define bitEstadoCalefaccionOn     0x10
    #define bitEstadoCalefaccionOff    0x20


    #define SLAVE_ADDRESS 0x08

    #define nEstados                7

    #define nmsGetEstadoCampanario  500 // Tiempo en milisegundos para solicitar el estado del campanario 
    


// Define los estados
enum Estados {
    EstadoInicio = 0,
    EstadoDifuntos,
    EstadoMisa,
    EstadoStop,
    EstadoCalefaccionOn,
    EstadoCalefaccionOff
};

// Variables globales
bool modoMenuReducido = false;
const int menuCompleto[] = {EstadoDifuntos, EstadoMisa, EstadoCalefaccionOn, EstadoCalefaccionOff};
const int menuReducido[] = {EstadoStop, EstadoCalefaccionOn, EstadoCalefaccionOff};


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




int nMenuItems = 4;                 // Cambia según el menú activo
int menuIndex = 0;                  // Índice del menú actual

long nMilisegundoTemporal = 0;      // Variable para almacenar el tiempo temporal



    int nEstado = EstadoInicio;                 // Estado inicial del sistema
    uint8_t nEstadoAnterior = EstadoInicio; // Estado anterior del menú
    uint8_t nEstadoActual = EstadoInicio; // Estado actual del menú
    int nPosicionAneterior = 0;         // Estado anterior del sistema
    int nPosicionActual = 0;           // Estado actual del sistema
    bool lCambioEstado = false;  // Indica si ha habido un cambio de estado

    void ClearPantalla (void);
    void InicioDisplay (void);
    void MensajeInicio (void);
    void MensajeDifuntos (void);
    void MensajeMisa (void);
    void MensajeStop (void );
    void MensajeCalefaccionOn (void);
    void MensajeCalefaccionOff (void);
    void EnviarEstado(int nEstadoMenu);
    void SolicitarEstadoCampanario(void);
    void MostrarMenu(const int* menu, int nItems, int seleccionado);
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
    }        

    

    /**
* @brief Limpia completamente la pantalla del dispositivo M5Dial.
* 
* @details Esta función borra todo el contenido de la pantalla rellenándola
*          con el color de fondo definido en Color_Fondo.
* 
* @note Utiliza la biblioteca M5Dial para acceder a las funciones de display.
*/
void ClearPantalla (void)
{
    M5Dial.Display.fillScreen(Color_Fondo);
}


void MostrarMenu(int seleccionado) {
    
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
* @brief Inicializa y dibuja la pantalla de inicio del dispositivo M5Dial
* 
* @details Esta función configura la pantalla inicial del dispositivo M5Dial:
*          - Un fondo negro
*          - Dos círculos concéntricos (amarillo y magenta)
*          - El texto "Campanario" en el centro con fuente Orbitron Light
*          - La firma "Jusaba" en la parte inferior con fuente FreeSans
* 
* La función utiliza la librería M5Dial para realizar todas las operaciones
* de dibujo en la pantalla.
*/
void MensajeInicio (void)
{
    ClearPantalla();
    M5Dial.Display.startWrite();    
    M5Dial.Display.fillScreen(TFT_BLACK);
    M5Dial.Display.setTextColor(TFT_WHITE, TFT_BLACK);
    M5Dial.Display.drawCircle ( M5Dial.Display.width() / 2, ( M5Dial.Display.height()  / 2) - 10, 70, TFT_YELLOW);
    M5Dial.Display.drawCircle ( M5Dial.Display.width() / 2, ( M5Dial.Display.height()  / 2) - 10, 50, TFT_MAGENTA);
    M5Dial.Display.setTextDatum(middle_center);
    M5Dial.Display.setTextFont(&fonts::Orbitron_Light_32);
    M5Dial.Display.setTextSize(1);
    M5Dial.Display.drawString(String("Rodador"), M5Dial.Display.width() / 2, ( M5Dial.Display.height()  / 2) - 10);
    M5Dial.Display.setTextColor(TFT_YELLOW, TFT_BLACK);
    M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);   
    M5Dial.Display.setTextSize(2);
    M5Dial.Display.drawString(String("by Jusaba"), M5Dial.Display.width() / 2, ( M5Dial.Display.height()  / 2) + 80);
}

void MensajeDifuntos (void)
{
    ClearPantalla();
    M5Dial.Display.setTextColor(Color_Info, Color_Fondo);
    M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
    M5Dial.Display.setTextSize(2);
    M5Dial.Display.drawString("Difuntos", M5Dial.Display.width() / 2, M5Dial.Display.height() / 2);
}

void MensajeMisa (void)
{
    ClearPantalla();
    M5Dial.Display.setTextColor(Color_Info, Color_Fondo);
    M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
    M5Dial.Display.setTextSize(2);
    M5Dial.Display.drawString("Misa", M5Dial.Display.width() / 2, M5Dial.Display.height() / 2);
}

void MensajeStop (void )
{
    ClearPantalla();
    M5Dial.Display.setTextColor(Color_Info, Color_Fondo);
    M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
    M5Dial.Display.setTextSize(2);
    M5Dial.Display.drawString("STOP", M5Dial.Display.width() / 2, (M5Dial.Display.height() / 2) - 15 );


    /*
    switch ( nSecuenciaOn)  // Cambia el mensaje según la secuencia
    {
        case EstadoDifuntos:
            M5Dial.Display.drawString("Difuntos", M5Dial.Display.width() / 2, (M5Dial.Display.height() / 2) + 15);
            break;
        case EstadoMisa:
            M5Dial.Display.drawString("Misa", M5Dial.Display.width() / 2, (M5Dial.Display.height() / 2) + 15);
            break;
    default:
        break;
    }
*/
} 



void MensajeCalefaccionOn (void)
{
    ClearPantalla();
    M5Dial.Display.setTextColor(Color_Info, Color_Fondo);
    M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
    M5Dial.Display.setTextSize(2);
    M5Dial.Display.drawString("Calefaccion", M5Dial.Display.width() / 2, (M5Dial.Display.height() / 2) - 20);
    M5Dial.Display.drawString("ON", M5Dial.Display.width() / 2, (M5Dial.Display.height() / 2)  + 30);
}
void MensajeCalefaccionOff (void)
{
    ClearPantalla();
    M5Dial.Display.setTextColor(Color_Info, Color_Fondo);
    M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
    M5Dial.Display.setTextSize(2);
    M5Dial.Display.drawString("Calefaccion", M5Dial.Display.width() / 2, (M5Dial.Display.height() / 2) - 20);
    M5Dial.Display.drawString("OFF", M5Dial.Display.width() / 2, (M5Dial.Display.height() / 2)  + 30);

}
void EnviarEstado(int nEstadoMenu) {
    Wire.beginTransmission(I2C_SLAVE_ADDR);
    Wire.write(nEstadoMenu); 
    uint8_t result = Wire.endTransmission();
    if (result == 0) {
        Serial.printf("Estado %d enviado correctamente al esclavo I2C\n", nEstadoMenu);
    } else {
        Serial.printf("Error al enviar el estado %d al esclavo I2C, código de error: %d\n", nEstadoMenu, result);
    }
}

void SolicitarEstadoCampanario() {
    Wire.requestFrom(I2C_SLAVE_ADDR, 1); // Solicita 1 byte al esclavo
    if (Wire.available()) {
        nEstadoActual = Wire.read();    
        if (nEstadoActual != nEstadoAnterior) {
            nEstadoAnterior = nEstadoActual;
            Serial.printf("Nuevo Estado actual del campanario recibido: %d\n", nEstadoActual);
            SeleccionaMenu(nEstadoActual); // Llama a la función para seleccionar el menú según el estado recibido
            nEstado = 0; // Reinicia el estado al primer elemento del menú
            lCambioEstado = true; // Marca que ha habido un cambio de estado
        } 
    } else {
            Serial.println("No se recibió respuesta del campanario");
    }    
}

void SeleccionaMenu(int nEstadoSeleccionado) {
    Serial.println("Seleccionando menú para el estado: " + String(nEstadoSeleccionado, BIN));
    
    if ((nEstadoSeleccionado & (bitEstadoDifuntos | bitEstadoMisa)) && !(nEstadoSeleccionado & bitEstadoCalefaccionOn)) {
        Serial.println("Menu2");
        nEstadoMenu = 2; // Cambia al menú 2
    } else if ((nEstadoSeleccionado & (bitEstadoDifuntos | bitEstadoMisa)) && (nEstadoSeleccionado & bitEstadoCalefaccionOn)) {
        Serial.println("Menu3");
        nEstadoMenu = 3; // Cambia al menú 3
    } else if (!(nEstadoSeleccionado & (bitEstadoDifuntos | bitEstadoMisa)) && !(nEstadoSeleccionado & bitEstadoCalefaccionOn)) {
        Serial.println("Menu0");
        nEstadoMenu = 0; // Cambia al menú 0
    } else if (!(nEstadoSeleccionado & (bitEstadoDifuntos | bitEstadoMisa)) && (nEstadoSeleccionado & bitEstadoCalefaccionOn)) {
        Serial.println("Menu1");
        nEstadoMenu = 1; // Cambia al menú 1
    }
}
#endif

