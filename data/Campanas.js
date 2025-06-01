var gateway = `ws://${window.location.hostname}:8080/ws`;
var websocket;

window.addEventListener('load', onload);

function onload(event) {
    console.log("Página cargada correctamente");
    initWebSocket();
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
    console.log("Conexión WebSocket abierta en Datos");
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
    let idx = parseInt(event.data.split(":")[1]);
    window.activarCampana(idx);
}
/*
    try {
        var myObj = JSON.parse(event.data);
        var keys = Object.keys(myObj);

        const cardGrid = document.querySelector('.card-grid');
        if (cardGrid.classList.contains('hidden')) {
            cardGrid.classList.remove('hidden');
            cardGrid.style.display = 'grid'; // Establecer explícitamente display: grid
        }

        for (var i = 0; i < keys.length; i++){
            var key = keys[i];
            document.getElementById(key).innerHTML = myObj[key];
        }
    } catch (error) {
        console.error("Error al procesar el mensaje:", error);
    }        
*/
}
