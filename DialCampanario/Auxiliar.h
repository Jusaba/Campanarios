//Tratamiento de iamgenes  ( redimensionado ) https://www.photopea.com/
#ifndef DIAL_H
    #define DIAL_H

    #include <Wire.h>
    #include <M5Dial.h>

    #include "Imagen.h"  // Incluye las imágenes de los iconos y sprites

    #define I2C_SLAVE_ADDR 0x12

    #define Color_Fondo         0x0000
    #define Color_Info          0xffff
    #define Color_Voltios       0xffff
    #define Color_Seleccion     TFT_ORANGE
    #define Color_Transparente  0x0002

    #define iStop  0                        //!<Posicion del array de sprites con el icono de Stop
    #define iDifuntos  1                    //!<Posicion del array de sprites con el icono de Difuntos
    #define iMisa  2                        //!<Posicion del array de sprites con el icono de Misa
    #define iCalefaccionOn  3               //!<Posicion del array de sprites con el icono de Calefaccion On
    #define iCalefaccionOff  4              //!<Posicion del array de sprites con el icono de Calefaccion Off

    #define IconoStop    Stop               //!<Nombre del icono termometro del menu en Imagen.h
    #define IconoDifuntos Difuntos          //!<Nombre del icono de difuntos del menu en Imagen.h
    #define IconoMisa Misa                  //!<Nombre del icono de misa del menu en Imagen.h
    #define IconoCalefaccionOn CalefaccionOn //!<Nombre del icono de calefaccion On del menu en Imagen.h
    #define IconoCalefaccionOff CalefaccionOff //!<Nombre del icono de

    #define bitEstadoInicio            0x00
    #define bitEstadoDifuntos          0x01
    #define bitEstadoMisa              0x02
    #define bitEstadoStop              0x04
    #define bitEstadoCalefaccionOn     0x10
    #define bitEstadoCalefaccionOff    0x20


    #define SLAVE_ADDRESS 0x08

    #define nEstados                7

    #define nmsGetEstadoCampanario  500 // Tiempo en milisegundos para solicitar el estado del campanario 
    

    M5Canvas* aSprites[5];             //Array de Sprites de Menu



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

    boolean lBrillo = 1;                                                        //Indica si hay brillo (1)

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
    
    void BajaBrillo (void);                                                                     //Baja el brillo hasta 0 de forma paulatina
    void SubeBrillo (int nBrilloFinal );                                                        //Sube el brillo hasta nBrilloFinal de forma paulatina
    void BrilloFull (void);                                                                     //Sube el brillo al maximo


    void CreaSpritesMenu (void);                                                                //Crea todos los Sprites del menu ( On, Off, Consigna, Automatico....)
    void CreaSpriteMenu ( M5Canvas*& sprite, const unsigned char* image );                      //Crea un Sprite de Menu. Esta funcion es llamada desde CreaSpritesMenu()



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

/**
* @brief Reduce gradualmente el brillo de la pantalla hasta apagarlo
* 
* @details Este método disminuye progresivamente el brillo de la pantalla M5Stack
*          desde el nivel actual hasta 0, creando un efecto de desvanecimiento suave.
*          El proceso se realiza con un retraso de 20ms entre cada decremento de brillo.
* 
* @note Al finalizar, establece la variable lBrillo a 0
* 
* @return void
*/
    void BajaBrillo (void)
    {
        int nBrilloActual = M5.Display.getBrightness();
        for (int nBrillo = nBrilloActual; nBrillo > 0; nBrillo -- )
        {
            M5.Display.setBrightness(nBrillo);
            delay(20);
        }
        M5.Display.setBrightness(0);
        lBrillo = 0;
    }

/**
* @brief Incrementa gradualmente el brillo de la pantalla hasta alcanzar el valor especificado
* 
* Esta función aumenta el brillo de la pantalla de forma progresiva desde 0 hasta el valor
* indicado, creando un efecto de fade-in suave. Cada incremento tiene una pausa de 20ms.
* Al finalizar, establece el indicador de brillo (lBrillo) a 1.
* 
* @param nBrilloFinal Valor final de brillo al que se desea llegar (0-255)
*/
    void SubeBrillo (int nBrilloFinal)
    {
        for (int nBrillo = 0; nBrillo < nBrilloFinal; nBrillo ++ )
        {
            M5.Display.setBrightness(nBrillo);
            delay(20);
        }
        lBrillo = 1;
    }

