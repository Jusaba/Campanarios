// ============================================================================
// NAMESPACE PARA EVITAR CONFLICTOS
// ============================================================================

window.Calefaccion = window.Calefaccion || {};

// ============================================================================
// VARIABLES GLOBALES PARA CONTROL DE CALEFACCIÓN
// ============================================================================

window.Calefaccion.estado = false;                    // Estado de la calefacción
window.Calefaccion.minutosDigitos = [0, 3, 0];       // Array de 3 dígitos [centenas, decenas, unidades]
window.Calefaccion.minutosDigitosTemp = [0, 3, 0];   // Array temporal para el modal
window.Calefaccion.tiempoRestante = 0;               // Tiempo restante en segundos
window.Calefaccion.intervaloCuentaRegresiva = null;  // Intervalo del contador regresivo

// ============================================================================
// FUNCIONES DE CONTROL DE CALEFACCIÓN
// ============================================================================

/**
 * Abre el modal para seleccionar minutos
 */
function abrirModalMinutos() {
    // Copiar valores actuales a los temporales
    window.Calefaccion.minutosDigitosTemp = [...window.Calefaccion.minutosDigitos];
    actualizarVisualizacionDigitos();
    actualizarTotalMinutos();
    const modal = document.getElementById('modalMinutos');
    if (modal) {
        modal.style.display = 'block';
    }
}

/**
 * Cierra el modal sin guardar cambios
 */
function cerrarModalMinutos() {
    const modal = document.getElementById('modalMinutos');
    if (modal) {
        modal.style.display = 'none';
    }
}

/**
 * Incrementa el dígito especificado con validación de 120 minutos máximo
 * @param {number} posicion - Posición del dígito (0, 1, 2)
 */
function incrementarDigito(posicion) {
    let nuevoValor = [...window.Calefaccion.minutosDigitosTemp];
    
    if (posicion === 0) {
        // Dígito 0 (centenas) solo puede ser 0 o 1
        nuevoValor[posicion] = (nuevoValor[posicion] + 1) % 2;
    } else {
        // Dígitos 1 y 2 (decenas y unidades) pueden ser 0-9 normalmente
        nuevoValor[posicion] = (nuevoValor[posicion] + 1) % 10;
    }
    
    // ✅ VALIDAR: No permitir más de 120 minutos
    const totalMinutos = nuevoValor[0] * 100 + nuevoValor[1] * 10 + nuevoValor[2];
    if (totalMinutos <= 120) {
        window.Calefaccion.minutosDigitosTemp = nuevoValor;
        actualizarVisualizacionDigitos();
        actualizarTotalMinutos();
        validarLimiteMaximo();
    } else {
        // ✅ MOSTRAR ADVERTENCIA SI SUPERA 120
        mostrarAdvertenciaLimite();
    }
}

/**
 * Decrementa el dígito especificado con validación de 120 minutos máximo
 * @param {number} posicion - Posición del dígito (0, 1, 2)
 */
function decrementarDigito(posicion) {
    if (posicion === 0) {
        // Dígito 0 (centenas) solo puede ser 0 o 1
        window.Calefaccion.minutosDigitosTemp[posicion] = (window.Calefaccion.minutosDigitosTemp[posicion] - 1 + 2) % 2;
    } else {
        // Dígitos 1 y 2 (decenas y unidades) pueden ser 0-9 normalmente
        window.Calefaccion.minutosDigitosTemp[posicion] = (window.Calefaccion.minutosDigitosTemp[posicion] - 1 + 10) % 10;
    }
    actualizarVisualizacionDigitos();
    actualizarTotalMinutos();
    validarLimiteMaximo(); // ✅ VALIDAR después de decrementar también
}

/**
 * Actualiza la visualización de los dígitos en el modal
 */
function actualizarVisualizacionDigitos() {
    for (let i = 0; i < 3; i++) {
        const elemento = document.getElementById(`digito${i}`);
        if (elemento) {
            elemento.textContent = window.Calefaccion.minutosDigitosTemp[i];
        }
    }
}

/**
 * Actualiza el total de minutos en el modal CON VALIDACIÓN
 */
