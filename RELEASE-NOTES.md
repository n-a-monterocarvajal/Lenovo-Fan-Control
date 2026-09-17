# 0.6 — Un solo exe, auto-elevación, auto-inicio / Single exe, self-elevation, start with Windows

## Español

- Un único ejecutable (`LenovoFanControl-x64.exe` / `-x86.exe`): el lector de
  sensores LibreHardwareMonitor va incrustado dentro, no es un archivo aparte.
  Sin ZIP ni instalador.
- El propio exe pide elevación (UAC) al abrir; ya no hace falta "Ejecutar como
  administrador" manualmente. Con eso, instala en silencio el controlador
  [PawnIO](https://pawnio.eu/) la primera vez que falta, para leer la CPU.
- Nueva opción **Iniciar con Windows** en la bandeja (registro, sin instalador
  ni tarea programada). Sigue pidiendo UAC en cada inicio de sesión porque el
  programa necesita administrador para EnergyDrv.
- Corregido: cambiar los umbrales con **Automático** activo ahora se aplica al
  instante, aunque la temperatura actual caiga dentro de la nueva banda entre
  ambos valores (antes había que desactivar y reactivar el modo).
- Texto de la banda de histéresis aclarado, y el «Acerca de» en español
  recuperó la mención al controlador Lenovo ACPI-Compliant Virtual Power
  Controller.

Se requieren el controlador Lenovo EnergyDrv compatible y .NET Framework 4.7.2
o posterior (ya presente en Windows 10/11). La aplicación no instala
EnergyDrv automáticamente.

Compilado y probado en x64 y x86 con sensores y controlador simulados:
umbrales (incluido el caso de histéresis corregido), cambios de modo, lecturas
inválidas/caducadas, menú y ajustes en español e inglés. La respuesta física
del ventilador y la compatibilidad de sensores aún deben verificarse en el
Lenovo de destino.

## English

- A single executable (`LenovoFanControl-x64.exe` / `-x86.exe`): the
  LibreHardwareMonitor-based sensor helper is embedded, not a separate file.
  No ZIP, no installer.
- The exe requests elevation (UAC) itself on launch; manually choosing "Run as
  administrator" is no longer necessary. With that, it silently installs the
  [PawnIO](https://pawnio.eu/) driver the first time it's missing, for CPU
  readings.
- New **Start with Windows** tray option (registry Run entry, no installer or
  scheduled task). Still prompts for UAC at every login since the app needs
  Administrator for EnergyDrv.
- Fixed: editing thresholds while **Automatic** is on now applies immediately,
  even when the current reading falls inside the new dead band between both
  values (previously required toggling the mode off and back on).
- Clarified the hysteresis dead-band hint text, and restored the Lenovo
  ACPI-Compliant Virtual Power Controller driver mention that was missing
  from the Spanish About text.

You need the compatible Lenovo EnergyDrv driver and .NET Framework 4.7.2 or
later (already present on Windows 10/11). The application does not install
EnergyDrv automatically.

Built and tested for x64 and x86 using simulated sensors and driver calls,
covering thresholds (including the fixed hysteresis case), mode changes,
invalid/stale readings and the English/Spanish tray and settings UI. Physical
fan response and sensor compatibility still need verification on the target
Lenovo laptop.
