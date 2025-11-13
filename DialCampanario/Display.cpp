    #include "Display.h"

        // ============================================================================
        // VARIABLES GLOBALES - DEFINICIONES (crear las variables aquí)
        // ============================================================================
        boolean lBrillo = 1;                            


        SpriteData SPRITES[Config::Display::MAX_SPRITES] = {
            // imagen,           nombre,             texto,        color,                        offsetY
            {Stop,               "Stop",             "Stop",       Config::Display::COLOR_INFO, 50},
            {Difuntos,           "Difuntos",         "Difunts",    Config::Display::COLOR_INFO, 70},
            {Misa,               "Misa",             "Missa",      Config::Display::COLOR_INFO, 50},
            {Fiesta,             "Fiesta",           "Festa",      Config::Display::COLOR_INFO, 50},
            {CalefaccionOn,      "CalefaccionOn",    "Encendre",   Config::Display::COLOR_INFO, 50},
            {CalefaccionOff,     "CalefaccionOff",   "Apagar",     Config::Display::COLOR_INFO, 50},
            {Campanario,         "Campanario",       "Campanar",   TFT_YELLOW,                  65},
            {NoInternet,         "NoInternet",       "No Xarxa",   Config::Display::COLOR_INFO, 50},
            {CalefaccionTemp,    "CalefaccionTemp",  "Temporitzada",Config::Display::COLOR_INFO, 45}
        };    

    // ---------------------------------------------------------------------------
    // Funciones de gestión del display
    // ---------------------------------------------------------------------------

        /**
         * @brief Inicializa la pantalla del dispositivo M5Dial
         * 
         * @details Configura e inicializa la pantalla del dispositivo M5Dial utilizando
         *          la configuración predeterminada del M5. Establece los parámetros
         *          iniciales necesarios para el funcionamiento de la pantalla.
         * 
         * @note Requiere que el objeto M5Dial esté correctamente definido
         * @note Debe llamarse en setup() antes de usar funciones de display
         * 
         * @see  M5Dial.begin()
         * 
         * @since v1.0
         * @author Julian Salas Bartolomé
         * @date 2025-06-15
         */
            void InicioDisplay (void)
            {
                DBG_DISPLAY("InicioDisplay->Iniciando display M5Dial");
                auto cfg = M5.config();
                M5Dial.begin(cfg, true, false);
            }

        /**
         * @brief Limpia completamente la pantalla estableciendo fondo negro
         * 
         * @details Borra todo el contenido visible de la pantalla M5Dial estableciendo
         *          todos los píxeles al color de fondo definido (negro). Es una operación
         *          rápida y optimizada que se puede usar frecuentemente.
         * 
         * @note Utiliza el color definido en Config::Display::COLOR_FONDO
         * @note Operación hardware optimizada, no causa problemas de rendimiento
         * @note Útil antes de mostrar nuevo contenido para evitar solapamientos
         * 
         * @see Config::Display::COLOR_FONDO
         * @see MensajeIconoTexto() - Usa ClearPantalla() automáticamente
         * @see MostrarSprite() - Usa ClearPantalla() automáticamente
         * 
         * @since v1.0
         * @author Julian Salas Bartolomé
         * @date 2025-06-15
         */        
            void ClearPantalla (void)
            {
                M5Dial.Display.fillScreen(Config::Display::COLOR_FONDO);
            }

        /**
         * @brief Baja el brillo gradualmente hasta cero (apaga pantalla)
         * 
         * @details Reduce progresivamente el brillo desde el nivel actual hasta cero,
         *          creando una transición suave de apagado. Actualiza automáticamente
         *          la variable lBrillo a false.
         * 
         * @note **TRANSICIÓN SUAVE:** Decrementa gradualmente para evitar apagado brusco
         * @note **ACTUALIZA ESTADO:** Establece lBrillo = false al finalizar
         * @note **FUNCIÓN BLOQUEANTE:** No retorna hasta completar la transición
         * @note **APAGADO COMPLETO:** Brillo final = 0 (pantalla totalmente apagada)
         * 
         * @see lBrillo - Variable global de estado actualizada
         * @see SubeBrillo() - Función opuesta para encender
         * @see BrilloFull() - Encendido a máximo brillo
         * 
         * @since v1.0
         * @author Julian Salas Bartolomé
         * @date 2025-06-15
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
         * @brief Sube el brillo de forma gradual hasta el nivel especificado
         * 
         * @details Aumenta progresivamente el brillo desde el nivel actual hasta el valor
         *          objetivo, creando una transición suave y agradable. Actualiza la variable
         *          global lBrillo según el resultado final.
         * 
         * @param nBrilloFinal Nivel de brillo objetivo (0-255)
         *                     - 0: Apaga completamente la pantalla
         *                     - 1-254: Niveles intermedios de brillo
         *                     - 255: Brillo máximo
         * 
         * @note **TRANSICIÓN SUAVE:** Incrementa gradualmente para evitar cambios bruscos
         * @note **ACTUALIZA ESTADO:** lBrillo = true si nBrilloFinal > 0, false si = 0
         * @note **FUNCIÓN BLOQUEANTE:** No retorna hasta completar la transición
         * 
         * @warning Valores fuera del rango 0-255 pueden causar comportamiento indefinido
         * @warning La función bloquea durante la transición (varios milisegundos)
         * 
         * @see lBrillo - Variable global de estado
         * @see BajaBrillo() - Función opuesta
         * @see BrilloFull() - Wrapper para brillo máximo
         * @see Config::Display::BRILLO_MAXIMO
         * 
         * @since v1.0
         * @author Julian Salas Bartolomé
         * @date 2025-06-15
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
         * @brief Establece brillo máximo con transición suave
         * 
         * @details Función de conveniencia que sube el brillo al nivel máximo (255)
         *          de forma gradual y actualiza el estado a encendido. Equivale a
         *          llamar SubeBrillo(255).
         * 
         * @note **CONVENIENCIA:** Wrapper de SubeBrillo(Config::Display::BRILLO_MAXIMO)
         * @note **BRILLO MÁXIMO:** Establece el brillo al valor máximo definido (255)
         * @note **ACTUALIZA ESTADO:** Establece lBrillo = true automáticamente
         * @note **TRANSICIÓN SUAVE:** Incrementa gradualmente hasta el máximo
         * 
         * @see SubeBrillo() - Función base que implementa la lógica
         * @see BajaBrillo() - Función opuesta para apagar
         * @see Config::Display::BRILLO_MAXIMO - Constante de brillo máximo
         * @see lBrillo - Variable de estado actualizada
         * 
         * @since v1.0
         * @author Julian Salas Bartolomé
         * @date 2025-06-15
         */
        void BrilloFull (void)
        {
            M5.Display.setBrightness(Config::Display::BRILLO_MAXIMO);
            lBrillo = 1;
        }

    // ---------------------------------------------------------------------------
    // Funciones de mensajes de menu en pantalla
    // ---------------------------------------------------------------------------

        /**
         * @brief Muestra un mensaje personalizado con icono y texto configurables
         * 
         * @details Función genérica que permite mostrar cualquier sprite con texto
         *          personalizable, color y posición configurables. Es la función base
         *          utilizada por muchas otras funciones de mensajes del sistema.
         *          
         *          **PROCESO EJECUTADO:**
         *          1. Limpia completamente la pantalla
         *          2. Inicializa automáticamente el sprite si es necesario (lazy loading)
         *          3. Renderiza el sprite centrado con zoom 1.2x
         *          4. Configura fuente, color y alineación del texto
         *          5. Dibuja el texto en la posición calculada
         * 
         * @param spriteIndex Índice del sprite a mostrar (usar Config::SpriteIndex::*)
         * @param texto Texto a mostrar junto al sprite (string terminado en null)
         * @param colorTexto Color del texto en formato RGB565 (por defecto: CONFIG_INFO)
         * @param offsetY Desplazamiento vertical del texto desde el centro (por defecto: 50px)
         * 
         * @note **LIMPIEZA AUTOMÁTICA:** Siempre limpia la pantalla antes de mostrar contenido
         * @note **LAZY LOADING:** Inicializa automáticamente el sprite si es necesario
         * @note **CENTRADO:** Sprite centrado en pantalla, posición Y-20 píxeles
         * @note **ZOOM:** Sprite renderizado con escala 1.2x para mejor visibilidad
         * @note **FUENTE:** FreeSans9pt7b con tamaño 2, centrado horizontalmente
         * 
         * @warning spriteIndex debe ser válido (0 a MAX_SPRITES-1)
         * @warning texto no debe ser nullptr
         * @warning Si falla la inicialización del sprite, la función retorna sin mostrar nada
         * 
         * @see SPRITES[] - Array de configuración de sprites
         * @see Config::SpriteIndex - Índices válidos para sprites
         * @see SpriteData::inicializar() - Inicialización automática
         * @see ClearPantalla() - Limpieza de pantalla
         * 
         * @since v1.0
         * @author Julian Salas Bartolomé
         * @date 2025-06-15
         */
        void MensajeIconoTexto(int spriteIndex, const char* texto, uint16_t colorTexto = Config::Display::COLOR_INFO, int offsetY = 50) 
        {
            ClearPantalla();
            int x = M5Dial.Display.width() / 2;
            int y = M5Dial.Display.height() / 2;

            if (!SPRITES[spriteIndex].inicializar()) {
                DBG_DISPLAY_PRINTF("MensajeIconoTexto() - Error inicializando sprite %d", spriteIndex);
                return;
            }
            SPRITES[spriteIndex].canvas->pushRotateZoom(x, y-20, 0, 1.2, 1.2, Config::Display::COLOR_TRANSPARENTE);
            M5Dial.Display.setTextDatum(middle_center);
            M5Dial.Display.setTextColor(colorTexto, Config::Display::COLOR_FONDO);
            M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
            M5Dial.Display.setTextSize(2);
            M5Dial.Display.drawString(texto, x, y + offsetY);
        }
        /**
         * @brief Muestra el mensaje de inicio del campanario
         * 
         * @details Presenta la pantalla de bienvenida con el icono del campanario
         *          y texto "Campanario" en color amarillo. Es la primera pantalla
         *          que ve el usuario al inicializar el sistema.
         * 
         * @note Utiliza MensajeIconoTexto() con configuración específica:
         *       - Sprite: CAMPANARIO
         *       - Texto: "Campanario"
         *       - Color: TFT_YELLOW
         *       - Offset Y: 65 píxeles
         * 
         * @see MensajeIconoTexto() - Función base utilizada
         * @see Config::SpriteIndex::CAMPANARIO - Sprite mostrado
         * 
         * @since v1.0
         * @author Julian Salas Bartolomé
         * @date 2025-06-15
         */
        void MensajeInicio (void)
        {
            MensajeIconoTexto(Config::SpriteIndex::CAMPANARIO, "Campanario", TFT_YELLOW, 65);
            DBG_DISPLAY("Display: Mensaje de inicio mostrado");
        }
        /**
         * @brief Muestra el estado de misas de difuntos
         * 
         * @details Presenta en pantalla el icono y texto correspondiente al estado
         *          de misas de difuntos. Utiliza la configuración predefinida del
         *          sprite DIFUNTOS para mostrar la información visual.
         * 
         * @note Utiliza MostrarSprite() que incluye configuración automática:
         *       - Imagen, texto, color y offset desde array SPRITES[]
         *       - Limpieza automática de pantalla
         *       - Centrado automático del sprite y texto
         * 
         * @see MostrarSprite() - Función utilizada para renderizado
         * @see Config::SpriteIndex::DIFUNTOS - Índice del sprite mostrado
         * @see SPRITES[DIFUNTOS] - Configuración específica utilizada
         * 
         * @since v1.0
         * @author Julian Salas Bartolomé
         * @date 2025-06-15
         */        
        void MensajeDifuntos (void)
        {
            MostrarSprite(Config::SpriteIndex::DIFUNTOS);
            DBG_DISPLAY("Display: Mensaje de Difuntos mostrado");
        }
        /**
         * @brief Muestra el estado de misas regulares
         * 
         * @details Presenta en pantalla el icono y texto correspondiente al estado
         *          de misas regulares. Utiliza la configuración predefinida del
         *          sprite MISA para mostrar la información visual.
         * 
         * @note Utiliza MostrarSprite() que incluye configuración automática:
         *       - Imagen, texto, color y offset desde array SPRITES[]
         *       - Limpieza automática de pantalla
         *       - Centrado automático del sprite y texto
         * 
         * @see MostrarSprite() - Función utilizada para renderizado
         * @see Config::SpriteIndex::MISA - Índice del sprite mostrado
         * @see SPRITES[MISA] - Configuración específica utilizada
         * 
         * @since v1.0
         * @author Julian Salas Bartolomé
         * @date 2025-06-15
         */        
        void MensajeMisa (void)
        {
            MostrarSprite(Config::SpriteIndex::MISA);
            DBG_DISPLAY("Display: Mensaje de Misa mostrado");
        }
        /**
         * @brief Muestra el estado de fiesta del campanario
         * 
         * @details Presenta en pantalla el icono y texto correspondiente al estado
         *          de fiesta. Utiliza la configuración predefinida del sprite FIESTA
         *          para mostrar la información visual.
         * 
         * @note Utiliza MostrarSprite() que incluye configuración automática:
         *       - Imagen, texto, color y offset desde array SPRITES[]
         *       - Limpieza automática de pantalla
         *       - Centrado automático del sprite y texto
         * 
         * @see MostrarSprite() - Función utilizada para renderizado
         * @see Config::SpriteIndex::FIESTA - Índice del sprite mostrado
         * @see SPRITES[FIESTA] - Configuración específica utilizada
         * 
         * @since v1.0
         * @author Julian Salas Bartolomé
         * @date 2025-06-15
         */        
        void MensajeFiesta (void)
        {
            MostrarSprite(Config::SpriteIndex::FIESTA);
            DBG_DISPLAY("Display: Mensaje de Fiesta mostrado");
        }

        /**
         * @brief Muestra el estado de parada del campanario
         * 
         * @details Presenta en pantalla el icono y texto correspondiente al estado
         *          de parada completa del sistema de campanario. Indica que no
         *          hay actividad programada en el sistema.
         * 
         * @note Utiliza MostrarSprite() que incluye configuración automática:
         *       - Imagen, texto, color y offset desde array SPRITES[]
         *       - Limpieza automática de pantalla
         *       - Centrado automático del sprite y texto
         * 
         * @see MostrarSprite() - Función utilizada para renderizado
         * @see Config::SpriteIndex::STOP - Índice del sprite mostrado
         * @see SPRITES[STOP] - Configuración específica utilizada
         * 
         * @since v1.0
         * @author Julian Salas Bartolomé
         * @date 2025-06-15
         */        
        void MensajeStop (void)
        {
            MostrarSprite(Config::SpriteIndex::STOP);
            DBG_DISPLAY("Display: Mensaje de Stop mostrado");
        }
        /**
         * @brief Muestra el estado de calefacción encendida
         * 
         * @details Presenta en pantalla el icono y texto correspondiente al estado
         *          de calefacción activada manualmente. Indica que el sistema
         *          de calefacción está funcionando activamente.
         * 
         * @note Utiliza MostrarSprite() que incluye configuración automática:
         *       - Imagen, texto, color y offset desde array SPRITES[]
         *       - Limpieza automática de pantalla
         *       - Centrado automático del sprite y texto
         * 
         * @see MostrarSprite() - Función utilizada para renderizado
         * @see Config::SpriteIndex::CALEFACCION_ON - Índice del sprite mostrado
         * @see SPRITES[CALEFACCION_ON] - Configuración específica utilizada
         * 
         * @since v1.0
         * @author Julian Salas Bartolomé
         * @date 2025-06-15
         */        
        void MensajeCalefaccionOn (void)
        {
            MostrarSprite(Config::SpriteIndex::CALEFACCION_ON);
            DBG_DISPLAY("Display: Mensaje de Calefacción On mostrado");
        }
        /**
         * @brief Muestra el estado de calefacción apagada
         * 
         * @details Presenta en pantalla el icono y texto correspondiente al estado
         *          de calefacción desactivada manualmente. Indica que el sistema
         *          de calefacción está parado o en standby.
         * 
         * @note Utiliza MostrarSprite() que incluye configuración automática:
         *       - Imagen, texto, color y offset desde array SPRITES[]
         *       - Limpieza automática de pantalla
         *       - Centrado automático del sprite y texto
         * 
         * @see MostrarSprite() - Función utilizada para renderizado
         * @see Config::SpriteIndex::CALEFACCION_OFF - Índice del sprite mostrado
         * @see SPRITES[CALEFACCION_OFF] - Configuración específica utilizada
         * 
         * @since v1.0
         * @author Julian Salas Bartolomé
         * @date 2025-06-15
         */        
        void MensajeCalefaccionOff (void)
        {
            MostrarSprite(Config::SpriteIndex::CALEFACCION_OFF);
            DBG_DISPLAY("Display: Mensaje de Calefacción Off mostrado");
        }
    // ---------------------------------------------------------------------------
    // Funciones de mensajes auxiliares en pantalla
    // ---------------------------------------------------------------------------        
        /**
         * @brief Muestra hora limpiando completamente la pantalla
         * 
         * @details Wrapper de MostrarHora() que siempre limpia la pantalla antes
         *          de mostrar la hora. Útil para cambios completos de pantalla
         *          o primera visualización de hora.
         * 
         * @param nHora Hora (0-23)
         * @param nMinutos Minutos (0-59)
         * @param nSegundos Segundos (0-59)
         * 
         * @note Equivale a MostrarHora(nHora, nMinutos, nSegundos, true)
         * @note Incluye sprite del campanario como fondo
         * @note Limpia completamente la pantalla antes de mostrar
         * 
         * @see MostrarHora() - Función base implementada
         * @see EscribeHora() - Versión optimizada sin limpiar
         * 
         * @since v1.0
         * @author Julian Salas Bartolomé
         * @date 2025-06-15
         */
        void MensajeHora (int nHora, int nMinutos, int nSegundos)
        {
            MostrarHora(nHora, nMinutos, nSegundos, true); 
            DBG_DISPLAY_PRINTF("Hora mostrada: %02d:%02d:%02d", nHora, nMinutos, nSegundos);
        }
        /**
         * @brief Actualiza solo la hora sin limpiar pantalla
         * 
         * @details Wrapper de MostrarHora() optimizado para actualizaciones
         *          frecuentes (cada segundo). Solo borra y actualiza la zona
         *          de hora, manteniendo el resto del contenido en pantalla.
         * 
         * @param nHora Hora (0-23)
         * @param nMinutos Minutos (0-59)
         * @param nSegundos Segundos (0-59)
         * 
         * @note Equivale a MostrarHora(nHora, nMinutos, nSegundos, false)
         * @note Optimizado para updates periódicos (cada segundo)
         * @note Solo borra la región de texto de hora antes de actualizar
         * @note Preserva sprites y contenido de fondo existente
         * 
         * @see MostrarHora() - Función base implementada
         * @see MensajeHora() - Versión que limpia completamente
         * 
         * @since v1.0
         * @author Julian Salas Bartolomé
         * @date 2025-06-15
         */        
        void EscribeHora (int nHora, int nMinutos, int nSegundos)
        {
            MostrarHora(nHora, nMinutos, nSegundos, false);
            DBG_DISPLAY_PRINTF("Hora escrita: %02d:%02d:%02d", nHora, nMinutos, nSegundos);
        }
        /**
         * @brief Actualiza fecha y hora sin limpiar pantalla
         * 
         * @details Wrapper de MostrarFechaHora() optimizado para actualizaciones
         *          periódicas. Solo actualiza las zonas de fecha y hora,
         *          manteniendo sprites y contenido existente en pantalla.
         * 
         * @param nHora Hora (0-23)
         * @param nMinutos Minutos (0-59)
         * @param nSegundos Segundos (0-59)
         * @param nDia Día del mes (1-31)
         * @param nMes Mes (1-12)
         * @param nAno Año (formato 2 dígitos, ej: 24 para 2024)
         * 
         * @note Equivale a MostrarFechaHora(..., false)
         * @note Optimizado para actualizaciones periódicas
         * @note Solo borra las regiones específicas de fecha y hora
         * @note Preserva sprites y contenido de fondo existente
         * 
         * @see MostrarFechaHora() - Función base implementada
         * @see MensajeFechaHora() - Versión que limpia completamente
         * 
         * @since v1.0
         * @author Julian Salas Bartolomé
         * @date 2025-06-15
         */        
        void EscribeFechaHora (int nHora, int nMinutos, int nSegundos, int nDia, int nMes, int nAno)
        {
            MostrarFechaHora(nHora, nMinutos, nSegundos, nDia, nMes, nAno, false);
            DBG_DISPLAY_PRINTF("Fecha y Hora escrita: %02d/%02d/%02d %02d:%02d:%02d", nDia, nMes, nAno, nHora, nMinutos, nSegundos);
        }
        /**
         * @brief Muestra fecha y hora limpiando completamente la pantalla
         * 
         * @details Wrapper de MostrarFechaHora() que siempre limpia la pantalla
         *          antes de mostrar fecha y hora. Útil para cambios completos
         *          de pantalla o primera visualización.
         * 
         * @param nHora Hora (0-23)
         * @param nMinutos Minutos (0-59)
         * @param nSegundos Segundos (0-59)
         * @param nDia Día del mes (1-31)
         * @param nMes Mes (1-12)
         * @param nAno Año (formato 2 dígitos, ej: 24 para 2024)
         * @param Internet Indica si hay conexión a Internet (true/false)
         * 
         * @note Equivale a MostrarFechaHora(..., true)
         * @note Útil para inicialización o cambios de pantalla completos
         * @note Limpia completamente la pantalla antes de mostrar
         * 
         * @see MostrarFechaHora() - Función base implementada
         * @see EscribeFechaHora() - Versión optimizada sin limpiar
         * 
         * @since v1.0
         * @author Julian Salas Bartolomé
         * @date 2025-06-15
         */        
        void MensajeFechaHora (int nHora, int nMinutos, int nSegundos, int nDia, int nMes, int nAno, bool Internet)
        {
            MostrarFechaHora(nHora, nMinutos, nSegundos, nDia, nMes, nAno, true, Internet); 
            DBG_DISPLAY_PRINTF("Fecha y Hora mostrada: %02d/%02d/%02d %02d:%02d:%02d", nDia, nMes, nAno, nHora, nMinutos, nSegundos);
        }   
        /**
         * @brief Muestra fecha y hora limpiando completamente la pantalla
         * 
         * @details Wrapper de MostrarFechaHora() que siempre limpia la pantalla
         *          antes de mostrar fecha y hora. Útil para cambios completos
         *          de pantalla o primera visualización.
         * 
         * @param nHora Hora (0-23)
         * @param nMinutos Minutos (0-59)
         * @param nSegundos Segundos (0-59)
         * @param nDia Día del mes (1-31)
         * @param nMes Mes (1-12)
         * @param nAno Año (formato 2 dígitos, ej: 24 para 2024)
         * 
         * @note Equivale a MostrarFechaHora(..., true)
         * @note Útil para inicialización o cambios de pantalla completos
         * @note Limpia completamente la pantalla antes de mostrar
         * 
         * @see MostrarFechaHora() - Función base implementada
         * @see EscribeFechaHora() - Versión optimizada sin limpiar
         * 
         * @since v1.0
         * @author Julian Salas Bartolomé
         * @date 2025-06-15
         */        
        void MostrarHora(int nHora, int nMinutos, int nSegundos, bool limpiar) 
        {
            if (limpiar) ClearPantalla();
            int x = M5Dial.Display.width() / 2;
            int y = ( M5Dial.Display.height()  / 2 );
            if (limpiar) {

            if (!SPRITES[Config::SpriteIndex::CAMPANARIO].inicializar()) return;
            M5Canvas*& sprite = SPRITES[Config::SpriteIndex::CAMPANARIO].canvas;                
                sprite->pushRotateZoom(x, y-20, 0, 1.0, 1.0, Config::Display::COLOR_TRANSPARENTE);
            }

            if (nHora == 255 && nMinutos == 255 && nSegundos == 255) {
                if (!SPRITES[Config::SpriteIndex::NO_INTERNET].inicializar()) return;
                M5Canvas*& sprite = SPRITES[Config::SpriteIndex::NO_INTERNET].canvas;
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
        /**
         * @brief Muestra fecha y hora limpiando completamente la pantalla
         * 
         * @details Wrapper de MostrarFechaHora() que siempre limpia la pantalla
         *          antes de mostrar fecha y hora. Útil para cambios completos
         *          de pantalla o primera visualización.
         * 
         * @param nHora Hora (0-23)
         * @param nMinutos Minutos (0-59)
         * @param nSegundos Segundos (0-59)
         * @param nDia Día del mes (1-31)
         * @param nMes Mes (1-12)
         * @param nAno Año (formato 2 dígitos, ej: 24 para 2024)
         * @param Internet Indica si hay conexión a Internet (true/false)
         * 
         * @note Equivale a MostrarFechaHora(..., true)
         * @note Útil para inicialización o cambios de pantalla completos
         * @note Limpia completamente la pantalla antes de mostrar
         * 
         * @see MostrarFechaHora() - Función base implementada
         * @see EscribeFechaHora() - Versión optimizada sin limpiar
         * 
         * @since v1.0
         * @author Julian Salas Bartolomé
         * @date 2025-06-15
         */
        void MostrarFechaHora(int nHora, int nMinutos, int nSegundos, int nDia, int nMes, int nAno, bool limpiar, bool Internet ) 
        {
            if (limpiar) ClearPantalla();
            int x = M5Dial.Display.width() / 2;
            int y = ( M5Dial.Display.height()  / 2 );
    
            if (nHora == 255 && nMinutos == 255 && nSegundos == 255) {
                if (!SPRITES[Config::SpriteIndex::NO_INTERNET].inicializar()) return;
                M5Canvas*& sprite = SPRITES[Config::SpriteIndex::NO_INTERNET].canvas;
                sprite->pushRotateZoom(x, y+60, 0, 0.7, 0.7, Config::Display::COLOR_TRANSPARENTE);
            }else{
                if (!limpiar) {
                    M5Dial.Display.fillRect(x - 60, y + 45 - 10, 120, 20, Config::Display::COLOR_FONDO); // Borra la hora dibujando un rectángulo del color de fondo
                    M5Dial.Display.fillRect(x - 60, y - 40 - 10, 120, 20, Config::Display::COLOR_FONDO); // Borra la Fecha dibujando un rectángulo del color de fondo
                }               
                M5Dial.Display.setTextDatum(middle_center);
                if (Internet) {                                                                          // Si hay Internet           
                    M5Dial.Display.setTextColor(Config::Display::COLOR_NOINTERNET, Config::Display::COLOR_FONDO); //Color de texto normal
                }else{
                    M5Dial.Display.setTextColor(Config::Display::COLOR_INFO, Config::Display::COLOR_FONDO); //Color de texto de no Internet
                }
                M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
                M5Dial.Display.setTextSize(2);
                char buffer[13];                
                snprintf(buffer, sizeof(buffer), "%02d : %02d : %02d", nHora, nMinutos, nSegundos);
                M5Dial.Display.drawString(buffer, x, y + 45);
                snprintf(buffer, sizeof(buffer), "%02d / %02d / %02d", nDia, nMes, nAno);
                M5Dial.Display.drawString(buffer, x, y -40);

            }
            
        }
        /**
         * @brief Muestra mensaje de temporización activa con minutos restantes
         * 
         * @details Presenta en pantalla un mensaje indicando que hay una temporización
         *          activa en el sistema, mostrando los minutos restantes en formato legible.
         *          Usado principalmente para notificaciones del sistema de calefacción temporizada.
         *          
         *          **PROCESO EJECUTADO:**
         *          1. Limpia pantalla completa (siempre)
         *          2. Opcionalmente limpia otra vez (si lLimpiar=true, redundante)
         *          3. Borra selectivamente zona de texto (si lLimpiar=false)
         *          4. Renderiza "Temporizacion" en línea superior (Y-30)
         *          5. Renderiza "X min" en línea inferior (Y+40)
         * 
         * @param nMinutos Número de minutos a mostrar en el mensaje (rango típico: 1-120)
         * @param lLimpiar true: limpia pantalla completa (redundante), false: solo actualiza zona texto
         * 
         * @note **LIMPIEZA REDUNDANTE:** Siempre limpia al inicio, parámetro lLimpiar parece innecesario
         * @note **FORMATO MENSAJE:** "Temporizacion" (línea superior) + "X min" (línea inferior)
         * @note **POSICIONES:** Título en Y-30 (arriba), Minutos en Y+40 (abajo)
         * @note **ZONA BORRADO:** Si lLimpiar=false, borra rectángulo 120x20 en posición de minutos
         * @note **FUENTE:** FreeSans9pt7b con tamaño 2, centrado horizontalmente
         * 
         * @todo Revisar lógica de lLimpiar que parece redundante con ClearPantalla() inicial
         * @todo Considerar añadir validación de rango para nMinutos
         * 
         * @see MostrarCalefaccionTemporizada() - Función relacionada para countdown detallado
         * @see Config::Display::COLOR_INFO - Color del texto utilizado
         * @see Config::Display::COLOR_FONDO - Color de fondo para borrado
         * 
         * @since v1.0
         * @author Julian Salas Bartolomé
         * @date 2025-09-10
         */        
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
            snprintf(buffer, sizeof(buffer), "Temporitzacio");
            M5Dial.Display.drawString(buffer, x, y - 30);
            snprintf(buffer, sizeof(buffer), " %d min", nMinutos);
            M5Dial.Display.drawString(buffer, x, y + 40);
        }
        /**
         * @brief Muestra countdown de calefacción temporizada con tiempo restante
         * 
         * @details Presenta el sprite de calefacción temporal junto con el tiempo restante
         *          en formato HH:MM:SS. Utilizado para mostrar el countdown visual cuando
         *          la calefacción está en modo temporal automático con tiempo límite.
         *          
         *          **PROCESO EJECUTADO:**
         *          1. Limpia pantalla y muestra sprite CALEFACCION_TEMP (si limpiar=true)
         *          2. Sprite centrado con escala 1.2x en posición Y-15
         *          3. Borra zona de texto de tiempo (siempre)
         *          4. Renderiza tiempo restante en formato HH:MM:SS en posición Y+65
         * 
         * @param nHora Horas restantes del countdown (0-23)
         * @param nMinutos Minutos restantes del countdown (0-59)
         * @param nSegundos Segundos restantes del countdown (0-59)
         * @param limpiar true: limpia pantalla y muestra sprite CALEFACCION_TEMP, false: solo actualiza tiempo
         * 
         * @note **SPRITE CALEFACCIÓN:** Si limpiar=true, muestra CALEFACCION_TEMP centrado con escala 1.2x
         * @note **POSICIÓN SPRITE:** Centro X, Centro Y-15 píxeles (ligeramente arriba)
         * @note **BORRADO SELECTIVO:** Siempre borra rectángulo 120x20 en zona de tiempo antes de actualizar
         * @note **FORMATO TIEMPO:** HH:MM:SS con separadores ":" y padding con ceros
         * @note **POSICIÓN TIEMPO:** Centro X, Centro Y + 65 píxeles (debajo del sprite)
         * @note **OPTIMIZACIÓN:** Si limpiar=false, solo actualiza zona de texto (para updates frecuentes)
         * 
         * @warning Si falla inicialización del sprite CALEFACCION_TEMP, la función retorna sin mostrar nada
         * @warning Los valores de tiempo deben estar en rangos válidos (HH: 0-23, MM/SS: 0-59)
         * 
         * @see Config::SpriteIndex::CALEFACCION_TEMP - Sprite de calefacción temporal mostrado
         * @see MensajeTemporizacion() - Función relacionada para mensaje simple de minutos
         * @see Config::Display::COLOR_INFO - Color del texto del countdown
         * @see Config::Display::COLOR_FONDO - Color para borrado selectivo
         * 
         * @since v1.0
         * @author Julian Salas Bartolomé
         * @date 2025-09-10
         */
        void MostrarCalefaccionTemporizada (int nHora, int nMinutos, int nSegundos, bool limpiar)
        {
            int x = M5Dial.Display.width() / 2;
            int y = ( M5Dial.Display.height()  / 2 );

            if (limpiar) 
            {
                ClearPantalla();
                if (!SPRITES[Config::SpriteIndex::CALEFACCION_TEMP].inicializar()) return;
                M5Canvas*& sprite = SPRITES[Config::SpriteIndex::CALEFACCION_TEMP].canvas;
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

    // ---------------------------------------------------------------------------
    // Funciones Sprites    
    // ---------------------------------------------------------------------------
        /**
         * @brief Muestra un sprite centrado con su texto asociado configurado automáticamente
         * 
         * @details Función principal para renderizar sprites del sistema campanario. Realiza
         *          inicialización automática (lazy loading) del sprite si es necesario, limpia
         *          la pantalla, renderiza el sprite centrado y muestra el texto asociado con
         *          la configuración específica definida en el array SPRITES[].
         *          
         *          **PROCESO EJECUTADO:**
         *          1. Limpia pantalla completa (siempre)
         *          2. Valida índice del sprite (rango 0 a MAX_SPRITES-1)
         *          3. Inicializa sprite automáticamente si no está creado (lazy loading)
         *          4. Renderiza sprite centrado con escala 1.0x
         *          5. Configura fuente, color y alineación desde array SPRITES[]
         *          6. Dibuja texto asociado con offset Y específico del sprite
         * 
         * @param indice Índice del sprite en el array SPRITES[] (usar Config::SpriteIndex::*)
         * 
         * @retval void No retorna valor, pero registra errores en debug si ocurren
         * 
         * @note **VALIDACIÓN AUTOMÁTICA:** Verifica que el índice esté en rango válido (0 a MAX_SPRITES-1)
         * @note **LAZY LOADING:** El canvas se crea automáticamente en primera llamada si no existe
         * @note **CONFIGURACIÓN AUTOMÁTICA:** Usa configuración del array SPRITES[indice]:
         *       - Imagen: SPRITES[indice].imagen (datos JPG)
         *       - Texto: SPRITES[indice].texto
         *       - Color: SPRITES[indice].color
         *       - Offset Y: SPRITES[indice].offsetY
         * @note **POSICIÓN SPRITE:** Centro de pantalla con escala 1.0x (tamaño original)
         * @note **POSICIÓN TEXTO:** Centro X, Centro Y + offsetY específico del sprite
         * @note **FUENTE:** FreeSans9pt7b con tamaño 2, alineación centrada
         * 
         * @warning **FUNCIÓN COSTOSA:** Primera llamada crea canvas en memoria (operación lenta)
         * @warning **ÍNDICES INVÁLIDOS:** Se registran en debug pero no causan crash del sistema
         * @warning **FALLO INICIALIZACIÓN:** Si no se puede crear sprite, función retorna sin mostrar nada
         * 
         * @see SPRITES[] - Array de configuración completa de sprites
         * @see SpriteData::inicializar() - Método de inicialización automática
         * @see Config::SpriteIndex - Constantes de índices válidos para sprites
         * @see Config::Display::MAX_SPRITES - Límite máximo de sprites soportados
         * @see MensajeIconoTexto() - Función alternativa con texto personalizable
         * 
         * @since v1.0 - Versión inicial con arrays separados y creación manual
         * @since v2.0 - Refactorizado para usar SpriteData unificado con lazy loading automático
         * 
         * @author Julian Salas Bartolomé
         * @date 2025-09-10
         */ 
        void MostrarSprite(int indice) {
             ClearPantalla();

            if (indice < 0 || indice >= Config::Display::MAX_SPRITES) {
                DBG_DISPLAY_PRINTF("MostrarSprite() - Índice inválido: %d", indice);
                return;
            }
        
            // ✅ USAR la nueva estructura:
            if (!SPRITES[indice].inicializar()) {
                DBG_DISPLAY_PRINTF("MostrarSprite() - Error inicializando sprite %d", indice);
                return;
            }

            // Mostrar en el centro de la pantalla
            int x = M5Dial.Display.width() / 2;
            int y = M5Dial.Display.height() / 2;
            SPRITES[indice].canvas->pushRotateZoom(x, y, 0, 1.0, 1.0, TFT_BLACK);
            M5Dial.Display.setTextDatum(middle_center);
            M5Dial.Display.setTextColor(SPRITES[indice].color, Config::Display::COLOR_FONDO);
            M5Dial.Display.setTextFont(&fonts::FreeSans9pt7b);
            M5Dial.Display.setTextSize(2);
            M5Dial.Display.drawString(SPRITES[indice].texto, x, y + SPRITES[indice].offsetY);

            DBG_DISPLAY_PRINTF("MostrarSprite() - Sprite %d (%s) mostrado con texto", indice, SPRITES[indice].nombre);
        }        
