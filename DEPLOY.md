# Deploy Automatizado - Campanarios

Este directorio contiene las herramientas para automatizar el proceso de compilación y despliegue de actualizaciones a múltiples dispositivos ESP32.

## 📋 Componentes

### 1. `deploy-devices.ps1`
Script PowerShell para desplegar actualizaciones a múltiples dispositivos.

**Características:**
- ✅ Backup automático de configuraciones SPIFFS
- ✅ Actualización OTA de firmware y SPIFFS
- ✅ Restauración automática de configuraciones
- ✅ Soporte para múltiples dispositivos en paralelo
- ✅ Logging detallado con colores
- ✅ Manejo de errores y reintentos

### 2. `Dispositivos.txt`
Archivo de configuración con la lista de dispositivos a actualizar.

**Formato:**
```
nombre_dispositivo,dominio:puerto
```

**Ejemplo:**
```
Campanario-Principal,campanario-principal.jusaba.es:8080
Campanario-Auxiliar,campanario-auxiliar.jusaba.es:8080
Campanario-Torre-Norte,campanario-torre-norte.jusaba.es:8080
```

### 3. `.github/workflows/build-and-deploy.yml`
Workflow de GitHub Actions para automatizar todo el proceso.

## 🚀 Uso Local

### Configuración Inicial

1. **Editar `Dispositivos.txt`:**
   ```
   nano Dispositivos.txt
   ```
   
   Agrega tus dispositivos con el formato:
   ```
   MiCampanario,micampanario.jusaba.es:8080
   OtroCampanario,otrocampanario.jusaba.es:8080
   ```

2. **Compilar la release (si no existe):**
   ```powershell
   .\build-release.ps1 -version "1.1.4"
   ```

### Comandos Disponibles

**1. Solo hacer backup de configuraciones:**
```powershell
.\deploy-devices.ps1 -version "1.1.4" -onlyBackup
```
- Descarga todas las configuraciones SPIFFS de cada dispositivo
- Las guarda en `device-backups\[nombre_dispositivo]\`
- No actualiza firmware

**2. Actualizar todos los dispositivos:**
```powershell
.\deploy-devices.ps1 -version "1.1.4"
```
- Hace backup de configuraciones
- Actualiza firmware y SPIFFS a la versión especificada
- Restaura las configuraciones personalizadas

**3. Solo restaurar configuraciones:**
```powershell
.\deploy-devices.ps1 -version "1.1.4" -onlyRestore
```
- Restaura las configuraciones desde backups existentes
- Útil si algo salió mal durante el despliegue

## 🤖 Uso con GitHub Actions

### Opción 1: Push de Tag (Automático)

```bash
# Compilar localmente
.\build-release.ps1 -version "1.1.5"

# Crear tag y push
git tag v1.1.5
git push origin v1.1.5
```

Esto automáticamente:
1. ✅ Compila el firmware en GitHub Actions
2. ✅ Genera binarios de firmware, SPIFFS y completo
3. ✅ Crea checksums SHA256
4. ✅ Crea una release en GitHub con los binarios
5. ❌ **NO despliega** (por seguridad)

### Opción 2: Workflow Manual (con Deploy Opcional)

1. Ve a **Actions** en GitHub
2. Selecciona **Build and Deploy Campanarios**
3. Click en **Run workflow**
4. Completa:
   - **version**: `1.1.5`
   - **deploy**: `true` (si quieres desplegar automáticamente)

Esto hace todo lo anterior **Y** puede desplegar si marcas `deploy: true`.

### Requisitos para Deploy Automático

Para que el workflow pueda desplegar a tus dispositivos, necesitas:

1. **Self-hosted Runner** en tu red local:
   ```bash
   # En Windows
   mkdir actions-runner; cd actions-runner
   
   # Descargar runner (sigue instrucciones de GitHub)
   # Settings > Actions > Runners > New self-hosted runner
   ```

2. **Configurar runner** con acceso a tu red local donde están los ESP32

3. **El archivo `Dispositivos.txt`** debe estar en el repositorio

## 📁 Estructura de Backups

```
device-backups/
├── Campanario-Principal/
│   ├── alarmas_personalizadas.json
│   ├── telegram_config.json
│   ├── config.json
│   └── Secuencias.json
├── Campanario-Auxiliar/
│   ├── alarmas_personalizadas.json
│   └── config.json
└── ...
```

Cada dispositivo tiene su propio directorio con sus configuraciones personalizadas.

## 🔧 Archivos de Configuración Respaldados

El script hace backup/restore de:
- ✅ `alarmas_personalizadas.json` - Alarmas configuradas
- ✅ `telegram_config.json` - Configuración de Telegram
- ✅ `config.json` - Configuración general (WiFi, etc.)
- ✅ `Secuencias.json` - Secuencias de campanas personalizadas

## 🎯 Flujo Completo de Actualización

### Escenario: Actualizar 10 dispositivos a v1.1.5

```powershell
# 1. Compilar nueva versión
.\build-release.ps1 -version "1.1.5" -uploadToGitHub

