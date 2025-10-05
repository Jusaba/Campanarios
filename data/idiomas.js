// ============================================================================
// SISTEMA DE INTERNACIONALIZACIÓN (i18n)
// ============================================================================

const IDIOMAS = {
    'ca': {
        // GENERAL
        'titulo_principal': '🔔 Control del Campanar',
        'conectando': 'Connectant...',
        'conectado': 'Connectat',
        'desconectado': 'Desconnectat',
        'error': 'Error',
        'aceptar': 'Acceptar',
        'cancelar': 'Cancel·lar',
        'guardar': 'Desar',
        'eliminar': 'Eliminar',
        'editar': 'Editar',
        'crear': 'Crear',
        'cerrar': 'Tancar',
        'si': 'Sí',
        'no': 'No',
        
        // SECCIONES PRINCIPALES
        'seccion_toques': 'Tocs i Campanades',
        'seccion_calefaccion': 'Sistema de Calefacció',
        'seccion_configuracion': 'Configuració i Gestió',
        
        // BOTONES DE TOQUES
        'boton_misa': 'Missa',
        'boton_difuntos': 'Difunts',
        'boton_fiesta': 'Festa',
        'parar_secuencia': '⏹️ Aturar Seqüència',
        
        // CALEFACCIÓN
        'calefaccion_apagada': 'Apagada',
        'calefaccion_encendida': 'Encesa',
        'calefaccion_encender': '🔥 Encendre',
        'calefaccion_configurar': '⏰ Configurar',
        'tiempo_restante': '⏱️ Temps restant:',
        'configurar_tiempo': 'Configurar Temps',
        'minutos': 'minuts',
        'horas': 'hores',
        'calefaccion_apagar': 'Apagar',
        'total_minutos': 'Total:',
        'tiempo_agotado': 'Temps acabat',
        'calefaccion_funcionando': 'Funcionant...',        
        'limite_maximo': 'Màxim permès: 120 minuts',

        // CONFIGURACIÓN
        'gestion_alarmas': 'Gestió d\'Alarmes',
        'gestion_alarmas_desc': 'Programar tocs automàtics',
        'estadisticas': 'Estadístiques',
        'estadisticas_desc': 'Historial i informes',
        'configuracion': 'Configuració',
        'configuracion_desc': 'Ajustos del sistema',
        'idioma': 'Idioma',
        'seleccionar_idioma': 'Seleccionar idioma',
        
        // ALARMAS
        'alarmas_titulo': 'Gestió d\'Alarmes',
        'alarmas_personalizadas': 'Alarmes Personalitzades',
        'nueva_alarma': 'Nova Alarma',
        'nombre_alarma': 'Nom de l\'alarma',
        'descripcion_alarma': 'Descripció',
        'dia_semana': 'Dia de la setmana',
        'hora': 'Hora',
        'minuto': 'Minut',
        'tipo_accion': 'Tipus d\'acció',
        'habilitada': 'Habilitada',
        'modificar_alarma': 'Modificar Alarma',
        'eliminar_alarma': 'Eliminar Alarma',
        'confirmar_eliminar': 'Estàs segur que vols eliminar aquesta alarma?',
        
        // DÍAS DE LA SEMANA
        'todos_los_dias': 'Tots els dies',
        'domingo': 'Diumenge',
        'lunes': 'Dilluns',
        'martes': 'Dimarts',
        'miercoles': 'Dimecres',
        'jueves': 'Dijous',
        'viernes': 'Divendres',
        'sabado': 'Dissabte',
        
        // TIPOS DE ACCIÓN
        'tipo_misa': 'Missa',
        'tipo_difuntos': 'Difunts',
        'tipo_fiesta': 'Festa',
        
        // MENSAJES
        'alarma_creada': 'Alarma creada correctament',
        'alarma_modificada': 'Alarma modificada correctament',
        'alarma_eliminada': 'Alarma eliminada correctament',
        'error_crear_alarma': 'Error en crear l\'alarma',
        'error_modificar_alarma': 'Error en modificar l\'alarma',
        'error_eliminar_alarma': 'Error en eliminar l\'alarma',
        'maximo_alarmas': 'Màxim d\'alarmes assolit',
        
        // ESTADOS
        'estado_activa': 'Activa',
        'estado_inactiva': 'Inactiva',
        'ultima_ejecucion': 'Última execució',
        'proxima_ejecucion': 'Propera execució',
        
        // VALIDACIONES
        'campo_requerido': 'Aquest camp és obligatori',
        'hora_invalida': 'Hora invàlida (0-23)',
        'minuto_invalido': 'Minut invàlid (0-59)',
        'nombre_muy_largo': 'El nom és massa llarg (màx 49 caràcters)',
        'descripcion_muy_larga': 'La descripció és massa llarga (màx 99 caràcters)',

        // ACERCA DE
        'acerca_de': 'Acerca de...',
        'acerca_de_desc': 'Informació del sistema',
        'sistema_campanario': 'Sistema de Control del Campanar',
        'version': 'Versió 1.0.0',
        'desarrollado_por': 'Desenvolupat per:',
        'caracteristicas': 'Característiques:',
        'feat_control_campanas': 'Control remot de campanes',
        'feat_calefaccion': 'Sistema de calefacció intel·ligent',
        'feat_multiidioma': 'Interfície multiidioma',
        'feat_alarmas': 'Programació d\'alarmes',
        'tecnologia': 'Tecnologia:',
        'copyright': '© 2024 Tu Empresa. Tots els drets reservats.',
        'licencia': 'Llicència: MIT',
        'cerrar': 'Tancar',
    
        // Repique de campanas
        'titulo_campanas': '🔔 Repic en Curs',
        'repique_en_curso': 'Repic en Curs',
        'secuencia_activa': 'Seqüència activa',
        'campana_1': 'Campana 1',
        'campana_2': 'Campana 2',
    },
    
    'es': {
        // GENERAL
        'titulo_principal': '🔔 Control de Campanario',
        'conectando': 'Conectando...',
        'conectado': 'Conectado',
        'desconectado': 'Desconectado',
        'error': 'Error',
        'aceptar': 'Aceptar',
        'cancelar': 'Cancelar',
        'guardar': 'Guardar',
        'eliminar': 'Eliminar',
        'editar': 'Editar',
        'crear': 'Crear',
        'cerrar': 'Cerrar',
        'si': 'Sí',
        'no': 'No',
        
        // SECCIONES PRINCIPALES
        'seccion_toques': 'Toques y Campanadas',
        'seccion_calefaccion': 'Sistema de Calefacción',
        'seccion_configuracion': 'Configuración y Gestión',
        
        // BOTONES DE TOQUES
        'boton_misa': 'Misa',
        'boton_difuntos': 'Difuntos',
        'boton_fiesta': 'Fiesta',
        'parar_secuencia': '⏹️ Parar Secuencia',
        
        // CALEFACCIÓN
        'calefaccion_apagada': 'Apagada',
        'calefaccion_encendida': 'Encendida',
        'calefaccion_encender': '🔥 Encender',
        'calefaccion_configurar': '⏰ Configurar',
        'tiempo_restante': '⏱️ Tiempo restante:',
        'configurar_tiempo': 'Configurar Tiempo',
        'minutos': 'minutos',
        'horas': 'horas',
        'calefaccion_apagar': 'Apagar',
        'total_minutos': 'Total:',
        'tiempo_agotado': 'Tiempo agotado',
        'calefaccion_funcionando': 'Funcionando...',
        'limite_maximo': 'Máximo permitido: 120 minutos',
        
        // CONFIGURACIÓN
        'gestion_alarmas': 'Gestión de Alarmas',
        'gestion_alarmas_desc': 'Programar toques automáticos',
        'estadisticas': 'Estadísticas',
        'estadisticas_desc': 'Historial y reportes',
        'configuracion': 'Configuración',
        'configuracion_desc': 'Ajustes del sistema',
        'idioma': 'Idioma',
        'seleccionar_idioma': 'Seleccionar idioma',
        
        // ALARMAS
        'alarmas_titulo': 'Gestión de Alarmas',
        'alarmas_personalizadas': 'Alarmas Personalizadas',
        'nueva_alarma': 'Nueva Alarma',
        'nombre_alarma': 'Nombre de la alarma',
        'descripcion_alarma': 'Descripción',
        'dia_semana': 'Día de la semana',
        'hora': 'Hora',
        'minuto': 'Minuto',
        'tipo_accion': 'Tipo de acción',
        'habilitada': 'Habilitada',
        'modificar_alarma': 'Modificar Alarma',
        'eliminar_alarma': 'Eliminar Alarma',
        'confirmar_eliminar': '¿Estás seguro que quieres eliminar esta alarma?',
        
        // DÍAS DE LA SEMANA
        'todos_los_dias': 'Todos los días',
        'domingo': 'Domingo',
        'lunes': 'Lunes',
        'martes': 'Martes',
        'miercoles': 'Miércoles',
        'jueves': 'Jueves',
        'viernes': 'Viernes',
        'sabado': 'Sábado',
        
        // TIPOS DE ACCIÓN
        'tipo_misa': 'Misa',
        'tipo_difuntos': 'Difuntos',
        'tipo_fiesta': 'Fiesta',
        
        // MENSAJES
        'alarma_creada': 'Alarma creada correctamente',
        'alarma_modificada': 'Alarma modificada correctamente',
        'alarma_eliminada': 'Alarma eliminada correctamente',
        'error_crear_alarma': 'Error al crear la alarma',
        'error_modificar_alarma': 'Error al modificar la alarma',
        'error_eliminar_alarma': 'Error al eliminar la alarma',
        'maximo_alarmas': 'Máximo de alarmas alcanzado',
        
        // ESTADOS
        'estado_activa': 'Activa',
        'estado_inactiva': 'Inactiva',
        'ultima_ejecucion': 'Última ejecución',
        'proxima_ejecucion': 'Próxima ejecución',
        
        // VALIDACIONES
        'campo_requerido': 'Este campo es obligatorio',
        'hora_invalida': 'Hora inválida (0-23)',
        'minuto_invalido': 'Minuto inválido (0-59)',
        'nombre_muy_largo': 'El nombre es demasiado largo (máx 49 caracteres)',
        'descripcion_muy_larga': 'La descripción es demasiado larga (máx 99 caracteres)',

        // ACERCA DE
         'acerca_de': 'Acerca de...',
         'acerca_de_desc': 'Información del sistema',
         'sistema_campanario': 'Sistema de Control del Campanario',
         'version': 'Versión 1.0.0',
         'desarrollado_por': 'Desarrollado por:',
         'caracteristicas': 'Características:',
         'feat_control_campanas': 'Control remoto de campanas',
         'feat_calefaccion': 'Sistema de calefacción inteligente',
         'feat_multiidioma': 'Interfaz multiidioma',
         'feat_alarmas': 'Programación de alarmas',
         'tecnologia': 'Tecnología:',
         'copyright': '© 2024 Tu Empresa. Todos los derechos reservados.',
         'licencia': 'Licencia: MIT',
         'cerrar': 'Cerrar',

        // Repique de campanas
        'titulo_campanas': '🔔 Repique en Curso',
        'repique_en_curso': 'Repique en Curso',
        'secuencia_activa': 'Secuencia activa',
        'campana_1': 'Campana 1',
        'campana_2': 'Campana 2',         
    }
};

