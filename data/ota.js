// ota.js - Gestión de actualizaciones OTA

let otaState = {
    versionActual: "1.0.0",
    versionDisponible: null,
    firmwareUrl: null,
    spiffsUrl: null,
    actualizando: false
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
    document.getElementById('btnActualizarOTA').style.display = 'none';
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
function iniciarActualizacion() {
    if (!confirm('Això reiniciarà el dispositiu. Continuar?')) {
        return;
    }
    
    otaState.actualizando = true;
    
    // Deshabilitar botones
    document.getElementById('btnActualizarOTA').disabled = true;
    document.getElementById('btnComprobarOTA').disabled = true;
    document.getElementById('btnCancelarOTA').disabled = true;
    
    // Mostrar progreso
    document.getElementById('otaProgreso').style.display = 'block';
    document.getElementById('otaProgressFill').style.width = '0%';
    document.getElementById('otaMensaje').textContent = t('ota_instalando');
    
    // Enviar comando al ESP32
    websocket.send('START_UPDATE_OTA');
}

// Procesar mensajes OTA del WebSocket
function procesarMensajeOTA(mensaje) {
    const partes = mensaje.split(':');
    const comando = partes[0];
    
    switch(comando) {
        case 'VERSION_OTA':
            // VERSION_OTA:1.0.0
            otaState.versionActual = partes[1];
            document.getElementById('versionActual').textContent = partes[1];
            break;
            
        case 'UPDATE_AVAILABLE':
            // UPDATE_AVAILABLE:1.0.1:https://...firmware.bin:https://...spiffs.bin:Release notes...
            otaState.versionDisponible = partes[1];
            otaState.firmwareUrl = partes[2];
            otaState.spiffsUrl = partes[3];
            const releaseNotes = partes.slice(4).join(':');
            
            mostrarActualizacionDisponible(partes[1], releaseNotes);
            break;
            
        case 'NO_UPDATE':
            // NO_UPDATE
            mostrarNoHayActualizacion();
            break;
            
        case 'OTA_PROGRESS':
            // OTA_PROGRESS:45:Descargando firmware...
            const progreso = parseInt(partes[1]);
            const mensaje = partes.slice(2).join(':');
            actualizarProgresoOTA(progreso, mensaje);
            break;
            
        case 'OTA_SUCCESS':
            // OTA_SUCCESS:1.0.1
            mostrarExitoOTA(partes[1]);
            break;
            
        case 'OTA_ERROR':
            // OTA_ERROR:Error message
            const error = partes.slice(1).join(':');
            mostrarErrorOTA(error);
            break;
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
    
    // Mostrar botón actualizar
    document.getElementById('btnActualizarOTA').style.display = 'inline-block';
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
    document.getElementById('otaProgressFill').style.width = progreso + '%';
    document.getElementById('otaMensaje').textContent = mensaje;
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
    document.getElementById('btnActualizarOTA').disabled = false;
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
