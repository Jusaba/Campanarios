#!/usr/bin/env python3
"""
Script para desplegar actualizaciones OTA a múltiples dispositivos ESP32 Campanarios.

Funcionalidades:
- Backup de configuraciones SPIFFS de cada dispositivo
- Actualización OTA de firmware y/o SPIFFS
- Restauración automática de configuraciones personalizadas

Uso:
    python deploy-devices.py --version 1.1.4
    python deploy-devices.py --version 1.1.4 --only-backup
    python deploy-devices.py --version 1.1.4 --only-restore
"""

import argparse
import base64
import json
import os
import sys
import time
from pathlib import Path
from typing import Dict, List, Optional, Tuple

try:
    import requests
except ImportError:
    print("ERROR: Módulo 'requests' no encontrado.")
    print("Instala con: pip install requests")
    sys.exit(1)

try:
    from websocket import create_connection
except ImportError:
    print("ERROR: Módulo 'websocket-client' no encontrado.")
    print("Instala con: pip install websocket-client")
    sys.exit(1)

# ============================================================================
# CONFIGURACIÓN
# ============================================================================

DEVICES_FILE = "Dispositivos.txt"
BACKUPS_DIR = "device-backups"

# Credenciales por defecto (HTTP Basic Auth)
DEFAULT_USER = "manolo"
DEFAULT_PASS = "garcia"

# Archivos de configuración a respaldar/restaurar
CONFIG_FILES = [
    "alarmas_personalizadas.json",
    "telegram_config.json",
    "config.json",
    "Secuencias.json"
]

# Archivos que se crean dinámicamente (verificación opcional)
OPTIONAL_FILES = [
    "alarmas_personalizadas.json",
    "telegram_config.json",
    "Secuencias.json"
]

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

def print_device(msg: str):
    """Imprime mensaje relacionado con dispositivo"""
    print(f"{Colors.MAGENTA}[DEVICE]{Colors.RESET} {msg}")

def print_banner(version: Optional[str] = None):
    """Imprime banner del script"""
    print()
    print(f"{Colors.MAGENTA}{'=' * 40}{Colors.RESET}")
    print(f"{Colors.MAGENTA}   CAMPANARIOS - DEPLOY TO DEVICES{Colors.RESET}")
    if version:
        print(f"{Colors.MAGENTA}   Version: {version}{Colors.RESET}")
    print(f"{Colors.MAGENTA}{'=' * 40}{Colors.RESET}")
    print()

# ============================================================================
# LECTURA DE DISPOSITIVOS
# ============================================================================

def load_devices(file_path: str) -> List[Dict[str, str]]:
    """
    Carga lista de dispositivos desde archivo de texto.
    
    Formato esperado: nombre_dispositivo,dominio:puerto,usuario,password
    Ejemplo: Campanario-Principal,campanario-principal.jusaba.es:8080,admin,secret123
    
    Args:
        file_path: Ruta al archivo de dispositivos
        
    Returns:
        Lista de diccionarios con 'name', 'host', 'username' y 'password'
        
    Raises:
        FileNotFoundError: Si el archivo no existe
        ValueError: Si el formato es incorrecto
    """
    if not os.path.exists(file_path):
        raise FileNotFoundError(
            f"No se encuentra el archivo {file_path}\n"
            f"Crea un archivo {file_path} con el formato:\n"
            f"nombre_dispositivo,dominio:puerto,usuario,password\n"
            f"Ejemplo:\n"
            f"  Campanario-Principal,campanario-principal.jusaba.es:8080,admin,secret123\n"
            f"  Campanario-Auxiliar,campanario-auxiliar.jusaba.es:8080,manolo,garcia"
        )
    
    devices = []
    with open(file_path, 'r', encoding='utf-8') as f:
        for line_num, line in enumerate(f, 1):
            line = line.strip()
            
            # Ignorar líneas vacías y comentarios
            if not line or line.startswith('#'):
                continue
            
            # Parsear línea
            parts = [p.strip() for p in line.split(',')]
            if len(parts) not in [2, 4]:
                raise ValueError(
                    f"Formato incorrecto en línea {line_num}: {line}\n"
                    f"Formato esperado: nombre,host:puerto[,usuario,password]"
                )
            
            name = parts[0]
            host = parts[1]
            username = parts[2] if len(parts) == 4 else DEFAULT_USER
            password = parts[3] if len(parts) == 4 else DEFAULT_PASS
            
            devices.append({
                'name': name,
                'host': host,
                'username': username,
                'password': password
            })
            print_info(f"  - {name} @ {host} (user: {username})")
    
    if not devices:
        raise ValueError(f"No se encontraron dispositivos en {file_path}")
    
    return devices

