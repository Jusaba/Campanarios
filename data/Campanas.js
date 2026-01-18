// ============================================================================
// CONSTANTES DE ESTADO - Sincronizadas con Config::States en ESP32
// ============================================================================
const CampanarioStates = {
    // Estados I2C (para comandos)
    DIFUNTOS: "Difuntos",
    MISA: "Misa",
    FIESTA: "Fiesta",
    STOP: "PARAR",
    GET_CAMPANARIO: "GET_CAMPANARIO",
    GET_TIEMPO_CALEFACCION: "GET_TIEMPOCALEFACCION",
    GET_SECUENCIA_ACTIVA: "GET_SECUENCIA_ACTIVA",

    // Bits de estado (para ESTADO_CAMPANARIO response)
    BIT_SECUENCIA: 0x01,
    BIT_LIBRE_1: 0x02,
    BIT_LIBRE_2: 0x04,
    BIT_HORA: 0x08,
    BIT_CUARTOS: 0x10,
    BIT_CALEFACCION: 0x20,
    BIT_SIN_INTERNET: 0x40,
    BIT_PROTECCION_CAMPANADAS: 0x80
};

var gateway = `ws://${window.location.hostname}:8080/ws`;
var websocket;

let lCampanas = false;

window.addEventListener('load', onload);

function onload(event) {
    console.log("Página cargada correctamente");
    initWebSocket();
}

function initWebSocket() {
    websocket = new WebSocket(gateway);
    websocket.onmessage = onMessageDatos;
    websocket.onopen = onOpenDatos;
    websocket.onclose = onCloseDatos;
}

function onOpenDatos(event) {
    const path = window.location.pathname;
    console.log("Conexión WebSocket abierta en Datos");
    if (path === "/" || path.endsWith("index.html")) {
        websocket.send("GET_CAMPANARIO");
    }
}

function onCloseDatos(event) {
    console.log("Conexión WebSocket cerrada, intentando reconectar...");
    setTimeout(initWebSocket, 2000);
}

