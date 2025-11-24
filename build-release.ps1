
<#
.SYNOPSIS
    Script para compilar y generar release de Campanarios para OTA
    
.DESCRIPTION
    Compila el proyecto ESP32, genera firmware.bin y spiffs.bin,
    y los prepara para subir a GitHub Releases
    
.PARAMETER version
    Versión del release (ej: "1.0.0")
    
.PARAMETER uploadToGitHub
    Si se especifica, crea automáticamente el release en GitHub
    
.EXAMPLE
    .\build-release.ps1 -version "1.0.0"
    .\build-release.ps1 -version "1.0.1" -uploadToGitHub
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$version,
    
    [Parameter(Mandatory=$false)]
    [switch]$uploadToGitHub
)

# Configuración
$BOARD = "esp32:esp32:esp32"
$PARTITION_SCHEME = "default"
$UPLOAD_SPEED = "115200"
$PROJECT_NAME = "Campanarios"
$BUILD_DIR = "build\esp32.esp32.esp32"
$DATA_DIR = "data"
$RELEASE_DIR = "releases\v$version"

# Herramientas (rutas automáticas)
$MKSPIFFS = "$env:LOCALAPPDATA\Arduino15\packages\esp32\tools\mkspiffs\0.2.3\mkspiffs.exe"

# Colores para output
function Write-Step { param($msg) Write-Host "[*] $msg" -ForegroundColor Cyan }
function Write-Success { param($msg) Write-Host "[OK] $msg" -ForegroundColor Green }
function Write-Error-Custom { param($msg) Write-Host "[ERROR] $msg" -ForegroundColor Red }
function Write-Info { param($msg) Write-Host "[INFO] $msg" -ForegroundColor Yellow }

# Banner
Write-Host "`n========================================" -ForegroundColor Magenta
Write-Host "   CAMPANARIOS - BUILD RELEASE v$version" -ForegroundColor Magenta
Write-Host "========================================`n" -ForegroundColor Magenta

# Verificar que existe carpeta data
if (-not (Test-Path $DATA_DIR)) {
    Write-Error-Custom "No se encuentra la carpeta '$DATA_DIR'"
    exit 1
}

# Verificar mkspiffs
if (-not (Test-Path $MKSPIFFS)) {
    Write-Error-Custom "No se encuentra mkspiffs.exe en: $MKSPIFFS"
    exit 1
}

# PASO 1: Compilar proyecto
Write-Step "Compilando proyecto con esquema '$PARTITION_SCHEME'..."
$compileCmd = "arduino-cli compile -b ${BOARD}:PartitionScheme=$PARTITION_SCHEME,UploadSpeed=$UPLOAD_SPEED -e"
Write-Info "Ejecutando: $compileCmd"

$compileResult = Invoke-Expression $compileCmd 2>&1
if ($LASTEXITCODE -ne 0) {
    Write-Error-Custom "Error en compilación"
    Write-Host $compileResult
    exit 1
}

Write-Success "Compilación exitosa"

# Extraer tamaño del sketch
$sizeInfo = $compileResult | Select-String "El Sketch usa.*bytes"
if ($sizeInfo) {
    Write-Info $sizeInfo.Line
}

# PASO 2: Verificar firmware.bin generado
$firmwareBin = "$BUILD_DIR\$PROJECT_NAME.ino.bin"
if (-not (Test-Path $firmwareBin)) {
    Write-Error-Custom "No se generó el firmware: $firmwareBin"
    exit 1
}

$firmwareSize = (Get-Item $firmwareBin).Length
Write-Success "Firmware generado: $firmwareBin ($([math]::Round($firmwareSize/1KB, 2)) KB)"

# PASO 3: Generar SPIFFS.bin
Write-Step "Generando SPIFFS.bin desde carpeta '$DATA_DIR'..."

# Tamaño SPIFFS para esquema 'default' = 1441792 bytes (0x160000)
$spiffsSize = 1441792
$spiffsBin = "$BUILD_DIR\$PROJECT_NAME.spiffs.bin"

$mkspiffsCmd = "& `"$MKSPIFFS`" -c `"$DATA_DIR`" -b 4096 -p 256 -s $spiffsSize `"$spiffsBin`""
Write-Info "Ejecutando: $mkspiffsCmd"

Invoke-Expression $mkspiffsCmd
if ($LASTEXITCODE -ne 0) {
    Write-Error-Custom "Error generando SPIFFS"
    exit 1
}

if (-not (Test-Path $spiffsBin)) {
    Write-Error-Custom "No se generó SPIFFS.bin"
    exit 1
}

$spiffsActualSize = (Get-Item $spiffsBin).Length
Write-Success "SPIFFS generado: $spiffsBin ($([math]::Round($spiffsActualSize/1KB, 2)) KB)"

