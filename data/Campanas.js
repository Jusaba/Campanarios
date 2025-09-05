// ============================================================================
// CONSTANTES DE ESTADO - Sincronizadas con Config::States en ESP32
// ============================================================================
const CampanarioStates = {
    // Estados I2C (para comandos)
    DIFUNTOS: "Difuntos",
    MISA: "Misa", 
    STOP: "PARAR",
    GET_CAMPANARIO: "GET_CAMPANARIO",
    GET_TIEMPO_CALEFACCION: "GET_TIEMPOCALEFACCION",
    
    // Bits de estado (para ESTADO_CAMPANARIO response)
    BIT_DIFUNTOS: 0x01,                      // Config::States::BIT_DIFUNTOS
    BIT_MISA: 0x02,                          // Config::States::BIT_MISA  
    BIT_HORA: 0x04,                          // Config::States::BIT_HORA
    BIT_CUARTOS: 0x08,                       // Config::States::BIT_CUARTOS
    BIT_CALEFACCION: 0x10,                   // Config::States::BIT_CALEFACCION
    BIT_SIN_INTERNET: 0x20,                  // Config::States::BIT_SIN_INTERNET
    BIT_PROTECCION_CAMPANADAS: 0x40          // Config::States::BIT_PROTECCION_CAMPANADAS
};

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
    
    // Ocultar botón del reloj cuando está encendida
    if (typeof mostrarBotonReloj === 'function') {
        mostrarBotonReloj(false);
    }
    
    // Extraer los minutos si están presentes
    const parts = event.data.split(":");
    if (parts.length >= 3) {
        const minutos = parseInt(parts[2]);
        console.log(`Calefacción encendida por ${minutos} minutos`);
        
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
    
    // Mostrar botón del reloj cuando está apagada
    if (typeof mostrarBotonReloj === 'function') {
        mostrarBotonReloj(true);
    }
    
    // Detener cuenta regresiva y resetear visualización
    if (typeof detenerCuentaRegresiva === 'function') {
        detenerCuentaRegresiva();
    }
    if (typeof actualizarDisplayMinutos === 'function') {
        actualizarDisplayMinutos();
    }
}
if (event.data.startsWith("CALEFACCION:ERROR")) {
    console.log ("Error encendiendo calefaccion: " + event.data);
    alert("⚠️ No se pudo encender la calefacción.\nEl sistema de tiempo no está disponible.\nIntenta de nuevo en unos momentos.");
    lCalefaccion = false;
    document.getElementById("iconoCalefaccion").setAttribute("stroke", "orange" );
    
    // Mostrar botón del reloj cuando hay error
    if (typeof mostrarBotonReloj === 'function') {
        mostrarBotonReloj(true);
    }
    
    // Detener cuenta regresiva y resetear visualización
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
        console.log(`Tiempo restante de calefacción: ${segundos} segundos`);
        
        if (segundos > 0) {
            // Asegurar que el estado visual esté correcto
            lCalefaccion = true;
            document.getElementById("iconoCalefaccion").setAttribute("stroke", "red");
            
            // Ocultar botón del reloj cuando está encendida
            if (typeof mostrarBotonReloj === 'function') {
                mostrarBotonReloj(false);
            }
            
            // Iniciar cuenta regresiva con los segundos restantes
            if (typeof iniciarCuentaRegresivaSegundos === 'function') {
                iniciarCuentaRegresivaSegundos(segundos);
            }
        } else {
            // Si no quedan segundos, apagar calefacción
            lCalefaccion = false;
            document.getElementById("iconoCalefaccion").setAttribute("stroke", "orange");
            
            // Mostrar botón del reloj cuando está apagada
            if (typeof mostrarBotonReloj === 'function') {
                mostrarBotonReloj(true);
            }
            
            // Detener cuenta regresiva y resetear visualización
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
    habilitarBotonesCampanadas(false); // Deshabilita los botones cuando la protección está activa
}
if (event.data.startsWith("PROTECCION:OFF")) {
    console.log ("Actualizando la proteccion de campanadas horaris: " + event.data);
    habilitarBotonesCampanadas(true); // Habilita los botones cuando la protección está inactiva
}
if (event.data.startsWith("ESTADO_CAMPANARIO:")) {
    console.log ("Comprobando estado de campanario: " + event.data);
    let EstadoCampanario = parseInt(event.data.split(":")[1]);
    if ((EstadoCampanario & CampanarioStates.BIT_DIFUNTOS) || (EstadoCampanario & CampanarioStates.BIT_MISA)) {
        lCampanas = true;
        if (EstadoCampanario & CampanarioStates.BIT_DIFUNTOS) {
           console.log("Difuntos")
        } else {
            console.log("Misa");
        }
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
    // Verificar el bit de protección de campanadas (BitEstadoProteccionCampanadas = 0x40)
    if (EstadoCampanario & CampanarioStates.BIT_PROTECCION_CAMPANADAS) {
        habilitarBotonesCampanadas(false); // Deshabilita los botones si la protección está activa
        console.log("Protección de campanadas activa (desde estado campanario)");
    } else {
        habilitarBotonesCampanadas(true); // Habilita los botones si la protección está inactiva
        console.log("Protección de campanadas inactiva (desde estado campanario)");
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

    /**
     * Habilita o deshabilita los botones de Difuntos y Misa
     * Compatible con iOS Safari y dispositivos móviles
     * 
     * @param {boolean} habilitar - true para habilitar los botones, false para deshabilitarlos
     */
    function habilitarBotonesCampanadas(habilitar) {
        // Buscar botones por su clase CSS
        const botonMisa = document.querySelector(".button.Misa");
        const botonDifuntos = document.querySelector(".button.Difuntos");
        
        if (botonMisa) {
            if (habilitar) {
                // Habilitar botón
                botonMisa.disabled = false;
                botonMisa.classList.remove("disabled-mobile");
                botonMisa.style.opacity = "1";
                botonMisa.style.cursor = "pointer";
                botonMisa.style.pointerEvents = "auto";
                botonMisa.style.backgroundColor = "#059e8a"; // Color original
                botonMisa.style.webkitTouchCallout = "default";
                botonMisa.style.webkitUserSelect = "auto";
                botonMisa.title = "";
                botonMisa.onclick = function() { SelMisa(); }; // Restaurar función
            } else {
                // Deshabilitar botón
                botonMisa.disabled = true;
                botonMisa.classList.add("disabled-mobile");
                botonMisa.style.opacity = "0.5";
                botonMisa.style.cursor = "not-allowed";
                botonMisa.style.pointerEvents = "none"; // Previene cualquier interacción
                botonMisa.style.backgroundColor = "#888"; // Color gris
                botonMisa.style.webkitTouchCallout = "none";
                botonMisa.style.webkitUserSelect = "none";
                botonMisa.title = "Campanadas protegidas - No disponible durante período de toque";
                botonMisa.onclick = function(e) { 
                    e.preventDefault(); 
                    e.stopPropagation(); 
                    return false; 
                }; // Bloquear función
            }
        }
        
        if (botonDifuntos) {
            if (habilitar) {
                // Habilitar botón
                botonDifuntos.disabled = false;
                botonDifuntos.classList.remove("disabled-mobile");
                botonDifuntos.style.opacity = "1";
                botonDifuntos.style.cursor = "pointer";
                botonDifuntos.style.pointerEvents = "auto";
                botonDifuntos.style.backgroundColor = "#e74c3c"; // Color original
                botonDifuntos.style.webkitTouchCallout = "default";
                botonDifuntos.style.webkitUserSelect = "auto";
                botonDifuntos.title = "";
                botonDifuntos.onclick = function() { SelDifuntos(); }; // Restaurar función
            } else {
                // Deshabilitar botón
                botonDifuntos.disabled = true;
                botonDifuntos.classList.add("disabled-mobile");
                botonDifuntos.style.opacity = "0.5";
                botonDifuntos.style.cursor = "not-allowed";
                botonDifuntos.style.pointerEvents = "none"; // Previene cualquier interacción
                botonDifuntos.style.backgroundColor = "#888"; // Color gris
                botonDifuntos.style.webkitTouchCallout = "none";
                botonDifuntos.style.webkitUserSelect = "none";
                botonDifuntos.title = "Campanadas protegidas - No disponible durante período de toque";
                botonDifuntos.onclick = function(e) { 
                    e.preventDefault(); 
                    e.stopPropagation(); 
                    return false; 
                }; // Bloquear función
            }
        }
        
        console.log("Botones de campanadas " + (habilitar ? "habilitados" : "deshabilitados"));
    }