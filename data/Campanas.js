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
    BIT_SECUENCIA: 0x01,                      // Config::States::BIT_SECUENCIA
    BIT_LIBRE_1: 0x02,                        // Config::States::BIT_LIBRE_1
    BIT_LIBRE_2: 0x04,                        // Config::States::BIT_LIBRE_2
    BIT_HORA: 0x08,                          // Config::States::BIT_HORA
    BIT_CUARTOS: 0x10,                       // Config::States::BIT_CUARTOS
    BIT_CALEFACCION: 0x20,                   // Config::States::BIT_CALEFACCION
    BIT_SIN_INTERNET: 0x40,                  // Config::States::BIT_SIN_INTERNET
    BIT_PROTECCION_CAMPANADAS: 0x80          // Config::States::BIT_PROTECCION_CAMPANADAS
};

var gateway = `ws://${window.location.hostname}:8080/ws`;
var websocket;

let lCalefaccion = false;
let lCampanas = false;

window.addEventListener('load', onload);

function onload(event) {
    console.log("PÃ¡gina cargada correctamente");
    initWebSocket();
    if (typeof websocket !== "undefined") {

    }
}

/**
 * Inicializa una conexiÃ³n WebSocket con el servidor especificado en la variable `gateway`.
 * Configura los manejadores de eventos para la conexiÃ³n WebSocket:
 * - `onopen`: Se ejecuta cuando la conexiÃ³n se abre exitosamente.
 * - `onclose`: Se ejecuta cuando la conexiÃ³n se cierra.
 * - `onmessage`: Se ejecuta cuando se recibe un mensaje desde el servidor.
 *
 * @function
 * @throws {Error} Si no se puede establecer la conexiÃ³n WebSocket.
 */
function initWebSocket() {
    websocket = new WebSocket(gateway);
    websocket.onmessage = onMessageDatos;
    websocket.onopen = onOpenDatos;
    websocket.onclose = onCloseDatos;
}

/**
 * FunciÃ³n que se ejecuta al abrir un evento.
 *
 * @param {Event} event - El evento que dispara la funciÃ³n al abrir.
 */
function onOpenDatos(event) {
    const path = window.location.pathname;
    console.log("ConexiÃ³n WebSocket abierta en Datos");
    if ( path === "/" || path.endsWith("index.html") ) {
        websocket.send("GET_CAMPANARIO");
    }
}

/**
 * Maneja el evento de cierre de una conexiÃ³n, reiniciando el WebSocket despuÃ©s de un retraso.
 *
 * @param {Event} event - El evento de cierre que dispara esta funciÃ³n.
 */
function onCloseDatos(event) {
    console.log("ConexiÃ³n WebSocket cerrada, intentando reconectar...");
    setTimeout(initWebSocket, 2000);
}


/**
 * Maneja el evento de mensaje recibido, actualizando el contenido de los elementos HTML
 * segÃºn los datos proporcionados en el mensaje.
 *
 * @param {MessageEvent} event - El evento de mensaje recibido, que contiene los datos en formato JSON.
 * @throws {SyntaxError} Si los datos del mensaje no son un JSON vÃ¡lido.
 * @throws {TypeError} Si no se encuentra un elemento HTML con un ID correspondiente a una clave del objeto.
 */
