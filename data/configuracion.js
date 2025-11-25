// ============================================================================
// SISTEMA DE CONFIGURACIÓN CON PIN
// ============================================================================

let configuracionTelegram = {
    nombre: "",
    ubicacion: "",
    notificaciones: {
        inicio: true,
        misa: true,
        difuntos: true,
        fiesta: true,
        stop: false,
        calefaccion: true,
        calefaccion_off: false,
        alarma: true,
        errores: false,
        internet: false,
        hora: false,
        mediahora: false,
        ntp: false,
        dns: false,
        alarma_programada: false
    }
};

// ============================================================================
// MODAL DE PIN
// ============================================================================

function abrirModalPin() {
    console.log("🔓 Abriendo modal de PIN");
    const modal = document.getElementById('modalPin');
    const input = document.getElementById('inputPin');
    const error = document.getElementById('errorPin');
    
    modal.style.display = 'block';
    error.style.display = 'none';
    input.value = '';
    
    // Enfocar el input después de que se muestre el modal
    setTimeout(() => input.focus(), 100);
    
    // Permitir Enter para verificar
    input.onkeypress = (e) => {
        if (e.key === 'Enter') verificarPin();
    };
}

function cerrarModalPin() {
    const modal = document.getElementById('modalPin');
    modal.style.display = 'none';
}

function verificarPin() {
    const input = document.getElementById('inputPin');
    const error = document.getElementById('errorPin');
    const pin = input.value;
    
    console.log("🔐 Verificando PIN en servidor...");
    
    // Enviar PIN al servidor para validación
    if (typeof websocket !== 'undefined' && websocket.readyState === WebSocket.OPEN) {
        websocket.send(`VERIFY_PIN:${pin}`);
        
        // Esperar respuesta del servidor (se procesa en el manejador WebSocket)
        // La respuesta será PIN_OK o PIN_ERROR
    } else {
        error.textContent = '❌ ' + (typeof t === 'function' ? t('error_conexion') : 'Error de conexión');
        error.style.display = 'block';
        console.error("❌ WebSocket no disponible");
    }
}

// ============================================================================
// MODAL DE CONFIGURACIÓN PRINCIPAL
// ============================================================================

function abrirModalConfiguracion() {
    console.log("⚙️ Abriendo modal de configuración principal");
    const modal = document.getElementById('modalConfiguracion');
    modal.style.display = 'block';
}

function cerrarModalConfiguracion() {
    const modal = document.getElementById('modalConfiguracion');
    modal.style.display = 'none';
}

// ============================================================================
// MODAL DE CONFIGURACIÓN TELEGRAM
// ============================================================================

function abrirConfigTelegram() {
    console.log("📱 Abriendo configuración de Telegram");
    cerrarModalConfiguracion();
    
    const modal = document.getElementById('modalConfigTelegram');
    modal.style.display = 'block';
    
    // Cargar configuración actual
    cargarConfigTelegram();
}

function cerrarModalConfigTelegram() {
    const modal = document.getElementById('modalConfigTelegram');
    modal.style.display = 'none';
    document.getElementById('mensajeGuardado').style.display = 'none';
}

function cargarConfigTelegram() {
    console.log("📥 Cargando configuración de Telegram desde servidor...");
    
    // Solicitar configuración al servidor
    if (typeof websocket !== 'undefined' && websocket.readyState === WebSocket.OPEN) {
        websocket.send('GET_CONFIG_TELEGRAM');
        console.log("📤 Solicitando configuración de Telegram al servidor");
    } else {
        console.warn("⚠️ WebSocket no disponible, usando valores locales");
        aplicarConfigTelegramEnFormulario();
    }
}

