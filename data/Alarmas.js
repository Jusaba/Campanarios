/**
 * Gestor de Alarmas Personalizadas para Campanarios ESP32
 * Autor: Julian Salas Bartolomé
 * Fecha: 02/01/2025
 */

console.log('🔔 Alarmas.js cargado correctamente');



class AlarmManager {
    constructor() {
        this.ws = null;
        this.reconnectTimeout = null;
        this.alarmas = [];
        this.editingId = null;
        
        this.init();
    }
    
    init() {
        this.setupWebSocket();
        this.setupEventListeners();
        this.showStatus("Conectando al servidor...", "info");
    }
    
    setupWebSocket() {
        const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
        this.ws = new WebSocket(`${protocol}//${window.location.host}/ws`);
        
        this.ws.onopen = () => {
            this.showStatus("🔗 Conectado al servidor", "success");
            this.requestData();
        };
        
        this.ws.onmessage = (event) => {
            this.handleMessage(event.data);
        };
        
        this.ws.onclose = () => {
            this.showStatus("❌ Conexión perdida. Reintentando...", "error");
            this.scheduleReconnect();
        };
        
        this.ws.onerror = (error) => {
            console.error('WebSocket error:', error);
            this.showStatus("❌ Error de conexión", "error");
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
            
            this.showStatus("🔄 Solicitando datos...", "info");
        } else {
            console.error("❌ WebSocket no disponible. Estado:", this.ws?.readyState);
            this.showStatus("❌ No hay conexión con el servidor", "error");
            
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
                this.renderAlarms();
                this.showStatus(`✅ ${this.alarmas.length} alarmas cargadas`, "success");
            } catch (e) {
                console.error('❌ Error parsing alarm data:', e);
                this.showStatus("❌ Error procesando datos de alarmas", "error");
            }
        }
        else if (message.startsWith('STATS_ALARMAS_WEB:')) {
            console.log("📊 Procesando STATS_ALARMAS_WEB");
            const jsonData = message.substring(18);
            try {
                const stats = JSON.parse(jsonData);
                console.log("✅ Stats procesado:", stats);
                this.renderStats({
                    totalAlarmas: stats.totalAlarmas,
                    habilitadas: stats.habilitadas,
                    deshabilitadas: stats.deshabilitadas,
                    espacioLibre: stats.espacioLibre
                });
            } catch (e) {
                console.error('❌ Error parsing stats:', e);
                this.renderStatsBasico();
            }
        }
        // ✅ CORREGIR: Usar el formato correcto que envía el servidor
        else if (message.startsWith('ALARMA_CREADA_WEB:')) {
            const id = message.substring(18); // 18 caracteres en "ALARMA_CREADA_WEB:"
            console.log(`✅ ALARMA_CREADA_WEB recibido: ${id}`);
            this.showStatus(`✅ Alarma creada con ID: ${id}`, "success");
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
            this.showStatus(`✅ Alarma ${id} modificada correctamente`, "success");
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
            this.showStatus(`🗑️ Alarma ${id} eliminada`, "success");
            
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
            this.showStatus(`🔄 Alarma ${id} ${state === 'ON' ? 'habilitada' : 'deshabilitada'}`, "success");
            
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
        else {
            console.log("🤷 Mensaje no reconocido:", message);
        }
    }
    
    renderStats(stats) {
        const statsPanel = document.getElementById('statsPanel');
        statsPanel.innerHTML = `
            <strong>📊 Estadísticas del Sistema</strong><br>
            Total: ${stats.totalAlarmas} | Habilitadas: ${stats.habilitadas} | 
            Deshabilitadas: ${stats.deshabilitadas} | Espacio libre: ${stats.espacioLibre}
        `;
    }
renderStatsBasico() {
        const habilitadas = this.alarmas.filter(a => a.habilitada).length;
        const deshabilitadas = this.alarmas.length - habilitadas;
        
        const statsPanel = document.getElementById('statsPanel');
        if (statsPanel) {
            statsPanel.innerHTML = `
                <strong>📊 Estadísticas del Sistema</strong><br>
                Total: ${this.alarmas.length} | 
                Habilitadas: ${habilitadas} | 
                Deshabilitadas: ${deshabilitadas} | 
                Espacio libre: Calculando...
            `;
        }
    }
    
    renderAlarms() {
        const container = document.getElementById('alarmList');
        
        if (this.alarmas.length === 0) {
            container.innerHTML = '<p>📭 No hay alarmas configuradas</p>';
            return;
        }
        
        container.innerHTML = '';
        
        this.alarmas.forEach(alarm => {
            const alarmDiv = this.createAlarmElement(alarm);
            container.appendChild(alarmDiv);
        });
    }
    