function onMessageDatos(event) {

console.log("Mensaje recibido en pagina Datos: " + event.data);
console.log("Mensaje recibido: " + event.data);
console.log(event.data.startsWith("REDIRECT:"))

if (event.data.startsWith("REDIRECT:")) {
        var url = event.data.substring(9);
        console.log("Redirigiendo a: " + url);
        window.location.href = url;
    }
if (event.data.startsWith("CAMPANA:")) {
    console.log ("Activando campana con ID: " + event.data);
    let idx = parseInt(event.data.split(":")[1]);
    window.activarCampana(idx);
}
if (event.data.startsWith("CALEFACCION:ON")) {
    console.log ("Actualizando calefaccion: " + event.data);
    lCalefaccion = true;
    document.getElementById("iconoCalefaccion").setAttribute("stroke", "red" );

    // Ocultar botÃ³n del reloj cuando estÃ¡ encendida
    if (typeof mostrarBotonReloj === 'function') {
        mostrarBotonReloj(false);
    }

    // Extraer los minutos si estÃ¡n presentes
    const parts = event.data.split(":");
    if (parts.length >= 3) {
        const minutos = parseInt(parts[2]);
        console.log(`CalefacciÃ³n encendida por ${minutos} minutos`);

        // Iniciar cuenta regresiva si hay minutos configurados
        if (minutos > 0 && typeof iniciarCuentaRegresiva === 'function') {
            iniciarCuentaRegresiva(minutos);
        }

        // Opcional: Mostrar los minutos en la interfaz
        // document.getElementById("tiempoCalefaccion").textContent = minutos + " min";
    }
}
if (event.data.startsWith("CALEFACCION:OFF")) {
    console.log ("Actualizando calefaccion: " + event.data);
    lCalefaccion = false;
    document.getElementById("iconoCalefaccion").setAttribute("stroke", "orange" );

    // Mostrar botÃ³n del reloj cuando estÃ¡ apagada
    if (typeof mostrarBotonReloj === 'function') {
        mostrarBotonReloj(true);
    }

    // Detener cuenta regresiva y resetear visualizaciÃ³n
    if (typeof detenerCuentaRegresiva === 'function') {
        detenerCuentaRegresiva();
    }
    if (typeof actualizarDisplayMinutos === 'function') {
        actualizarDisplayMinutos();
    }
}
if (event.data.startsWith("CALEFACCION:ERROR")) {
    console.log ("Error encendiendo calefaccion: " + event.data);
    alert("âš ï¸ No se pudo encender la calefacciÃ³n.\nEl sistema de tiempo no estÃ¡ disponible.\nIntenta de nuevo en unos momentos.");
    lCalefaccion = false;
    document.getElementById("iconoCalefaccion").setAttribute("stroke", "orange" );

    // Mostrar botÃ³n del reloj cuando hay error
    if (typeof mostrarBotonReloj === 'function') {
        mostrarBotonReloj(true);
    }
    
    // Detener cuenta regresiva y resetear visualizaciÃ³n
    if (typeof detenerCuentaRegresiva === 'function') {
        detenerCuentaRegresiva();
    }
    if (typeof actualizarDisplayMinutos === 'function') {
        actualizarDisplayMinutos();
    }
}
if (event.data.startsWith("TIEMPO_CALEFACCION:")) {
    const parts = event.data.split(":");
    if (parts.length >= 2) {
        const segundos = parseInt(parts[1]);
        console.log(`Tiempo restante de calefacciÃ³n: ${segundos} segundos`);

        if (segundos > 0) {
            // Asegurar que el estado visual estÃ© correcto
            lCalefaccion = true;
            document.getElementById("iconoCalefaccion").setAttribute("stroke", "red");

            // Ocultar botÃ³n del reloj cuando estÃ¡ encendida
            if (typeof mostrarBotonReloj === 'function') {
                mostrarBotonReloj(false);
            }

            // Iniciar cuenta regresiva con los segundos restantes
            if (typeof iniciarCuentaRegresivaSegundos === 'function') {
                iniciarCuentaRegresivaSegundos(segundos);
            }
        } else {
            // Si no quedan segundos, apagar calefacciÃ³n
            lCalefaccion = false;
            document.getElementById("iconoCalefaccion").setAttribute("stroke", "orange");

            // Mostrar botÃ³n del reloj cuando estÃ¡ apagada
            if (typeof mostrarBotonReloj === 'function') {
                mostrarBotonReloj(true);
            }

            // Detener cuenta regresiva y resetear visualizaciÃ³n
            if (typeof detenerCuentaRegresiva === 'function') {
                detenerCuentaRegresiva();
            }
            if (typeof actualizarDisplayMinutos === 'function') {
                actualizarDisplayMinutos();
            }
        }
    }
}
if (event.data.startsWith("PROTECCION:ON")) {
    console.log ("Actualizando la proteccion de campanadas horaris: " + event.data);
    habilitarBotonesCampanadas(false); // Deshabilita los botones cuando la protecciÃ³n estÃ¡ activa
}
if (event.data.startsWith("PROTECCION:OFF")) {
    console.log ("Actualizando la proteccion de campanadas horaris: " + event.data);
    habilitarBotonesCampanadas(true); // Habilita los botones cuando la protecciÃ³n estÃ¡ inactiva
}
if (event.data.startsWith("ESTADO_CAMPANARIO:")) {
    console.log ("Comprobando estado de campanario: " + event.data);
    let EstadoCampanario = parseInt(event.data.split(":")[1]);
    if ((EstadoCampanario & CampanarioStates.BIT_SECUENCIA) ) {
        lCampanas = true;
        window.location.href = "/Campanas.html";
    }else{
        lCampanas = false;
    }
    if (EstadoCampanario & CampanarioStates.BIT_CALEFACCION) {
        lCalefaccion =   true;
        document.getElementById("iconoCalefaccion").setAttribute("stroke", "red" );
        websocket.send("GET_TIEMPOCALEFACCION");
    }else{
        lCalefaccion = false;
        document.getElementById("iconoCalefaccion").setAttribute("stroke", "orange" );
    }
    // Verificar el bit de protecciÃ³n de campanadas (BitEstadoProteccionCampanadas = 0x40)
    if (EstadoCampanario & CampanarioStates.BIT_PROTECCION_CAMPANADAS) {
        habilitarBotonesCampanadas(false); // Deshabilita los botones si la protecciÃ³n estÃ¡ activa
        console.log("ProtecciÃ³n de campanadas activa (desde estado campanario)");
    } else {
        habilitarBotonesCampanadas(true); // Habilita los botones si la protecciÃ³n estÃ¡ inactiva
        console.log("ProtecciÃ³n de campanadas inactiva (desde estado campanario)");
    }
    if (event.data.startsWith("GET_SECUENCIA_ACTIVA:")) {
        console.log ("Recibido numero de secuencia activa: " + event.data);        
        //Tratamiento de secuencia activa
        //No utilizado en esta version
    }
}

}