# ============================================================================
# FUNCIONES DE BACKUP
# ============================================================================

def backup_device_config(
    device_name: str,
    device_host: str,
    username: str = DEFAULT_USER,
    password: str = DEFAULT_PASS
) -> bool:
    """
    Hace backup de las configuraciones SPIFFS de un dispositivo.
    
    Args:
        device_name: Nombre del dispositivo
        device_host: Host del dispositivo (dominio:puerto)
        username: Usuario para HTTP Basic Auth
        password: Contraseña para HTTP Basic Auth
        
    Returns:
        True si el backup fue exitoso, False en caso contrario
    """
    print_device(f"Haciendo backup de {device_name} ({device_host})...")
    
    # Crear directorio del dispositivo
    device_dir = Path(BACKUPS_DIR) / device_name
    if device_dir.exists():
        print_info("  Limpiando directorio existente...")
        for file in device_dir.glob('*'):
            file.unlink()
    else:
        device_dir.mkdir(parents=True, exist_ok=True)
    
    # Preparar autenticación
    auth = (username, password)
    
    # Verificar conectividad
    print_info("  Verificando conexión...")
    try:
        response = requests.head(
            f"http://{device_host}/download?file=config.json",
            auth=auth,
            timeout=5
        )
        print_success("  Dispositivo accesible")
    except requests.exceptions.RequestException as e:
        print_error(f"  No se puede conectar a {device_host}: {e}")
        return False
    
    # Descargar cada archivo de configuración
    all_success = True
    for config_file in CONFIG_FILES:
        print_info(f"  Descargando {config_file}...")
        try:
            url = f"http://{device_host}/download?file={config_file}"
            response = requests.get(url, auth=auth, timeout=30)
            
            if response.status_code == 200:
                output_path = device_dir / config_file
                output_path.write_bytes(response.content)
                size = len(response.content)
                print_success(f"    [OK] {config_file} ({size} bytes)")
            elif response.status_code in [403, 404]:
                print_info(f"    [-] {config_file} (no existe en dispositivo)")
            else:
                print_error(f"    [X] {config_file} (HTTP {response.status_code})")
                all_success = False
                
        except requests.exceptions.RequestException as e:
            print_info(f"    [-] {config_file} (no existe en dispositivo)")
    
    return all_success

# ============================================================================
# FUNCIONES DE ACTUALIZACIÓN
# ============================================================================

