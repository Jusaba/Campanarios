var gateway = `ws://${window.location.hostname}:8080/ws`;
var websocket;

let lCalefaccion = false;
let lCampanas = false;

window.addEventListener('load', onload);

function onload(event) {
    console.log("Página cargada correctamente");
    initWebSocket();
    if (typeof websocket !== "undefined") {
       
    }    
}

/**
 * Inicializa una conexión WebSocket con el servidor especificado en la variable `gateway`.
 * Configura los manejadores de eventos para la conexión WebSocket:
 * - `onopen`: Se ejecuta cuando la conexión se abre exitosamente.
 * - `onclose`: Se ejecuta cuando la conexión se cierra.
 * - `onmessage`: Se ejecuta cuando se recibe un mensaje desde el servidor.
 * 
 * @function
 * @throws {Error} Si no se puede establecer la conexión WebSocket.
 */
function initWebSocket() {
    websocket = new WebSocket(gateway);
    websocket.onmessage = onMessageDatos;
    websocket.onopen = onOpenDatos;
    websocket.onclose = onCloseDatos;    
}

/**
 * Función que se ejecuta al abrir un evento.
 *
 * @param {Event} event - El evento que dispara la función al abrir.
 */
function onOpenDatos(event) {
    const path = window.location.pathname;
    console.log("Conexión WebSocket abierta en Datos");
    if ( path === "/" || path.endsWith("index.html") ) {
        websocket.send("GET_CAMPANARIO");
    }
}

/**
 * Maneja el evento de cierre de una conexión, reiniciando el WebSocket después de un retraso.
 * 
 * @param {Event} event - El evento de cierre que dispara esta función.
 */
function onCloseDatos(event) {
    console.log("Conexión WebSocket cerrada, intentando reconectar...");
    setTimeout(initWebSocket, 2000);
}


/**
 * Maneja el evento de mensaje recibido, actualizando el contenido de los elementos HTML
 * según los datos proporcionados en el mensaje.
 *
 * @param {MessageEvent} event - El evento de mensaje recibido, que contiene los datos en formato JSON.
 * @throws {SyntaxError} Si los datos del mensaje no son un JSON válido.
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
}
if (event.data.startsWith("CALEFACCION:OFF")) {
    console.log ("Actualizando calefaccion: " + event.data);
    lCalefaccion = false;
    document.getElementById("iconoCalefaccion").setAttribute("stroke", "orange" );
}
if (event.data.startsWith("ESTADO_CAMPANARIO:")) {
    console.log ("Comprobando estado de campanario: " + event.data);
    let EstadoCampanario = parseInt(event.data.split(":")[1]);
    if ((EstadoCampanario & 0x01) || (EstadoCampanario & 0x02)) {
        lCampanas = true;
        if (EstadoCampanario & 0x01) {
           console.log("Difuntos")
        } else {
            console.log("Misa");
        }
        window.location.href = "/Campanas.html";
    }else{
        lCampanas = false;
    }    
    if (EstadoCampanario & 0x10) {
        lCalefaccion =   true;
        document.getElementById("iconoCalefaccion").setAttribute("stroke", "red" );
    }else{
        lCalefaccion = false;
        document.getElementById("iconoCalefaccion").setAttribute("stroke", "orange" );
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
      // Opcional: desactivar la animación después de un tiempo
      setTimeout(() => {
        document.getElementById("campana0").classList.remove("activa");
        document.getElementById("campana1").classList.remove("activa");
      }, 400); // 400 ms de animación
    }

    function pararSecuencia() {
        if (window.confirm("¿Seguro que quieres parar la secuencia?")) {
            if (typeof websocket !== "undefined" && websocket.readyState === 1) {
                websocket.send("PARAR");
            }
        }
    }