function activarCampana(num) {
      // Desactiva todas
      document.getElementById("campana0").classList.remove("activa");
      document.getElementById("campana1").classList.remove("activa");
      // Activa la correspondiente (nÃºmeros desde 1)
      if (num === 1) {
        document.getElementById("campana0").classList.add("activa");
      } else if (num === 2) {
        document.getElementById("campana1").classList.add("activa");
      }
      // Opcional: desactivar la animaciÃ³n despuÃ©s de un tiempo
      setTimeout(() => {
        document.getElementById("campana0").classList.remove("activa");
        document.getElementById("campana1").classList.remove("activa");
      }, 400); // 400 ms de animaciÃ³n
    }

    function pararSecuencia() {
        if (window.confirm("¿Segur que vols aturar la seqüència?")) {
            if (typeof websocket !== "undefined" && websocket.readyState === 1) {
                websocket.send("PARAR");
            }
        }
    }

    /**
     * Habilita o deshabilita los botones de Difuntos y Misa
     * Compatible con iOS Safari y dispositivos mÃ³viles
     *
     * @param {boolean} habilitar - true para habilitar los botones, false para deshabilitarlos
     */
    function habilitarBotonesCampanadas(habilitar) {
        // Buscar botones por su clase CSS
        const botonMisa = document.querySelector(".button.Misa");
        const botonDifuntos = document.querySelector(".button.Difuntos");
        const botonFiesta = document.querySelector(".button.Fiesta");

        if (botonMisa) {
            if (habilitar) {
                // Habilitar botÃ³n
                botonMisa.disabled = false;
                botonMisa.classList.remove("disabled-mobile");
                botonMisa.style.opacity = "1";
                botonMisa.style.cursor = "pointer";
                botonMisa.style.pointerEvents = "auto";
                botonMisa.style.backgroundColor = "#059e8a"; // Color original
                botonMisa.style.webkitTouchCallout = "default";
                botonMisa.style.webkitUserSelect = "auto";
                botonMisa.title = "";
                botonMisa.onclick = function() { SelMisa(); }; // Restaurar funciÃ³n
            } else {
                // Deshabilitar botÃ³n
                botonMisa.disabled = true;
                botonMisa.classList.add("disabled-mobile");
                botonMisa.style.opacity = "0.5";
                botonMisa.style.cursor = "not-allowed";
                botonMisa.style.pointerEvents = "none"; // Previene cualquier interacciÃ³n
                botonMisa.style.backgroundColor = "#888"; // Color gris
                botonMisa.style.webkitTouchCallout = "none";
                botonMisa.style.webkitUserSelect = "none";
                botonMisa.title = "Campanadas protegidas - No disponible durante perÃ­odo de toque";
                botonMisa.onclick = function(e) {
                    e.preventDefault();
                    e.stopPropagation();
                    return false;
                }; // Bloquear funciÃ³n
            }
        }

        if (botonDifuntos) {
            if (habilitar) {
                // Habilitar botÃ³n
                botonDifuntos.disabled = false;
                botonDifuntos.classList.remove("disabled-mobile");
                botonDifuntos.style.opacity = "1";
                botonDifuntos.style.cursor = "pointer";
                botonDifuntos.style.pointerEvents = "auto";
                botonDifuntos.style.backgroundColor = "#bc4fac"; // Color original
                botonDifuntos.style.webkitTouchCallout = "default";
                botonDifuntos.style.webkitUserSelect = "auto";
                botonDifuntos.title = "";
                botonDifuntos.onclick = function() { SelDifuntos(); }; // Restaurar funciÃ³n
            } else {
                // Deshabilitar botÃ³n
                botonDifuntos.disabled = true;
                botonDifuntos.classList.add("disabled-mobile");
                botonDifuntos.style.opacity = "0.5";
                botonDifuntos.style.cursor = "not-allowed";
                botonDifuntos.style.pointerEvents = "none"; // Previene cualquier interacciÃ³n
                botonDifuntos.style.backgroundColor = "#888"; // Color gris
                botonDifuntos.style.webkitTouchCallout = "none";
                botonDifuntos.style.webkitUserSelect = "none";
                botonDifuntos.title = "Campanadas protegidas - No disponible durante perÃ­odo de toque";
                botonDifuntos.onclick = function(e) {
                    e.preventDefault();
                    e.stopPropagation();
                    return false;
                }; // Bloquear funciÃ³n
            }
        }
       if (botonFiesta) {
            if (habilitar) {
                // Habilitar botÃ³n
                botonFiesta.disabled = false;
                botonFiesta.classList.remove("disabled-mobile");
                botonFiesta.style.opacity = "1";
                botonFiesta.style.cursor = "pointer";
                botonFiesta.style.pointerEvents = "auto";
                botonFiesta.style.backgroundColor = "#e74c3c"; // Color original
                botonFiesta.style.webkitTouchCallout = "default";
                botonFiesta.style.webkitUserSelect = "auto";
                botonFiesta.title = "";
                botonFiesta.onclick = function() { SelFiesta(); }; // Restaurar funciÃ³n
            } else {
                // Deshabilitar botÃ³n
                botonFiesta.disabled = true;
                botonFiesta.classList.add("disabled-mobile");
                botonFiesta.style.opacity = "0.5";
                botonFiesta.style.cursor = "not-allowed";
                botonFiesta.style.pointerEvents = "none"; // Previene cualquier interacciÃ³n
                botonFiesta.style.backgroundColor = "#888"; // Color gris
                botonFiesta.style.webkitTouchCallout = "none";
                botonFiesta.style.webkitUserSelect = "none";
                botonFiesta.title = "Campanadas protegidas - No disponible durante perÃ­odo de toque";
                botonFiesta.onclick = function(e) {
                    e.preventDefault();
                    e.stopPropagation();
                    return false;
                }; // Bloquear funciÃ³n
            }
        }

        console.log("Botones de campanadas " + (habilitar ? "habilitados" : "deshabilitados"));
    }

    function SelMisa() {
    console.log("🔔 Activando Misa");
    if (typeof ws !== 'undefined' && ws.readyState === WebSocket.OPEN) {
        ws.send("MISA");
    } else {
        console.error("WebSocket no disponible");
    }
}

