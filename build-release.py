#!/usr/bin/env python3
"""
Script para compilar y generar release de Campanarios para OTA.

Funcionalidades:
- Compila el proyecto ESP32 con arduino-cli
- Genera firmware.bin y spiffs.bin
- Prepara archivos para OTA
- Opcionalmente sube a GitHub Releases

Uso:
    python build-release.py --version 1.0.0
    python build-release.py --version 1.0.1 --upload-to-github
"""

import argparse
import json
import os
import platform
import shutil
import subprocess
import sys
from datetime import datetime
from pathlib import Path
from typing import Dict, Optional

# ============================================================================
# CONFIGURACIÓN
# ============================================================================

BOARD = "esp32:esp32:esp32"
PARTITION_SCHEME = "default"
UPLOAD_SPEED = "115200"
PROJECT_NAME = "Campanarios"
BUILD_DIR = "build/esp32.esp32.esp32"
DATA_DIR = "data"

# Tamaño SPIFFS para esquema 'default' = 1441792 bytes (0x160000)
SPIFFS_SIZE = 1441792
SPIFFS_BLOCK_SIZE = 4096
SPIFFS_PAGE_SIZE = 256

# ============================================================================
# COLORES PARA TERMINAL
# ============================================================================

class Colors:
    """Códigos ANSI para colores en terminal"""
    CYAN = '\033[96m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    RED = '\033[91m'
    MAGENTA = '\033[95m'
    WHITE = '\033[97m'
    RESET = '\033[0m'
    BOLD = '\033[1m'

def print_step(msg: str):
    """Imprime mensaje de paso en curso"""
    print(f"{Colors.CYAN}[*]{Colors.RESET} {msg}")

def print_success(msg: str):
    """Imprime mensaje de éxito"""
    print(f"{Colors.GREEN}[OK]{Colors.RESET} {msg}")

def print_error(msg: str):
    """Imprime mensaje de error"""
    print(f"{Colors.RED}[ERROR]{Colors.RESET} {msg}")

def print_info(msg: str):
    """Imprime mensaje informativo"""
    print(f"{Colors.YELLOW}[INFO]{Colors.RESET} {msg}")

def print_banner(version: str):
    """Imprime banner del script"""
    print()
    print(f"{Colors.MAGENTA}{'=' * 40}{Colors.RESET}")
    print(f"{Colors.MAGENTA}   CAMPANARIOS - BUILD RELEASE v{version}{Colors.RESET}")
    print(f"{Colors.MAGENTA}{'=' * 40}{Colors.RESET}")
    print()

# ============================================================================
# FUNCIONES DE UTILIDAD
# ============================================================================

def find_mkspiffs() -> Optional[Path]:
    """
    Encuentra la ruta de mkspiffs según el sistema operativo.
    
    Returns:
        Path al ejecutable de mkspiffs o None si no se encuentra
    """
    system = platform.system()
    
    if system == "Windows":
        # Windows: %LOCALAPPDATA%\Arduino15\packages\esp32\tools\mkspiffs\0.2.3\mkspiffs.exe
        base_path = Path(os.environ.get("LOCALAPPDATA", "")) / "Arduino15"
    else:
        # Linux/macOS: ~/.arduino15/packages/esp32/tools/mkspiffs/0.2.3/mkspiffs
        base_path = Path.home() / ".arduino15"
    
    mkspiffs_path = base_path / "packages/esp32/tools/mkspiffs/0.2.3"
    
    if system == "Windows":
        mkspiffs_exe = mkspiffs_path / "mkspiffs.exe"
    else:
        mkspiffs_exe = mkspiffs_path / "mkspiffs"
    
    if mkspiffs_exe.exists():
        return mkspiffs_exe
    
    return None

def run_command(cmd: list, check: bool = True) -> subprocess.CompletedProcess:
    """
    Ejecuta un comando y devuelve el resultado.
    
    Args:
        cmd: Lista con el comando y sus argumentos
        check: Si True, lanza excepción en error
        
    Returns:
        CompletedProcess con el resultado
    """
    print_info(f"Ejecutando: {' '.join(cmd)}")
    result = subprocess.run(
        cmd,
        capture_output=True,
        text=True,
        check=check
    )
    return result

def format_size(size_bytes: int) -> str:
    """
    Formatea tamaño en bytes a KB/MB legible.
    
    Args:
        size_bytes: Tamaño en bytes
        
    Returns:
        String formateado (ej: "123.45 KB")
    """
    kb = size_bytes / 1024
    if kb > 1024:
        mb = kb / 1024
        return f"{mb:.2f} MB"
    return f"{kb:.2f} KB"

# ============================================================================
# FUNCIONES DE BUILD
# ============================================================================

def compile_firmware() -> Path:
    """
    Compila el proyecto ESP32 con arduino-cli.
    
    Returns:
        Path al firmware.bin generado
        
    Raises:
        RuntimeError: Si falla la compilación
    """
    print_step(f"Compilando proyecto con esquema '{PARTITION_SCHEME}'...")
    
    cmd = [
        "arduino-cli", "compile",
        "-b", f"{BOARD}:PartitionScheme={PARTITION_SCHEME},UploadSpeed={UPLOAD_SPEED}",
        "-e"
    ]
    
    try:
        result = run_command(cmd)
        
        # Buscar info de tamaño en el output
        for line in result.stdout.split('\n'):
            if "Sketch uses" in line or "El Sketch usa" in line:
                print_info(line.strip())
        
        print_success("Compilación exitosa")
        
    except subprocess.CalledProcessError as e:
        print_error("Error en compilación")
        print(e.stderr)
        raise RuntimeError("Compilación fallida")
    
    # Verificar que se generó el firmware
    firmware_path = Path(BUILD_DIR) / f"{PROJECT_NAME}.ino.bin"
    if not firmware_path.exists():
        raise RuntimeError(f"No se generó el firmware: {firmware_path}")
    
    size = firmware_path.stat().st_size
    print_success(f"Firmware generado: {firmware_path} ({format_size(size)})")
    
    return firmware_path

def generate_spiffs(mkspiffs_path: Path) -> Path:
    """
    Genera el archivo SPIFFS.bin desde la carpeta data/.
    
    Args:
        mkspiffs_path: Path al ejecutable mkspiffs
        
    Returns:
        Path al spiffs.bin generado
        
    Raises:
        RuntimeError: Si falla la generación
    """
    print_step(f"Generando SPIFFS.bin desde carpeta '{DATA_DIR}'...")
    
    spiffs_path = Path(BUILD_DIR) / f"{PROJECT_NAME}.spiffs.bin"
    
    cmd = [
        str(mkspiffs_path),
        "-c", DATA_DIR,
        "-b", str(SPIFFS_BLOCK_SIZE),
        "-p", str(SPIFFS_PAGE_SIZE),
        "-s", str(SPIFFS_SIZE),
        str(spiffs_path)
    ]
    
    try:
        run_command(cmd)
    except subprocess.CalledProcessError as e:
        print_error("Error generando SPIFFS")
        print(e.stderr)
        raise RuntimeError("Generación SPIFFS fallida")
    
    if not spiffs_path.exists():
        raise RuntimeError("No se generó SPIFFS.bin")
    
    size = spiffs_path.stat().st_size
    print_success(f"SPIFFS generado: {spiffs_path} ({format_size(size)})")
    
    return spiffs_path

def create_release_dir(version: str) -> Path:
    """
    Crea el directorio de release y limpia si existe.
    
    Args:
        version: Versión del release
        
    Returns:
        Path al directorio de release
    """
    print_step(f"Creando carpeta de release: releases/v{version}")
    
    release_dir = Path("releases") / f"v{version}"
    
    if release_dir.exists():
        shutil.rmtree(release_dir)
    
    release_dir.mkdir(parents=True, exist_ok=True)
    
    return release_dir

def copy_release_files(
    firmware_path: Path,
    spiffs_path: Path,
    release_dir: Path,
    version: str
) -> Dict[str, Path]:
    """
    Copia archivos compilados al directorio de release.
    
    Args:
        firmware_path: Path al firmware.bin
        spiffs_path: Path al spiffs.bin
        release_dir: Directorio de destino
        version: Versión del release
        
    Returns:
        Diccionario con paths de archivos copiados
    """
    print_step("Copiando archivos a release...")
    
    files = {}
    
    # Copiar firmware
    firmware_dest = release_dir / f"{PROJECT_NAME}-firmware-v{version}.bin"
    shutil.copy2(firmware_path, firmware_dest)
    files['firmware'] = firmware_dest
    
    # Copiar SPIFFS
    spiffs_dest = release_dir / f"{PROJECT_NAME}-spiffs-v{version}.bin"
    shutil.copy2(spiffs_path, spiffs_dest)
    files['spiffs'] = spiffs_dest
    
    # Copiar particiones si existe
    partitions_path = Path(BUILD_DIR) / f"{PROJECT_NAME}.ino.partitions.bin"
    if partitions_path.exists():
        partitions_dest = release_dir / f"{PROJECT_NAME}-partitions-v{version}.bin"
        shutil.copy2(partitions_path, partitions_dest)
        files['partitions'] = partitions_dest
        print_success("Partitions.bin copiado")
    
    return files

def create_version_json(
    release_dir: Path,
    version: str,
    firmware_size: int,
    spiffs_size: int
):
    """
    Crea archivo version.json con metadata del release.
    
    Args:
        release_dir: Directorio del release
        version: Versión del release
        firmware_size: Tamaño del firmware en bytes
        spiffs_size: Tamaño de SPIFFS en bytes
    """
    version_data = {
        "version": version,
        "date": datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
        "firmware_size": firmware_size,
        "spiffs_size": spiffs_size,
        "partition_scheme": PARTITION_SCHEME
    }
    
    version_file = release_dir / "version.json"
    with open(version_file, 'w', encoding='utf-8') as f:
        json.dump(version_data, f, indent=2)
    
    print_success("version.json creado")

def upload_to_github(release_dir: Path, version: str, firmware_size: int, spiffs_size: int) -> bool:
    """
    Sube el release a GitHub usando gh CLI.
    
    Args:
        release_dir: Directorio con archivos del release
        version: Versión del release
        firmware_size: Tamaño del firmware
        spiffs_size: Tamaño de SPIFFS
        
    Returns:
        True si se subió exitosamente, False en caso contrario
    """
    print()
    print_step("Subiendo a GitHub Releases...")
    
    # Verificar si existe gh CLI
    if not shutil.which("gh"):
        print_error("GitHub CLI (gh) no está instalado")
        print_info("Instalar desde: https://cli.github.com/")
        print_info(f"O sube manualmente los archivos desde: {release_dir}")
        return False
    
    # Crear release notes
    current_date = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    
    release_notes = f"""## Campanarios v{version}

**Fecha:** {current_date}

### Archivos para OTA:
* Firmware: {format_size(firmware_size)}
* SPIFFS: {format_size(spiffs_size)}
"""
    
    release_notes_file = release_dir / "release-notes.md"
    release_notes_file.write_text(release_notes, encoding='utf-8')
    
    # Obtener archivos .bin
    bin_files = list(release_dir.glob("*.bin"))
    
    # Ejecutar gh release create
    cmd = [
        "gh", "release", "create", f"v{version}",
        "--title", f"Campanarios v{version}",
        "--notes-file", str(release_notes_file)
    ] + [str(f) for f in bin_files]
    
    try:
        run_command(cmd)
        print_success(f"Release v{version} creado en GitHub")
        print_info(f"URL: https://github.com/Jusaba/Campanarios/releases/tag/v{version}")
        return True
    except subprocess.CalledProcessError:
        print_error("Error creando release en GitHub")
        print_info(f"Puedes subirlo manualmente desde: {release_dir}")
        return False

# ============================================================================
# FUNCIÓN PRINCIPAL
# ============================================================================

def main():
    """Función principal del script"""
    parser = argparse.ArgumentParser(
        description="Script para compilar y generar release de Campanarios",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Ejemplos de uso:
  python build-release.py --version 1.0.0
  python build-release.py --version 1.0.1 --upload-to-github
        """
    )
    
    parser.add_argument(
        '--version',
        required=True,
        help='Versión del release (ej: 1.0.0)'
    )
    parser.add_argument(
        '--upload-to-github',
        action='store_true',
        help='Subir automáticamente a GitHub Releases'
    )
    
    args = parser.parse_args()
    
    # Banner
    print_banner(args.version)
    
    # Verificar carpeta data
    if not Path(DATA_DIR).exists():
        print_error(f"No se encuentra la carpeta '{DATA_DIR}'")
        return 1
    
    # Verificar mkspiffs
    mkspiffs_path = find_mkspiffs()
    if not mkspiffs_path:
        print_error("No se encuentra mkspiffs")
        print_info("Asegúrate de tener instalado el core ESP32 para Arduino")
        return 1
    
    try:
        # PASO 1: Compilar firmware
        firmware_path = compile_firmware()
        firmware_size = firmware_path.stat().st_size
        
        # PASO 2: Generar SPIFFS
        spiffs_path = generate_spiffs(mkspiffs_path)
        spiffs_size = spiffs_path.stat().st_size
        
        # PASO 3: Crear directorio de release
        release_dir = create_release_dir(args.version)
        
        # PASO 4: Copiar archivos
        release_files = copy_release_files(
            firmware_path,
            spiffs_path,
            release_dir,
            args.version
        )
        
        # PASO 5: Crear version.json
        create_version_json(
            release_dir,
            args.version,
            firmware_size,
            spiffs_size
        )
        
        # PASO 6: Resumen
        print()
        print(f"{Colors.GREEN}{'=' * 40}{Colors.RESET}")
        print(f"{Colors.GREEN}   RELEASE v{args.version} GENERADO{Colors.RESET}")
        print(f"{Colors.GREEN}{'=' * 40}{Colors.RESET}")
        print(f"{Colors.WHITE}Ubicación: {release_dir}{Colors.RESET}")
        print()
        
        for file_path in release_dir.iterdir():
            if file_path.is_file():
                size = format_size(file_path.stat().st_size)
                print(f"{Colors.WHITE}  {file_path.name} - {size}{Colors.RESET}")
        
        # PASO 7: Subir a GitHub (opcional)
        if args.upload_to_github:
            upload_to_github(release_dir, args.version, firmware_size, spiffs_size)
        else:
            print()
            print(f"{Colors.YELLOW}Para subir a GitHub:{Colors.RESET}")
            print(f"{Colors.CYAN}  python build-release.py --version {args.version} --upload-to-github{Colors.RESET}")
        
        print()
        print(f"{Colors.GREEN}Proceso completado exitosamente!{Colors.RESET}")
        print()
        
        return 0
        
    except Exception as e:
        print_error(f"Error durante el build: {e}")
        return 1

if __name__ == "__main__":
    sys.exit(main())