// ============================================================================
// FUNCIONES DE INTERNACIONALIZACIÓN
// ============================================================================

let idiomaActual = 'ca'; // Por defecto catalán

/**
 * Obtiene un texto traducido según el idioma actual
 * @param {string} clave - Clave del texto a traducir
 * @param {string} idioma - Idioma opcional (si no se especifica usa el actual)
 * @returns {string} Texto traducido o la clave si no se encuentra
 */
function t(clave, idioma = null) {
    const lang = idioma || idiomaActual;
    
    if (IDIOMAS[lang] && IDIOMAS[lang][clave]) {
        return IDIOMAS[lang][clave];
    }
    
    // Fallback al español si no existe en el idioma actual
    if (lang !== 'es' && IDIOMAS['es'] && IDIOMAS['es'][clave]) {
        return IDIOMAS['es'][clave];
    }
    
    // Si no encuentra nada, devuelve la clave
    console.warn(`Traducción no encontrada para: ${clave}`);
    return clave;
}

/**
 * Cambia el idioma actual y actualiza toda la interfaz
 * @param {string} nuevoIdioma - Código del idioma ('ca' o 'es')
 */
function cambiarIdioma(nuevoIdioma) {
    if (!IDIOMAS[nuevoIdioma]) {
        console.error(`Idioma no soportado: ${nuevoIdioma}`);
        return;
    }
    
    idiomaActual = nuevoIdioma;
    
    // Guardar preferencia en localStorage
    localStorage.setItem('idioma_campanario', nuevoIdioma);
    
    // Actualizar toda la interfaz
    actualizarTextosInterfaz();
    
    // Enviar al servidor para sincronizar
    if (typeof websocket !== 'undefined' && websocket.readyState === WebSocket.OPEN) {
        websocket.send(`SET_IDIOMA:${nuevoIdioma}`);
    }
}

