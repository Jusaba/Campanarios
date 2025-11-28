/**
 * Gestor de Alarmas Personalizadas para Campanarios ESP32
 * Autor: Julian Salas Bartolomé
 * Fecha: 02/01/2025
 */

console.log('🔔 Alarmas.js cargado correctamente');

// Verificar disponibilidad del sistema de idiomas
console.log('🌍 Estado del sistema de idiomas:', {
    t_disponible: typeof t,
    idiomaActual: typeof idiomaActual !== 'undefined' ? idiomaActual : 'no definido',
    IDIOMAS_disponible: typeof IDIOMAS !== 'undefined'
});



class AlarmManager {
    constructor() {
        this.ws = null;
        this.reconnectTimeout = null;
        this.alarmas = [];
        this.editingId = null;
        
        // Esperar a que el sistema de idiomas esté disponible
        this.esperarSistemaIdiomas().then(() => {
            this.init();
        });
    }
    
    async esperarSistemaIdiomas() {
        console.log('⏳ Esperando sistema de idiomas...');
        let intentos = 0;
        const maxIntentos = 100; // 10 segundos máximo
        
        while (intentos < maxIntentos) {
            if (typeof t === 'function' && typeof idiomaActual !== 'undefined' && typeof IDIOMAS !== 'undefined') {
                console.log('✅ Sistema de idiomas disponible, idioma actual:', idiomaActual);
                // Esperar un poco más para asegurar que todo está cargado
                await new Promise(resolve => setTimeout(resolve, 300));
                return;
            }
            await new Promise(resolve => setTimeout(resolve, 100));
            intentos++;
        }
        
        console.warn('⚠️ Sistema de idiomas no disponible después de 10 segundos, continuando...');
    }
    
    // Función helper para obtener traducciones de forma segura
    tr(clave, fallback = clave) {
        if (typeof t === 'function' && typeof idiomaActual !== 'undefined') {
            try {
                const traduccion = t(clave);
                console.log(`🔤 Traduciendo '${clave}': '${traduccion}' (idioma: ${idiomaActual})`);
                return traduccion !== clave ? traduccion : fallback;
            } catch (e) {
                console.warn(`⚠️ Error traduciendo '${clave}':`, e);
                return fallback;
            }
        }
        console.log(`🔤 Sistema de idiomas no disponible, usando fallback para '${clave}': '${fallback}'`);
        return fallback;
    }
    
    init() {
        this.setupWebSocket();
        this.setupEventListeners();
        this.showStatus(`${this.tr('conectando_servidor', 'Conectando al servidor')}...`, "info");
        
        // Registrar para actualizaciones de idioma
        if (window.actualizarTextosAlarmas) {
            delete window.actualizarTextosAlarmas;
        }
        window.actualizarTextosAlarmas = () => this.actualizarTextosIdioma();
        
        // ✅ FORZAR actualización de textos de interfaz al iniciar
        if (typeof actualizarTextosInterfaz === 'function') {
            console.log('🔄 Forzando actualización de textos de interfaz desde Alarmas.js');
            actualizarTextosInterfaz();
        }
    }
    
    setupWebSocket() {
        const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
        this.ws = new WebSocket(`${protocol}//${window.location.host}/ws`);
        
        this.ws.onopen = () => {
            this.showStatus(`🔗 ${this.tr('conectado_servidor', 'Conectado al servidor')}`, "success");
            
            // Solicitar idioma del servidor
            setTimeout(() => {
                console.log("🌍 Solicitando idioma del servidor desde Alarmas.js");
                this.ws.send("GET_IDIOMA");
            }, 500);
            
            this.requestData();
        };
        
        this.ws.onmessage = (event) => {
            this.handleMessage(event.data);
        };
        
        this.ws.onclose = () => {
            this.showStatus(`❌ ${this.tr('conexion_perdida', 'Conexión perdida')}...`, "error");
            this.scheduleReconnect();
        };
        
        this.ws.onerror = (error) => {
            console.error('WebSocket error:', error);
            this.showStatus(`❌ ${this.tr('error_conexion', 'Error de conexión')}`, "error");
        };
    }
    
    scheduleReconnect() {
        if (this.reconnectTimeout) {
            clearTimeout(this.reconnectTimeout);
        }
        
        this.reconnectTimeout = setTimeout(() => {
            this.setupWebSocket();
        }, 3000);
    }
    
    requestData() {
        console.log("📡 requestData() llamada");
        console.log("🌐 WebSocket estado:", this.ws?.readyState);
        console.log("🌐 WebSocket constantes:", {
            CONNECTING: WebSocket.CONNECTING,
            OPEN: WebSocket.OPEN,
            CLOSING: WebSocket.CLOSING,
            CLOSED: WebSocket.CLOSED
        });
        
        if (this.ws && this.ws.readyState === WebSocket.OPEN) {
            console.log("📤 Enviando GET_ALARMAS_WEB");
            this.ws.send('GET_ALARMAS_WEB');
            
            console.log("📤 Enviando GET_STATS_ALARMAS_WEB");
            this.ws.send('GET_STATS_ALARMAS_WEB');
            
            this.showStatus(`🔄 ${this.tr('solicitando_datos', 'Solicitando datos')}...`, "info");
        } else {
            console.error("❌ WebSocket no disponible. Estado:", this.ws?.readyState);
            this.showStatus(`❌ ${this.tr('no_conexion_servidor', 'No hay conexión con el servidor')}`, "error");
            
            // ✅ INTENTAR RECONECTAR
            console.log("🔄 Intentando reconectar WebSocket...");
            this.setupWebSocket();
        }
    }
    