# PASO 4: Crear carpeta de release
Write-Step "Creando carpeta de release: $RELEASE_DIR"
if (Test-Path $RELEASE_DIR) {
    Remove-Item $RELEASE_DIR -Recurse -Force
}
New-Item -ItemType Directory -Path $RELEASE_DIR -Force | Out-Null

# PASO 5: Copiar archivos
Write-Step "Copiando archivos a release..."

Copy-Item $firmwareBin "$RELEASE_DIR\$PROJECT_NAME-firmware-v$version.bin"
Copy-Item $spiffsBin "$RELEASE_DIR\$PROJECT_NAME-spiffs-v$version.bin"

# Copiar también particiones
$partitionsBin = "$BUILD_DIR\$PROJECT_NAME.ino.partitions.bin"
if (Test-Path $partitionsBin) {
    Copy-Item $partitionsBin "$RELEASE_DIR\$PROJECT_NAME-partitions-v$version.bin"
    Write-Success "Partitions.bin copiado"
}

# PASO 6: Crear archivo de versión
$versionFile = "$RELEASE_DIR\version.json"
$versionData = @{
    version = $version
    date = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    firmware_size = $firmwareSize
    spiffs_size = $spiffsActualSize
    partition_scheme = $PARTITION_SCHEME
} | ConvertTo-Json

$versionData | Out-File -FilePath $versionFile -Encoding UTF8
Write-Success "version.json creado"

# PASO 7: Resumen
Write-Host "========================================" -ForegroundColor Green
Write-Host "   RELEASE v$version GENERADO" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host "Ubicacion: $RELEASE_DIR" -ForegroundColor White
Write-Host ""

Get-ChildItem $RELEASE_DIR | ForEach-Object {
    $size = [math]::Round($_.Length/1KB, 2)
    Write-Host "  $($_.Name) - $size KB" -ForegroundColor White
}

# PASO 8: Subir a GitHub (opcional)
if ($uploadToGitHub) {
    Write-Host "`n" -NoNewline
    Write-Step "Subiendo a GitHub Releases..."
    
    # Verificar si existe gh CLI
    $ghExists = Get-Command gh -ErrorAction SilentlyContinue
    if (-not $ghExists) {
        Write-Error-Custom "GitHub CLI (gh) no está instalado"
        Write-Info "Instalar desde: https://cli.github.com/"
        Write-Info "O sube manualmente los archivos desde: $RELEASE_DIR"
        exit 0
    }
    
    # Crear release
    $firmwareSizeKB = [math]::Round($firmwareSize/1KB, 2)
    $spiffsSizeKB = [math]::Round($spiffsActualSize/1KB, 2)
    $currentDate = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    
    $releaseNotes = "## Campanarios v$version`n`n"
    $releaseNotes += "**Fecha:** $currentDate`n`n"
    $releaseNotes += "### Archivos para OTA:`n"
    $releaseNotes += "* Firmware: $firmwareSizeKB KB`n"
    $releaseNotes += "* SPIFFS: $spiffsSizeKB KB`n`n"
    $releaseNotes += "### Instalacion:`n"
    $releaseNotes += "1. Descargar ambos archivos (.bin)`n"
    $releaseNotes += "2. Acceder a la interfaz OTA del ESP32`n"
    $releaseNotes += "3. Subir firmware y SPIFFS`n"
    $releaseNotes += "4. Reiniciar el dispositivo`n"
    
    $releaseNotesFile = Join-Path $RELEASE_DIR "release-notes.md"
    $releaseNotes | Out-File -FilePath $releaseNotesFile -Encoding UTF8
    
    # Ejecutar gh release create
    $ghCmd = "gh release create v$version --title `"Campanarios v$version`" --notes-file `"$releaseNotesFile`" `"$RELEASE_DIR\*.bin`""
    Write-Info "Ejecutando: $ghCmd"
    
    Invoke-Expression $ghCmd
    if ($LASTEXITCODE -eq 0) {
        Write-Success "Release v$version creado en GitHub"
        Write-Info "URL: https://github.com/Jusaba/Campanarios/releases/tag/v$version"
    } else {
        Write-Error-Custom "Error creando release en GitHub"
        Write-Info "Puedes subirlo manualmente desde: $RELEASE_DIR"
    }
}

Write-Host ""
Write-Host "Proceso completado exitosamente!" -ForegroundColor Green
Write-Host ""

# Instrucciones finales
if (-not $uploadToGitHub) {
    Write-Host "Para subir a GitHub:" -ForegroundColor Yellow
    Write-Host "  .\build-release.ps1 -version '$version' -uploadToGitHub" -ForegroundColor Cyan
    Write-Host ""
}