    createAlarmElement(alarm) {
        const div = document.createElement('div');
        div.className = `alarm-item ${alarm.habilitada ? 'enabled' : 'disabled'}`;
        
        div.innerHTML = `
            <div class="alarm-info">
                <h4>${alarm.nombre}</h4>
                <p><strong>${alarm.diaNombre}</strong> a las <strong>${alarm.horaTexto}</strong></p>
                <p>Acción: <strong>${alarm.accion}</strong></p>
                ${alarm.descripcion ? `<p><em>${alarm.descripcion}</em></p>` : ''}
            </div>
            <div class="alarm-controls">
                <button onclick="alarmManager.toggleAlarm(${alarm.id})" 
                        class="${alarm.habilitada ? 'disable-btn' : 'enable-btn'}">
                    ${alarm.habilitada ? '⏸️ Deshabilitar' : '▶️ Habilitar'}
                </button>
                <button onclick="alarmManager.editAlarm(${alarm.id})" class="edit-btn">
                    ✏️ Editar
                </button>
                <button onclick="alarmManager.deleteAlarm(${alarm.id})" class="delete-btn">
                    🗑️ Eliminar
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
            this.showStatus("⏳ Procesando...", "info");
        } else {
            console.log('❌ WebSocket no conectado. Estado:', this.ws?.readyState);
            this.showStatus("❌ No hay conexión con el servidor", "error");
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
            habilitada: true
        };
    }
    
    validateForm(data) {
        if (!data.nombre) {
            this.showStatus("❌ El nombre es obligatorio", "error");
            return false;
        }
        
        if (data.hora < 0 || data.hora > 23) {
            this.showStatus("❌ La hora debe estar entre 0 y 23", "error");
            return false;
        }
        
        if (data.minuto < 0 || data.minuto > 59) {
            this.showStatus("❌ Los minutos deben estar entre 0 y 59", "error");
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
            this.showStatus("⏳ Cambiando estado...", "info");
        } else {
            console.error("❌ WebSocket no conectado");
            this.showStatus("❌ No hay conexión con el servidor", "error");
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
            
            // Cambiar texto del botón
            const submitBtn = document.querySelector('#alarmForm button[type="submit"]');
            if (submitBtn) {
                submitBtn.textContent = '✏️ Modificar Alarma';
                submitBtn.style.backgroundColor = '#ff9800';
            }
            
            // Scroll al formulario
            document.querySelector('.add-alarm-form').scrollIntoView({ behavior: 'smooth' });
            
            this.showStatus(`✏️ Editando alarma: ${alarm.nombre}`, "info");
        }, 100);
    }
    
    deleteAlarm(id) {
        const alarm = this.alarmas.find(a => a.id === id);
        if (!alarm) {
            console.warn(`⚠️ Alarma con ID ${id} no encontrada`);
            return;
        }
        
        const confirmed = confirm(`¿Estás seguro de eliminar la alarma "${alarm.nombre}"?\n\nEsta acción no se puede deshacer.`);
        
        if (confirmed) {
            console.log(`🗑️ Eliminando alarma ID: ${id}`);
            
            if (this.ws && this.ws.readyState === WebSocket.OPEN) {
                // ✅ ENVIAR JSON VÁLIDO EN LUGAR DE SOLO EL ID
                const deleteData = JSON.stringify({ id: id });
                const command = `DELETE_ALARMA_WEB:${deleteData}`;
                
                console.log(`📤 Enviando comando: ${command}`);
                this.ws.send(command);
                this.showStatus("⏳ Eliminando alarma...", "info");
                
            } else {
                console.error("❌ WebSocket no conectado. Estado:", this.ws?.readyState);
                this.showStatus("❌ No hay conexión con el servidor", "error");
            }
        } else {
            console.log("❌ Eliminación cancelada por el usuario");
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
            console.log("✅ Formulario reseteado");
        }
        
        // ✅ RESTAURAR BOTÓN A ESTADO ORIGINAL
        const submitBtn = form.querySelector('button[type="submit"]');
        if (submitBtn && this.editingId === null) {
            submitBtn.textContent = '➕ Crear Alarma';
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
                
                const submitBtn = form.querySelector('button[type="submit"]');
                if (submitBtn) {
                    submitBtn.textContent = '➕ Crear Alarma';
                    submitBtn.style.backgroundColor = '#4CAF50';
                }
                
                console.log("✅ Formulario limpiado tras edición");
            }, 100);
        }
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
            botonActualizar.innerHTML = '<span style="display: inline-block; animation: rotar 1s linear infinite;">🔄</span> Actualizando...';
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

// Inicializar cuando se carga la página
const alarmManager = new AlarmManager();
window.alarmManager = alarmManager; 