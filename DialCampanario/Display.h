#ifndef DISPLAYL_H
    #define DISPLAY_H

        #include <M5Dial.h>
        #include "Imagen.h"  // Incluye las imágenes de los iconos y sprites
        #include "Configuracion.h"
        #include "Debug.h"

/*
        #define Color_Fondo         0x0000
        #define Color_Info          0xffff
        #define Color_Voltios       0xffff
        #define Color_Seleccion     TFT_ORANGE
        #define Color_Transparente  0x0002
*/    
        #define iStop               0                   //!<Posicion del array de sprites con el icono de Stop
        #define iDifuntos           1                   //!<Posicion del array de sprites con el icono de Difuntos
        #define iMisa               2                   //!<Posicion del array de sprites con el icono de Misa
        #define iCalefaccionOn      3                   //!<Posicion del array de sprites con el icono de Calefaccion On
        #define iCalefaccionOff     4                   //!<Posicion del array de sprites con el icono de Calefaccion Off
        #define iCampanario         5                   //!<Posicion del array de sprites con el icono de Campanario
        #define iNoInternet         6                   //!<Posicion del array de sprites con el icono de No Internet
        #define iCalefaccionTemp    7                   //!<Posicion del array de sprites con el icono de Calefaccion Temp

                // Array de configuración de sprites (implementado en Display.cpp)
        struct SpriteData {
            const uint8_t* imagen;      // Puntero a imagen en Imagen.h
            const char* nombre;         // Nombre para debug
            const char* texto;          // Texto a mostrar
            uint16_t color;             // Color del texto
            int offsetY;                // Offset Y
        };

        extern const SpriteData SPRITES[9];    // Array de configuración

        // Macros para acceso fácil
        #define GET_SPRITE_IMAGE(i)     (SPRITES[i].imagen)
        #define GET_SPRITE_NAME(i)      (SPRITES[i].nombre)
        #define GET_SPRITE_TEXT(i)      (SPRITES[i].texto)
        #define GET_SPRITE_COLOR(i)     (SPRITES[i].color)
        #define GET_SPRITE_OFFSET(i)    (SPRITES[i].offsetY)

        #define IconoStop    Stop                       //!<Nombre del icono termometro del menu en Imagen.h
        #define IconoDifuntos Difuntos                  //!<Nombre del icono de difuntos del menu en Imagen.h
        #define IconoMisa Misa                          //!<Nombre del icono de misa del menu en Imagen.h
        #define IconoCalefaccionOn CalefaccionOn        //!<Nombre del icono de calefaccion On del menu en Imagen.h
        #define IconoCalefaccionOff CalefaccionOff      //!<Nombre del icono de
        #define IconoCampanario Campanario              //!<Nombre del icono de campanario del menu en Imagen.h
        #define IconoNoInternet NoInternet              //!<Nombre del icono de no internet del menu en Imagen.h
        #define IconoCalefaccionTemp CalefaccionTemp    //!<Nombre del icono de calefaccion temp del menu en Imagen.h

        extern boolean lBrillo;                         //Indica si hay brillo en el display (1)
        extern M5Canvas* aSprites[9];                   //Array de Sprites de Menu


        // Prototipos de funciones
        // Funciones de inicialización y configuración del display
        void InicioDisplay (void);                                              //Inicializa la pantalla del dispositivo M5Dial
        void ClearPantalla (void);                                              //Limpia completamente la pantalla del dispositivo M5Dial
        void BajaBrillo (void);                                                 //Baja el brillo hasta 0 de forma paulatina
        void SubeBrillo (int nBrilloFinal );                                    //Sube el brillo hasta nBrilloFinal de forma paulatina
        void BrilloFull (void);                                                 //Sube el brillo al máximo (127) y actualiza la variable lBrillo a 1        
        // Funciones de mensajes en pantalla        
        void MensajeIconoTexto(int spriteIndex, const char* texto, uint16_t colorTexto, int offsetY);   //Muestra un mensaje en pantalla con un icono y un texto
        void MensajeInicio();                                                   // Muestra el mensaje de inicio en la pantalla principal
        void MensajeDifuntos();                                                 // Muestra el mensaje de difuntos en la pantalla principal
        void MensajeMisa();                                                     // Muestra el mensaje de misa en la pantalla principal
        void MensajeStop();                                                     // Muestra el mensaje de stop en la pantalla principal
        void MensajeCalefaccionOn();                                            // Muestra el mensaje de calefacción encendida en la pantalla principal
        void MensajeCalefaccionOff();                                           // Muestra el mensaje de calefacción apagada en la pantalla principal

        // Funciones de hora
        // Estas funciones muestran la hora en pantalla, si limpiar es true limpia la pantalla antes de mostrar la hora
        void MostrarHora(int nHora, int nMinutos, int nSegundos, bool limpiar);                                            //Muestra la hora en pantalla, si limpiar es true limpia la pantalla antes de mostrar la hora
        void MensajeHora(int nHora, int nMinutos, int nSegundos);                                                          // Muestra la hora en pantalla, limpiando la pantalla antes de mostrarla
        void EscribeHora(int nHora, int nMinutos, int nSegundos);                                                          // Escribe la hora en pantalla sin limpiar la pantalla antes de mostrarla
        void MostrarFechaHora(int nHora, int nMinutos, int nSegundos, int nDia, int nMes, int nAno, bool limpiar = false); // Muestra la fecha y hora en pantalla, si limpiar es true limpia la pantalla antes de mostrar la fecha y hora
        void MensajeFechaHora(int nHora, int nMinutos, int nSegundos, int nDia, int nMes, int nAno);                       // Muestra la fecha y hora en pantalla, limpiando la pantalla antes de mostrarla
        void EscribeFechaHora(int nHora, int nMinutos, int nSegundos, int nDia, int nMes, int nAno);                       // Escribe la fecha y hora en pantalla sin limpiar la pantalla antes de mostrarla
        void MostrarCalefaccionTemporizada (int nHora, int nMinutos, int nSegundos, bool limpiar);                         // Muestra la temporización de la calefacción  en pantalla, si limpiar es true limpia la pantalla antes de mostrar la hora
        void MensajeTemporizacion (int nMinutos, bool lLimpiar);                                                           // Muestra la temporización de la calefacción  en pantalla, si limpiar es true limpia la pantalla antes de mostrar la hora
        // Funciones de creación de sprites
        // Estas funciones crean los sprites que se utilizan en el menú principal
        void CreaSpritesMenu (void);                                            //Crea todos los Sprites del menu ( On, Off, Consigna, Automatico....)
        void CreaSpriteMenu ( M5Canvas*& sprite, const unsigned char* image );  //Crea un Sprite de Menu. Esta funcion es llamada desde CreaSpritesMenu()

        void MostrarSprite ( int indice );                  //Muestra un Sprite en la posición (x,y) de la pantalla
#endif    