// ota.js - Gestión de actualizaciones OTA

let otaState = {
    versionActual: "1.0.0",
    versionDisponible: null,
    firmwareUrl: null,
    spiffsUrl: null,
    actualizando: false,
    tipoActualizacion: null // 'firmware', 'spiffs', 'completo'
};

// Abrir modal OTA
function abrirModalOTA() {
    const modal = document.getElementById('modalOTA');
    modal.style.display = 'block';
    
    // Obtener versión actual del ESP32
    solicitarVersionActual();
    
    // Reset UI
    document.getElementById('versionDisponibleContainer').style.display = 'none';
    document.getElementById('otaProgreso').style.display = 'none';
    document.getElementById('otaNotas').style.display = 'none';
    document.getElementById('botonesActualizacion').style.display = 'none';
    document.getElementById('btnComprobarOTA').style.display = 'inline-block';
    document.getElementById('otaEstado').innerHTML = `<p>${t('ota_comprobar_desc')}</p>`;
}

// Cerrar modal OTA
function cerrarModalOTA() {
    if (otaState.actualizando) {
        if (!confirm('Hi ha una actualització en curs. Estàs segur de voler tancar?')) {
            return;
        }
    }
    document.getElementById('modalOTA').style.display = 'none';
}

// Solicitar versión actual al ESP32
function solicitarVersionActual() {
    if (websocket && websocket.readyState === WebSocket.OPEN) {
        websocket.send('GET_VERSION_OTA');
    }
}

// Comprobar si hay actualizaciones
function comprobarActualizacion() {
    if (!websocket || websocket.readyState !== WebSocket.OPEN) {
        mostrarErrorOTA('No hi ha connexió amb el servidor');
        return;
    }
    
    // Deshabilitar botón
    const btnComprobar = document.getElementById('btnComprobarOTA');
    btnComprobar.disabled = true;
    btnComprobar.textContent = t('ota_actualizando');
    
    // Mostrar progreso
    document.getElementById('otaProgreso').style.display = 'block';
    document.getElementById('otaProgressFill').style.width = '0%';
    document.getElementById('otaMensaje').textContent = t('ota_descargando');
    
    // Enviar comando al ESP32
    websocket.send('CHECK_UPDATE_OTA');
}

// Iniciar actualización
function iniciarActualizacion(tipo) {
    let mensaje = '';
    let comando = '';
    
    switch(tipo) {
        case 'firmware':
            mensaje = t('ota_confirmar_firmware') || 'Actualitzar només el firmware? Això reiniciarà el dispositiu.';
            comando = 'START_UPDATE_FIRMWARE';
            break;
        case 'spiffs':
            mensaje = t('ota_confirmar_spiffs') || 'Actualitzar només SPIFFS? Això reiniciarà el dispositiu i substituirà els fitxers.';
            comando = 'START_UPDATE_SPIFFS';
            break;
        case 'completo':
            mensaje = t('ota_confirmar_completo') || 'Actualització completa (firmware + SPIFFS)? Això reiniciarà el dispositiu.';
            comando = 'START_UPDATE_COMPLETE';
            break;
        default:
            console.error('Tipo de actualización desconocido:', tipo);
            return;
    }
    
    if (!confirm(mensaje)) {
        return;
    }
    
    console.log(`🚀 Iniciando actualización OTA tipo: ${tipo}`);
    
    otaState.actualizando = true;
    otaState.tipoActualizacion = tipo;
    
    // Deshabilitar botones
    document.getElementById('btnActualizarFirmware').disabled = true;
    document.getElementById('btnActualizarSPIFFS').disabled = true;
    document.getElementById('btnActualizarTodo').disabled = true;
    document.getElementById('btnComprobarOTA').disabled = true;
    document.getElementById('btnCancelarOTA').disabled = true;
    
    // Ocultar botones de actualización
    document.getElementById('botonesActualizacion').style.display = 'none';
    
    // Mostrar progreso inmediatamente
    const progresoDiv = document.getElementById('otaProgreso');
    progresoDiv.style.display = 'block';
    document.getElementById('otaProgressFill').style.width = '0%';
    
    let mensajeInstalando = '';
    switch(tipo) {
        case 'firmware':
            mensajeInstalando = t('ota_instalando_firmware') || 'Instal·lant firmware...';
            break;
        case 'spiffs':
            mensajeInstalando = t('ota_instalando_spiffs') || 'Instal·lant SPIFFS...';
            break;
        case 'completo':
            mensajeInstalando = t('ota_instalando_completo') || 'Instal·lant actualització completa...';
            break;
    }
    document.getElementById('otaMensaje').textContent = mensajeInstalando;
    
    // Actualizar estado visual
    document.getElementById('otaEstado').innerHTML = 
        `<p style="color: #2196F3; font-weight: bold;">⏳ ${mensajeInstalando}</p>`;
    
    console.log(`📫 Enviando comando: ${comando}`);
    
    // Enviar comando al ESP32
    if (websocket && websocket.readyState === WebSocket.OPEN) {
        websocket.send(comando);
        console.log('✅ Comando enviado correctamente');
        
        // Iniciar animación de progreso simulado (el WebSocket puede cerrarse)
        simularProgresoOTA(tipo);
    } else {
        console.error('❌ WebSocket no está conectado');
        mostrarErrorOTA('No hay conexión con el servidor');
    }
}

