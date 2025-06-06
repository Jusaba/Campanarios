#ifndef DIAL_H
    #define DIAL_H

    #include <Wire.h>
    #include <M5Dial.h>

    #define I2C_SLAVE_ADDR 0x12

    #define Color_Fondo         0x0000
    #define Color_Info          0xffff
    #define Color_Voltios       0xffff
    #define Color_Seleccion     TFT_ORANGE


    #define SLAVE_ADDRESS 0x08

    #define nEstados                7

    /*
    #define EstadoInicio            0
    #define EstadoDifuntos          1
    #define EstadoFiesta            2
    #define EstadoMisa              3
    #define EstadoStop              4
    #define EstadoCalefaccionOn     5
    #define EstadoCalefaccionOff    6
    */


// Define los estados
enum Estados {
    EstadoInicio = 0,
    EstadoDifuntos,
    EstadoFiesta,
    EstadoMisa,
    EstadoStop,
    EstadoCalefaccionOn,
    EstadoCalefaccionOff
};

// Variables globales
bool modoMenuReducido = false;
const int menuCompleto[] = {EstadoDifuntos, EstadoFiesta, EstadoMisa, EstadoCalefaccionOn, EstadoCalefaccionOff};
const int menuReducido[] = {EstadoStop, EstadoCalefaccionOn, EstadoCalefaccionOff};

int nMenuItems = 5; // Cambia según el menú activo

// Índice del menú actual
int menuIndex = 0;


    int nEstado = EstadoInicio;                 // Estado inicial del sistema
    int nPosicionAneterior = 0;         // Estado anterior del sistema
    int nPosicionActual = 0;           // Estado actual del sistema
    int nEstadoAnterior = EstadoInicio; // Estado anterior del menú
    bool lCambioEstado = false;  // Indica si ha habido un cambio de estado

    void ClearPantalla (void);
    void InicioDisplay (void);
    void MensajeInicio (void);
    void MensajeDifuntos (void);
    void MensajeMisa (void);
    void MensajeFiesta (void);
    void MensajeStop (void);
    void MensajeCalefaccionOn (void);
    void MensajeCalefaccionOff (void);
    void EnviarEstado(int nEstadoMenu);
    void MostrarMenu(const int* menu, int nItems, int seleccionado);


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
            case EstadoFiesta:          
                MensajeFiesta(); 
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
void MensajeFiesta (void)
{
    ClearPantalla();
    M5Dial.Display.setTextColor(Color_Info, Color_Fondo);
    M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
    M5Dial.Display.setTextSize(2);
    M5Dial.Display.drawString("Fiesta", M5Dial.Display.width() / 2, M5Dial.Display.height() / 2);
}
void MensajeStop (void)
{
    ClearPantalla();
    M5Dial.Display.setTextColor(Color_Info, Color_Fondo);
    M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
    M5Dial.Display.setTextSize(2);
    M5Dial.Display.drawString("STOP", M5Dial.Display.width() / 2, M5Dial.Display.height() / 2);
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
    Wire.write(nEstadoMenu); // 0=Parar, 1=Difuntos, 2=Fiesta, 3=Misa
    Wire.endTransmission();
}

#endif

