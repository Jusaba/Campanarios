/**
 * @file Display.cpp
 * @brief Implementación completa del sistema de display para M5Dial DialCampanario
 * 
 * @details Este archivo implementa todas las funcionalidades del sistema de display:
 *          
 *          **FUNCIONALIDADES IMPLEMENTADAS:**
 *          - Gestión unificada de sprites con inicialización automática (lazy loading)
 *          - Control suave de brillo con transiciones graduales
 *          - Visualización de estados del campanario (Stop, Misa, Difuntos, etc.)
 *          - Sistema completo de fecha/hora con formateo personalizado
 *          - Mensajes contextuales con iconos y texto sincronizado
 *          - Funciones optimizadas para actualizaciones frecuentes vs cambios completos
 *          - Sistema de temporización para calefacción con countdown visual
 *          
 *          **ARQUITECTURA IMPLEMENTADA:**
 *          - Array SPRITES[] con configuración y canvas integrados
 *          - Inicialización lazy: canvas se crean solo cuando se necesitan
 *          - Funciones wrapper optimizadas (Mensaje vs Escribe vs Mostrar)
 *          - Gestión automática de memoria y recursos de sprites
 *          - Control inteligente de limpieza de pantalla (completa vs parcial)
 * 
 * @note **PATRÓN DE USO:**
 *       - Funciones MensajeXXX(): Cambio completo de pantalla (limpian todo)
 *       - Funciones EscribeXXX(): Actualización optimizada (solo texto)
 *       - Funciones MostrarXXX(): Control manual de limpieza (parámetro limpiar)
 * 
 * @warning **DEPENDENCIAS CRÍTICAS:**
 *          - Configuracion.h debe definir todos los Config::Display::* y Config::SpriteIndex::*
 *          - Imagen.h debe contener todas las imágenes JPG referenciadas
 *          - Debug.h debe estar configurado para DBG_DISPLAY y DBG_DISPLAY_PRINTF
 *          - M5Dial.h debe estar correctamente inicializado antes de usar estas funciones
 * 
 * @author Julian Salas Bartolomé
 * @date 2025
 * @version 2.0
 * 
 * @since v1.0 - Sistema inicial con arrays separados y creación manual de sprites
 * @since v2.0 - Sistema unificado con SpriteData, lazy loading e inicialización automática
 * 
 * @see Display.h - Definiciones y documentación de la API
 * @see Configuracion.h - Constantes de configuración del sistema
 * @see Imagen.h - Definiciones de imágenes JPG
 * @see Debug.h - Sistema de logging y debug
 * 
 */
