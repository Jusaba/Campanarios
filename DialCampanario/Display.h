#ifndef DISPLAYL_H
    #define DISPLAY_H

        #include <M5Dial.h>
        #include "Imagen.h"  // Incluye las imágenes de los iconos y sprites


        #define Color_Fondo         0x0000
        #define Color_Info          0xffff
        #define Color_Voltios       0xffff
        #define Color_Seleccion     TFT_ORANGE
        #define Color_Transparente  0x0002
    
        #define iStop               0                   //!<Posicion del array de sprites con el icono de Stop
        #define iDifuntos           1                   //!<Posicion del array de sprites con el icono de Difuntos
        #define iMisa               2                   //!<Posicion del array de sprites con el icono de Misa
        #define iCalefaccionOn      3                   //!<Posicion del array de sprites con el icono de Calefaccion On
        #define iCalefaccionOff     4                   //!<Posicion del array de sprites con el icono de Calefaccion Off
        #define iCampanario         5                   //!<Posicion del array de sprites con el icono de Campanario
        #define iNoInternet         6                   //!<Posicion del array de sprites con el icono de No Internet
        #define iCalefaccionTemp    7                   //!<Posicion del array de sprites con el icono de Calefaccion Temp

        #define IconoStop    Stop                       //!<Nombre del icono termometro del menu en Imagen.h
        #define IconoDifuntos Difuntos                  //!<Nombre del icono de difuntos del menu en Imagen.h
        #define IconoMisa Misa                          //!<Nombre del icono de misa del menu en Imagen.h
        #define IconoCalefaccionOn CalefaccionOn        //!<Nombre del icono de calefaccion On del menu en Imagen.h
        #define IconoCalefaccionOff CalefaccionOff      //!<Nombre del icono de
        #define IconoCampanario Campanario              //!<Nombre del icono de campanario del menu en Imagen.h
        #define IconoNoInternet NoInternet              //!<Nombre del icono de no internet del menu en Imagen.h
        #define IconoCalefaccionTemp CalefaccionTemp    //!<Nombre del icono de calefaccion temp del menu en Imagen.h

        boolean lBrillo = 1;                            //Indica si hay brillo en el display (1)
        M5Canvas* aSprites[8];                          //Array de Sprites de Menu


        // Prototipos de funciones
        // Funciones de inicialización y configuración del display
        void InicioDisplay (void);                                              //Inicializa la pantalla del dispositivo M5Dial
        void ClearPantalla (void);                                              //Limpia completamente la pantalla del dispositivo M5Dial
        void BajaBrillo (void);                                                 //Baja el brillo hasta 0 de forma paulatina
        void SubeBrillo (int nBrilloFinal );                                    //Sube el brillo hasta nBrilloFinal de forma paulatina
        void BrilloFull (void);                                                 //Sube el brillo al máximo (127) y actualiza la variable lBrillo a 1        
        // Funciones de mensajes en pantalla        
        void MensajeIconoTexto(int spriteIndex, const char* texto, uint16_t colorTexto, int offsetY);   //Muestra un mensaje en pantalla con un icono y un texto
        void MensajeInicio()          { MensajeIconoTexto(iCampanario, "Campanario", TFT_YELLOW, 65); } // Muestra el mensaje de inicio en la pantalla principal
        void MensajeDifuntos()        { MensajeIconoTexto(iDifuntos, "Difuntos", Color_Info, 70); }     // Muestra el mensaje de difuntos en la pantalla principal
        void MensajeMisa()            { MensajeIconoTexto(iMisa, "Misa", Color_Info, 50); }             // Muestra el mensaje de misa en la pantalla principal
        void MensajeStop()            { MensajeIconoTexto(iStop, "Stop", Color_Info, 50); }             // Muestra el mensaje de stop en la pantalla principal
        void MensajeCalefaccionOn()   { MensajeIconoTexto(iCalefaccionOn, "Encender", Color_Info, 50); }// Muestra el mensaje de calefacción encendida en la pantalla principal
        void MensajeCalefaccionOff()  { MensajeIconoTexto(iCalefaccionOff, "Apagar", Color_Info, 50); } // Muestra el mensaje de calefacción apagada en la pantalla principal

        // Funciones de hora
        // Estas funciones muestran la hora en pantalla, si limpiar es true limpia la pantalla antes de mostrar la hora
        void MostrarHora(int nHora, int nMinutos, int nSegundos, bool limpiar); //Muestra la hora en pantalla, si limpiar es true limpia la pantalla antes de mostrar la hora
        void MensajeHora(int nHora, int nMinutos, int nSegundos) { MostrarHora(nHora, nMinutos, nSegundos, true); } // Muestra la hora en pantalla, limpiando la pantalla antes de mostrarla
        void EscribeHora(int nHora, int nMinutos, int nSegundos) { MostrarHora(nHora, nMinutos, nSegundos, false); } // Escribe la hora en pantalla sin limpiar la pantalla antes de mostrarla
        void MostrarFechaHora(int nHora, int nMinutos, int nSegundos, int nDia, int nMes, int nAno, bool limpiar = false); // Muestra la fecha y hora en pantalla, si limpiar es true limpia la pantalla antes de mostrar la fecha y hora
        void MensajeFechaHora(int nHora, int nMinutos, int nSegundos, int nDia, int nMes, int nAno) { MostrarFechaHora(nHora, nMinutos, nSegundos, nDia, nMes, nAno, true); } // Muestra la fecha y hora en pantalla, limpiando la pantalla antes de mostrarla
        void EscribeFechaHora(int nHora, int nMinutos, int nSegundos, int nDia, int nMes, int nAno) { MostrarFechaHora(nHora, nMinutos, nSegundos, nDia, nMes, nAno, false); } // Escribe la fecha y hora en pantalla sin limpiar la pantalla antes de mostrarla
        void MostrarCalefaccionTemporizada (int nHora, int nMinutos, int nSegundos, bool limpiar);
        void MensajeTemporizacion (int nMinutos, bool lLimpiar);
        // Funciones de creación de sprites
        // Estas funciones crean los sprites que se utilizan en el menú principal
        void CreaSpritesMenu (void);                                            //Crea todos los Sprites del menu ( On, Off, Consigna, Automatico....)
        void CreaSpriteMenu ( M5Canvas*& sprite, const unsigned char* image );  //Crea un Sprite de Menu. Esta funcion es llamada desde CreaSpritesMenu()


        //---------------------------------------------------------
        // Funciones de inicialización y configuración del display
        //--------------------------------------------------------
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

        /**
         * @brief Muestra un icono (sprite) y un texto centrado en la pantalla.
         *
         * Esta función limpia la pantalla, dibuja un sprite centrado con un pequeño zoom,
         * y luego muestra un texto centrado debajo del sprite, con el color y desplazamiento vertical especificados.
         *
         * @param spriteIndex Índice del sprite a mostrar en el arreglo aSprites.
         * @param texto Texto a mostrar debajo del icono.
         * @param colorTexto (Opcional) Color del texto. Por defecto es Color_Info.
         * @param offsetY (Opcional) Desplazamiento vertical del texto respecto al centro de la pantalla. Por defecto es 50.
         */
        void MensajeIconoTexto(int spriteIndex, const char* texto, uint16_t colorTexto = Color_Info, int offsetY = 50) 
        {
            ClearPantalla();
            int x = M5Dial.Display.width() / 2;
            int y = M5Dial.Display.height() / 2;
            M5Canvas*& sprite = aSprites[spriteIndex];
            sprite->pushRotateZoom(x, y-20, 0, 1.2, 1.2, Color_Transparente);
            M5Dial.Display.setTextDatum(middle_center);
            M5Dial.Display.setTextColor(colorTexto, Color_Fondo);
            M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
            M5Dial.Display.setTextSize(2);
            M5Dial.Display.drawString(texto, x, y + offsetY);
        }

        /**
         * @brief Muestra la hora en el display.
         * 
         * Esta función muestra la hora especificada por los parámetros nHora, nMinutos y nSegundos
         * en el display principal. Si el parámetro limpiar es verdadero, limpia la pantalla antes
         * de mostrar la hora y actualiza el sprite correspondiente.
         * 
         * @param nHora        Hora a mostrar (0-23).
         * @param nMinutos     Minutos a mostrar (0-59).
         * @param nSegundos    Segundos a mostrar (0-59).
         * @param limpiar      Si es true, limpia la pantalla antes de mostrar la hora (por defecto es false).
         */
        void MostrarHora(int nHora, int nMinutos, int nSegundos, bool limpiar) 
        {
            if (limpiar) ClearPantalla();
            int x = M5Dial.Display.width() / 2;
            int y = ( M5Dial.Display.height()  / 2 );
            if (limpiar) {
                M5Canvas*& sprite = aSprites[iCampanario];
                sprite->pushRotateZoom(x, y-20, 0, 1.0, 1.0, Color_Transparente);
            }

            if (nHora == 255 && nMinutos == 255 && nSegundos == 255) {
                //snprintf(buffer, sizeof(buffer), "--:--:--");
                M5Canvas*& sprite = aSprites[iNoInternet];
                sprite->pushRotateZoom(x, y+60, 0, 0.7, 0.7, Color_Transparente);
            }else{
                if (!limpiar) {
                    M5Dial.Display.fillRect(x - 60, y + 65 - 10, 120, 20, Color_Fondo); // Borra la hora dibujando un rectángulo del color de fondo
                }               
                M5Dial.Display.setTextDatum(middle_center);
                M5Dial.Display.setTextColor(Color_Info, Color_Fondo);
                M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
                M5Dial.Display.setTextSize(2);
                char buffer[9];                
                snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", nHora, nMinutos, nSegundos);
                M5Dial.Display.drawString(buffer, x, y + 65);
            }
            
        }

        void MostrarFechaHora(int nHora, int nMinutos, int nSegundos, int nDia, int nMes, int nAno, bool limpiar) 
        {
            if (limpiar) ClearPantalla();
            int x = M5Dial.Display.width() / 2;
            int y = ( M5Dial.Display.height()  / 2 );
    
            if (nHora == 255 && nMinutos == 255 && nSegundos == 255) {
                //snprintf(buffer, sizeof(buffer), "--:--:--");
                M5Canvas*& sprite = aSprites[iNoInternet];
                sprite->pushRotateZoom(x, y+60, 0, 0.7, 0.7, Color_Transparente);
            }else{
                if (!limpiar) {
                    M5Dial.Display.fillRect(x - 60, y + 45 - 10, 120, 20, Color_Fondo); // Borra la hora dibujando un rectángulo del color de fondo
                    M5Dial.Display.fillRect(x - 60, y - 40 - 10, 120, 20, Color_Fondo); // Borra la Fecha dibujando un rectángulo del color de fondo
                }               
                M5Dial.Display.setTextDatum(middle_center);
                M5Dial.Display.setTextColor(Color_Info, Color_Fondo);
                M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
                M5Dial.Display.setTextSize(2);
                char buffer[13];                
                snprintf(buffer, sizeof(buffer), "%02d : %02d : %02d", nHora, nMinutos, nSegundos);
                M5Dial.Display.drawString(buffer, x, y + 45);
                snprintf(buffer, sizeof(buffer), "%02d / %02d / %02d", nDia, nMes, nAno);
                M5Dial.Display.drawString(buffer, x, y -40);

            }
            
        }
        void MensajeTemporizacion (int nMinutos, bool lLimpiar)
        {
 
            ClearPantalla();
            int x = M5Dial.Display.width() / 2;
            int y = M5Dial.Display.height() / 2;
            if ( lLimpiar ) {
                ClearPantalla();
            }else{
                M5Dial.Display.fillRect(x - 60, y + 40 - 10, 120, 20, Color_Fondo); // Borra la hora dibujando un rectángulo del color de fondo 
            }            
            M5Dial.Display.setTextDatum(middle_center);
            M5Dial.Display.setTextColor(Color_Info, Color_Fondo);
            M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
            M5Dial.Display.setTextSize(2);
            char buffer[20];
            snprintf(buffer, sizeof(buffer), "Temporizacion");
            M5Dial.Display.drawString(buffer, x, y - 30);
            snprintf(buffer, sizeof(buffer), " %d min", nMinutos);
            M5Dial.Display.drawString(buffer, x, y + 40);
        }

        void MostrarCalefaccionTemporizada (int nHora, int nMinutos, int nSegundos, bool limpiar)
        {
            int x = M5Dial.Display.width() / 2;
            int y = ( M5Dial.Display.height()  / 2 );

            if (limpiar) 
            {
                ClearPantalla();
                M5Canvas*& sprite = aSprites[iCalefaccionTemp];
                sprite->pushRotateZoom(x, y-15, 0, 1.2, 1.2, Color_Transparente);
            }
            
            M5Dial.Display.fillRect(x - 60, y + 65 - 10, 120, 20, Color_Fondo); // Borra la hora dibujando un rectángulo del color de fondo
            M5Dial.Display.setTextDatum(middle_center);
            M5Dial.Display.setTextColor(Color_Info, Color_Fondo);
            M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
            M5Dial.Display.setTextSize(2);
            char buffer[9];                
            snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", nHora, nMinutos, nSegundos);
            M5Dial.Display.drawString(buffer, x, y + 65);
            
        }

        /**
         * @brief Crea los sprites del menú principal.
         *
         * Esta función inicializa los diferentes sprites utilizados en el menú principal,
         * asignando a cada uno su icono correspondiente. Los sprites incluyen las opciones
         * de detener, misa, difuntos, calefacción encendida/apagada y campanario.
         *
         * No recibe parámetros ni devuelve ningún valor.
         */
        void CreaSpritesMenu (void)
        {
            CreaSpriteMenu(aSprites[iStop], IconoStop);
            CreaSpriteMenu(aSprites[iMisa], IconoMisa);
            CreaSpriteMenu(aSprites[iDifuntos], IconoDifuntos);
            CreaSpriteMenu(aSprites[iCalefaccionOn], IconoCalefaccionOn);
            CreaSpriteMenu(aSprites[iCalefaccionOff], IconoCalefaccionOff);
            CreaSpriteMenu(aSprites[iCampanario], IconoCampanario);
            CreaSpriteMenu(aSprites[iNoInternet], IconoNoInternet);
            CreaSpriteMenu(aSprites[iCalefaccionTemp], IconoCalefaccionTemp);
        }

        /**
         * @brief Crea un sprite de menú y, opcionalmente, dibuja una imagen JPG en él.
         * 
         * @param[out] sprite Referencia al puntero donde se almacenará el nuevo objeto M5Canvas creado.
         * @param[in] image   Puntero a los datos de la imagen JPG a dibujar en el sprite (opcional, por defecto nullptr).
         * 
         * @details
         * - El sprite se crea con un tamaño fijo de 120x120 píxeles.
         * - Si se proporciona una imagen, se dibuja centrada en el sprite usando drawJpg.
         * - El sprite se asocia al display principal de M5Dial.
         */
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