    handleMessage(message) {
        console.log('📨 Mensaje recibido:', message.substring(0, 50) + '...');
        
        if (message.startsWith('ALARMAS_WEB:')) {
            console.log("📋 Procesando ALARMAS_WEB");
            const jsonData = message.substring(12);
            try {
                const data = JSON.parse(jsonData);
                this.alarmas = data.alarmas || [];
                console.log(`✅ ${this.alarmas.length} alarmas cargadas`);
                
                // Esperar un poco para asegurar que el sistema de idiomas esté listo
                setTimeout(() => {
                    this.renderAlarms();
                }, 200);
                
                this.showStatus(`✅ ${this.alarmas.length} ${this.tr('alarmas_cargadas', 'alarmas cargadas')}`, "success");
            } catch (e) {
                console.error('❌ Error parsing alarm data:', e);
                this.showStatus(`❌ ${this.tr('error_procesando_alarmas', 'Error procesando datos de alarmas')}`, "error");
            }
        }
        else if (message.startsWith('STATS_ALARMAS_WEB:')) {
            console.log("📊 Procesando STATS_ALARMAS_WEB");
            const jsonData = message.substring(18);
            try {
                const stats = JSON.parse(jsonData);
                console.log("✅ Stats procesado:", stats);
                
                // Esperar un poco para asegurar que el sistema de idiomas esté listo
                setTimeout(() => {
                    this.renderStats({
                        totalAlarmas: stats.totalAlarmas,
                        habilitadas: stats.habilitadas,
                        deshabilitadas: stats.deshabilitadas,
                        espacioLibre: stats.espacioLibre
                    });
                }, 200);
            } catch (e) {
                console.error('❌ Error parsing stats:', e);
                setTimeout(() => {
                    this.renderStatsBasico();
                }, 200);
            }
        }
        // ✅ CORREGIR: Usar el formato correcto que envía el servidor
        else if (message.startsWith('ALARMA_CREADA_WEB:')) {
            const id = message.substring(18); // 18 caracteres en "ALARMA_CREADA_WEB:"
            console.log(`✅ ALARMA_CREADA_WEB recibido: ${id}`);
            this.showStatus(`✅ ${this.tr('alarma_creada_id', 'Alarma creada con ID')}: ${id}`, "success");
            this.limpiarFormulario();
            
            // ✅ AUTO-REFRESH CON DEBUG
            console.log("🔄 Auto-refrescando después de crear...");
            setTimeout(() => {
                this.requestData();
            }, 500);
        }
        else if (message.startsWith('ALARMA_MODIFICADA_WEB:')) {
            const id = message.substring(21); // 21 caracteres en "ALARMA_MODIFICADA_WEB:"
            console.log(`✅ ALARMA_MODIFICADA_WEB recibido: ${id}`);
            this.showStatus(`✅ Alarma ${id} ${this.tr('alarma_modificada_correctamente', 'modificada correctamente')}`, "success");
            this.editingId = null;
            this.limpiarFormulario();
            
            // ✅ AUTO-REFRESH CON DEBUG
            console.log("🔄 Auto-refrescando después de modificar...");
            setTimeout(() => {
                this.requestData();
            }, 500);
        }
        else if (message.startsWith('ALARMA_ELIMINADA_WEB:')) {
            const id = message.substring(20); // 20 caracteres en "ALARMA_ELIMINADA_WEB:"
            console.log(`✅ ALARMA_ELIMINADA_WEB recibido: ${id}`);
            this.showStatus(`🗑️ ${this.tr('alarma_eliminada', 'Alarma eliminada')} ${id}`, "success");
            
            // ✅ AUTO-REFRESH CON DEBUG
            console.log("🔄 Auto-refrescando después de eliminar...");
            setTimeout(() => {
                this.requestData();
            }, 500);
        }
        else if (message.startsWith('ALARMA_TOGGLED_WEB:')) {
            const parts = message.split(':');
            const id = parts[1];
            const state = parts[2];
            console.log(`✅ ALARMA_TOGGLE_WEB recibido: ${id} = ${state}`);
            this.showStatus(`🔄 Alarma ${id} ${state === 'ON' ? this.tr('habilitadas', 'habilitada') : this.tr('deshabilitadas', 'deshabilitada')}`, "success");
            
            // Actualizar el atributo del botón específico
            const btn = document.querySelector(`button[data-alarm-id="${id}"]`);
            if (btn) {
                btn.setAttribute('data-alarm-enabled', state === 'ON' ? 'true' : 'false');
                if (state === 'ON') {
                    btn.className = 'disable-btn';
                    btn.innerHTML = `⏸️ <span data-i18n="deshabilitar">${this.tr('deshabilitar', 'Deshabilitar')}</span>`;
                } else {
                    btn.className = 'enable-btn';
                    btn.innerHTML = `▶️ <span data-i18n="habilitar">${this.tr('habilitar', 'Habilitar')}</span>`;
                }
            }
            
            // ✅ AUTO-REFRESH CON DEBUG
            console.log("🔄 Auto-refrescando después de toggle...");
            setTimeout(() => {
                this.requestData();
            }, 500);
        }
        else if (message.startsWith('ERROR_ALARMA_WEB:')) {
            const error = message.substring(16); // 16 caracteres en "ERROR_ALARMA_WEB:"
            console.error("❌ ERROR_ALARMA_WEB recibido:", error);
            this.showStatus(`❌ Error: ${error}`, "error");
        }
        else if (message.startsWith('IDIOMA_CAMBIADO:')) {
            const nuevoIdioma = message.substring(16);
            console.log(`🌍 Idioma cambiado en servidor: ${nuevoIdioma}`);
            if (typeof idiomaActual !== 'undefined' && idiomaActual !== nuevoIdioma) {
                idiomaActual = nuevoIdioma;
                localStorage.setItem('idioma_campanario', nuevoIdioma);
                
                // Actualizar interfaz después de un pequeño delay
                setTimeout(() => {
                    if (typeof actualizarTextosInterfaz === 'function') {
                        actualizarTextosInterfaz();
                    }
                    this.actualizarTextosIdioma();
                }, 100);
            }
        }
        else if (message.startsWith('IDIOMA_SERVIDOR:')) {
            const idiomaServidor = message.substring(16);
            console.log(`🌍 Idioma del servidor recibido: ${idiomaServidor}`);
            if (typeof idiomaActual !== 'undefined' && idiomaActual !== idiomaServidor) {
                idiomaActual = idiomaServidor;
                localStorage.setItem('idioma_campanario', idiomaServidor);
                
                // Actualizar interfaz
                setTimeout(() => {
                    if (typeof actualizarTextosInterfaz === 'function') {
                        actualizarTextosInterfaz();
                    }
                    this.actualizarTextosIdioma();
                }, 100);
            }
        }
        else {
            console.log("🤷 Mensaje no reconocido:", message);
        }
    }
    
