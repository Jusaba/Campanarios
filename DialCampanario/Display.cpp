    #include "Display.h"

    // ============================================================================
    // VARIABLES GLOBALES - DEFINICIONES (crear las variables aquí)
    // ============================================================================
    boolean lBrillo = 1;                            
    M5Canvas* aSprites[9];                          


    const SpriteData SPRITES[9] = {
        // imagen,           nombre,             texto,        color,                        offsetY
        {Stop,               "Stop",             "Stop",       Config::Display::COLOR_INFO, 50},
        {Difuntos,           "Difuntos",         "Difuntos",   Config::Display::COLOR_INFO, 70},
        {Misa,               "Misa",             "Misa",       Config::Display::COLOR_INFO, 50},
        {CalefaccionOn,      "CalefaccionOn",    "Encender",   Config::Display::COLOR_INFO, 50},
        {CalefaccionOff,     "CalefaccionOff",   "Apagar",     Config::Display::COLOR_INFO, 50},
        {Campanario,         "Campanario",       "Campanario", TFT_YELLOW,                  65},
        {NoInternet,         "NoInternet",       "Sin Red",    Config::Display::COLOR_INFO, 50},
        {CalefaccionTemp,    "CalefaccionTemp",  "Temporizada",Config::Display::COLOR_INFO, 45},
        {nullptr,            "NuevaOpcion",      "Nueva Op.",  Config::Display::COLOR_INFO, 50}  // Placeholder
    };    
        //---------------------------------------------------------
        // Funciones de inicialización y configuración del display
        //--------------------------------------------------------
        /**
         * @brief Limpia completamente la pantalla del dispositivo M5Dial.
         * 
         * @details Esta función borra todo el contenido de la pantalla rellenándola
         *          con el color de fondo definido en COLOR_FONDO.
         * 
         * @note Utiliza la biblioteca M5Dial para acceder a las funciones de display.
         */
        void ClearPantalla (void)
        {
            M5Dial.Display.fillScreen(Config::Display::COLOR_FONDO);
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
            for (int nBrillo = nBrilloActual; nBrillo > Config::Display::BRILLO_MINIMO; nBrillo -- )
            {
                M5.Display.setBrightness(nBrillo);
                delay(20);
            }
            M5.Display.setBrightness(Config::Display::BRILLO_MINIMO); // Asegura que el brillo esté en el nivel mínimo definido
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
            M5.Display.setBrightness(Config::Display::BRILLO_MAXIMO);
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
         * @param colorTexto (Opcional) Color del texto. Por defecto es COLOR_INFO.
         * @param offsetY (Opcional) Desplazamiento vertical del texto respecto al centro de la pantalla. Por defecto es 50.
         */
        void MensajeIconoTexto(int spriteIndex, const char* texto, uint16_t colorTexto = Config::Display::COLOR_INFO, int offsetY = 50) 
        {
            ClearPantalla();
            int x = M5Dial.Display.width() / 2;
            int y = M5Dial.Display.height() / 2;
            M5Canvas*& sprite = aSprites[spriteIndex];
            sprite->pushRotateZoom(x, y-20, 0, 1.2, 1.2, Config::Display::COLOR_TRANSPARENTE);
            M5Dial.Display.setTextDatum(middle_center);
            M5Dial.Display.setTextColor(colorTexto, Config::Display::COLOR_FONDO);
            M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
            M5Dial.Display.setTextSize(2);
            M5Dial.Display.drawString(texto, x, y + offsetY);
        }
        void MensajeInicio (void)
        {
            MensajeIconoTexto(iCampanario, "Campanario", TFT_YELLOW, 65);
            DBG_DISPLAY("Display: Mensaje de inicio mostrado");
        }
        void MensajeDifuntos (void)
        {
            MensajeIconoTexto(iDifuntos, GET_SPRITE_TEXT(iDifuntos), GET_SPRITE_COLOR(iDifuntos), GET_SPRITE_OFFSET(iDifuntos)); 
            DBG_DISPLAY("Display: Mensaje de Difuntos mostrado");
        }
        void MensajeMisa (void)
        {
            MensajeIconoTexto(iMisa, GET_SPRITE_TEXT(iMisa), GET_SPRITE_COLOR(iMisa), GET_SPRITE_OFFSET(iMisa));
            DBG_DISPLAY("Display: Mensaje de Misa mostrado");
        }
        void MensajeStop (void)
        {
            MensajeIconoTexto(iStop, GET_SPRITE_TEXT(iStop), GET_SPRITE_COLOR(iStop), GET_SPRITE_OFFSET(iStop));
            DBG_DISPLAY("Display: Mensaje de Stop mostrado");
        }
        void MensajeCalefaccionOn (void)
        {
            MensajeIconoTexto(iCalefaccionOn, GET_SPRITE_TEXT(iCalefaccionOn), GET_SPRITE_COLOR(iCalefaccionOn), GET_SPRITE_OFFSET(iCalefaccionOn));
            DBG_DISPLAY("Display: Mensaje de Calefacción On mostrado");
        }
        void MensajeCalefaccionOff (void)
        {
            MensajeIconoTexto(iCalefaccionOff, GET_SPRITE_TEXT(iCalefaccionOff), GET_SPRITE_COLOR(iCalefaccionOff), GET_SPRITE_OFFSET(iCalefaccionOff));
            DBG_DISPLAY("Display: Mensaje de Calefacción Off mostrado");
        }
        void MensajeHora (int nHora, int nMinutos, int nSegundos)
        {
            MostrarHora(nHora, nMinutos, nSegundos, true); 
            DBG_DISPLAY_PRINTF("Hora mostrada: %02d:%02d:%02d", nHora, nMinutos, nSegundos);
        }
        void EscribeHora (int nHora, int nMinutos, int nSegundos)
        {
            MostrarHora(nHora, nMinutos, nSegundos, false);
            DBG_DISPLAY_PRINTF("Hora escrita: %02d:%02d:%02d", nHora, nMinutos, nSegundos);
        }
        void EscribeFechaHora (int nHora, int nMinutos, int nSegundos, int nDia, int nMes, int nAno)
        {
            MostrarFechaHora(nHora, nMinutos, nSegundos, nDia, nMes, nAno, false);
            DBG_DISPLAY_PRINTF("Fecha y Hora escrita: %02d/%02d/%02d %02d:%02d:%02d", nDia, nMes, nAno, nHora, nMinutos, nSegundos);
        }
        void MensajeFechaHora (int nHora, int nMinutos, int nSegundos, int nDia, int nMes, int nAno)
        {
            MostrarFechaHora(nHora, nMinutos, nSegundos, nDia, nMes, nAno, true); 
            DBG_DISPLAY_PRINTF("Fecha y Hora mostrada: %02d/%02d/%02d %02d:%02d:%02d", nDia, nMes, nAno, nHora, nMinutos, nSegundos);
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
                sprite->pushRotateZoom(x, y-20, 0, 1.0, 1.0, Config::Display::COLOR_TRANSPARENTE);
            }

            if (nHora == 255 && nMinutos == 255 && nSegundos == 255) {
                //snprintf(buffer, sizeof(buffer), "--:--:--");
                M5Canvas*& sprite = aSprites[iNoInternet];
                sprite->pushRotateZoom(x, y+60, 0, 0.7, 0.7, Config::Display::COLOR_TRANSPARENTE);
            }else{
                if (!limpiar) {
                    M5Dial.Display.fillRect(x - 60, y + 65 - 10, 120, 20, Config::Display::COLOR_FONDO); // Borra la hora dibujando un rectángulo del color de fondo
                }               
                M5Dial.Display.setTextDatum(middle_center);
                M5Dial.Display.setTextColor(Config::Display::COLOR_INFO, Config::Display::COLOR_FONDO);
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
                sprite->pushRotateZoom(x, y+60, 0, 0.7, 0.7, Config::Display::COLOR_TRANSPARENTE);
            }else{
                if (!limpiar) {
                    M5Dial.Display.fillRect(x - 60, y + 45 - 10, 120, 20, Config::Display::COLOR_FONDO); // Borra la hora dibujando un rectángulo del color de fondo
                    M5Dial.Display.fillRect(x - 60, y - 40 - 10, 120, 20, Config::Display::COLOR_FONDO); // Borra la Fecha dibujando un rectángulo del color de fondo
                }               
                M5Dial.Display.setTextDatum(middle_center);
                M5Dial.Display.setTextColor(Config::Display::COLOR_INFO, Config::Display::COLOR_FONDO);
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
                M5Dial.Display.fillRect(x - 60, y + 40 - 10, 120, 20, Config::Display::COLOR_FONDO); // Borra la hora dibujando un rectángulo del color de fondo 
            }            
            M5Dial.Display.setTextDatum(middle_center);
            M5Dial.Display.setTextColor(Config::Display::COLOR_INFO, Config::Display::COLOR_FONDO);
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
                sprite->pushRotateZoom(x, y-15, 0, 1.2, 1.2, Config::Display::COLOR_TRANSPARENTE);
            }

            M5Dial.Display.fillRect(x - 60, y + 65 - 10, 120, 20, Config::Display::COLOR_FONDO); // Borra la hora dibujando un rectángulo del color de fondo
            M5Dial.Display.setTextDatum(middle_center);
            M5Dial.Display.setTextColor(Config::Display::COLOR_INFO, Config::Display::COLOR_FONDO);
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
        void MostrarSprite(int indice) {
            if (indice >= 0 && indice < 9 && GET_SPRITE_IMAGE(indice) != nullptr) {
                MensajeIconoTexto(indice, 
                                 GET_SPRITE_TEXT(indice), 
                                 GET_SPRITE_COLOR(indice), 
                                 GET_SPRITE_OFFSET(indice));
                DBG_DISPLAY_PRINTF("Sprite mostrado: %s", GET_SPRITE_NAME(indice));
            } else {
                DBG_DISPLAY_PRINTF("ERROR: Índice sprite inválido: %d", indice);
            }
        }