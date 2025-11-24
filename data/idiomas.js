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
        
        // Adicionales para Alarmas.js
        'actualizar': 'Actualitzar',
        'estadisticas_sistema': 'Estadístiques del Sistema',
        'total': 'Total',
        'habilitadas': 'Habilitades',
        'deshabilitadas': 'Deshabilitades',
        'espacio_libre': 'Espai lliure',
        'calculando': 'Calculant',
        'no_alarmas': 'No hi ha alarmes configurades',
        'deshabilitar': 'Desactivar',
        'habilitar': 'Activar', 
        'editar': 'Editar',
        'eliminar': 'Esborrar',
        'modificar_alarma': 'Modificar Alarma',
        'crear_alarma': 'Crear Alarma',
        'confirmar_eliminar_alarma': 'Estàs segur d\'eliminar l\'alarma',
        'accion_no_deshacer': 'Aquesta acció no es pot desfer',
        'conectando_servidor': 'Connectant al servidor',
        'conectado_servidor': 'Connectat al servidor',
        'conexion_perdida': 'Connexió perduda. Reintentant',
        'error_conexion': 'Error de connexió',
        'solicitando_datos': 'Sol·licitant dades',
        'no_conexion_servidor': 'No hi ha connexió amb el servidor',
        'alarmas_cargadas': 'alarmes carregades',
        'error_procesando_alarmas': 'Error processant dades d\'alarmes',
        'alarma_creada_id': 'Alarma creada amb ID',
        'alarma_modificada_correctamente': 'modificada correctament',
        'alarma_eliminada': 'Alarma eliminada',
        'editando_alarma': 'Editant alarma',
        'eliminando_alarma': 'Eliminant alarma',
        'nombre_obligatorio': 'El nom és obligatori',
        'hora_entre_0_23': 'L\'hora ha d\'estar entre 0 i 23',
        'minutos_entre_0_59': 'Els minuts han d\'estar entre 0 i 59',
        'cambiando_estado': 'Canviant estat',
        'procesando': 'Processant',
        'actualizando': 'Actualitzant',
        'eliminacion_cancelada': 'Eliminació cancel·lada per l\'usuari',
        
        // Días de la semana
        'Lunes': 'Dilluns',
        'Martes': 'Dimarts', 
        'Miércoles': 'Dimecres',
        'Jueves': 'Dijous',
        'Viernes': 'Divendres',
        'Sábado': 'Dissabte',
        'Domingo': 'Diumenge',
        'Todos los días': 'Tots els dies',
        
        // Acciones
        'Encender': 'Encendre',
        'Apagar': 'Apagar',
        'Subir': 'Pujar',
        'Bajar': 'Baixar',
        'Activar': 'Activar',
        'Desactivar': 'Desactivar',
        'Acción': 'Acció',
        
        // Valores específicos de acciones
        'Misa': 'Missa',
        'Difuntos': 'Difunts',
        'Fiesta': 'Festa',
        'Angelus': 'Àngelus',
        'Cuartos': 'Quarts',
        'Horas': 'Hores',
        'Repique': 'Repic',
        'Volteo': 'Volteig',
        'Toque': 'Toc',
        'Manual': 'Manual',
        'Campana': 'Campana',
        'Calefaccion': 'Calefacció',
        'Auxiliar': 'Auxiliar',
        
        // Variantes posibles con diferentes formatos
        'misa': 'missa',
        'difuntos': 'difunts',
        'fiesta': 'festa',
        'angelus': 'àngelus',
        'cuartos': 'quarts',
        'horas': 'hores',
        'repique': 'repic',
        'volteo': 'volteig',
        'toque': 'toc',
        'manual': 'manual',
        'campana': 'campana',
        'calefaccion': 'calefacció',
        'auxiliar': 'auxiliar',

        // CONFIGURACIÓN AVANZADA
        'pin_acceso': 'Accés a Configuració',
        'introducir_pin': 'Introdueix el PIN d\'accés:',
        'pin_incorrecto': 'PIN incorrecte',
        'config_telegram': 'Telegram',
        'config_telegram_desc': 'Configurar notificacions',
        'config_wifi': 'WiFi',
        'config_wifi_desc': 'Xarxa i connexió',
        'config_sistema': 'Sistema',
        'config_sistema_desc': 'Ajustos generals',
        'nombre_dispositivo': 'Nom del dispositiu:',
        'nombre_dispositivo_ayuda': 'Aquest nom s\'utilitzarà per identificar el campanari',
        'ubicacion_dispositivo': 'Ubicació:',
        'notificaciones_activar': 'Notificacions a enviar:',
        'notif_inicio': '🔔 Inici del sistema',
        'notif_misa': '⛪ Toc de Missa',
        'notif_difuntos': '🕊️ Toc de Difunts',
        'notif_fiesta': '🎉 Toc de Festa',
        'notif_stop': '🛑 Parada de seqüències',
        'notif_calefaccion': '🔥 Calefacció activada',
        'notif_calefaccion_off': '❄️ Calefacció desactivada',
        'notif_alarma': '⏰ Execució d\'alarmes',
        'notif_errores': '⚠️ Errors crítics',
        'notif_internet': '🌐 Reconnexíó a Internet',
        'notif_hora': '🕐 Tocs d\'hora',
        'notif_mediahora': '🕜 Tocs de mitges hores',
        'notif_ntp': '⏱️ Sincronització NTP',
        'notif_dns': '🌐 Actualització DNS',
        'notif_alarma_programada': '⏰ Alarma programada executada',
        'configuracion_guardada': 'Configuració guardada correctament',
        //Calefacción
        "tipo_calefaccion": "🔥 Calefacció",
        "alarma_calefaccion": "Alarma de Calefacció",
        "duracion_15min": "15 minuts",
        "duracion_30min": "30 minuts", 
        "duracion_45min": "45 minuts",
        "duracion_1h": "1 hora",
        "duracion_1h30m": "1h 30m",
        "duracion_2h": "2 hores",
        "duracion_3h": "3 hores",
        "duracion_4h": "4 hores",
        //ota
        "actualizacion": "Actualització",
        "actualizacion_desc": "Actualitzar Firmware",
        "version_actual": "Versió actual:",
        "version_disponible": "Versió disponible:",
        "ota_comprobar_desc": "Comprova si hi ha actualitzacions disponibles",
        "comprobar": "Comprovar",
        "notas_version": "Notes de la versió:",
        "ota_actualizando": "Actualitzant...",
        "ota_descargando": "Descarregant firmware...",
        "ota_instalando": "Instal·lant actualització...",
        "ota_completado": "Actualització completada!",
        "ota_reiniciando": "Reiniciant dispositiu...",
        "ota_error": "Error en l'actualització",
        "ota_no_disponible": "No hi ha actualitzacions disponibles",
        "ota_nueva_version": "Nova versió disponible!",
        "ota_firmware_actualizado": "El firmware està actualitzat"
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
        
        // Adicionales para Alarmas.js
        'actualizar': 'Actualizar',
        'estadisticas_sistema': 'Estadísticas del Sistema',
        'total': 'Total',
        'habilitadas': 'Habilitadas',
        'deshabilitadas': 'Deshabilitadas',
        'espacio_libre': 'Espacio libre',
        'calculando': 'Calculando',
        'no_alarmas': 'No hay alarmas configuradas',
        'deshabilitar': 'Deshabilitar',
        'habilitar': 'Habilitar',
        'editar': 'Editar',
        'eliminar': 'Eliminar',
        'modificar_alarma': 'Modificar Alarma',
        'crear_alarma': 'Crear Alarma',
        'confirmar_eliminar_alarma': '¿Estás seguro de eliminar la alarma',
        'accion_no_deshacer': 'Esta acción no se puede deshacer',
        'conectando_servidor': 'Conectando al servidor',
        'conectado_servidor': 'Conectado al servidor',
        'conexion_perdida': 'Conexión perdida. Reintentando',
        'error_conexion': 'Error de conexión',
        'solicitando_datos': 'Solicitando datos',
        'no_conexion_servidor': 'No hay conexión con el servidor',
        'alarmas_cargadas': 'alarmas cargadas',
        'error_procesando_alarmas': 'Error procesando datos de alarmas',
        'alarma_creada_id': 'Alarma creada con ID',
        'alarma_modificada_correctamente': 'modificada correctamente',
        'alarma_eliminada': 'Alarma eliminada',
        'editando_alarma': 'Editando alarma',
        'eliminando_alarma': 'Eliminando alarma',
        'nombre_obligatorio': 'El nombre es obligatorio',
        'hora_entre_0_23': 'La hora debe estar entre 0 y 23',
        'minutos_entre_0_59': 'Los minutos deben estar entre 0 y 59',
        'cambiando_estado': 'Cambiando estado',
        'procesando': 'Procesando',
        'actualizando': 'Actualizando',
        'eliminacion_cancelada': 'Eliminación cancelada por el usuario',
        
        // Días de la semana
        'Lunes': 'Lunes',
        'Martes': 'Martes', 
        'Miércoles': 'Miércoles',
        'Jueves': 'Jueves',
        'Viernes': 'Viernes',
        'Sábado': 'Sábado',
        'Domingo': 'Domingo',
        'Todos los días': 'Todos los días',
        
        // Acciones
        'Encender': 'Encender',
        'Apagar': 'Apagar',
        'Subir': 'Subir',
        'Bajar': 'Bajar',
        'Activar': 'Activar',
        'Desactivar': 'Desactivar',
        'Acción': 'Acción',
        
        // Valores específicos de acciones
        'Misa': 'Misa',
        'Difuntos': 'Difuntos',
        'Fiesta': 'Fiesta',
        'Angelus': 'Angelus',
        'Cuartos': 'Cuartos',
        'Horas': 'Horas',
        'Repique': 'Repique',
        'Volteo': 'Volteo',
        'Toque': 'Toque',
        'Manual': 'Manual',
        'Campana': 'Campana',
        'Calefaccion': 'Calefacción',
        'Auxiliar': 'Auxiliar',
        
        // Variantes posibles con diferentes formatos
        'misa': 'misa',
        'difuntos': 'difuntos',
        'fiesta': 'fiesta',
        'angelus': 'angelus',
        'cuartos': 'cuartos',
        'horas': 'horas',
        'repique': 'repique',
        'volteo': 'volteo',
        'toque': 'toque',
        'manual': 'manual',
        'campana': 'campana',
        'calefaccion': 'calefacción',
        'auxiliar': 'auxiliar',

        // CONFIGURACIÓN AVANZADA
        'pin_acceso': 'Acceso a Configuración',
        'introducir_pin': 'Introduce el PIN de acceso:',
        'pin_incorrecto': 'PIN incorrecto',
        'config_telegram': 'Telegram',
        'config_telegram_desc': 'Configurar notificaciones',
        'config_wifi': 'WiFi',
        'config_wifi_desc': 'Red y conexión',
        'config_sistema': 'Sistema',
        'config_sistema_desc': 'Ajustes generales',
        'nombre_dispositivo': 'Nombre del dispositivo:',
        'nombre_dispositivo_ayuda': 'Este nombre se utilizará para identificar el campanario',
        'ubicacion_dispositivo': 'Ubicación:',
        'notificaciones_activar': 'Notificaciones a enviar:',
        'notif_inicio': '🔔 Inicio del sistema',
        'notif_misa': '⛪ Toque de Misa',
        'notif_difuntos': '🕊️ Toque de Difuntos',
        'notif_fiesta': '🎉 Toque de Fiesta',
        'notif_stop': '🛑 Parada de secuencias',
        'notif_calefaccion': '🔥 Calefacción activada',
        'notif_calefaccion_off': '❄️ Calefacción desactivada',
        'notif_alarma': '⏰ Ejecución de alarmas',
        'notif_errores': '⚠️ Errores críticos',
        'notif_internet': '🌐 Reconexión a Internet',
        'notif_hora': '🕐 Toques de hora',
        'notif_mediahora': '🕜 Toques de medias horas',
        'notif_ntp': '⏱️ Sincronización NTP',
        'notif_dns': '🌐 Actualización DNS',
        'notif_alarma_programada': '⏰ Alarma programada ejecutada',
        'configuracion_guardada': 'Configuración guardada correctamente',

        //Calefacción
       "tipo_calefaccion": "🔥 Calefacción",
        "alarma_calefaccion": "Alarma de Calefacción", 
        "duracion_15min": "15 minutos",
        "duracion_30min": "30 minutos",
        "duracion_45min": "45 minutos",
        "duracion_1h": "1 hora",
        "duracion_1h30m": "1h 30m",
        "duracion_2h": "2 horas",
        "duracion_3h": "3 horas",
        "duracion_4h": "4 horas",
        //ota
        "actualizacion": "Actualización",
        "actualizacion_desc": "Actualizar Firmware",
        "version_actual": "Versión actual:",
        "version_disponible": "Versión disponible:",
        "ota_comprobar_desc": "Comprueba si hay actualizaciones disponibles",
        "comprobar": "Comprobar",
        "notas_version": "Notas de la versión:",
        "ota_actualizando": "Actualizando...",
        "ota_descargando": "Descargando firmware...",
        "ota_instalando": "Instalando actualización...",
        "ota_completado": "¡Actualización completada!",
        "ota_reiniciando": "Reiniciando dispositivo...",
        "ota_error": "Error en la actualización",
        "ota_no_disponible": "No hay actualizaciones disponibles",
        "ota_nueva_version": "¡Nueva versión disponible!",
        "ota_firmware_actualizado": "El firmware está actualizado"
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
        
        // Actualizar textos dinámicos de la página de alarmas
        if (typeof actualizarTextosAlarmas === 'function') {
            actualizarTextosAlarmas();
            console.log("⏰ Textos de alarmas actualizados después del cambio de idioma");
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
        // Actualizar interfaz con idioma temporal inmediatamente
        actualizarSelectorIdioma();
        actualizarTextosInterfaz();
        
        // Esperar a que WebSocket esté conectado con más paciencia
        let intentos = 0;
        const maxIntentos = 10;
        
        const intentarSolicitar = () => {
            intentos++;
            console.log(`🔄 Intento ${intentos} de solicitar idioma del servidor`);
            
            solicitarIdiomaDelServidor();
            
            if (intentos < maxIntentos) {
                setTimeout(intentarSolicitar, 2000);
            } else {
                console.log("⚠️ Máximo de intentos alcanzado, usando idioma local");
            }
        };
        
        setTimeout(intentarSolicitar, 1000);
    });
    
    // Si ya está cargado, actualizar inmediatamente
    if (document.readyState !== 'loading') {
        actualizarSelectorIdioma();
        actualizarTextosInterfaz();
        
        // Intentar solicitar idioma del servidor
        let intentos = 0;
        const maxIntentos = 10;
        
        const intentarSolicitar = () => {
            intentos++;
            console.log(`🔄 Intento ${intentos} de solicitar idioma del servidor (fallback)`);
            
            solicitarIdiomaDelServidor();
            
            if (intentos < maxIntentos) {
                setTimeout(intentarSolicitar, 2000);
            } else {
                console.log("⚠️ Máximo de intentos alcanzado, usando idioma local");
            }
        };
        
        setTimeout(intentarSolicitar, 1000);
    }
}
/**
 * Solicita el idioma configurado en el servidor
 */