    renderStats(stats) {
        const statsPanel = document.getElementById('statsPanel');
        
        console.log('📊 Renderizando estadísticas completas...');
        console.log('🌍 Estado del sistema de idiomas:', {
            t_disponible: typeof t,
            idiomaActual: typeof idiomaActual !== 'undefined' ? idiomaActual : 'no definido'
        });
        
        const textoEstadisticas = this.tr('estadisticas_sistema', 'Estadísticas del Sistema');
        const textoTotal = this.tr('total', 'Total');
        const textoHabilitadas = this.tr('habilitadas', 'Habilitadas');
        const textoDeshabilitadas = this.tr('deshabilitadas', 'Deshabilitadas');
        const textoEspacioLibre = this.tr('espacio_libre', 'Espacio libre');
        
        statsPanel.innerHTML = `
            <strong>📊 ${textoEstadisticas}</strong><br>
            ${textoTotal}: ${stats.totalAlarmas} | ${textoHabilitadas}: ${stats.habilitadas} | 
            ${textoDeshabilitadas}: ${stats.deshabilitadas} | ${textoEspacioLibre}: ${stats.espacioLibre}
        `;
        
        console.log('✅ Estadísticas renderizadas con textos:', {
            estadisticas: textoEstadisticas,
            total: textoTotal,
            habilitadas: textoHabilitadas,
            deshabilitadas: textoDeshabilitadas,
            espacioLibre: textoEspacioLibre
        });
    }
renderStatsBasico() {
        const habilitadas = this.alarmas.filter(a => a.habilitada).length;
        const deshabilitadas = this.alarmas.length - habilitadas;
        
        const statsPanel = document.getElementById('statsPanel');
        if (statsPanel) {
            console.log('📊 Renderizando estadísticas básicas...');
            console.log('🌍 Estado del sistema de idiomas:', {
                t_disponible: typeof t,
                idiomaActual: typeof idiomaActual !== 'undefined' ? idiomaActual : 'no definido'
            });
            
            const textoEstadisticas = this.tr('estadisticas_sistema', 'Estadísticas del Sistema');
            const textoTotal = this.tr('total', 'Total');
            const textoHabilitadas = this.tr('habilitadas', 'Habilitadas');
            const textoDeshabilitadas = this.tr('deshabilitadas', 'Deshabilitadas');
            const textoEspacioLibre = this.tr('espacio_libre', 'Espacio libre');
            const textoCalculando = this.tr('calculando', 'Calculando');
            
            statsPanel.innerHTML = `
                <strong>📊 ${textoEstadisticas}</strong><br>
                ${textoTotal}: ${this.alarmas.length} | 
                ${textoHabilitadas}: ${habilitadas} | 
                ${textoDeshabilitadas}: ${deshabilitadas} | 
                ${textoEspacioLibre}: ${textoCalculando}...
            `;
            
            console.log('✅ Estadísticas básicas renderizadas con textos:', {
                estadisticas: textoEstadisticas,
                total: textoTotal,
                habilitadas: textoHabilitadas,
                deshabilitadas: textoDeshabilitadas,
                espacioLibre: textoEspacioLibre,
                calculando: textoCalculando
            });
        }
    }
    
