# 0.7.2: aspecto actual de Windows / current Windows look

## Español

- Los diálogos y mensajes (umbrales de temperatura, pregunta de elevación,
  «Acerca de») usan ahora los controles de Windows 10/11: botones
  redondeados, cajas de texto actuales e íconos nuevos. Antes el programa
  no tenía manifiesto y Windows mostraba los controles de la época de
  Windows 95.
- El programa declara soporte de DPI por monitor. En pantallas con escala
  mayor al 100 % los diálogos deberían verse nítidos en vez de borrosos;
  esto no se probó en esas pantallas.
- La pregunta de elevación ya no corta líneas a mitad de frase, y en español
  termina en: «…, y el modo Automático va a mantener la velocidad alta de
  forma predeterminada».

Se requieren el controlador Lenovo EnergyDrv compatible y .NET Framework
4.7.2 o posterior (ya presente en Windows 10/11). La aplicación no instala
EnergyDrv automáticamente.

Compilado y probado en x64 y x86 con sensores y controlador simulados.

## English

- Dialogs and message boxes (temperature thresholds, elevation prompt,
  About) now use Windows 10/11 controls: rounded buttons, current text
  boxes, and new icons. The program previously had no manifest, so Windows
  showed Windows 95 era controls.
- The program declares per-monitor DPI awareness. On displays scaled above
  100% the dialogs should look sharp instead of blurry; this was not tested
  on such displays.
- The elevation prompt no longer breaks lines mid-sentence, and now ends
  with: "…, and Automatic mode will keep High Speed by default."

You need the compatible Lenovo EnergyDrv driver and .NET Framework 4.7.2 or
later (already present on Windows 10/11). The application does not install
EnergyDrv automatically.

Built and tested for x64 and x86 using simulated sensors and driver calls.