#ifndef DISPLAY_H
    #define DISPLAY_H

        #include <M5Dial.h>
        #include "Imagen.h"  // Incluye las imágenes de los iconos y sprites
        #include "Configuracion.h"
        #include "Debug.h"

        // ============================================================================
        // ESTRUCTURAS DE DATOS
        // ============================================================================

        /**
         * @brief Estructura completa de sprite con configuración y canvas renderizado
         * 
         * @details Unifica toda la información de un sprite en una sola estructura:
         *          - Configuración (imagen, texto, colores)
         *          - Canvas renderizado (M5Canvas ya creado)
         *          - Estado de inicialización
         *          
         *          Los canvas se crean automáticamente cuando se necesitan.
         * 
         * @see Config::SpriteIndex, SPRITES[]
         * 
         * @since v2.0
         */
        struct SpriteData {
            // ============================================================================
            // CONFIGURACIÓN DEL SPRITE
            // ============================================================================
            const uint8_t* imagen;          /**< @brief Puntero a imagen raw en Imagen.h */
            const char* nombre;             /**< @brief Nombre descriptivo para debug */
            const char* texto;              /**< @brief Texto a mostrar con el sprite */
            uint16_t color;                 /**< @brief Color del texto en formato RGB565 */
            int offsetY;                    /**< @brief Desplazamiento vertical del texto */
        
            // ============================================================================
            // SPRITE RENDERIZADO
            // ============================================================================
            M5Canvas* canvas;               /**< @brief Canvas renderizado del sprite */
            bool inicializado;              /**< @brief true si el canvas está creado */
        
            // ============================================================================
            // CONSTRUCTOR POR DEFECTO
            // ============================================================================
        
            /**
             * @brief Constructor por defecto - sprite vacío
             */
            SpriteData() : imagen(nullptr), nombre(""), texto(""), color(0), offsetY(0), 
                           canvas(nullptr), inicializado(false) {}
        
            /**
             * @brief Constructor con configuración
             * 
             * @param img Puntero a imagen raw
             * @param nom Nombre descriptivo
             * @param txt Texto a mostrar
             * @param col Color del texto
             * @param off Offset Y del texto
             */
            SpriteData(const uint8_t* img, const char* nom, const char* txt, 
                       uint16_t col, int off) 
                : imagen(img), nombre(nom), texto(txt), color(col), offsetY(off),
                  canvas(nullptr), inicializado(false) {}
        
            // ============================================================================
            // MÉTODO BÁSICO DE INICIALIZACIÓN
            // ============================================================================
        
            /**
             * @brief Inicializa el canvas del sprite si no está ya inicializado
             * 
             * @return true si la inicialización fue exitosa o ya estaba inicializado
             * @return false si hubo error en la creación del canvas
             */
            bool inicializar() {
                if (inicializado) return true;  // Ya está inicializado

                if (imagen == nullptr) {
                    DBG_DISPLAY_PRINTF("SpriteData::inicializar() - Imagen nullptr para sprite '%s'", nombre);
                    return false;
                }

                // Crear canvas
                canvas = new M5Canvas(&M5Dial.Display);
                if (canvas == nullptr) {
                    DBG_DISPLAY_PRINTF("SpriteData::inicializar() - Error creando canvas para sprite '%s'", nombre);
                    return false;
                }

                // Configurar canvas con la imagen
                canvas->createSprite(120, 120);  // Tamaño estándar por ahora
                canvas->drawJpg(imagen,                     // data_pointer
                           ~0u,                        // data_length (~0u = auto)
                           0,                          // X position
                           0,                          // Y position
                           Config::Display::SPRITE_WIDTH,  // Width (120)
                           Config::Display::SPRITE_HEIGHT, // Height (120)
                           0,                          // X offset
                           0,                          // Y offset
                           1.0,                        // X magnification
                           1.0,                        // Y magnification
                           datum_t::middle_center      // Datum alignment
                           );        
                inicializado = true;
                DBG_DISPLAY_PRINTF("SpriteData::inicializar() - Sprite '%s' inicializado correctamente", nombre);
                return true;
            }

            /**
             * @brief Verifica si el sprite es válido
             * 
             * @return true si tiene imagen y configuración válida
             */
            bool esValido() const {
                return (imagen != nullptr && nombre != nullptr);
            }
        };

        // ============================================================================
        // VARIABLES GLOBALES
        // ============================================================================
        
        /**
         * @brief Array unificado de sprites con configuración y canvas integrados
         * 
         * @details Array global que contiene todos los sprites del sistema con:
         *          - Configuración inicial (imagen, nombre, texto, color, offset)
         *          - Canvas renderizado (se crea automáticamente con lazy loading)
         *          - Estado de inicialización (controla creación única del canvas)
         * 
         * @note Los canvas se crean automáticamente en la primera llamada a
         *       SPRITES[indice].inicializar() o funciones que los usen
         * 
         * @warning NO es const porque los campos canvas e inicializado se modifican
         *          durante la ejecución (lazy loading)
         * 
         * @see SpriteData
         * @see Config::Display::MAX_SPRITES
         * @see Config::SpriteIndex
         */
        extern SpriteData SPRITES[Config::Display::MAX_SPRITES];
        
        /**
         * @brief Indicador de estado del brillo de pantalla
         * 
         * @details Variable global que mantiene el estado actual del brillo:
         *          - false (0): Pantalla apagada o con brillo mínimo
         *          - true (1): Pantalla encendida con brillo normal/máximo
         * 
         * @note Se actualiza automáticamente por las funciones de control de brillo
         * @note Útil para determinar si se debe encender o apagar la pantalla
         * 
         * @see BajaBrillo()
         * @see SubeBrillo()
         * @see BrilloFull()
         */
        extern boolean lBrillo;
        
        // ============================================================================
        // PROTOTIPOS DE FUNCIONES
        // ============================================================================
        // ---------------------------------------------------------------------------
        // Funciones de gestión del display
        // ---------------------------------------------------------------------------
        void InicioDisplay (void);                                                                                         //Inicializa la pantalla del dispositivo M5Dial
        void ClearPantalla (void);                                                                                         //Limpia completamente la pantalla del dispositivo M5Dial
        void BajaBrillo (void);                                                                                            //Baja el brillo hasta 0 de forma paulatina
        void SubeBrillo (int nBrilloFinal );                                                                               //Sube el brillo hasta nBrilloFinal de forma paulatina
        void BrilloFull (void);                                                                                            //Sube el brillo al máximo (127) y actualiza la variable lBrillo a 1        
        // ---------------------------------------------------------------------------
        // Funciones de mensajes de menu en pantalla
        // ---------------------------------------------------------------------------
        void MensajeIconoTexto(int spriteIndex, const char* texto, uint16_t colorTexto, int offsetY);                      //Muestra un mensaje en pantalla con un icono y un texto
        void MensajeInicio();                                                                                              // Muestra el mensaje de inicio en la pantalla principal
        void MensajeDifuntos();                                                                                            // Muestra el mensaje de difuntos en la pantalla principal
        void MensajeMisa();                                                                                                // Muestra el mensaje de misa en la pantalla principal
        void MensajeFiesta();                                                                                              // Muestra el mensaje de fiesta en la pantalla principal
        void MensajeStop();                                                                                                // Muestra el mensaje de stop en la pantalla principal
        void MensajeCalefaccionOn();                                                                                       // Muestra el mensaje de calefacción encendida en la pantalla principal
        void MensajeCalefaccionOff();                                                                                      // Muestra el mensaje de calefacción apagada en la pantalla principal
        // ---------------------------------------------------------------------------
        // Funciones de mensajes auxiliares en pantalla
        // ---------------------------------------------------------------------------
        void MostrarHora(int nHora, int nMinutos, int nSegundos, bool limpiar);                                            //Muestra la hora en pantalla, si limpiar es true limpia la pantalla antes de mostrar la hora
        void MensajeHora(int nHora, int nMinutos, int nSegundos);                                                          // Muestra la hora en pantalla, limpiando la pantalla antes de mostrarla
        void EscribeHora(int nHora, int nMinutos, int nSegundos);                                                          // Escribe la hora en pantalla sin limpiar la pantalla antes de mostrarla
        void MostrarFechaHora(int nHora, int nMinutos, int nSegundos, int nDia, int nMes, int nAno, bool limpiar = false, bool Internet = false); // Muestra la fecha y hora en pantalla, si limpiar es true limpia la pantalla antes de mostrar la fecha y hora
        void MensajeFechaHora(int nHora, int nMinutos, int nSegundos, int nDia, int nMes, int nAno, bool Internet);                       // Muestra la fecha y hora en pantalla, limpiando la pantalla antes de mostrarla
        void EscribeFechaHora(int nHora, int nMinutos, int nSegundos, int nDia, int nMes, int nAno);                       // Escribe la fecha y hora en pantalla sin limpiar la pantalla antes de mostrarla
        void MostrarCalefaccionTemporizada (int nHora, int nMinutos, int nSegundos, bool limpiar);                         // Muestra la temporización de la calefacción  en pantalla, si limpiar es true limpia la pantalla antes de mostrar la hora
        void MensajeTemporizacion (int nMinutos, bool lLimpiar);                                                           // Muestra la temporización de la calefacción  en pantalla, si limpiar es true limpia la pantalla antes de mostrar la hora
        // ---------------------------------------------------------------------------
        // Funciones Sprites    
        // ---------------------------------------------------------------------------
        void MostrarSprite ( int indice );                                                                                  //Muestra un Sprite 

#endif