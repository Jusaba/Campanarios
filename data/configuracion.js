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
        mediahora: false
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
            mediahora: document.getElementById('notifMediahora').checked
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

function abrirConfigSistema() {
    alert("🔧 Configuración Sistema - Próximamente");
    // TODO: Implementar configuración sistema (PIN, actualización OTA, etc.)
}

// ============================================================================
// CERRAR MODALES CON CLICK FUERA
// ============================================================================

window.onclick = function(event) {
    const modalPin = document.getElementById('modalPin');
    const modalConfig = document.getElementById('modalConfiguracion');
    const modalTelegram = document.getElementById('modalConfigTelegram');
    
    if (event.target === modalPin) {
        cerrarModalPin();
    }
    if (event.target === modalConfig) {
        cerrarModalConfiguracion();
    }
    if (event.target === modalTelegram) {
        cerrarModalConfigTelegram();
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
    }
}

// Hacer disponible globalmente para que Campanas.js pueda llamarla
window.procesarMensajeConfiguracion = procesarMensajeConfiguracion;

console.log("✅ Sistema de configuración cargado");