/**
 * @brief Establece el brillo de la pantalla M5Stack al máximo.
 * 
 * Esta función ajusta el brillo de la pantalla al valor máximo (127) y
 * actualiza la variable de estado 'lBrillo' a 1 para indicar que el brillo
 * está al máximo.
 * 
 * @note Requiere que el objeto M5 esté inicializado previamente.
 */
    void BrilloFull (void)
    {
        M5.Display.setBrightness(127);
        lBrillo = 1;
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

    int x = M5Dial.Display.width() / 2;
    int y = ( M5Dial.Display.height()  / 2 );

//sprite->pushRotateZoom(x-25, y+60, 0, 0.8, 0.8, Color_Transparente);

}

void MensajeDifuntos (void)
{
    ClearPantalla();
    /*
    M5Dial.Display.setTextColor(Color_Info, Color_Fondo);
    M5Dial.Display.setTextDatum(middle_center);
    M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
    M5Dial.Display.setTextSize(2);
    M5Dial.Display.drawString("Difuntos", M5Dial.Display.width() / 2, M5Dial.Display.height() / 2);
    */
    int x = M5Dial.Display.width() / 2;
    int y = ( M5Dial.Display.height()  / 2 );

    M5Canvas*& sprite = aSprites[iDifuntos]; // Obtiene el sprite del icono de Difuntos
    sprite->pushRotateZoom(x, y-20, 0, 1.2, 1.2, Color_Transparente);

    M5Dial.Display.setTextColor(Color_Info, Color_Fondo);
    M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
    M5Dial.Display.setTextSize(2);
//    M5Dial.Display.drawString("Calefaccion", M5Dial.Display.width() / 2, (M5Dial.Display.height() / 2) - 20);
    M5Dial.Display.drawString("Difuntos", x, y  + 50);
}

void MensajeMisa (void)
{
    ClearPantalla();
    /*
    M5Dial.Display.setTextColor(Color_Info, Color_Fondo);
    M5Dial.Display.setTextDatum(middle_center);
    M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
    M5Dial.Display.setTextSize(2);
    M5Dial.Display.drawString("Misa", M5Dial.Display.width() / 2, M5Dial.Display.height() / 2);
    */
    
    int x = M5Dial.Display.width() / 2;
    int y = ( M5Dial.Display.height()  / 2 );

    M5Canvas*& sprite = aSprites[iMisa]; // Obtiene el sprite del icono de Misa
    sprite->pushRotateZoom(x, y-20, 0, 1.2, 1.2, Color_Transparente);

    M5Dial.Display.setTextColor(Color_Info, Color_Fondo);
    M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
    M5Dial.Display.setTextSize(2);
//    M5Dial.Display.drawString("Calefaccion", M5Dial.Display.width() / 2, (M5Dial.Display.height() / 2) - 20);
    M5Dial.Display.drawString("Misa", x, y  + 50);
}

void MensajeStop (void )
{
    ClearPantalla();
    /*
    M5Dial.Display.setTextColor(Color_Info, Color_Fondo);
    M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
    M5Dial.Display.setTextSize(2);
    M5Dial.Display.drawString("STOP", M5Dial.Display.width() / 2, (M5Dial.Display.height() / 2) - 15 );
    */
    int x = M5Dial.Display.width() / 2;
    int y = ( M5Dial.Display.height()  / 2 );

    M5Canvas*& sprite = aSprites[iStop]; // Obtiene el sprite del icono de Stop
    sprite->pushRotateZoom(x, y-20, 0, 1.2, 1.2, Color_Transparente);
 
    M5Dial.Display.setTextColor(Color_Info, Color_Fondo);
    M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
    M5Dial.Display.setTextSize(2);
//    M5Dial.Display.drawString("Calefaccion", M5Dial.Display.width() / 2, (M5Dial.Display.height() / 2) - 20);
    M5Dial.Display.drawString("Stop", x, y  + 50);
    
} 