function actualizarTotalMinutos() {
    const total = window.Calefaccion.minutosDigitosTemp[0] * 100 + window.Calefaccion.minutosDigitosTemp[1] * 10 + window.Calefaccion.minutosDigitosTemp[2];
    const elemento = document.getElementById('totalMinutos');
    if (elemento) {
        elemento.textContent = String(total).padStart(3, '0');
    }
    
    // ✅ VALIDAR después de actualizar
    validarLimiteMaximo();
}

/**
 * Acepta los minutos seleccionados y cierra el modal
 */
function aceptarMinutos() {
    const total = window.Calefaccion.minutosDigitosTemp[0] * 100 + window.Calefaccion.minutosDigitosTemp[1] * 10 + window.Calefaccion.minutosDigitosTemp[2];
    
    // ✅ VALIDACIÓN FINAL antes de aceptar
    if (total > 120) {
        mostrarAdvertenciaLimite();
        return; // No permitir aceptar
    }
    
    // Guardar valores temporales como definitivos
    window.Calefaccion.minutosDigitos = [...window.Calefaccion.minutosDigitosTemp];
    actualizarDisplayMinutos();
    cerrarModalMinutos();
    console.log("✅ Minutos aceptados: " + total + " (máximo permitido: 120)");
}

/**
 * Inicia el contador regresivo desde los minutos especificados
 * @param {number} minutosIniciales - Minutos para el contador regresivo
 */
function iniciarCuentaRegresiva(minutosIniciales) {
    // Detener contador anterior si existe
    detenerCuentaRegresiva();

    // Establecer tiempo restante en segundos
    window.Calefaccion.tiempoRestante = minutosIniciales * 60;

    // Actualizar inmediatamente la visualización
    actualizarVisualizacionCuentaRegresiva();

    // Iniciar el intervalo que se ejecuta cada segundo
    window.Calefaccion.intervaloCuentaRegresiva = setInterval(function() {
        window.Calefaccion.tiempoRestante--;

        if (window.Calefaccion.tiempoRestante <= 0) {
            // Tiempo agotado
            detenerCuentaRegresiva();
            window.Calefaccion.estado = false; // Cambiar estado
            actualizarEstadoCalefaccion();
            actualizarDisplayMinutos(); // Volver a mostrar minutos configurados
            
            // Notificar al servidor que se agotó el tiempo
            const ws = obtenerWebSocket();
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send("CALEFACCION_TIMEOUT");
            }
        } else {
            actualizarVisualizacionCuentaRegresiva();
        }
    }, 1000);
}

/**
 * Inicia el contador regresivo desde los segundos especificados
 * @param {number} segundosIniciales - Segundos para el contador regresivo
 */
function iniciarCuentaRegresivaSegundos(segundosIniciales) {
    // Detener contador anterior si existe
    detenerCuentaRegresiva();

    // Establecer tiempo restante en segundos directamente
    window.Calefaccion.tiempoRestante = segundosIniciales;

    // Actualizar inmediatamente la visualización
    actualizarVisualizacionCuentaRegresiva();

    // Iniciar el intervalo que se ejecuta cada segundo
    window.Calefaccion.intervaloCuentaRegresiva = setInterval(function() {
        window.Calefaccion.tiempoRestante--;

        if (window.Calefaccion.tiempoRestante <= 0) {
            // Tiempo agotado
            detenerCuentaRegresiva();
            window.Calefaccion.estado = false; // Cambiar estado
            actualizarEstadoCalefaccion();
            actualizarDisplayMinutos(); // Volver a mostrar minutos configurados
            
            // Notificar al servidor que se agotó el tiempo
            const ws = obtenerWebSocket();
            if (ws && ws.readyState === WebSocket.OPEN) {
                ws.send("CALEFACCION_TIMEOUT");
            }
        } else {
            actualizarVisualizacionCuentaRegresiva();
        }
    }, 1000);
}

/**
 * Detiene el contador regresivo
 */
function detenerCuentaRegresiva() {
    if (window.Calefaccion.intervaloCuentaRegresiva) {
        clearInterval(window.Calefaccion.intervaloCuentaRegresiva);
        window.Calefaccion.intervaloCuentaRegresiva = null;
    }
    window.Calefaccion.tiempoRestante = 0;
}

/**
 * Actualiza la visualización del tiempo restante en formato mmm:ss
 */
