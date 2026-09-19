# 0.7.1: tooltip sin parpadeo y grados enteros / flicker-free tooltip, whole degrees

## Español

- El tooltip del ícono de bandeja ya no parpadea al dejar el mouse encima en
  modo Automático. El programa lo actualizaba cada segundo aunque el texto no
  cambiara, y Windows 11 lo cerraba y reabría con cada actualización. Ahora
  solo se actualiza cuando cambia el estado o la temperatura.
- Las temperaturas se muestran en grados enteros (por ejemplo,
  `CPU: 52 °C / GPU: 43 °C`). Los sensores de CPU y GPU reportan grados
  enteros, así que el decimal siempre era `.0`.

Se requieren el controlador Lenovo EnergyDrv compatible y .NET Framework
4.7.2 o posterior (ya presente en Windows 10/11). La aplicación no instala
EnergyDrv automáticamente.

La corrección del tooltip se verificó en una Lenovo con Windows 11. Las dos
arquitecturas (x64 y x86) pasan las pruebas con sensores y controlador
simulados.

## English

- The tray icon tooltip no longer flickers while hovering in Automatic mode.
  The program updated it every second even when the text was unchanged, and
  Windows 11 closed and reopened it on each update. It now updates only when
  the state or temperature changes.
- Temperatures are shown in whole degrees (for example,
  `CPU: 52 °C / GPU: 43 °C`). The CPU and GPU sensors report whole degrees,
  so the decimal was always `.0`.

You need the compatible Lenovo EnergyDrv driver and .NET Framework 4.7.2 or
later (already present on Windows 10/11). The application does not install
EnergyDrv automatically.

The tooltip fix was verified on a Lenovo laptop running Windows 11. Both
architectures (x64 and x86) pass the tests with simulated sensors and driver
calls.