function aplicarConfigTelegramEnFormulario() {
    document.getElementById('nombreDispositivo').value = configuracionTelegram.nombre;
    document.getElementById('ubicacionDispositivo').value = configuracionTelegram.ubicacion;
    
    document.getElementById('notifInicio').checked = configuracionTelegram.notificaciones.inicio;
    document.getElementById('notifMisa').checked = configuracionTelegram.notificaciones.misa;
    document.getElementById('notifDifuntos').checked = configuracionTelegram.notificaciones.difuntos;
    document.getElementById('notifFiesta').checked = configuracionTelegram.notificaciones.fiesta;
    document.getElementById('notifStop').checked = configuracionTelegram.notificaciones.stop || false;
    document.getElementById('notifCalefaccion').checked = configuracionTelegram.notificaciones.calefaccion;
    document.getElementById('notifCalefaccionOff').checked = configuracionTelegram.notificaciones.calefaccion_off || false;
    document.getElementById('notifAlarma').checked = configuracionTelegram.notificaciones.alarma;
    document.getElementById('notifErrores').checked = configuracionTelegram.notificaciones.errores || false;
    document.getElementById('notifInternet').checked = configuracionTelegram.notificaciones.internet || false;
    document.getElementById('notifHora').checked = configuracionTelegram.notificaciones.hora || false;
    document.getElementById('notifMediahora').checked = configuracionTelegram.notificaciones.mediahora || false;
    document.getElementById('notifNtp').checked = configuracionTelegram.notificaciones.ntp || false;
    document.getElementById('notifDns').checked = configuracionTelegram.notificaciones.dns || false;
    document.getElementById('notifAlarmaProgramada').checked = configuracionTelegram.notificaciones.alarma_programada || false;
}

function guardarConfigTelegram() {
    console.log("💾 Guardando configuración de Telegram...");
    
    // Recoger valores del formulario
    const config = {
        nombre: document.getElementById('nombreDispositivo').value.trim(),
        ubicacion: document.getElementById('ubicacionDispositivo').value.trim(),
        notificaciones: {
            inicio: document.getElementById('notifInicio').checked,
            misa: document.getElementById('notifMisa').checked,
            difuntos: document.getElementById('notifDifuntos').checked,
            fiesta: document.getElementById('notifFiesta').checked,
            stop: document.getElementById('notifStop').checked,
            calefaccion: document.getElementById('notifCalefaccion').checked,
            calefaccion_off: document.getElementById('notifCalefaccionOff').checked,
            alarma: document.getElementById('notifAlarma').checked,
            errores: document.getElementById('notifErrores').checked,
            internet: document.getElementById('notifInternet').checked,
            hora: document.getElementById('notifHora').checked,
            mediahora: document.getElementById('notifMediahora').checked,
            ntp: document.getElementById('notifNtp').checked,
            dns: document.getElementById('notifDns').checked,
            alarma_programada: document.getElementById('notifAlarmaProgramada').checked
        }
    };
    
    // Validar nombre no vacío
    if (config.nombre === '') {
        alert('El nombre del dispositivo no puede estar vacío');
        document.getElementById('nombreDispositivo').focus();
        return;
    }
    
    // Enviar al servidor vía WebSocket
    if (typeof websocket !== 'undefined' && websocket.readyState === WebSocket.OPEN) {
        const mensaje = `SAVE_CONFIG_TELEGRAM:${JSON.stringify(config)}`;
        websocket.send(mensaje);
        console.log("📤 Configuración enviada al servidor");
        
        // Actualizar variable local
        configuracionTelegram = config;
        
        // Mostrar mensaje de confirmación
        const mensajeDiv = document.getElementById('mensajeGuardado');
        mensajeDiv.style.display = 'block';
        
        // Cerrar modal después de 2 segundos
        setTimeout(() => {
            cerrarModalConfigTelegram();
        }, 2000);
    } else {
        alert('Error: No hay conexión con el servidor');
        console.error("❌ WebSocket no disponible");
    }
}

// ============================================================================
// PLACEHOLDERS PARA OTRAS CONFIGURACIONES
// ============================================================================

function abrirConfigWifi() {
    alert("🔧 Configuración WiFi - Próximamente");
    // TODO: Implementar configuración WiFi
}

function abrirConfigReset() {
    console.log("🔄 Solicitando reinicio del sistema");
    
    // Confirmar antes de reiniciar
    const mensaje = typeof t === 'function' ? 
        t('reset_confirmacion') + '\n\n' + t('reset_descripcion') :
        '¿Estás seguro que quieres reiniciar el sistema?\n\nEl sistema se reiniciará y se perderá la conexión temporalmente.';
    
    if (confirm(mensaje)) {
        console.log("✅ Reinicio confirmado");
        
        // Enviar comando de reset al servidor
        if (typeof websocket !== 'undefined' && websocket.readyState === WebSocket.OPEN) {
            websocket.send('RESET_SYSTEM');
            console.log("📤 Comando de reinicio enviado al servidor");
            
            // Cerrar modales
            cerrarModalConfiguracion();
            
            // Mostrar mensaje al usuario
            const mensajeEjecutando = typeof t === 'function' ? t('reset_ejecutando') : 'Reiniciando el sistema...';
            alert(mensajeEjecutando);
            
        } else {
            const mensajeError = typeof t === 'function' ? t('error_conexion') : 'Error de conexión';
            alert('Error: ' + mensajeError);
            console.error("❌ WebSocket no disponible");
        }
    } else {
        console.log("❌ Reinicio cancelado por el usuario");
    }
}