function onMessageDatos(event) {
    console.log("Mensaje recibido: " + event.data);

    if (event.data.startsWith("REDIRECT:")) {
        var url = event.data.substring(9);
        console.log("Redirigiendo a: " + url);
        window.location.href = url;
    }
    
    if (event.data.startsWith("CAMPANA:")) {
        console.log("Activando campana con ID: " + event.data);
        let idx = parseInt(event.data.split(":")[1]);
        activarCampana(idx);
    }
    
    // ✅ DELEGAR MENSAJES DE CALEFACCIÓN AL MÓDULO CORRESPONDIENTE
    if (event.data.startsWith("CALEFACCION:") || event.data.startsWith("TIEMPO_CALEFACCION:")) {
        if (typeof procesarMensajeCalefaccion === 'function') {
            procesarMensajeCalefaccion(event.data);
        } else {
            console.warn("⚠️ Función procesarMensajeCalefaccion no disponible");
        }
    }
    
    // ✅ DELEGAR MENSAJES DE CONFIGURACIÓN (PIN, CONFIG_TELEGRAM)
    if (event.data === "PIN_OK" || event.data === "PIN_ERROR" || event.data.startsWith("CONFIG_TELEGRAM:")) {
        if (typeof procesarMensajeConfiguracion === 'function') {
            procesarMensajeConfiguracion(event.data);
        } else {
            console.warn("⚠️ Función procesarMensajeConfiguracion no disponible");
        }
    }
    
    // ✅ DELEGAR MENSAJES DE OTA
    if (event.data.startsWith("VERSION_OTA:") || 
        event.data.startsWith("UPDATE_AVAILABLE:") || 
        event.data === "NO_UPDATE" ||
        event.data.startsWith("OTA_PROGRESS:") ||
        event.data.startsWith("OTA_SUCCESS:") ||
        event.data.startsWith("OTA_ERROR:")) {
        // VERSION_OTA se usa tanto en modal OTA como en modal Acerca de
        if (typeof procesarMensajeOTA === 'function') {
            procesarMensajeOTA(event.data);
        } else {
            console.warn("⚠️ Función procesarMensajeOTA no disponible");
        }
        if (event.data.startsWith("VERSION_OTA:") && typeof procesarMensajeConfiguracion === 'function') {
            procesarMensajeConfiguracion(event.data);
        }
    }
    
    if (event.data.startsWith("PROTECCION:ON")) {
        console.log("Actualizando la protección de campanadas horarias: " + event.data);
        habilitarBotonesCampanadas(false);
    }
    
    if (event.data.startsWith("PROTECCION:OFF")) {
        console.log("Actualizando la protección de campanadas horarias: " + event.data);
        habilitarBotonesCampanadas(true);
    }
    
    if (event.data.startsWith("IDIOMA_ACTUAL:")) {
        const idiomaServidor = event.data.substring(14);
        console.log(`📥 Idioma del servidor: ${idiomaServidor}`);
        
        if (idiomaServidor !== idiomaActual) {
            console.log(`🔄 Actualizando idioma local: ${idiomaActual} → ${idiomaServidor}`);
            idiomaActual = idiomaServidor;
            localStorage.setItem('idioma_campanario', idiomaServidor);
            actualizarSelectorIdioma();
            actualizarTextosInterfaz();
        }
    }
    else if (event.data.startsWith("IDIOMA_CAMBIADO:")) {
        const nuevoIdioma = event.data.substring(16);
        console.log(`✅ Idioma cambiado en servidor: ${nuevoIdioma}`);
        
        if (nuevoIdioma !== idiomaActual) {
            idiomaActual = nuevoIdioma;
            localStorage.setItem('idioma_campanario', nuevoIdioma);
            actualizarSelectorIdioma();
            actualizarTextosInterfaz();
        }
    }
    else if (event.data.startsWith("ERROR_IDIOMA:")) {
        const error = event.data.substring(13);
        console.error(`❌ Error al cambiar idioma: ${error}`);
        alert(`Error: ${error}`);
    }
    
    if (event.data.startsWith("ESTADO_CAMPANARIO:")) {
        console.log("Comprobando estado de campanario: " + event.data);
        let EstadoCampanario = parseInt(event.data.split(":")[1]);
        
        if ((EstadoCampanario & CampanarioStates.BIT_SECUENCIA)) {
            lCampanas = true;
            window.location.href = "/Campanas.html";
        } else {
            lCampanas = false;
        }
        
        // ✅ DELEGAR ESTADO DE CALEFACCIÓN AL MÓDULO CORRESPONDIENTE
        if (EstadoCampanario & CampanarioStates.BIT_CALEFACCION) {
            // ✅ CALEFACCIÓN ENCENDIDA: Delegar actualización completa
            if (typeof window.Calefaccion !== 'undefined') {
                window.Calefaccion.estado = true;
            }
            
            // ✅ ACTUALIZAR INTERFAZ COMPLETA si la función está disponible
            if (typeof actualizarEstadoCalefaccion === 'function') {
                actualizarEstadoCalefaccion();
            } else {
                // Fallback: Solo actualizar icono si la función no está disponible
                const icono = document.getElementById("iconoCalefaccion");
                if (icono) {
                    icono.setAttribute("stroke", "red");
                }
            }
            
            // Solicitar tiempo restante
            websocket.send("GET_TIEMPOCALEFACCION");
            console.log("🔥 Calefacción detectada como ENCENDIDA - Solicitando tiempo restante");
            
        } else {
            // ✅ CALEFACCIÓN APAGADA: Delegar actualización completa
            if (typeof window.Calefaccion !== 'undefined') {
                window.Calefaccion.estado = false;
            }
            
            // ✅ ACTUALIZAR INTERFAZ COMPLETA si la función está disponible
            if (typeof actualizarEstadoCalefaccion === 'function') {
                actualizarEstadoCalefaccion();
                
                // ✅ TAMBIÉN: Detener cuenta regresiva si existe
                if (typeof detenerCuentaRegresiva === 'function') {
                    detenerCuentaRegresiva();
                }
                
                // ✅ Y: Actualizar display de minutos si existe
                if (typeof actualizarDisplayMinutos === 'function') {
                    actualizarDisplayMinutos();
                }
            } else {
                // Fallback: Solo actualizar icono si la función no está disponible
                const icono = document.getElementById("iconoCalefaccion");
                if (icono) {
                    icono.setAttribute("stroke", "orange");
                }
            }
            
            console.log("🔥 Calefacción detectada como APAGADA - Interfaz actualizada");
        }
        
        // Verificar el bit de protección de campanadas
        if (EstadoCampanario & CampanarioStates.BIT_PROTECCION_CAMPANADAS) {
            habilitarBotonesCampanadas(false);
            console.log("Protección de campanadas activa");
        } else {
            habilitarBotonesCampanadas(true);
            console.log("Protección de campanadas inactiva");
        }
        
        if (event.data.startsWith("GET_SECUENCIA_ACTIVA:")) {
            console.log("Recibido número de secuencia activa: " + event.data);
        }
    }
}