def update_device_firmware(
    device_name: str,
    device_host: str,
    version: str,
    update_type: str = "complete",
    username: str = DEFAULT_USER,
    password: str = DEFAULT_PASS
) -> bool:
    """
    Actualiza el firmware/SPIFFS de un dispositivo vía OTA usando WebSocket.
    
    NOTA: El dispositivo descarga automáticamente desde GitHub Releases.
    Esta función solo envía el comando de actualización vía WebSocket.
    
    Args:
        device_name: Nombre del dispositivo
        device_host: Host del dispositivo (dominio:puerto)
        version: Versión a instalar
        update_type: Tipo de actualización ("firmware", "spiffs", "complete")
        username: Usuario para HTTP Basic Auth
        password: Contraseña para HTTP Basic Auth
        
    Returns:
        True si la actualización fue exitosa, False en caso contrario
    """
    print_device(f"Actualizando {update_type} en {device_name} ({device_host})...")
    print_info(f"  El dispositivo descargará v{version} desde GitHub Releases")
    
    # Mapear tipo de actualización a comando WebSocket
    ws_commands = {
        "firmware": "START_UPDATE_FIRMWARE",
        "spiffs": "START_UPDATE_SPIFFS",
        "complete": "START_UPDATE_COMPLETE"
    }
    
    command = ws_commands.get(update_type)
    if not command:
        print_error(f"  Tipo de actualización inválido: {update_type}")
        return False
    
    try:
        # Conectar al WebSocket
        ws_url = f"ws://{device_host}/ws"
        print_info(f"  Conectando a WebSocket: {ws_url}")
        
        ws = create_connection(ws_url, timeout=10)
        print_success("  Conectado al WebSocket")
        
        # Enviar comando de actualización
        print_info(f"  Enviando comando: {command}")
        ws.send(command)
        
        # Escuchar respuestas
        # NOTA: El proceso OTA es largo (2-5 minutos) y bloquea el WebSocket.
        # El dispositivo se reinicia automáticamente al terminar sin enviar OTA_SUCCESS.
        print_info("  Esperando inicio de actualización...")
        
        update_started = False
        max_wait = 30  # 30 segundos para confirmar inicio
        start_time = time.time()
        
        while (time.time() - start_time) < max_wait:
            try:
                # Timeout de 5 segundos para recv
                ws.settimeout(5.0)
                response = ws.recv()
                
                if response.startswith("OTA_PROGRESS:"):
                    # Parsear: OTA_PROGRESS:0:Iniciando actualización...
                    parts = response.split(':', 2)
                    if len(parts) >= 2:
                        progress = parts[1]
                        message = parts[2] if len(parts) > 2 else ""
                        print_info(f"    [{progress}%] {message}")
                        update_started = True
                        # Después del primer mensaje, el proceso es largo y el WS se cierra
                        # Dejamos de escuchar y esperamos el reinicio
                        break
                
                elif response.startswith("OTA_ERROR:"):
                    # Error en actualización
                    error_msg = response.split(':', 1)[1] if ':' in response else "Error desconocido"
                    print_error(f"    [X] Error OTA: {error_msg}")
                    ws.close()
                    return False
                
            except Exception as e:
                # Timeout o conexión cerrada
                error_str = str(e).lower()
                if "timed out" in error_str or "timeout" in error_str:
                    if update_started:
                        # Ya empezó la actualización, el timeout es normal
                        break
                    continue
                else:
                    # Conexión cerrada inesperadamente
                    if update_started:
                        # Es normal que se cierre durante la actualización
                        break
                    else:
                        print_error(f"    [X] Error de conexión: {e}")
                        ws.close()
                        return False
        
        ws.close()
        
        if not update_started:
            print_error("    [X] La actualización no se inició correctamente")
            return False
        
        print_success("    [OK] Actualización iniciada correctamente")
        
        # Esperar a que el dispositivo reinicie (proceso OTA completo: 2-5 minutos)
        # El firmware se descarga desde GitHub, se instala y el ESP32 se reinicia
        print_info("    Proceso OTA en ejecución en segundo plano...")
        print_info("    Esperando reinicio del dispositivo (~3-5 minutos)...")
        
        # Espera inteligente: verificar cada 30s si el dispositivo está disponible
        auth = (username, password)
        max_attempts = 12  # 12 intentos x 30s = 6 minutos máximo
        
        for attempt in range(max_attempts):
            time.sleep(30)
            
            try:
                response = requests.head(
                    f"http://{device_host}/",
                    auth=auth,
                    timeout=3
                )
                # Si responde, el dispositivo ya reinició
                print_success(f"    [OK] Dispositivo reiniciado después de {(attempt + 1) * 30}s")
                
                # CRÍTICO: Esperar tiempo adicional para que SPIFFS esté completamente listo
                print_info("    Esperando inicialización completa del sistema...")
                time.sleep(15)  # 15 segundos adicionales para que SPIFFS esté listo
                
                break
            except requests.exceptions.RequestException:
                # Aún no responde, seguir esperando
                elapsed = (attempt + 1) * 30
                print_info(f"    Esperando... ({elapsed}s / 360s)")
        
        # Verificación final de disponibilidad
        print_info("    Verificando disponibilidad...")
        max_retries = 10
        online = False
        auth = (username, password)
        
        for retry in range(max_retries):
            try:
                response = requests.head(
                    f"http://{device_host}/",
                    auth=auth,
                    timeout=3
                )
                online = True
                print_success("    [OK] Dispositivo nuevamente online")
                break
            except requests.exceptions.RequestException:
                print_info(f"    Reintento {retry + 1}/{max_retries}...")
                time.sleep(3)
        
        if not online:
            print_error("    [X] Dispositivo no responde después del reinicio")
            return False
        
        return True
        
    except Exception as e:
        print_error(f"    Error en actualización WebSocket: {e}")
        return False