// Simular progreso OTA cuando el WebSocket se cierra
function simularProgresoOTA(tipo) {
    let progreso = 0;
    const intervalo = setInterval(() => {
        if (!otaState.actualizando) {
            clearInterval(intervalo);
            return;
        }
        
        // Incremento lento y realista
        if (progreso < 90) {
            progreso += Math.random() * 5; // Incremento variable
            if (progreso > 90) progreso = 90; // Máximo 90% hasta confirmación
            
            document.getElementById('otaProgressFill').style.width = Math.floor(progreso) + '%';
            
            // Mensajes según progreso
            let mensaje = '';
            if (progreso < 30) {
                mensaje = tipo === 'firmware' ? 'Descargant firmware...' : 
                         tipo === 'spiffs' ? 'Descargant SPIFFS...' : 
                         'Descargant actualització...';
            } else if (progreso < 60) {
                mensaje = 'Instal·lant...';
            } else {
                mensaje = 'Finalitzant actualització...';
            }
            
            document.getElementById('otaMensaje').textContent = mensaje;
        }
    }, 1000); // Actualizar cada segundo
    
    // Timeout de seguridad (5 minutos)
    setTimeout(() => {
        clearInterval(intervalo);
        if (otaState.actualizando) {
            // Si después de 5 minutos no hay respuesta, mostrar mensaje
            document.getElementById('otaMensaje').textContent = 
                'Reiniciant dispositiu... Espera uns segons i recarrega la pàgina.';
            document.getElementById('otaProgressFill').style.width = '95%';
        }
    }, 300000); // 5 minutos
}

// Procesar mensajes OTA del WebSocket
function procesarMensajeOTA(mensaje) {
    console.log('🔷 procesarMensajeOTA:', mensaje);
    
    const partes = mensaje.split(':');
    const comando = partes[0];
    
    switch(comando) {
        case 'VERSION_OTA':
            // VERSION_OTA:1.0.0
            otaState.versionActual = partes[1];
            document.getElementById('versionActual').textContent = partes[1];
            console.log('✅ Versión OTA actualizada:', partes[1]);
            break;
            
        case 'UPDATE_AVAILABLE':
            // UPDATE_AVAILABLE:1.0.1:https://...firmware.bin:https://...spiffs.bin:Release notes...
            otaState.versionDisponible = partes[1];
            otaState.firmwareUrl = partes[2];
            otaState.spiffsUrl = partes[3];
            const releaseNotes = partes.slice(4).join(':');
            
            console.log('✅ Actualización disponible:', partes[1]);
            mostrarActualizacionDisponible(partes[1], releaseNotes);
            break;
            
        case 'NO_UPDATE':
            // NO_UPDATE
            console.log('ℹ️ No hay actualizaciones');
            mostrarNoHayActualizacion();
            break;
            
        case 'OTA_PROGRESS':
            // OTA_PROGRESS:45:Descargando firmware...
            const progreso = parseInt(partes[1]);
            const mensajeProgreso = partes.slice(2).join(':');
            console.log(`📊 OTA_PROGRESS recibido: ${progreso}% - ${mensajeProgreso}`);
            actualizarProgresoOTA(progreso, mensajeProgreso);
            break;
            
        case 'OTA_SUCCESS':
            // OTA_SUCCESS:1.0.1
            console.log('✅ OTA_SUCCESS:', partes[1]);
            mostrarExitoOTA(partes[1]);
            break;
            
        case 'OTA_ERROR':
            // OTA_ERROR:Error message
            const error = partes.slice(1).join(':');
            console.error('❌ OTA_ERROR:', error);
            mostrarErrorOTA(error);
            break;
            
        default:
            console.warn('⚠️ Comando OTA desconocido:', comando);
    }
}