function SelDifuntos() {
    console.log("🕊️ Activando Difuntos");
    if (typeof ws !== 'undefined' && ws.readyState === WebSocket.OPEN) {
        ws.send("DIFUNTOS");
    } else {
        console.error("WebSocket no disponible");
    }
}

function SelFiesta() {
    console.log("🎉 Activando Fiesta");
    if (typeof ws !== 'undefined' && ws.readyState === WebSocket.OPEN) {
        ws.send("FIESTA");
    } else {
        console.error("WebSocket no disponible");
    }
}

function pararSecuencia() {
    console.log("⏹️ Parando secuencia");
    if (typeof ws !== 'undefined' && ws.readyState === WebSocket.OPEN) {
        ws.send("STOP");
    } else {
        console.error("WebSocket no disponible");
    }
}

function encenderCalefaccion() {
    console.log("🔥 Encendiendo calefacción");
    if (typeof ws !== 'undefined' && ws.readyState === WebSocket.OPEN) {
        ws.send("CALEFACCION_ON");
    } else {
        console.error("WebSocket no disponible");
    }
}

function mostrarConfiguracionTiempo() {
    console.log("⏰ Mostrando configuración de tiempo");
    // Mostrar modal o formulario para configurar tiempo
    const minutos = prompt(t('configurar_tiempo'), '30');
    if (minutos && !isNaN(minutos)) {
        if (typeof ws !== 'undefined' && ws.readyState === WebSocket.OPEN) {
            ws.send(`CALEFACCION_TIMER:${minutos}`);
        }
    }
}