# ============================================================================
# FUNCIONES DE RESTAURACIÓN
# ============================================================================

def restore_device_config(
    device_name: str,
    device_host: str,
    username: str = DEFAULT_USER,
    password: str = DEFAULT_PASS
) -> bool:
    """
    Restaura las configuraciones SPIFFS de un dispositivo desde backup.
    
    Args:
        device_name: Nombre del dispositivo
        device_host: Host del dispositivo (dominio:puerto)
        username: Usuario para HTTP Basic Auth
        password: Contraseña para HTTP Basic Auth
        
    Returns:
        True si la restauración fue exitosa, False en caso contrario
    """
    print_device(f"Restaurando configuración de {device_name} ({device_host})...")
    
    device_dir = Path(BACKUPS_DIR) / device_name
    if not device_dir.exists():
        print_error(f"  No existe backup para {device_name}")
        return False
    
    # Preparar autenticación
    auth = (username, password)
    
    # Verificar conectividad
    print_info("  Verificando conexión...")
    try:
        response = requests.head(
            f"http://{device_host}/",
            auth=auth,
            timeout=5
        )
        print_success("  Dispositivo accesible")
    except requests.exceptions.RequestException as e:
        print_error(f"  No se puede conectar a {device_host}: {e}")
        return False
    
    # Dar tiempo a que SPIFFS se monte completamente tras reinicio
    print_info("  Esperando inicialización de SPIFFS...")
    time.sleep(10)  # Aumentado de 5 a 10 segundos
    
    # Verificar que SPIFFS está accesible intentando descargar un archivo
    print_info("  Verificando SPIFFS...")
    for retry in range(10):  # Aumentado de 5 a 10 reintentos
        try:
            test_response = requests.get(
                f"http://{device_host}/download?file=config.json",
                auth=auth,
                timeout=5
            )
            if test_response.status_code in [200, 404]:  # 200=existe, 404=no existe pero SPIFFS funciona
                print_success("  SPIFFS listo")
                time.sleep(3)  # Espera adicional de seguridad
                break
        except requests.exceptions.RequestException:
            if retry < 9:
                print_info(f"  Reintento SPIFFS {retry + 1}/10...")
                time.sleep(5)  # Aumentado de 3 a 5 segundos entre reintentos
            else:
                print_error("  SPIFFS no responde")
                return False
    
    # Subir cada archivo de configuración
    all_success = True
    for config_file in CONFIG_FILES:
        file_path = device_dir / config_file
        
        if not file_path.exists():
            print_info(f"  [-] {config_file} (no existe en backup)")
            continue
        
        print_info(f"  Subiendo {config_file}...")
        try:
            url = f"http://{device_host}/upload"
            
            with open(file_path, 'rb') as f:
                # IMPORTANTE: El tercer parámetro (config_file) es el nombre del archivo
                # que el servidor usará, no el nombre del campo del formulario
                files = {
                    'file': (config_file, f, 'application/octet-stream')
                }
                
                # DEBUG: Mostrar información de la petición
                file_size = file_path.stat().st_size
                print_info(f"    Enviando {file_size} bytes como '{config_file}'...")
                
                response = requests.post(url, files=files, auth=auth, timeout=30)
                
                # DEBUG: Mostrar respuesta completa
                print_info(f"    Respuesta: HTTP {response.status_code}")
                if response.text:
                    print_info(f"    Body: {response.text[:200]}")
            
            if response.status_code == 200:
                file_size = file_path.stat().st_size
                print_success(f"    [OK] {config_file} restaurado ({file_size} bytes)")
            else:
                print_error(f"    [X] Error HTTP {response.status_code}: {response.text}")
                all_success = False
                
        except requests.exceptions.RequestException as e:
            print_error(f"    [X] Error subiendo {config_file}: {e}")
            all_success = False
    
    # Verificar que los archivos se guardaron correctamente
    if all_success:
        print_info("  Esperando sincronización de SPIFFS...")
        time.sleep(10)  # Aumentado de 2 a 10 segundos - CRÍTICO para que SPIFFS sincronice
        
        print_info("  Verificando archivos guardados en SPIFFS...")
        
        for config_file in CONFIG_FILES:
            file_path = device_dir / config_file
            if not file_path.exists():
                continue
                
            try:
                # Descargar el archivo y comparar tamaño
                response = requests.get(
                    f"http://{device_host}/download?file={config_file}",
                    auth=auth,
                    timeout=10
                )
                
                if response.status_code == 200:
                    downloaded_size = len(response.content)
                    original_size = file_path.stat().st_size
                    
                    if downloaded_size == original_size:
                        print_success(f"    [OK] {config_file} verificado ({downloaded_size} bytes)")
                    else:
                        print_error(f"    [X] {config_file} tamaño incorrecto: {downloaded_size} vs {original_size}")
                        all_success = False
                else:
                    # Verificar si el archivo es opcional (se crea dinámicamente)
                    if config_file in OPTIONAL_FILES:
                        print_info(f"    [-] {config_file} no encontrado (se creará automáticamente)")
                    else:
                        print_error(f"    [X] {config_file} no encontrado en SPIFFS")
                        all_success = False
                    
            except requests.exceptions.RequestException as e:
                print_error(f"    [X] Error verificando {config_file}: {e}")
                all_success = False
    
    return all_success

