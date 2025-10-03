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
        if (this.ws && this.ws.readyState === WebSocket.OPEN) {
            this.ws.send('GET_ALARMAS_WEB');
            this.ws.send('GET_STATS_ALARMAS_WEB');
        }
    }
    
    handleMessage(message) {
        console.log('Received:', message);
        
        if (message.startsWith('ALARMAS_WEB:')) {
            const jsonData = message.substring(12);
            try {
                const data = JSON.parse(jsonData);
                this.alarmas = data.alarmas || [];
                this.renderAlarms();
            } catch (e) {
                console.error('Error parsing alarm data:', e);
                this.showStatus("❌ Error procesando datos de alarmas", "error");
            }
        }
        else if (message.startsWith('STATS_ALARMAS_WEB:')) {
            const jsonData = message.substring(19);
            try {
                const stats = JSON.parse(jsonData);
                this.renderStats(stats);
            } catch (e) {
                console.error('Error parsing stats:', e);
            }
        }
        else if (message.startsWith('ALARMA_CREADA:')) {
            const id = message.substring(14);
            this.showStatus(`✅ Alarma creada con ID: ${id}`, "success");
            this.limpiarFormulario();
        }
        else if (message.startsWith('ALARMA_MODIFICADA:')) {
            const id = message.substring(18);
            this.showStatus(`✅ Alarma ${id} modificada correctamente`, "success");
            this.editingId = null;
            this.limpiarFormulario();
        }
        else if (message.startsWith('ALARMA_ELIMINADA:')) {
            const id = message.substring(17);
            this.showStatus(`🗑️ Alarma ${id} eliminada`, "success");
        }
        else if (message.startsWith('ALARMA_TOGGLE:')) {
            const parts = message.split(':');
            const id = parts[1];
            const state = parts[2];
            this.showStatus(`🔄 Alarma ${id} ${state === 'ON' ? 'habilitada' : 'deshabilitada'}`, "success");
        }
        else if (message.startsWith('ERROR_ALARMA:')) {
            const error = message.substring(13);
            this.showStatus(`❌ Error: ${error}`, "error");
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
        if (this.ws && this.ws.readyState === WebSocket.OPEN) {
            this.ws.send(`TOGGLE_ALARMA_WEB:${id}`);
        }
    }
    
    editAlarm(id) {
        const alarm = this.alarmas.find(a => a.id === id);
        if (!alarm) return;
        
        this.editingId = id;
        
        // Llenar formulario
        document.getElementById('nombre').value = alarm.nombre;
        document.getElementById('descripcion').value = alarm.descripcion || '';
        document.getElementById('dia').value = alarm.dia;
        document.getElementById('hora').value = alarm.hora;
        document.getElementById('minuto').value = alarm.minuto;
        document.getElementById('accion').value = alarm.accion;
        
        // Scroll al formulario
        document.querySelector('.add-alarm-form').scrollIntoView({ behavior: 'smooth' });
        
        this.showStatus(`✏️ Editando alarma: ${alarm.nombre}`, "info");
    }
    
    deleteAlarm(id) {
        const alarm = this.alarmas.find(a => a.id === id);
        if (!alarm) return;
        
        if (confirm(`¿Estás seguro de eliminar la alarma "${alarm.nombre}"?`)) {
            if (this.ws && this.ws.readyState === WebSocket.OPEN) {
                this.ws.send(`DELETE_ALARMA_WEB:${id}`);
            }
        }
    }
    
    limpiarFormulario() {
        document.getElementById('alarmForm').reset();
        document.getElementById('segundo').value = 0;
        document.getElementById('parametro').value = 0;
        this.editingId = null;
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

// Inicializar cuando se carga la página
const alarmManager = new AlarmManager();