/**
 * Actualiza todos los textos de la interfaz con el idioma actual
 */
function actualizarTextosInterfaz() {
    // Actualizar elementos con atributo data-i18n
    document.querySelectorAll('[data-i18n]').forEach(elemento => {
        const clave = elemento.getAttribute('data-i18n');
        elemento.textContent = t(clave);
    });
    
    // Actualizar placeholders
    document.querySelectorAll('[data-i18n-placeholder]').forEach(elemento => {
        const clave = elemento.getAttribute('data-i18n-placeholder');
        elemento.placeholder = t(clave);
    });
    
    // Actualizar títulos
    document.querySelectorAll('[data-i18n-title]').forEach(elemento => {
        const clave = elemento.getAttribute('data-i18n-title');
        elemento.title = t(clave);
    });
    
    // Actualizar valores de selects
    document.querySelectorAll('option[data-i18n]').forEach(elemento => {
        const clave = elemento.getAttribute('data-i18n');
        elemento.textContent = t(clave);
    });
    setTimeout(function() {
        if (typeof actualizarEstadoCalefaccion === 'function') {
            actualizarEstadoCalefaccion();
            console.log("🔥 Estado de calefacción actualizado después del cambio de idioma");
        }
    }, 100); // Pequeño delay para asegurar que los textos se han actualizado
}

