# 0.5 — Control automático por temperatura / Automatic temperature control

## Español

- Temperaturas actuales de CPU y GPU en la parte superior del menú de la bandeja,
  junto al modo de velocidad solicitado, también en modo manual.
- Opción **Automático por temperatura** y ajuste de umbrales desde la bandeja.
  Valores iniciales: velocidad alta a 70 °C y normal al bajar a 65 °C.
- Histéresis entre ambos valores y velocidad alta si fallan o caducan las lecturas
  en modo automático. Los controles manuales siguen disponibles.
- Interfaz completa en español neutro e inglés, según el idioma de Windows.
  Se conserva el chino del proyecto original.
- Créditos del fork y del autor original en **Acerca de**.

Extrae el ZIP completo de tu arquitectura. La biblioteca LibreHardwareMonitor viene
incluida: no necesitas instalar FanControl ni Open Hardware Monitor. Se requieren
el controlador Lenovo EnergyDrv compatible, .NET Framework 4.7.2 o posterior y,
para leer la CPU, [PawnIO](https://pawnio.eu/) y ejecución como administrador.
La aplicación no instala controladores automáticamente.

Compilado y probado en x64 y x86 con sensores y controlador simulados: umbrales,
cambios de modo, lecturas inválidas/caducadas, menú y ajustes en español e inglés.
La respuesta física del ventilador y la compatibilidad de sensores aún deben
verificarse en el Lenovo de destino. Consulta `README.es.md` incluido en el ZIP.

## English

- Current CPU/GPU temperatures at the top of the tray menu, beside the requested
  fan mode, including while using manual speed controls.
- **Automatic (temperature)** and configurable thresholds in the tray menu.
  Defaults: High Speed at 70 °C; Normal Speed at or below 65 °C.
- Hysteresis and a High Speed fallback for missing/invalid/stale readings in
  automatic mode; manual controls remain available.
- Complete English and neutral Spanish UI, selected from the Windows language.
  Original Chinese support is retained.
- Original author and fork credits in **About**.

Extract the entire ZIP for your architecture. LibreHardwareMonitor is included;
the FanControl and Open Hardware Monitor applications are not required. You need
the compatible Lenovo EnergyDrv driver, .NET Framework 4.7.2 or later and, for CPU
readings, [PawnIO](https://pawnio.eu/) and administrator privileges. The application
does not install drivers automatically.

Built and tested for x64 and x86 using simulated sensors and driver calls, covering
thresholds, mode changes, invalid/stale readings and the English/Spanish tray and
settings UI. Physical fan response and sensor compatibility still need verification
on the target Lenovo laptop. See the bundled `README.md`.