function actualizarVisualizacionCuentaRegresiva() {
    const minutos = Math.floor(window.Calefaccion.tiempoRestante / 60);
    const segundos = window.Calefaccion.tiempoRestante % 60;
    const textoTiempo = String(minutos).padStart(3, '0') + ':' + String(segundos).padStart(2, '0');
    const elemento = document.getElementById('minutosCalefaccion');
    if (elemento) {
        elemento.textContent = textoTiempo;
    }
}

/**
 * Muestra u oculta el botón del reloj según el estado de la calefacción
 * @param {boolean} mostrar - true para mostrar, false para ocultar
 */
function mostrarBotonReloj(mostrar) {
    const botonReloj = document.getElementById('btnReloj');
    if (botonReloj) {
        botonReloj.style.display = mostrar ? 'flex' : 'none';
    }
}

/**
 * Obtiene el valor total de minutos basado en los 3 dígitos
 * @returns {number} Total de minutos (0-999)
 */
function obtenerMinutosTotales() {
    return window.Calefaccion.minutosDigitos[0] * 100 + window.Calefaccion.minutosDigitos[1] * 10 + window.Calefaccion.minutosDigitos[2];
}

/**
 * Establece los minutos desde un valor numérico
 * @param {number} minutos - Minutos a establecer (0-999)
 */
function establecerMinutos(minutos) {
    // Asegurar que está en el rango 0-999
    minutos = Math.max(0, Math.min(999, minutos));

    window.Calefaccion.minutosDigitos[0] = Math.floor(minutos / 100); // Centenas
    window.Calefaccion.minutosDigitos[1] = Math.floor((minutos % 100) / 10); // Decenas
    window.Calefaccion.minutosDigitos[2] = minutos % 10; // Unidades

    actualizarDisplayMinutos();
}

/**
 * Obtiene la instancia de WebSocket disponible
 * @returns {WebSocket|null} WebSocket disponible o null
 */
function obtenerWebSocket() {
    return window.websocket || window.ws || (typeof websocket !== "undefined" ? websocket : null);
}

/**
 * Actualiza el estado visual COMPLETO de la calefacción
 */
function actualizarEstadoCalefaccion() {
    const icono = document.getElementById("iconoCalefaccion");
    const texto = document.getElementById("textoCalefaccion");
    const boton = document.querySelector(".button-calefaccion");
    
    // ✅ ACTUALIZAR ICONO
    if (icono) {
        icono.setAttribute("stroke", window.Calefaccion.estado ? "red" : "orange");
    }
    
    // ✅ ACTUALIZAR TEXTO DE ESTADO
    if (texto) {
        if (typeof t === 'function') {
            texto.textContent = window.Calefaccion.estado ? t('calefaccion_encendida') : t('calefaccion_apagada');
        } else {
            texto.textContent = window.Calefaccion.estado ? 'Encendida' : 'Apagada';
        }
    }
    
    // ✅ ACTUALIZAR BOTÓN PRINCIPAL
    if (boton) {
        if (typeof t === 'function') {
            if (window.Calefaccion.estado) {
                boton.innerHTML = `🔥 <span data-i18n="calefaccion_apagar">${t('calefaccion_apagar')}</span>`;
            } else {
                boton.innerHTML = `🔥 <span data-i18n="calefaccion_encender">${t('calefaccion_encender')}</span>`;
            }
        } else {
            boton.textContent = window.Calefaccion.estado ? '🔥 Apagar' : '🔥 Encender';
        }
    }
    
    // ✅ MOSTRAR/OCULTAR BOTÓN DE CONFIGURACIÓN
    mostrarBotonReloj(!window.Calefaccion.estado);
    
    console.log(`🔥 Estado completo actualizado: ${window.Calefaccion.estado ? 'ENCENDIDA' : 'APAGADA'}`);
}

/**
 * Alterna el estado de la calefacción
 */
function toggleCalefaccion() {
    window.Calefaccion.estado = !window.Calefaccion.estado;

    // Actualizar estado visual
    actualizarEstadoCalefaccion();

    // Verificar diferentes variables de WebSocket que pueden existir
    const ws = obtenerWebSocket();
    
    if (ws && ws.readyState === WebSocket.OPEN) {
        if (window.Calefaccion.estado) {
            const minutos = obtenerMinutosTotales();
            ws.send(`CALEFACCION_ON:${minutos}`);
            console.log(`🔥 Calefacción encendida por ${minutos} minutos`);

            // Iniciar cuenta regresiva si hay minutos configurados
            if (minutos > 0) {
                iniciarCuentaRegresiva(minutos);
            }
        } else {
            ws.send("CALEFACCION_OFF");
            console.log(`🔥 Calefacción apagada`);
            // Detener cuenta regresiva y resetear visualización
            detenerCuentaRegresiva();
            actualizarDisplayMinutos();
        }
    } else {
        console.warn("⚠️ WebSocket no disponible para calefacción");
    }
}