    renderAlarms() {
        const container = document.getElementById('alarmList');
        
        console.log('📋 Renderizando lista de alarmas...');
        console.log('🌍 Estado del sistema de idiomas:', {
            t_disponible: typeof t,
            idiomaActual: typeof idiomaActual !== 'undefined' ? idiomaActual : 'no definido'
        });
        
        if (this.alarmas.length === 0) {
            const textoNoAlarmas = this.tr('no_alarmas', 'No hay alarmas configuradas');
            console.log('📭 Texto para "no alarmas":', textoNoAlarmas);
            container.innerHTML = `<p>📭 ${textoNoAlarmas}</p>`;
            return;
        }
        
        container.innerHTML = '';
        
        this.alarmas.forEach(alarm => {
            const alarmDiv = this.createAlarmElement(alarm);
            container.appendChild(alarmDiv);
        });
        
        console.log('✅ Lista de alarmas renderizada');
    }
    
    createAlarmElement(alarm) {
        const div = document.createElement('div');
        div.className = `alarm-item ${alarm.habilitada ? 'enabled' : 'disabled'}`;
        
        console.log(`🔘 Creando elemento para alarma ${alarm.id}`);
        console.log('🌍 Estado del sistema de idiomas:', {
            t_disponible: typeof t,
            idiomaActual: typeof idiomaActual !== 'undefined' ? idiomaActual : 'no definido'
        });
        
        const textoHabilitar = this.tr('habilitar', 'Habilitar');
        const textoDeshabilitar = this.tr('deshabilitar', 'Deshabilitar');
        const textoEditar = this.tr('editar', 'Editar');
        const textoEliminar = this.tr('eliminar', 'Eliminar');
        
        console.log('🔤 Textos de botones:', {
            habilitar: textoHabilitar,
            deshabilitar: textoDeshabilitar,
            editar: textoEditar,
            eliminar: textoEliminar
        });
        
        // Traducir día y acción
        const diaTraducido = this.tr(alarm.diaNombre, alarm.diaNombre);
        
        // Normalizar y traducir acción - probar diferentes variantes
        let accionTraducida = this.tr(alarm.accion, alarm.accion);
        
        // Si no se tradujo, probar con variantes comunes
        if (accionTraducida === alarm.accion) {
            const accionLimpia = alarm.accion.trim();
            accionTraducida = this.tr(accionLimpia, accionLimpia);
            
            // Probar con capitalización diferente
            if (accionTraducida === accionLimpia) {
                const accionCapitalizada = accionLimpia.charAt(0).toUpperCase() + accionLimpia.slice(1).toLowerCase();
                accionTraducida = this.tr(accionCapitalizada, accionCapitalizada);
            }
        }
        
        const textoAccion = this.tr('Acción', 'Acción');
        
        console.log(`🎯 Procesando alarma: día="${alarm.diaNombre}" → "${diaTraducido}", acción="${alarm.accion}" → "${accionTraducida}"`);
        console.log(`🔍 Detalles de la acción:`, {
            valor_original: alarm.accion,
            tipo: typeof alarm.accion,
            longitud: alarm.accion?.length,
            codigo_ascii: alarm.accion?.split('').map(c => c.charCodeAt(0)),
            traduccion: accionTraducida,
            idioma_actual: typeof idiomaActual !== 'undefined' ? idiomaActual : 'undefined'
        });
        
        // Si la traducción no cambió, loggear para debugging
        if (accionTraducida === alarm.accion && typeof IDIOMAS !== 'undefined' && idiomaActual === 'ca') {
            console.log(`⚠️ ATENCIÓN: La acción "${alarm.accion}" no se tradujo al catalán.`);
            console.log(`🔍 Claves disponibles que contienen la palabra:`, Object.keys(IDIOMAS.ca).filter(k => k.toLowerCase().includes(alarm.accion.toLowerCase())));
            console.log(`🔍 Todas las claves de acciones:`, Object.keys(IDIOMAS.ca).filter(k => ['Misa', 'Difuntos', 'Fiesta', 'Angelus', 'Cuartos', 'Horas', 'Repique', 'Volteo', 'Toque', 'Manual', 'Campana', 'Calefaccion', 'Auxiliar'].includes(k)));
        }
        
        // Determinar la clave correcta para data-i18n
        let claveAccion = alarm.accion;
        if (accionTraducida !== alarm.accion) {
            // Si se tradujo usando una variante, usar esa clave
            const accionLimpia = alarm.accion.trim();
            const accionCapitalizada = accionLimpia.charAt(0).toUpperCase() + accionLimpia.slice(1).toLowerCase();
            if (this.tr(accionCapitalizada, accionCapitalizada) !== accionCapitalizada) {
                claveAccion = accionCapitalizada;
            } else if (this.tr(accionLimpia, accionLimpia) !== accionLimpia) {
                claveAccion = accionLimpia;
            }
        }
        let textoTipoCompleto = accionTraducida;
        if (alarm.accion === 'CALEFACCION' && alarm.duracion) {
            textoTipoCompleto = `🔥 ${accionTraducida} (${formatearDuracion(alarm.duracion)})`;
        } else if (alarm.accion !== 'CALEFACCION') {
            textoTipoCompleto = `🔔 ${accionTraducida}`;
        }        
        div.innerHTML = `
            <div class="alarm-info">
                <h4>${alarm.nombre}</h4>
                <p><strong><span data-i18n="${alarm.diaNombre}">${diaTraducido}</span></strong> a las <strong>${alarm.horaTexto}</strong></p>
                <p><span data-i18n="Acción">${textoAccion}</span>: <strong>${textoTipoCompleto}</strong></p>
                ${alarm.descripcion ? `<p><em>${alarm.descripcion}</em></p>` : ''}
            </div>
            <div class="alarm-controls">
                <button onclick="alarmManager.toggleAlarm(${alarm.id})" 
                        class="${alarm.habilitada ? 'disable-btn' : 'enable-btn'}"
                        data-alarm-id="${alarm.id}" data-alarm-enabled="${alarm.habilitada}">
                    ${alarm.habilitada ? 
                        `⏸️ <span data-i18n="deshabilitar">${textoDeshabilitar}</span>` : 
                        `▶️ <span data-i18n="habilitar">${textoHabilitar}</span>`}
                </button>
                <button onclick="alarmManager.editAlarm(${alarm.id})" class="edit-btn">
                    ✏️ <span data-i18n="editar">${textoEditar}</span>
                </button>
                <button onclick="alarmManager.deleteAlarm(${alarm.id})" class="delete-btn">
                    🗑️ <span data-i18n="eliminar">${textoEliminar}</span>
                </button>
            </div>
        `;
        
        return div;
    }
    