// ============================================================================
// MODAL DE BACKUP
// ============================================================================

function abrirModalBackup() {
    console.log("💾 Abriendo modal de backup");
    cerrarModalConfiguracion();
    
    const modal = document.getElementById('modalBackup');
    modal.style.display = 'block';
}

function cerrarModalBackup() {
    const modal = document.getElementById('modalBackup');
    modal.style.display = 'none';
}

function descargarArchivo(filename) {
    console.log(`📥 Descargando archivo: ${filename}`);
    
    // Usar fetch para descargar el archivo
    const url = `/download?file=${encodeURIComponent(filename)}`;
    
    fetch(url, {
        method: 'GET',
        credentials: 'include' // Incluir credenciales de autenticación
    })
    .then(response => {
        if (!response.ok) {
            throw new Error(`Error HTTP: ${response.status} - ${response.statusText}`);
        }
        return response.blob();
    })
    .then(blob => {
        // Crear URL del blob
        const blobUrl = window.URL.createObjectURL(blob);
        
        // Crear enlace temporal y hacer clic
        const a = document.createElement('a');
        a.href = blobUrl;
        a.download = filename;
        a.style.display = 'none';
        
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
        
        // Liberar la URL del blob
        window.URL.revokeObjectURL(blobUrl);
        
        console.log(`✅ Descarga completada: ${filename}`);
        
        // Mostrar notificación visual
        const notificacion = document.createElement('div');
        notificacion.style.cssText = `
            position: fixed;
            bottom: 20px;
            right: 20px;
            background: #4CAF50;
            color: white;
            padding: 15px 25px;
            border-radius: 5px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.3);
            z-index: 10000;
            font-weight: bold;
        `;
        notificacion.textContent = `✅ ${filename} descarregat`;
        document.body.appendChild(notificacion);
        
        setTimeout(() => {
            notificacion.remove();
        }, 3000);
    })
    .catch(error => {
        console.error(`❌ Error descargando ${filename}:`, error);
        
        // Mostrar notificación de error
        const notificacion = document.createElement('div');
        notificacion.style.cssText = `
            position: fixed;
            bottom: 20px;
            right: 20px;
            background: #f44336;
            color: white;
            padding: 15px 25px;
            border-radius: 5px;
            box-shadow: 0 4px 6px rgba(0,0,0,0.3);
            z-index: 10000;
            font-weight: bold;
        `;
        notificacion.textContent = `❌ Error: ${error.message}`;
        document.body.appendChild(notificacion);
        
        setTimeout(() => {
            notificacion.remove();
        }, 5000);
    });
}

function subirArchivo(input, targetFilename) {
    const file = input.files[0];
    if (!file) {
        console.log('❌ No se seleccionó archivo');
        return;
    }
    
    console.log(`📤 Subiendo archivo: ${file.name} como ${targetFilename}`);
    
    // Confirmar antes de sobrescribir
    if (!confirm(`¿Restaurar ${targetFilename}? Esto sobrescribirá la configuración actual.`)) {
        input.value = ''; // Limpiar input
        return;
    }
    
    // Crear FormData y añadir el archivo con el nombre correcto
    const formData = new FormData();
    formData.append('file', file, targetFilename);
    
    // Mostrar notificación de carga
    const notificacion = document.createElement('div');
    notificacion.id = 'uploadNotification';
    notificacion.style.cssText = `
        position: fixed;
        bottom: 20px;
        right: 20px;
        background: #2196F3;
        color: white;
        padding: 15px 25px;
        border-radius: 5px;
        box-shadow: 0 4px 6px rgba(0,0,0,0.3);
        z-index: 10000;
        font-weight: bold;
    `;
    notificacion.textContent = `⏳ Pujant ${targetFilename}...`;
    document.body.appendChild(notificacion);
    
    // Subir archivo
    fetch('/upload', {
        method: 'POST',
        body: formData,
        credentials: 'include'
    })
    .then(response => {
        if (!response.ok) {
            throw new Error(`Error HTTP: ${response.status}`);
        }
        return response.text();
    })
    .then(() => {
        console.log(`✅ Archivo subido: ${targetFilename}`);
        
        // Actualizar notificación
        notificacion.style.background = '#4CAF50';
        notificacion.textContent = `✅ ${targetFilename} restaurat correctament`;
        
        setTimeout(() => {
            notificacion.remove();
        }, 3000);
        
        // Limpiar input
        input.value = '';
    })
    .catch(error => {
        console.error(`❌ Error subiendo ${targetFilename}:`, error);
        
        // Actualizar notificación de error
        notificacion.style.background = '#f44336';
        notificacion.textContent = `❌ Error: ${error.message}`;
        
        setTimeout(() => {
            notificacion.remove();
        }, 5000);
        
        // Limpiar input
        input.value = '';
    });
}