/**
 * Función para manejar mensajes del servidor relacionados con calefacción
 * @param {string} mensaje - Mensaje recibido del servidor
 */
function procesarMensajeCalefaccion(mensaje) {
    console.log("🔥 Procesando mensaje de calefacción:", mensaje);
    
    if (mensaje.startsWith("TIEMPO_CALEFACCION:")) {
        const parts = mensaje.split(":");
        if (parts.length >= 2) {
            const segundos = parseInt(parts[1]);
            console.log(`⏰ Tiempo restante de calefacción: ${segundos} segundos`);

            if (segundos > 0) {
                // ✅ CALEFACCIÓN ACTIVA: Configurar estado completo
                window.Calefaccion.estado = true;
                
                // ✅ ACTUALIZAR INTERFAZ COMPLETA
                actualizarEstadoCalefaccion();
                
                // ✅ INICIAR CUENTA REGRESIVA
                iniciarCuentaRegresivaSegundos(segundos);
                
                console.log("✅ Calefacción configurada como ACTIVA con cuenta regresiva");
            } else {
                // ✅ TIEMPO AGOTADO: Apagar calefacción
                window.Calefaccion.estado = false;
                actualizarEstadoCalefaccion();
                detenerCuentaRegresiva();
                actualizarDisplayMinutos();
                
                console.log("⏰ Tiempo de calefacción agotado - Apagada");
            }
        }
    }
    else if (mensaje.startsWith("CALEFACCION:ON")) {
        console.log("🔥 Calefacción encendida desde servidor");
        window.Calefaccion.estado = true;
        actualizarEstadoCalefaccion();

        // Extraer los minutos si están presentes
        const parts = mensaje.split(":");
        if (parts.length >= 3) {
            const minutos = parseInt(parts[2]);
            console.log(`🔥 Calefacción encendida por ${minutos} minutos`);

            if (minutos > 0) {
                iniciarCuentaRegresiva(minutos);
            }
        }
    }
    else if (mensaje.startsWith("CALEFACCION:OFF")) {
        console.log("🔥 Calefacción apagada desde servidor");
        window.Calefaccion.estado = false;
        actualizarEstadoCalefaccion();
        detenerCuentaRegresiva();
        actualizarDisplayMinutos();
    }
    else if (mensaje.startsWith("CALEFACCION:ERROR")) {
        console.log("❌ Error en calefacción:", mensaje);
        alert("⚠️ No se pudo encender la calefacción.\nEl sistema de tiempo no está disponible.\nIntenta de nuevo en unos momentos.");
        
        window.Calefaccion.estado = false;
        actualizarEstadoCalefaccion();
        detenerCuentaRegresiva();
        actualizarDisplayMinutos();
    }
}

/**
 * ✅ NUEVA FUNCIÓN: Valida si se supera el límite de 120 minutos
 */
function validarLimiteMaximo() {
    const total = window.Calefaccion.minutosDigitosTemp[0] * 100 + window.Calefaccion.minutosDigitosTemp[1] * 10 + window.Calefaccion.minutosDigitosTemp[2];
    const botonAceptar = document.getElementById('btnAceptarMinutos');
    const advertencia = document.getElementById('advertenciaLimite');
    const totalElement = document.getElementById('totalMinutos');
    
    if (total > 120) {
        // ✅ SUPERA EL LÍMITE: Deshabilitar botón y mostrar advertencia
        if (botonAceptar) {
            botonAceptar.disabled = true;
            botonAceptar.classList.add('btn-deshabilitado');
        }
        if (advertencia) {
            advertencia.style.display = 'block';
        }
        if (totalElement) {
            totalElement.style.color = '#dc3545'; // Rojo
            totalElement.parentElement.style.borderColor = '#dc3545';
        }
        console.warn("⚠️ Límite máximo superado: " + total + " minutos (máximo: 120)");
    } else {
        // ✅ DENTRO DEL LÍMITE: Habilitar botón y ocultar advertencia
        if (botonAceptar) {
            botonAceptar.disabled = false;
            botonAceptar.classList.remove('btn-deshabilitado');
        }
        if (advertencia) {
            advertencia.style.display = 'none';
        }
        if (totalElement) {
            totalElement.style.color = '#cc5500'; // Color original
            totalElement.parentElement.style.borderColor = '#4a90e2'; // Color original
        }
    }
}