    setupEventListeners() {
        const form = document.getElementById('alarmForm');
        form.addEventListener('submit', (e) => {
            e.preventDefault();
            this.submitAlarm();
        });
    }
    
    submitAlarm() {
        console.log('📝 submitAlarm() llamada');
        const formData = this.getFormData();
        console.log('📋 Datos del formulario:', formData);
        
        if (!this.validateForm(formData)) {
            console.log('❌ Validación falló');
            return;
        }
        
        const command = this.editingId ? 'EDIT_ALARMA_WEB:' : 'ADD_ALARMA_WEB:';
        if (this.editingId) {
            formData.id = this.editingId;
        }

        console.log('📤 Enviando comando:', command, formData);

        if (this.ws && this.ws.readyState === WebSocket.OPEN) {
            this.ws.send(command + JSON.stringify(formData));
            this.showStatus(`⏳ ${this.tr('procesando', 'Procesando')}...`, "info");
        } else {
            console.log('❌ WebSocket no conectado. Estado:', this.ws?.readyState);
            this.showStatus(`❌ ${this.tr('no_conexion_servidor', 'No hay conexión con el servidor')}`, "error");
        }
    }
    
    getFormData() {
        return {
            nombre: document.getElementById('nombre').value.trim(),
            descripcion: document.getElementById('descripcion').value.trim(),
            dia: parseInt(document.getElementById('dia').value),
            hora: parseInt(document.getElementById('hora').value),
            minuto: parseInt(document.getElementById('minuto').value),
            segundo: 0,
            accion: document.getElementById('accion').value,
            parametro:  0,
            habilitada: true,
            duracion: (() => {
                const duracionSelect = document.getElementById('duracion');
                return duracionSelect ? parseInt(duracionSelect.value) || 0 : 0;
            })()
        };
    }
    
    validateForm(data) {
        if (!data.nombre) {
            this.showStatus(`❌ ${this.tr('nombre_obligatorio', 'El nombre es obligatorio')}`, "error");
            return false;
        }
        
        if (data.hora < 0 || data.hora > 23) {
            this.showStatus(`❌ ${this.tr('hora_entre_0_23', 'La hora debe estar entre 0 y 23')}`, "error");
            return false;
        }
        
        if (data.minuto < 0 || data.minuto > 59) {
            this.showStatus(`❌ ${this.tr('minutos_entre_0_59', 'Los minutos deben estar entre 0 y 59')}`, "error");
            return false;
        }
        
        return true;
    }
    
    toggleAlarm(id) {
        console.log(`🔄 Toggle alarma ID: ${id}`);
        
        const alarm = this.alarmas.find(a => a.id === id);
        if (!alarm) return;
        
        if (this.ws && this.ws.readyState === WebSocket.OPEN) {
            // ✅ ENVIAR JSON VÁLIDO
            const toggleData = JSON.stringify({ 
                id: id, 
                habilitada: !alarm.habilitada 
            });
            const command = `TOGGLE_ALARMA_WEB:${toggleData}`;
            
            console.log(`📤 Enviando comando: ${command}`);
            this.ws.send(command);
            this.showStatus(`⏳ ${this.tr('cambiando_estado', 'Cambiando estado')}...`, "info");
        } else {
            console.error("❌ WebSocket no conectado");
            this.showStatus(`❌ ${this.tr('no_conexion_servidor', 'No hay conexión con el servidor')}`, "error");
        }
    }
    
