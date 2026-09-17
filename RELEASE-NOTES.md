# 0.6.1: elevación bajo demanda, no todo el tiempo / on-demand elevation, not always

## Español

0.6 pedía UAC en cada apertura del programa, incluso para el control manual de
ventilador, que nunca lo necesitó (el driver EnergyDrv de Lenovo funciona sin
administrador; solo la lectura de temperatura de CPU vía PawnIO lo necesita).
Esta versión corrige eso:

- El programa abre sin pedir UAC, igual que la versión 0.4 original.
- Activar **Automático por temperatura** sin estar elevado ofrece reiniciar
  el programa como administrador. Si aceptas, en ese arranque se instala en
  silencio el controlador [PawnIO](https://pawnio.eu/) si falta. Si rechazas,
  el programa sigue sin elevar y Automático pide velocidad alta por
  seguridad, porque la CPU sigue sin poder leerse.
- **Iniciar con Windows** también arranca sin elevar; la elevación se pide
  solo cuando activas Automático.
- El README en español ahora es una traducción completa del original
  (requisitos, uso manual, cómo funciona el driver EnergyDrv, límites y
  referencias), no solo un resumen de los cambios del fork. Pasa a ser el
  README principal del repositorio, con enlaces a las versiones en inglés y
  chino.

Se requieren el controlador Lenovo EnergyDrv compatible y .NET Framework
4.7.2 o posterior (ya presente en Windows 10/11). La aplicación no instala
EnergyDrv automáticamente.

Compilado y probado en x64 y x86 con sensores y controlador simulados. La
respuesta física del ventilador, el flujo real de elevación y la
compatibilidad de sensores aún deben verificarse en el Lenovo de destino.

## English

0.6 asked for UAC every time the program opened, even for manual fan
control, which never needed it (Lenovo's EnergyDrv driver works without
Administrator; only CPU temperature reading via PawnIO needs it). This
version fixes that:

- The program opens without a UAC prompt, matching the original 0.4.
- Turning on **Automatic (temperature)** while unelevated offers to restart
  the program as Administrator. Accepting silently installs the
  [PawnIO](https://pawnio.eu/) driver if it's missing. Declining keeps the
  app unelevated, and Automatic then requests High Speed for safety, since
  CPU temperature stays unreadable.
- **Start with Windows** also launches unelevated; elevation is only
  requested once you turn Automatic on.
- The Spanish README is now a full translation of the original (prerequisites,
  manual usage, how the EnergyDrv driver works, limitations, references),
  not just a summary of the fork's changes. It's now the repository's main
  README, linking to the English and Chinese versions.

You need the compatible Lenovo EnergyDrv driver and .NET Framework 4.7.2 or
later (already present on Windows 10/11). The application does not install
EnergyDrv automatically.

Built and tested for x64 and x86 using simulated sensors and driver calls.
Physical fan response, the actual elevation flow, and sensor compatibility
still need verification on the target Lenovo laptop.