# ============================================================================
# MENÚ INTERACTIVO
# ============================================================================

def mostrar_menu_principal():
    """Muestra el menú principal y retorna la opción seleccionada"""
    print()
    print(f"{Colors.CYAN}{'=' * 50}{Colors.RESET}")
    print(f"{Colors.CYAN}    CAMPANARIOS - MENÚ DE ACTUALIZACIÓN OTA{Colors.RESET}")
    print(f"{Colors.CYAN}{'=' * 50}{Colors.RESET}")
    print()
    print(f"{Colors.WHITE}Selecciona una opción:{Colors.RESET}")
    print(f"{Colors.GREEN}  1.{Colors.RESET} Actualizar TODO (Firmware + SPIFFS)")
    print(f"{Colors.GREEN}  2.{Colors.RESET} Actualizar SOLO Firmware (sin tocar SPIFFS)")
    print(f"{Colors.YELLOW}  3.{Colors.RESET} Solo hacer BACKUP de configuraciones")
    print(f"{Colors.YELLOW}  4.{Colors.RESET} Solo RESTAURAR configuraciones desde backup")
    print(f"{Colors.RED}  5.{Colors.RESET} Salir")
    print()
    
    while True:
        try:
            opcion = input(f"{Colors.CYAN}Opción [1-5]: {Colors.RESET}").strip()
            if opcion in ['1', '2', '3', '4', '5']:
                return int(opcion)
            else:
                print(f"{Colors.RED}❌ Opción inválida. Usa 1-5{Colors.RESET}")
        except (KeyboardInterrupt, EOFError):
            print()
            return 5

def listar_versiones_disponibles():
    """Lista las versiones disponibles en el directorio releases"""
    releases_dir = Path("releases")
    if not releases_dir.exists():
        return []
    
    versiones = []
    for item in sorted(releases_dir.iterdir(), reverse=True):
        if item.is_dir() and item.name.startswith('v'):
            version = item.name[1:]  # Quitar la 'v'
            versiones.append(version)
    
    return versiones