// ============================================================================
// CERRAR MODALES CON CLICK FUERA
// ============================================================================

function abrirModalAcercaDe() {
    const modal = document.getElementById('modalAcercaDe');
    modal.style.display = 'block';
    
    // Solicitar versión actual del firmware
    if (typeof websocket !== 'undefined' && websocket.readyState === WebSocket.OPEN) {
        websocket.send('GET_VERSION_OTA');
    }
}

function cerrarModalAcercaDe() {
    const modal = document.getElementById('modalAcercaDe');
    modal.style.display = 'none';
}

window.onclick = function(event) {
    const modalPin = document.getElementById('modalPin');
    const modalConfig = document.getElementById('modalConfiguracion');
    const modalTelegram = document.getElementById('modalConfigTelegram');
    const modalBackup = document.getElementById('modalBackup');
    
    if (event.target === modalPin) {
        cerrarModalPin();
    }
    if (event.target === modalConfig) {
        cerrarModalConfiguracion();
    }
    if (event.target === modalTelegram) {
        cerrarModalConfigTelegram();
    }
    if (event.target === modalBackup) {
        cerrarModalBackup();
    }
};

// ============================================================================
// MANEJADOR DE RESPUESTAS DEL SERVIDOR
// ============================================================================

// Esta función debe ser llamada desde Campanas.js cuando llegue un mensaje WebSocket
function procesarMensajeConfiguracion(mensaje) {
    console.log("📨 Procesando mensaje de configuración:", mensaje);
    
    if (mensaje === "PIN_OK") {
        console.log("✅ PIN correcto");
        const error = document.getElementById('errorPin');
        error.style.display = 'none';
        cerrarModalPin();
        abrirModalConfiguracion();
        
    } else if (mensaje === "PIN_ERROR") {
        console.log("❌ PIN incorrecto");
        const input = document.getElementById('inputPin');
        const error = document.getElementById('errorPin');
        error.textContent = '❌ ' + (typeof t === 'function' ? t('pin_incorrecto') : 'PIN incorrecto');
        error.style.display = 'block';
        input.value = '';
        input.focus();
        
        // Vibración en móviles
        if (navigator.vibrate) {
            navigator.vibrate([100, 50, 100]);
        }
        
    } else if (mensaje.startsWith("CONFIG_TELEGRAM:")) {
        const jsonConfig = mensaje.substring(16);
        console.log("📦 Configuración de Telegram recibida:", jsonConfig);
        
        try {
            const config = JSON.parse(jsonConfig);
            configuracionTelegram = config;
            aplicarConfigTelegramEnFormulario();
            console.log("✅ Configuración aplicada al formulario");
        } catch (e) {
            console.error("❌ Error al parsear configuración:", e);
        }
    } else if (mensaje.startsWith("VERSION_OTA:")) {
        // VERSION_OTA:1.0.4
        const version = mensaje.substring(12);
        
        // Actualizar en modal OTA si existe
        const versionOTA = document.getElementById('versionActual');
        if (versionOTA) {
            versionOTA.textContent = version;
        }
        
        // Actualizar en modal Acerca de
        const versionSistema = document.getElementById('versionSistema');
        if (versionSistema) {
            const idioma = idiomaActual || 'ca';
            const textoVersion = idioma === 'ca' ? 'Versió' : 'Versión';
            versionSistema.textContent = `${textoVersion} ${version}`;
        }
    }
}

// Hacer disponible globalmente para que Campanas.js pueda llamarla
window.procesarMensajeConfiguracion = procesarMensajeConfiguracion;

console.log("✅ Sistema de configuración cargado");
