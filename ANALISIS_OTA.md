# Análisis OTA (Campanarios)

## 1) Resumen ejecutivo

El módulo OTA está **funcional y bastante completo**: detecta releases en GitHub, soporta actualización de firmware y SPIFFS, reporta progreso por WebSocket y contempla ejecución asíncrona para no bloquear la app web.

Aun así, hay riesgos relevantes de robustez/seguridad que conviene priorizar:

1. **TLS inseguro (`setInsecure`)** en todas las conexiones HTTPS de OTA.
2. **Posible desbordamiento de mensaje WebSocket** al enviar `releaseNotes` completo en una sola trama.
3. **Verificación incompleta del artefacto**: no se valida checksum/firmas, solo tamaño y `Update.end(true)`.
4. **Lógica de UI con “éxito asumido” por timeout**, que puede ocultar fallos reales.

## 2) Flujo actual OTA

- El sistema inicializa OTA al arrancar cuando hay conectividad Wi‑Fi y hace checks automáticos en `loop()` según configuración.
- La UI abre un modal OTA, consulta versión actual, pide check de actualización y puede iniciar firmware, SPIFFS o completa.
- El servidor WebSocket recibe comandos OTA, llama al servicio OTA, y devuelve eventos (`UPDATE_AVAILABLE`, `OTA_PROGRESS`, `OTA_SUCCESS`, `OTA_ERROR`).
- El servicio OTA consulta `releases/latest` de GitHub, selecciona assets `.bin` por nombre, descarga e instala vía `Update`.

## 3) Hallazgos técnicos (detalle)

### 3.1 Seguridad de transporte

- Se usa `WiFiClientSecure` con `client.setInsecure()` tanto en check de versión como en descarga de firmware/SPIFFS.
- Impacto: vulnerable a MITM si hay red comprometida.
- Recomendación: fijar CA (`setCACert`) o pinning de cert/public key, y fallback controlado solo en entornos de pruebas.

### 3.2 Validación de artefactos

- Validaciones actuales:
  - código HTTP OK;
  - tamaño > 0 y límites máximos por partición;
  - `Update.end(true)`.
- Falta validación criptográfica de integridad/autenticidad del binario (SHA-256 firmado o firma detached).
- Recomendación: publicar hash en release (o firma), descargarlo y validar antes de `Update.end`.

### 3.3 Riesgo por tamaño de mensaje en WebSocket

- `UPDATE_AVAILABLE` concatena versión, URLs y `releaseNotes` en un solo string delimitado por `:`.
- Si `releaseNotes` es largo, puede fragmentar o superar buffers, con errores de parseo/límite.
- Recomendación: enviar JSON o campos separados (p.ej. `UPDATE_META` sin notas + endpoint para notas).

### 3.4 Comportamiento UI optimista

- La UI arranca simulación de progreso y tiene timeout de 3 min que “asume éxito” y recarga.
- Esto puede enmascarar errores reales cuando no llega confirmación.
- Recomendación: no asumir éxito sin `OTA_SUCCESS`; en timeout mostrar estado incierto con opción de reconectar/reintentar.

### 3.5 Robustez de parseo de versión

- Comparación semver básica con `sscanf("%d.%d.%d")`; para formatos no estándar (`1.5`, `1.5.7-beta`) puede dar comparaciones inesperadas.
- Recomendación: normalizar semver (major/minor/patch opcional + prerelease) o restringir formato y validarlo explícitamente.

### 3.6 Consistencia menor de código

- Hay líneas duplicadas en retornos/`http.end()` en rutas de error (`updateFirmware`, `updateSPIFFS`), sin romper funcionalidad, pero conviene limpiar.
- Además, el parámetro `expectedSize` se recibe pero no se usa para validar `contentLength`.

## 4) Puntos fuertes detectados

- Buena separación de responsabilidades entre UI, servidor WebSocket y servicio OTA.
- Soporte de actualización parcial (firmware/SPIFFS) y completa.
- Reporting de progreso y callbacks de éxito/error.
- Modo asíncrono con FreeRTOS para actualización completa evitando bloqueo del WebSocket.
- Límites máximos de tamaño definidos por configuración.

## 5) Priorización recomendada (plan corto)

### Prioridad alta

1. TLS seguro (CA/pinning) para check y descarga.
2. Integridad/autenticidad de binarios (hash/firma).
3. Ajustar protocolo WebSocket OTA a JSON para evitar sobrecargas y parseo por `:`.

### Prioridad media

4. Quitar “éxito asumido” por timeout en frontend.
5. Validar formato de versión y endurecer comparación.
6. Usar `expectedSize` cuando venga informado por API/release.

### Prioridad baja

7. Limpieza de duplicados/menores en rutas de error.
8. Telemetría OTA más detallada (motivo de fallo, código HTTP, retries).

## 6) Conclusión

La base OTA es buena para entorno controlado, pero para producción conviene endurecer **seguridad criptográfica** y **fiabilidad del protocolo UI/WS**. Con esos ajustes, el sistema ganará mucha resiliencia sin requerir rediseño completo.