# 2. Verificar que Dispositivos.txt está actualizado
cat Dispositivos.txt

# 3. Hacer backup preventivo
.\deploy-devices.ps1 -version "1.1.5" -onlyBackup

# 4. Verificar backups
ls device-backups

# 5. Desplegar a todos los dispositivos
.\deploy-devices.ps1 -version "1.1.5"

# 6. Si algo falla, restaurar manualmente:
.\deploy-devices.ps1 -version "1.1.5" -onlyRestore
```

## ⚙️ Configuración Avanzada

### Cambiar archivos a respaldar

Edita `deploy-devices.ps1`, línea ~30:
```powershell
$CONFIG_FILES = @(
    "alarmas_personalizadas.json",
    "telegram_config.json",
    "config.json",
    "Secuencias.json",
    "tu_archivo_personalizado.json"  # <-- Agregar aquí
)
```

### Cambiar tipo de actualización

Por defecto se usa `"complete"` (firmware + SPIFFS). Puedes cambiarlo en línea ~370:
```powershell
Update-DeviceFirmware ... -updateType "firmware"  # Solo firmware
Update-DeviceFirmware ... -updateType "spiffs"    # Solo SPIFFS
Update-DeviceFirmware ... -updateType "complete"  # Ambos (default)
```

### Ajustar timeouts

Edita en `deploy-devices.ps1`:
```powershell
# Timeout de conexión inicial (línea ~150)
Invoke-WebRequest ... -TimeoutSec 5

# Tiempo de espera después de reinicio (línea ~240)
Start-Sleep -Seconds 30

# Reintentos para verificar online (línea ~245)
$maxRetries = 10
```

## 📊 Salida del Script

El script muestra información detallada con colores:

- 🔵 **[*]** - Paso en progreso
- 🟢 **[OK]** - Operación exitosa
- 🔴 **[ERROR]** - Error crítico
- 🟡 **[INFO]** - Información adicional
- 🟣 **[DEVICE]** - Operación en dispositivo específico

**Ejemplo de salida:**
```
========================================
   CAMPANARIOS - DEPLOY TO DEVICES
   Version: 1.1.4
========================================

[*] Leyendo dispositivos desde Dispositivos.txt...
[INFO]   - Campanario-Principal @ 192.168.1.100
[INFO]   - Campanario-Auxiliar @ 192.168.1.101
[OK] Encontrados 2 dispositivos

========================================
 Dispositivo 1/2 : Campanario-Principal
========================================
[DEVICE] Haciendo backup de Campanario-Principal (192.168.1.100)...
[INFO]   Verificando conexión...
[OK]   Dispositivo accesible
[INFO]   Descargando config.json...
[OK]     ✓ config.json (1024 bytes)
...
```

## 🐛 Troubleshooting

### Error: "No se puede conectar a Host"
- Verifica que el dispositivo está encendido y accesible
- Verifica la resolución DNS: `nslookup campanario.jusaba.es`
- Verifica conectividad: `Test-NetConnection campanario.jusaba.es -Port 8080`
- Intenta en navegador: `http://campanario.jusaba.es:8080`

### Error: "Archivo no encontrado en release"
- Ejecuta primero: `.\build-release.ps1 -version "X.X.X"`
- Verifica que existe `releases\vX.X.X\`

### Error: "Dispositivo no responde después del reinicio"
- Aumenta timeout en línea ~240: `Start-Sleep -Seconds 60`
- Aumenta reintentos en línea ~245: `$maxRetries = 20`

### Backup incompleto
- Algunos archivos pueden no existir en todos los dispositivos (normal)
- El script continúa con los archivos que existen

## 📝 Logs y Debugging

Para debugging detallado, activa verbose:
```powershell
$VerbosePreference = "Continue"
.\deploy-devices.ps1 -version "1.1.4" -Verbose
```

## 🔐 Seguridad

**Importante:**
- ❌ **NO** subas `Dispositivos.txt` al repositorio si contiene IPs públicas
- ✅ Agrega a `.gitignore` si contiene información sensible
- ✅ Usa el archivo solo localmente o en self-hosted runner privado

## 📖 Más Información

- [Documentación OTA ESP32](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/ota.html)
- [GitHub Actions Self-hosted Runners](https://docs.github.com/en/actions/hosting-your-own-runners)
- [AsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)

---

**Autor**: Campanarios Project  
**Licencia**: Ver LICENSE en el repositorio principal