/**
 * ✅ NUEVA FUNCIÓN: Muestra advertencia temporal cuando se intenta superar 120
 */
function mostrarAdvertenciaLimite() {
    const advertencia = document.getElementById('advertenciaLimite');
    if (advertencia) {
        advertencia.style.display = 'block';
        advertencia.classList.add('advertencia-shake');
        
        // Quitar la animación después de medio segundo
        setTimeout(() => {
            if (advertencia) {
                advertencia.classList.remove('advertencia-shake');
            }
        }, 500);
    }
    
    // También mostrar en consola
    console.warn("⚠️ Límite máximo: 120 minutos");
}

/**
 * Actualiza la visualización de minutos en el botón de calefacción
 */
function actualizarDisplayMinutos() {
    const total = obtenerMinutosTotales();
    const elemento = document.getElementById('minutosCalefaccion');
    if (elemento) {
        elemento.textContent = String(total).padStart(3, '0') + 'm';
    }
}

// ============================================================================
// EVENTOS Y INICIALIZACIÓN ESPECÍFICOS DE CALEFACCIÓN
// ============================================================================

/**
 * Maneja el click fuera del modal para cerrarlo
 */
function manejarClickFueraModal(event) {
    const modal = document.getElementById('modalMinutos');
    if (modal && event.target === modal) {
        cerrarModalMinutos();
    }
}

/**
 * Inicializa el sistema de calefacción
 */
function inicializarCalefaccion() {
    console.log("🔥 Inicializando sistema de calefacción...");
    
    // ✅ VERIFICAR que los elementos existan
    const icono = document.getElementById("iconoCalefaccion");
    const texto = document.getElementById("textoCalefaccion");
    const boton = document.querySelector(".button-calefaccion");
    const display = document.getElementById("minutosCalefaccion");
    
    console.log("🔍 Verificando elementos DOM:");
    console.log("  - Icono calefacción:", icono ? "✅" : "❌");
    console.log("  - Texto calefacción:", texto ? "✅" : "❌");
    console.log("  - Botón calefacción:", boton ? "✅" : "❌");
    console.log("  - Display minutos:", display ? "✅" : "❌");
    
    // Actualizar displays
    actualizarDisplayMinutos();
    mostrarBotonReloj(true);
    actualizarEstadoCalefaccion();
    
    console.log("✅ Sistema de calefacción inicializado");
    console.log("🔥 Estado inicial:", window.Calefaccion.estado ? "ENCENDIDA" : "APAGADA");
}

/**
 * Limpia los recursos del sistema de calefacción
 */
function limpiarCalefaccion() {
    detenerCuentaRegresiva();
    document.removeEventListener('click', manejarClickFueraModal);
    console.log("🧹 Sistema de calefacción limpiado");
}

// ============================================================================
// AUTO-INICIALIZACIÓN
// ============================================================================

// Verificar si el DOM ya está cargado o esperar a que se cargue
if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', inicializarCalefaccion);
} else {
    // DOM ya está cargado
    inicializarCalefaccion();
}

// Limpiar al cerrar la página
window.addEventListener('beforeunload', limpiarCalefaccion);

// ============================================================================
// EXPORTAR FUNCIONES GLOBALMENTE PARA EVITAR ERRORES
// ============================================================================

// Asegurar que las funciones estén disponibles globalmente
window.abrirModalMinutos = abrirModalMinutos;
window.cerrarModalMinutos = cerrarModalMinutos;
window.aceptarMinutos = aceptarMinutos;
window.incrementarDigito = incrementarDigito;
window.decrementarDigito = decrementarDigito;
window.toggleCalefaccion = toggleCalefaccion;
window.procesarMensajeCalefaccion = procesarMensajeCalefaccion;