def seleccionar_version():
    """Muestra las versiones disponibles y permite seleccionar una"""
    versiones = listar_versiones_disponibles()
    
    if not versiones:
        print_error("No se encontraron versiones en el directorio releases/")
        return None
    
    print()
    print(f"{Colors.MAGENTA}Versiones disponibles:{Colors.RESET}")
    for i, version in enumerate(versiones, 1):
        print(f"  {Colors.GREEN}{i}.{Colors.RESET} v{version}")
    print(f"  {Colors.RED}0.{Colors.RESET} Cancelar")
    print()
    
    while True:
        try:
            opcion = input(f"{Colors.CYAN}Selecciona versión [0-{len(versiones)}]: {Colors.RESET}").strip()
            if opcion == '0':
                return None
            idx = int(opcion) - 1
            if 0 <= idx < len(versiones):
                return versiones[idx]
            else:
                print(f"{Colors.RED}❌ Opción inválida{Colors.RESET}")
        except (ValueError, KeyboardInterrupt, EOFError):
            print(f"{Colors.RED}❌ Entrada inválida{Colors.RESET}")
            return None

def modo_interactivo():
    """Modo interactivo con menú"""
    while True:
        opcion = mostrar_menu_principal()
        
        if opcion == 5:
            print_info("👋 Saliendo...")
            return 0
        
        # Para opciones que requieren versión (1 y 2)
        version = None
        if opcion in [1, 2]:
            version = seleccionar_version()
            if not version:
                print_info("Operación cancelada")
                continue
        
        # Crear objeto args simulado
        class Args:
            pass
        
        args = Args()
        args.version = version
        args.only_backup = (opcion == 3)
        args.only_restore = (opcion == 4)
        args.firmware_only = (opcion == 2)
        args.username = DEFAULT_USER
        args.password = DEFAULT_PASS
        
        # Ejecutar la operación
        resultado = ejecutar_operacion(args)
        
        # Preguntar si desea continuar
        print()
        continuar = input(f"{Colors.CYAN}¿Realizar otra operación? [S/n]: {Colors.RESET}").strip().lower()
        if continuar in ['n', 'no']:
            print_info("👋 ¡Hasta luego!")
            return resultado
        print()

def ejecutar_operacion(args):
    """Ejecuta la operación seleccionada con los argumentos dados"""
    # Banner
    print_banner(args.version)
    
    # Cargar dispositivos
    print_step(f"Leyendo dispositivos desde {DEVICES_FILE}...")
    try:
        devices = load_devices(DEVICES_FILE)
    except (FileNotFoundError, ValueError) as e:
        print_error(str(e))
        return 1
    
    print_success(f"Encontrados {len(devices)} dispositivos")
    
    # Verificar versión si no es solo backup
    if not args.only_backup and not args.only_restore:
        release_dir = Path("releases") / f"v{args.version}"
        if not release_dir.exists():
            print_error(f"No existe el directorio de release: {release_dir}")
            print_info(f"Ejecuta primero: .\\build-release.ps1 -version \"{args.version}\"")
            return 1
        print_success(f"Directorio de release encontrado: {release_dir}")
    
    # Crear directorio de backups si no existe
    Path(BACKUPS_DIR).mkdir(exist_ok=True)
    
    # Procesar cada dispositivo
    total_devices = len(devices)
    success_count = 0
    fail_count = 0
    
    for idx, device in enumerate(devices, 1):
        print()
        print(f"{Colors.CYAN}{'=' * 40}{Colors.RESET}")
        print(f"{Colors.CYAN} Dispositivo {idx}/{total_devices} : {device['name']}{Colors.RESET}")
        print(f"{Colors.CYAN}{'=' * 40}{Colors.RESET}")
        
        device_success = True
        
        # PASO 1: Backup
        if not args.only_restore:
            if not backup_device_config(
                device['name'],
                device['host'],
                device.get('username', args.username),
                device.get('password', args.password)
            ):
                print_error(f"Falló el backup de {device['name']}")
                device_success = False
        
        # PASO 2: Actualizar
        if not args.only_backup and not args.only_restore and device_success:
            # Determinar tipo de actualización según parámetros
            update_type = "firmware" if args.firmware_only else "complete"
            if not update_device_firmware(
                device['name'],
                device['host'],
                args.version,
                update_type,
                device.get('username', args.username),
                device.get('password', args.password)
            ):
                print_error(f"Falló la actualización de {device['name']}")
                device_success = False
        
        # PASO 3: Restaurar configuración
        if not args.only_backup and device_success:
            if not restore_device_config(
                device['name'],
                device['host'],
                device.get('username', args.username),
                device.get('password', args.password)
            ):
                print_error(f"Falló la restauración de {device['name']}")
                device_success = False
        
        # Resultado del dispositivo
        if device_success:
            success_count += 1
            print_success(f"\n[OK] {device['name']} completado exitosamente")
        else:
            fail_count += 1
            print_error(f"\n[X] {device['name']} falló")
    
    # Resumen final
    print()
    print(f"{Colors.MAGENTA}{'=' * 40}{Colors.RESET}")
    print(f"{Colors.MAGENTA}   RESUMEN FINAL{Colors.RESET}")
    print(f"{Colors.MAGENTA}{'=' * 40}{Colors.RESET}")
    print_success(f"Dispositivos exitosos: {success_count}")
    if fail_count > 0:
        print_error(f"Dispositivos fallidos: {fail_count}")
    print(f"{Colors.WHITE}Total procesados: {total_devices}{Colors.RESET}")
    print()
    
    if fail_count == 0:
        print_success("¡Todos los dispositivos actualizados correctamente!")
        return 0
    else:
        print_error("Algunos dispositivos fallaron. Revisa los logs arriba.")
        return 1

