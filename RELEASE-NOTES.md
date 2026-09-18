# 0.7: iniciar siempre como administrador / always run as administrator

## Español

- Nueva opción **Iniciar siempre como administrador** en el menú de bandeja,
  junto a **Iniciar con Windows**. Guarda la preferencia en `settings.ini` y,
  en cada arranque, relanza el programa elevado conservando los parámetros de
  línea de comandos (`--auto`, `--low-speed`, etc.). Sirve si usas
  **Automático por temperatura** a diario y no quieres activarlo cada vez
  después de abrir el programa.
- Windows sigue mostrando el diálogo de UAC en cada arranque con la opción
  activa: recordar el permiso exigiría una tarea programada, que este
  programa no instala.
- Limpieza interna sin cambios visibles: se quitó código muerto (bucles de
  control previos al hilo del ventilador, el Makefile de MinGW y el modo
  `--diagnose` del lector de sensores) y los textos de la interfaz quedaron en
  una sola tabla.

Se requieren el controlador Lenovo EnergyDrv compatible y .NET Framework
4.7.2 o posterior (ya presente en Windows 10/11). La aplicación no instala
EnergyDrv automáticamente.

Compilado y probado en x64 y x86 con sensores y controlador simulados. La
respuesta física del ventilador, el flujo real de elevación y la
compatibilidad de sensores aún deben verificarse en el Lenovo de destino.

## English

- New **Always run as administrator** option in the tray menu, next to
  **Start with Windows**. It stores the preference in `settings.ini` and
  relaunches the program elevated on every start, keeping command-line
  parameters (`--auto`, `--low-speed`, etc.). Useful if you use
  **Automatic (temperature)** daily and don't want to turn it on after each
  launch.
- Windows still shows the UAC dialog on every start while the option is on:
  remembering the grant would need a scheduled task, which this program does
  not install.
- Internal cleanup with no visible changes: dead code was removed (control
  loops that predate the fan worker thread, the MinGW Makefile, and the
  sensor helper's `--diagnose` mode), and the UI strings now live in a
  single table.

You need the compatible Lenovo EnergyDrv driver and .NET Framework 4.7.2 or
later (already present on Windows 10/11). The application does not install
EnergyDrv automatically.

Built and tested for x64 and x86 using simulated sensors and driver calls.
Physical fan response, the actual elevation flow, and sensor compatibility
still need verification on the target Lenovo laptop.