    editAlarm(id) {
        const alarm = this.alarmas.find(a => a.id === id);
        if (!alarm) return;
        
        this.editingId = id;
        
        // ✅ USAR setTimeout PARA EVITAR QUE SE LIMPIE AUTOMÁTICAMENTE
        setTimeout(() => {
            // Llenar formulario
            document.getElementById('nombre').value = alarm.nombre;
            document.getElementById('descripcion').value = alarm.descripcion || '';
            document.getElementById('dia').value = alarm.dia;
            document.getElementById('hora').value = alarm.hora;
            document.getElementById('minuto').value = alarm.minuto;
            document.getElementById('accion').value = alarm.accion;
            
            const duracionSelect = document.getElementById('duracion');
            if (duracionSelect && alarm.duracion) {
                duracionSelect.value = alarm.duracion;
            }
            mostrarDuracionSiEsCalefaccion(); // Mostrar campo duración si es calefacción
            
            // Cambiar texto del botón
            const submitBtn = document.querySelector('#alarmForm button[type="submit"]');
            if (submitBtn) {
                submitBtn.innerHTML = `✏️ ${this.tr('modificar_alarma', 'Modificar Alarma')}`;
                submitBtn.style.backgroundColor = '#ff9800';
            }
            
            // Scroll al formulario
            document.querySelector('.add-alarm-form').scrollIntoView({ behavior: 'smooth' });
            
            this.showStatus(`✏️ ${this.tr('editando_alarma', 'Editando alarma')}: ${alarm.nombre}`, "info");
        }, 100);
    }
    
    deleteAlarm(id) {
        const alarm = this.alarmas.find(a => a.id === id);
        if (!alarm) {
            console.warn(`⚠️ Alarma con ID ${id} no encontrada`);
            return;
        }
        
        const confirmed = confirm(`${this.tr('confirmar_eliminar_alarma', '¿Estás seguro de eliminar la alarma')} "${alarm.nombre}"?\n\n${this.tr('accion_no_deshacer', 'Esta acción no se puede deshacer')}.`);
        
        if (confirmed) {
            console.log(`🗑️ Eliminando alarma ID: ${id}`);
            
            if (this.ws && this.ws.readyState === WebSocket.OPEN) {
                // ✅ ENVIAR JSON VÁLIDO EN LUGAR DE SOLO EL ID
                const deleteData = JSON.stringify({ id: id });
                const command = `DELETE_ALARMA_WEB:${deleteData}`;
                
                console.log(`📤 Enviando comando: ${command}`);
                this.ws.send(command);
                this.showStatus(`⏳ ${this.tr('eliminando_alarma', 'Eliminando alarma')}...`, "info");
                
            } else {
                console.error("❌ WebSocket no conectado. Estado:", this.ws?.readyState);
                this.showStatus(`❌ ${this.tr('no_conexion_servidor', 'No hay conexión con el servidor')}`, "error");
            }
        } else {
            console.log(`❌ ${this.tr('eliminacion_cancelada', 'Eliminación cancelada por el usuario')}`);
        }
    }
    
    limpiarFormulario() {
        console.log("🧹 Limpiando formulario...");
        
        // ✅ OBTENER FORMULARIO DE FORMA SEGURA
        const form = document.getElementById('alarmForm');
        if (!form) {
            console.warn("⚠️ Formulario no encontrado");
            return;
        }
        
        // ✅ SOLO LIMPIAR CUANDO NO ESTAMOS EDITANDO
        if (this.editingId === null) {
            form.reset();
            mostrarDuracionSiEsCalefaccion(); // Ocultar campo duración al resetear
            console.log("✅ Formulario reseteado");
        }
        
        // ✅ RESTAURAR BOTÓN A ESTADO ORIGINAL
        const submitBtn = form.querySelector('button[type="submit"]');
        if (submitBtn && this.editingId === null) {
            submitBtn.innerHTML = `🔔 <span>${this.tr('crear', 'Crear')}</span> <span>${this.tr('nueva_alarma', 'Alarma')}</span>`;
            submitBtn.style.backgroundColor = '#4CAF50';
            console.log("✅ Botón restaurado");
        }
        
        // ✅ LIMPIAR ID DE EDICIÓN SOLO CUANDO CORRESPONDE
        if (this.editingId !== null) {
            console.log(`🔄 Finalizando edición de alarma ${this.editingId}`);
            this.editingId = null;
            
            // Ahora sí limpiar formulario
            setTimeout(() => {
                form.reset();
                mostrarDuracionSiEsCalefaccion(); // Ocultar campo duración al limpiar tras edición
                
                const submitBtn = form.querySelector('button[type="submit"]');
                if (submitBtn) {
                    submitBtn.innerHTML = `🔔 <span>${this.tr('crear', 'Crear')}</span> <span>${this.tr('nueva_alarma', 'Alarma')}</span>`;
                    submitBtn.style.backgroundColor = '#4CAF50';
                }
                
                console.log("✅ Formulario limpiado tras edición");
            }, 100);
        }
    }
    