function solicitarIdiomaDelServidor() {
    // Buscar WebSocket disponible en diferentes páginas
    let ws = null;
    
    if (typeof websocket !== 'undefined' && websocket.readyState === WebSocket.OPEN) {
        ws = websocket;
    } else if (typeof window.alarmManager !== 'undefined' && window.alarmManager.ws && window.alarmManager.ws.readyState === WebSocket.OPEN) {
        ws = window.alarmManager.ws;
    } else if (typeof window.ws !== 'undefined' && window.ws.readyState === WebSocket.OPEN) {
        ws = window.ws;
    }
    
    if (ws) {
        console.log("📤 Solicitando idioma del servidor...");
        ws.send("GET_IDIOMA");
    } else {
        console.warn("⚠️ WebSocket no disponible, usando idioma local");
        // Reintentar después con más paciencia
        setTimeout(solicitarIdiomaDelServidor, 3000);
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

    // Buscar WebSocket disponible para enviar al servidor
    let ws = null;
    
    if (typeof websocket !== 'undefined' && websocket.readyState === WebSocket.OPEN) {
        ws = websocket;
    } else if (typeof window.alarmManager !== 'undefined' && window.alarmManager.ws && window.alarmManager.ws.readyState === WebSocket.OPEN) {
        ws = window.alarmManager.ws;
    } else if (typeof window.ws !== 'undefined' && window.ws.readyState === WebSocket.OPEN) {
        ws = window.ws;
    }
    
    if (ws) {
        ws.send(`SET_IDIOMA:${nuevoIdioma}`);
        console.log("📤 Idioma enviado al servidor para persistir");
    } else {
        console.warn("⚠️ No se pudo enviar el idioma al servidor (WebSocket no disponible)");
    }
}

/**
 * Fuerza la recarga del idioma desde el servidor
 */
function forzarRecargaIdioma() {
    console.log("🔄 Forzando recarga del idioma...");
    
    // Intentar con todos los WebSockets disponibles
    let ws = null;
    
    if (typeof websocket !== 'undefined' && websocket.readyState === WebSocket.OPEN) {
        ws = websocket;
    } else if (typeof window.alarmManager !== 'undefined' && window.alarmManager.ws && window.alarmManager.ws.readyState === WebSocket.OPEN) {
        ws = window.alarmManager.ws;
    } else if (typeof window.ws !== 'undefined' && window.ws.readyState === WebSocket.OPEN) {
        ws = window.ws;
    }
    
    if (ws) {
        ws.send("GET_IDIOMA");
        console.log("📤 Solicitud de idioma enviada");
    } else {
        console.warn("⚠️ No hay WebSocket disponible para solicitar idioma");
    }
}

// Hacer la función disponible globalmente
window.forzarRecargaIdioma = forzarRecargaIdioma;

// Inicializar automáticamente
inicializarIdiomas();