function activarCampana(num) {
    // Desactiva todas
    document.getElementById("campana0").classList.remove("activa");
    document.getElementById("campana1").classList.remove("activa");
    
    // Activa la correspondiente (números desde 1)
    if (num === 1) {
        document.getElementById("campana0").classList.add("activa");
    } else if (num === 2) {
        document.getElementById("campana1").classList.add("activa");
    }
    
    // Desactivar la animación después de un tiempo
    setTimeout(() => {
        document.getElementById("campana0").classList.remove("activa");
        document.getElementById("campana1").classList.remove("activa");
    }, 400);
}

function pararSecuencia() {
    if (window.confirm("¿Segur que vols aturar la seqüència?")) {
        if (typeof websocket !== "undefined" && websocket.readyState === 1) {
            websocket.send("PARAR");
        }
    }
}

function habilitarBotonesCampanadas(habilitar) {
    const botonMisa = document.querySelector(".button.Misa");
    const botonDifuntos = document.querySelector(".button.Difuntos");
    const botonFiesta = document.querySelector(".button.Fiesta");

    [botonMisa, botonDifuntos, botonFiesta].forEach((boton, index) => {
        if (!boton) return;
        
        const coloresOriginales = ["#059e8a", "#bc4fac", "#e74c3c"];
        const funciones = [SelMisa, SelDifuntos, SelFiesta];
        
        if (habilitar) {
            boton.disabled = false;
            boton.classList.remove("disabled-mobile");
            boton.style.opacity = "1";
            boton.style.cursor = "pointer";
            boton.style.pointerEvents = "auto";
            boton.style.backgroundColor = coloresOriginales[index];
            boton.title = "";
            boton.onclick = funciones[index];
        } else {
            boton.disabled = true;
            boton.classList.add("disabled-mobile");
            boton.style.opacity = "0.5";
            boton.style.cursor = "not-allowed";
            boton.style.pointerEvents = "none";
            boton.style.backgroundColor = "#888";
            boton.title = "Campanadas protegidas - No disponible durante período de toque";
            boton.onclick = function(e) {
                e.preventDefault();
                e.stopPropagation();
                return false;
            };
        }
    });

    console.log("Botones de campanadas " + (habilitar ? "habilitados" : "deshabilitados"));
}

function SelMisa() {
    console.log("🔔 Activando Misa");
    websocket.send("Misa");
}

function SelDifuntos() {
    console.log("🔔 Activando Difuntos");
    websocket.send("Difuntos");
}

function SelFiesta() {
    console.log("🔔 Activando Fiesta");
    websocket.send("Fiesta");
}

// ✅ MANTENER SOLO FUNCIONES DEL MODAL "ACERCA DE"
function abrirModalAcercaDe() {
    document.getElementById('modalAcercaDe').style.display = 'block';
}

function cerrarModalAcercaDe() {
    document.getElementById('modalAcercaDe').style.display = 'none';
}

// Cerrar modal si se hace clic fuera de él
window.addEventListener('click', function(event) {
    const modalAcerca = document.getElementById('modalAcercaDe');
    if (event.target === modalAcerca) {
        cerrarModalAcercaDe();
    }
});