    /**
     * Actualiza los botones de las alarmas con el idioma actual
     */
    actualizarBotonesAlarmas() {
        console.log("🔘 Actualizando botones de alarmas...");
        
        // Buscar todos los botones de toggle (habilitar/deshabilitar)
        document.querySelectorAll('button[data-alarm-id]').forEach(btn => {
            const alarmId = btn.getAttribute('data-alarm-id');
            const isEnabled = btn.getAttribute('data-alarm-enabled') === 'true';
            
            if (isEnabled) {
                btn.innerHTML = `⏸️ <span data-i18n="deshabilitar">${this.tr('deshabilitar', 'Deshabilitar')}</span>`;
            } else {
                btn.innerHTML = `▶️ <span data-i18n="habilitar">${this.tr('habilitar', 'Habilitar')}</span>`;
            }
        });
        
        // Actualizar botones de editar
        document.querySelectorAll('.edit-btn span[data-i18n="editar"]').forEach(span => {
            span.textContent = this.tr('editar', 'Editar');
        });
        
        // Actualizar botones de eliminar
        document.querySelectorAll('.delete-btn span[data-i18n="eliminar"]').forEach(span => {
            span.textContent = this.tr('eliminar', 'Eliminar');
        });
        
        console.log("✅ Botones de alarmas actualizados");
    }

    /**
     * Actualiza todos los textos dinámicos cuando cambia el idioma
     */
    actualizarTextosIdioma() {
        console.log("🌍 Actualizando textos de Alarmas.js al idioma:", typeof idiomaActual !== 'undefined' ? idiomaActual : 'no definido');
        console.log("🔍 Estado completo del sistema:", {
            t_function: typeof t,
            idiomaActual: typeof idiomaActual !== 'undefined' ? idiomaActual : 'undefined',
            IDIOMAS_object: typeof IDIOMAS !== 'undefined',
            alarmas_length: this.alarmas ? this.alarmas.length : 'undefined'
        });
        
        // Esperar un poco para asegurar que el cambio de idioma se ha procesado
        setTimeout(() => {
            // Re-renderizar estadísticas
            const statsPanel = document.getElementById('statsPanel');
            if (statsPanel) {
                console.log("🔄 Forzando actualización del panel de estadísticas");
                this.renderStatsBasico();
            }
            
            // Re-renderizar lista de alarmas
            if (this.alarmas !== undefined) {
                console.log("🔄 Forzando actualización de la lista de alarmas");
                this.renderAlarms();
            }
            
            // Actualizar botones de alarmas específicamente
            this.actualizarBotonesAlarmas();
            
            // Actualizar días y acciones de alarmas
            this.actualizarDiasYAccionesAlarmas();
            
            // Actualizar botón del formulario
            const submitBtn = document.querySelector('#alarmForm button[type="submit"]');
            if (submitBtn) {
                if (this.editingId !== null) {
                    submitBtn.innerHTML = `✏️ ${this.tr('modificar_alarma', 'Modificar Alarma')}`;
                } else {
                    submitBtn.innerHTML = `🔔 <span>${this.tr('crear', 'Crear')}</span> <span>${this.tr('nueva_alarma', 'Alarma')}</span>`;
                }
                console.log("🔄 Botón de formulario actualizado");
            }
            
            console.log("✅ Textos de Alarmas.js actualizados completamente");
        }, 100);
    }

    actualizarDiasYAccionesAlarmas() {
        console.log("🌍 Actualizando días y acciones de alarmas...");
        
        // Actualizar elementos con data-i18n que sean días o acciones
        document.querySelectorAll('[data-i18n]').forEach(element => {
            const key = element.getAttribute('data-i18n');
            
            // Solo actualizar si es un día de la semana o una acción
            const isDia = ['Lunes', 'Martes', 'Miércoles', 'Jueves', 'Viernes', 'Sábado', 'Domingo', 'Todos los días'].includes(key);
            const isAccion = ['Encender', 'Apagar', 'Subir', 'Bajar', 'Activar', 'Desactivar', 'Acción'].includes(key);
            const valoresAccion = ['Misa', 'Difuntos', 'Fiesta', 'Angelus', 'Cuartos', 'Horas', 'Repique', 'Volteo', 'Toque', 'Manual', 'Campana', 'Calefaccion', 'Auxiliar', 'misa', 'difuntos', 'fiesta', 'angelus', 'cuartos', 'horas', 'repique', 'volteo', 'toque', 'manual', 'campana', 'calefaccion', 'auxiliar'];
            const isValorAccion = valoresAccion.includes(key);
            
            if (isDia || isAccion || isValorAccion) {
                const traduccion = this.tr(key, key);
                element.textContent = traduccion;
                console.log(`🔄 Actualizado "${key}" → "${traduccion}"`);
            }
        });
    }

    showStatus(message, type) {
        const container = document.getElementById('statusMessages');
        const div = document.createElement('div');
        div.className = `status ${type}`;
        div.textContent = message;
        
        container.appendChild(div);
        
        // Auto-remove after 5 seconds
        setTimeout(() => {
            if (div.parentNode) {
                div.parentNode.removeChild(div);
            }
        }, 5000);
        
        // Keep only last 3 messages
        while (container.children.length > 3) {
            container.removeChild(container.firstChild);
        }
    }
}