// Mostrar que hay actualización disponible
function mostrarActualizacionDisponible(version, releaseNotes) {
    // Habilitar botón
    const btnComprobar = document.getElementById('btnComprobarOTA');
    btnComprobar.disabled = false;
    btnComprobar.textContent = t('comprobar');
    
    // Ocultar progreso
    document.getElementById('otaProgreso').style.display = 'none';
    
    // Mostrar versión disponible
    document.getElementById('versionDisponible').textContent = version;
    document.getElementById('versionDisponibleContainer').style.display = 'block';
    
    // Mostrar estado
    document.getElementById('otaEstado').innerHTML = 
        `<p style="color: #4CAF50; font-weight: bold;">✅ ${t('ota_nueva_version')}</p>`;
    
    // Mostrar notas de la versión
    if (releaseNotes && releaseNotes.trim() !== '') {
        document.getElementById('otaNotasContenido').innerHTML = 
            releaseNotes.replace(/\n/g, '<br>');
        document.getElementById('otaNotas').style.display = 'block';
    }
    
    // Mostrar botones de actualización
    document.getElementById('botonesActualizacion').style.display = 'flex';
}

// Mostrar que no hay actualización
function mostrarNoHayActualizacion() {
    // Habilitar botón
    const btnComprobar = document.getElementById('btnComprobarOTA');
    btnComprobar.disabled = false;
    btnComprobar.textContent = t('comprobar');
    
    // Ocultar progreso
    document.getElementById('otaProgreso').style.display = 'none';
    
    // Mostrar estado
    document.getElementById('otaEstado').innerHTML = 
        `<p style="color: #666;">ℹ️ ${t('ota_firmware_actualizado')}</p>`;
}

// Actualizar progreso de OTA
function actualizarProgresoOTA(progreso, mensaje) {
    // Asegurar que el progreso sea visible
    const progresoDiv = document.getElementById('otaProgreso');
    if (progresoDiv.style.display === 'none') {
        progresoDiv.style.display = 'block';
    }
    
    // Actualizar barra de progreso (sobrescribe el simulado)
    document.getElementById('otaProgressFill').style.width = progreso + '%';
    document.getElementById('otaMensaje').textContent = mensaje;
    
    console.log(`📊 OTA Progreso REAL: ${progreso}% - ${mensaje}`);
    
    // Si llega progreso del 100%, detener simulación
    if (progreso >= 100) {
        otaState.actualizando = false;
    }
}

// Mostrar éxito de actualización
function mostrarExitoOTA(version) {
    document.getElementById('otaProgressFill').style.width = '100%';
    document.getElementById('otaMensaje').textContent = t('ota_reiniciando');
    
    document.getElementById('otaEstado').innerHTML = 
        `<p style="color: #4CAF50; font-weight: bold;">✅ ${t('ota_completado')}</p>`;
    
    // Esperar 3 segundos y recargar la página
    setTimeout(() => {
        location.reload();
    }, 3000);
}

// Mostrar error de OTA
function mostrarErrorOTA(error) {
    // Habilitar botones
    document.getElementById('btnComprobarOTA').disabled = false;
    document.getElementById('btnComprobarOTA').textContent = t('comprobar');
    document.getElementById('btnActualizarFirmware').disabled = false;
    document.getElementById('btnActualizarSPIFFS').disabled = false;
    document.getElementById('btnActualizarTodo').disabled = false;
    document.getElementById('btnCancelarOTA').disabled = false;
    
    otaState.actualizando = false;
    
    // Mostrar error
    document.getElementById('otaEstado').innerHTML = 
        `<p style="color: #f44336; font-weight: bold;">❌ ${t('ota_error')}: ${error}</p>`;
    
    // Ocultar progreso
    document.getElementById('otaProgreso').style.display = 'none';
}

// Cerrar modal al hacer clic fuera
window.onclick = function(event) {
    const modal = document.getElementById('modalOTA');
    if (event.target === modal && !otaState.actualizando) {
        cerrarModalOTA();
    }
}