/**
 * Inicializa el sistema de idiomas
 */
function inicializarIdiomas() {
    console.log("🌍 Inicializando sistema de idiomas...");
    
    // 1. Primero cargar desde localStorage como fallback
    const idiomaLocal = localStorage.getItem('idioma_campanario') || 'ca';
    idiomaActual = idiomaLocal;
    
    console.log(`Idioma local temporal: ${idiomaActual}`);
    
    // 2. Solicitar idioma del servidor cuando la conexión esté lista
    document.addEventListener('DOMContentLoaded', function() {
        // Esperar a que WebSocket esté conectado
        setTimeout(function() {
            solicitarIdiomaDelServidor();
        }, 1000); // Dar tiempo a que se conecte el WebSocket
        
        // Actualizar interfaz con idioma temporal
        actualizarSelectorIdioma();
        actualizarTextosInterfaz();
    });
    
    // Si ya está cargado, actualizar inmediatamente
    if (document.readyState !== 'loading') {
        actualizarSelectorIdioma();
        actualizarTextosInterfaz();
        setTimeout(solicitarIdiomaDelServidor, 1000);
    }
}
/**
 * Solicita el idioma configurado en el servidor
 */
function solicitarIdiomaDelServidor() {
    if (typeof websocket !== 'undefined' ) {
        console.log("📤 Solicitando idioma del servidor...");
        websocket.send("GET_IDIOMA");
    } else {
        console.warn("⚠️ WebSocket no disponible, usando idioma local");
        // Reintentar después
        setTimeout(solicitarIdiomaDelServidor, 2000);
    }
}
/**
 * Actualiza el selector de idioma en el HTML
 */
function actualizarSelectorIdioma() {
    const selector = document.getElementById('selectorIdioma');
    if (selector) {
        selector.value = idiomaActual;
        console.log(`Selector actualizado a: ${idiomaActual}`);
    }
}

/**
 * Cambia el idioma y lo sincroniza con el servidor
 */
function cambiarIdioma(nuevoIdioma) {
    if (!IDIOMAS[nuevoIdioma]) {
        console.error(`Idioma no soportado: ${nuevoIdioma}`);
        return;
    }
    
    console.log(`🔄 Cambiando idioma a: ${nuevoIdioma}`);
    
    idiomaActual = nuevoIdioma;
    
    // Guardar localmente como backup
    localStorage.setItem('idioma_campanario', nuevoIdioma);
    
    // Actualizar interfaz
    actualizarSelectorIdioma();
    actualizarTextosInterfaz();

    websocket.send(`SET_IDIOMA:${nuevoIdioma}`);
    console.log("📤 Idioma enviado al servidor para persistir");
}

// Inicializar automáticamente
inicializarIdiomas();