// Función global para limpiar formulario
function limpiarFormulario() {
    alarmManager.limpiarFormulario();
}

function actualizarAlarmasManual() {
    console.log("🔄 actualizarAlarmasManual() llamada");
    
    if (window.alarmManager) {
        console.log("📡 Solicitando datos al servidor...");
        window.alarmManager.requestData();
        
        // ✅ SELECTOR MEJORADO PARA ENCONTRAR EL BOTÓN
        const botonActualizar = document.querySelector('a[onclick*="actualizarAlarmasManual"]') ||
                               document.querySelector('.btn-actualizar') ||
                               document.querySelector('.navigation a:last-child');
        
        if (botonActualizar) {
            console.log("✅ Botón encontrado:", botonActualizar);
            
            const textoOriginal = botonActualizar.innerHTML;
            
            // ✅ AÑADIR ANIMACIÓN CSS
            const textoActualizando = (typeof t === 'function') ? t('actualizando') : 'Actualizando';
            botonActualizar.innerHTML = `<span style="display: inline-block; animation: rotar 1s linear infinite;">🔄</span> ${textoActualizando}...`;
            botonActualizar.style.pointerEvents = 'none';
            botonActualizar.style.opacity = '0.7';
            
            setTimeout(() => {
                botonActualizar.innerHTML = textoOriginal;
                botonActualizar.style.pointerEvents = 'auto';
                botonActualizar.style.opacity = '1';
            }, 2000);
        } else {
            console.warn("⚠️ Botón actualizar no encontrado");
            console.log("🔍 Todos los enlaces:", document.querySelectorAll('.navigation a'));
        }
    } else {
        console.error("❌ alarmManager no disponible");
    }
}

// Esperar a que el DOM esté listo antes de inicializar
document.addEventListener('DOMContentLoaded', function() {
    console.log('📄 DOM cargado, inicializando AlarmManager...');
    
    // Esperar más tiempo para asegurar que idiomas.js se ha cargado completamente
    setTimeout(() => {
        console.log('🔍 Verificando disponibilidad del sistema de idiomas antes de inicializar...');
        console.log('Estado:', {
            t_function: typeof t,
            idiomaActual: typeof idiomaActual !== 'undefined' ? idiomaActual : 'undefined',
            IDIOMAS_object: typeof IDIOMAS !== 'undefined'
        });
        
        const alarmManager = new AlarmManager();
        window.alarmManager = alarmManager;
        console.log('✅ AlarmManager inicializado y disponible globalmente');
        const accionSelect = document.getElementById('accion');
        if (accionSelect) {
            accionSelect.addEventListener('change', mostrarDuracionSiEsCalefaccion);
        }        
    }, 500); // Aumentado de 200ms a 500ms
});

// Fallback para navegadores que ya tienen el DOM cargado
if (document.readyState !== 'loading') {
    setTimeout(() => {
        console.log('🔍 Verificando disponibilidad del sistema de idiomas (fallback)...');
        console.log('Estado:', {
            t_function: typeof t,
            idiomaActual: typeof idiomaActual !== 'undefined' ? idiomaActual : 'undefined',
            IDIOMAS_object: typeof IDIOMAS !== 'undefined'
        });
        
        const alarmManager = new AlarmManager();
        window.alarmManager = alarmManager;
        console.log('✅ AlarmManager inicializado (fallback)');
    }, 500); // Aumentado de 200ms a 500ms
} 

/**
 * Mostrar/ocultar campo duración según la acción seleccionada
 */
function mostrarDuracionSiEsCalefaccion() {
    const accionSelect = document.getElementById('accion');
    const duracionSelect = document.getElementById('duracion');
    const btnCrear = document.querySelector('button[type="submit"]');
    
    if (accionSelect && accionSelect.value === 'CALEFACCION') {
        // Mostrar selector de duración
        if (duracionSelect) {
            duracionSelect.style.display = 'inline-block';
            duracionSelect.required = true;
        }
        
        // Cambiar texto del botón
        if (btnCrear) {
            btnCrear.innerHTML = '🔥 <span data-i18n="crear">Crear</span> <span data-i18n="alarma_calefaccion">Alarma de Calefacción</span>';
        }
    } else {
        // Ocultar selector de duración
        if (duracionSelect) {
            duracionSelect.style.display = 'none';
            duracionSelect.required = false;
        }
        
        // Restaurar texto del botón
        if (btnCrear) {
            btnCrear.innerHTML = '🔔 <span data-i18n="crear">Crear</span> <span data-i18n="nueva_alarma">Alarma</span>';
        }
    }
}

/**
 * Formatear duración en texto legible
 */
function formatearDuracion(minutos) {
    if (!minutos) return '0 min';
    
    if (minutos < 60) {
        return `${minutos} min`;
    } else if (minutos === 60) {
        return '1h';
    } else if (minutos % 60 === 0) {
        return `${minutos / 60}h`;
    } else {
        const horas = Math.floor(minutos / 60);
        const mins = minutos % 60;
        return `${horas}h ${mins}m`;
    }
}