# ============================================================================
# FUNCIÓN PRINCIPAL
# ============================================================================

def main():
    """Función principal del script"""
    parser = argparse.ArgumentParser(
        description="Script para desplegar actualizaciones OTA a dispositivos ESP32 Campanarios",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Ejemplos de uso:
  python deploy-devices.py                                    # Modo interactivo (RECOMENDADO)
  python deploy-devices.py --version 1.1.4                    # Actualiza firmware + SPIFFS
  python deploy-devices.py --version 1.1.4 --firmware-only   # Solo actualiza firmware
  python deploy-devices.py --version 1.1.4 --only-backup
  python deploy-devices.py --version 1.1.4 --only-restore
  python deploy-devices.py --version 1.1.4 --username admin --password 1234
        """
    )
    
    parser.add_argument(
        '--version',
        help='Versión a desplegar (debe existir en releases/)',
        required=False
    )
    parser.add_argument(
        '--only-backup',
        action='store_true',
        help='Solo hacer backup de configuraciones sin actualizar'
    )
    parser.add_argument(
        '--only-restore',
        action='store_true',
        help='Solo restaurar configuraciones desde backups existentes'
    )
    parser.add_argument(
        '--firmware-only',
        action='store_true',
        help='Actualizar solo firmware, sin tocar SPIFFS (por defecto actualiza firmware + SPIFFS)'
    )
    parser.add_argument(
        '--username',
        default=DEFAULT_USER,
        help=f'Usuario por defecto para HTTP Basic Auth si no se especifica en Dispositivos.txt (default: {DEFAULT_USER})'
    )
    parser.add_argument(
        '--password',
        default=DEFAULT_PASS,
        help='Contraseña por defecto para HTTP Basic Auth si no se especifica en Dispositivos.txt'
    )
    
    args = parser.parse_args()
    
    # Si no se proporcionan argumentos, usar modo interactivo
    if len(sys.argv) == 1:
        return modo_interactivo()
    
    # Validaciones para modo línea de comandos
    if not args.only_backup and not args.only_restore and not args.version:
        parser.error("Debe especificar --version para actualizar")
    
    # Ejecutar operación con argumentos de línea de comandos
    return ejecutar_operacion(args)

if __name__ == "__main__":
    sys.exit(main())