void MensajeCalefaccionOn (void)
{
    ClearPantalla();
 
    /*
    M5Dial.Display.setTextColor(Color_Info, Color_Fondo);
    M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
    M5Dial.Display.setTextSize(2);
    M5Dial.Display.drawString("Calefaccion", M5Dial.Display.width() / 2, (M5Dial.Display.height() / 2) - 20);
    M5Dial.Display.drawString("ON", M5Dial.Display.width() / 2, (M5Dial.Display.height() / 2)  + 30);
    */

    int x = M5Dial.Display.width() / 2;
    int y = ( M5Dial.Display.height()  / 2 );

    M5Canvas*& sprite = aSprites[iCalefaccionOn]; // Obtiene el sprite del icono de Calefaccion On
    sprite->pushRotateZoom(x, y-20, 0, 1.2, 1.2, Color_Transparente);

    M5Dial.Display.setTextColor(Color_Info, Color_Fondo);
    M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
    M5Dial.Display.setTextSize(2);
//    M5Dial.Display.drawString("Calefaccion", M5Dial.Display.width() / 2, (M5Dial.Display.height() / 2) - 20);
    M5Dial.Display.drawString("Encender", x, y  + 50);

    }
void MensajeCalefaccionOff (void)
{
    ClearPantalla();
    /*
    M5Dial.Display.setTextColor(Color_Info, Color_Fondo);
    M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
    M5Dial.Display.setTextSize(2);
    M5Dial.Display.drawString("Calefaccion", M5Dial.Display.width() / 2, (M5Dial.Display.height() / 2) - 20);
    M5Dial.Display.drawString("OFF", M5Dial.Display.width() / 2, (M5Dial.Display.height() / 2)  + 30);
    */
    int x = M5Dial.Display.width() / 2;
    int y = ( M5Dial.Display.height()  / 2 );   
    M5Canvas*& sprite = aSprites[iCalefaccionOff]; // Obtiene el sprite del icono de Calefaccion Off
    sprite->pushRotateZoom(x, y-20, 0, 1.2, 1.2, Color_Transparente);

    M5Dial.Display.setTextColor(Color_Info, Color_Fondo);
    M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
    M5Dial.Display.setTextSize(2);
//    M5Dial.Display.drawString("Calefaccion", M5Dial.Display.width() / 2, (M5Dial.Display.height() / 2) - 20);
    M5Dial.Display.drawString("Apagar", x, y  + 50);

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

    void CreaSpritesMenu (void)
    {
        CreaSpriteMenu(aSprites[iStop], IconoStop);
        CreaSpriteMenu(aSprites[iMisa], IconoMisa);
        CreaSpriteMenu(aSprites[iDifuntos], IconoDifuntos);
        CreaSpriteMenu(aSprites[iCalefaccionOn], IconoCalefaccionOn);
        CreaSpriteMenu(aSprites[iCalefaccionOff], IconoCalefaccionOff);

    }

    void CreaSpriteMenu (M5Canvas*& sprite, const unsigned char* image = nullptr)
    {
            int nAncho = 120;                                                       //Ancho del Sprite
            int nAlto  = 120;                                                       //Alto del Sprite        

            int x = M5Dial.Display.width() / 2;                                     //X centro de pantalla    
            int y = M5Dial.Display.height()  / 2;                                   //Y Centro de pantalla            

            sprite = new M5Canvas(&M5.Lcd);

            sprite->createSprite(nAncho, nAlto);                                    //Creamos el Sprite
            if (image != nullptr) {
                sprite->drawJpg( image  // data_pointer
                     , ~0u  // data_length (~0u = auto)
                     , 0    // X position
                     , 0    // Y position
                     , 120 // Width
                     , 120 // Height
                     , 0    // X offset
                     , 0    // Y offset
                     , 1.0  // X magnification(default = 1.0 , 0 = fitsize , -1 = follow the Y magni)
                     , 1.0  // Y magnification(default = 1.0 , 0 = fitsize , -1 = follow the X magni)
                     , datum_t::middle_center
                );    
            }   
    }
            
#endif

