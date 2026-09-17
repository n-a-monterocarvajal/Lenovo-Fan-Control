# Control de ventilador para laptops Lenovo

Language: [English](README.en.md) | [中文](README.zh_CN.md)

---

Controla el ventilador de laptops Lenovo que tienen el controlador `Lenovo ACPI-Compliant Virtual Power Controller` en Windows.

Este proyecto es para las laptops Lenovo cuyo ventilador no se puede controlar con las aplicaciones de control de ventilador más conocidas (por ejemplo Notebook FanControl, SpeedFan, Fan Control) en Windows.

Sin embargo, este proyecto no es una solución perfecta para el control de ventilador. Solo puede hacer que el ventilador gire a velocidad mínima, máxima o normal, sin ningún control de velocidad preciso.

Este fork agrega umbrales de temperatura opcionales a la aplicación de bandeja original. Cambia entre Velocidad alta y la Velocidad normal de la laptop de forma automática, sin necesidad de instalar FanControl ni Open Hardware Monitor. Para curvas de ventilador completas, [FanControl](https://github.com/Rem0o/FanControl.Releases) y [FanControl.LenovoPlugin](https://github.com/jiarandiana0307/FanControl.LenovoPlugin) siguen siendo alternativas.

## Control automático por temperatura

1. `LenovoFanControl-x64.exe` (o `-x86.exe`) es el único ejecutable: el lector
   de sensores LibreHardwareMonitor va incrustado dentro, no es un archivo
   aparte. Se descarga directo, sin ZIP ni instalador.
2. El control manual de velocidad y el driver EnergyDrv de Lenovo no
   necesitan administrador, así que el programa abre sin pedir UAC. Leer la
   temperatura de CPU sí lo necesita: activar **Automático por temperatura**
   sin estar elevado ofrece reiniciar el programa como administrador, y en
   ese primer arranque elevado instala en silencio el
   [driver PawnIO](https://pawnio.eu/) si falta (su instalador firmado
   también va incrustado). Si eliges que no, el programa sigue sin elevar,
   pero Automático va a pedir velocidad alta por seguridad, porque la
   temperatura de CPU sigue sin estar disponible. El lector de sensores
   incrustado necesita .NET Framework 4.7.2 o posterior, ya presente en
   Windows 10/11.
3. Abre **Umbrales de temperatura...** en el menú de bandeja. Los valores
   iniciales son **70 °C** para entrar en Velocidad alta y **65 °C** para
   volver a Velocidad normal.
4. Activa **Automático por temperatura**, o inicia con `--auto`.

El menú muestra las lecturas actuales de CPU y GPU. Se prefiere CPU Package
(o la temperatura de die en AMD); si no está disponible, se usa la mayor
temperatura absoluta de CPU que haya. Para cada GPU detectada se usa su
temperatura GPU Core. El valor más alto entre CPU y GPU controla el
ventilador: cualquiera de los dos puede disparar Velocidad alta; todas las
lecturas monitoreadas deben estar en el umbral inferior o por debajo para
volver a Velocidad normal. Se excluyen hotspot de GPU, memoria de video,
discos, sensores de placa base y Distance to TjMax. Una GPU sin sensor de
núcleo compatible se muestra como no disponible; la CPU sí es obligatoria.
Si una GPU que antes se leía bien deja de responder, se trata como un fallo
de sensor.

Las dos filas informativas en la parte superior del menú de bandeja muestran
la velocidad y las temperaturas. Las temperaturas quedan visibles también en
modo manual. La interfaz usa español neutro e inglés completos, seleccionados
según el idioma de Windows, además de chino. «Acerca de» incluye los créditos
del autor original y de este fork.

Entre ambos umbrales se mantiene la velocidad anterior (histéresis).

**Iniciar con Windows**, en el menú de bandeja, agrega o quita una entrada en
el registro (Run del usuario actual), sin instalador ni tarea programada.
Arranca sin elevar, igual que un doble clic normal; el modo Automático sigue
pidiendo elevación la primera vez que necesita leer la CPU.

Los umbrales aceptan `20 ≤ Normal < Alta ≤ 100` grados Celsius. Los valores
iniciales son un punto de partida, no temperaturas máximas seguras para tu
hardware específico. Las lecturas llegan cada dos segundos aproximadamente.
Si faltan, son inválidas, o pasan diez segundos sin una lectura nueva, se
pide Velocidad alta y se muestra **Temperatura no disponible**. Al iniciar
también se pide Velocidad alta hasta que llegue una lectura válida. Después
de resolver un problema de sensor o driver, desactiva y vuelve a activar
Automático para reiniciar el lector.

Elegir cualquier velocidad manual, o su atajo de teclado, desactiva
Automático pero mantiene activa la lectura de temperaturas. Desmarcar
Automático vuelve a Velocidad normal. Los ajustes se guardan en
`%LOCALAPPDATA%\LenovoFanControl\settings.ini`; los parámetros de línea de
comandos para velocidad manual tienen prioridad sobre un Automático guardado,
solo para ese inicio. La Velocidad baja manual no tiene ninguna protección
térmica.

A diferencia del conjunto más amplio de temperaturas OHM de IdeaFan, este
modo monitorea deliberadamente solo la refrigeración de CPU/GPU. Conserva el
método de eliminación de polvo original de Lenovo y sus limitaciones:
Velocidad alta es un modo solicitado, no una medición real de las RPM del
ventilador.

## Compilación y verificación de este fork

Instala Visual Studio 2022 Build Tools con **Desktop development with C++**,
un Windows SDK y un SDK de .NET. Desde PowerShell:

```powershell
./build.ps1 -Architecture x64 -Test
./build.ps1 -Architecture x86 -Test
```

`bin/x64/LenovoFanControl-x64.exe` y `bin/x86/LenovoFanControl-x86.exe` son
los distribuibles; el lector de sensores y sus dependencias quedan
fusionados adentro, no hace falta nada más junto a ellos. Las pruebas
simulan lecturas de temperatura y llamadas al driver, y ejercitan la bandeja
y el diálogo de ajustes en español e inglés; no operan el ventilador real.
La compatibilidad de hardware todavía debe comprobarse en la laptop Lenovo
de destino. El Makefile original de MinGW solo compila la aplicación nativa;
`build.ps1` también compila y empaqueta el lector de sensores. Ver
[dependencias de terceros](THIRD-PARTY-NOTICES.md).

# Requisitos

- Laptop Lenovo
- Sistema operativo Windows
- Driver `Lenovo ACPI-Compliant Virtual Power Controller` instalado

# Uso

1. Compila este fork como se describe arriba. Las [versiones del repositorio
   original](https://github.com/jiarandiana0307/Lenovo-Fan-Control/releases)
   no incluyen la opción de temperatura automática.

2. Haz doble clic en el programa LenovoFanControl para ejecutarlo; lo vas a
   ver en la bandeja del sistema.

Si aparece un mensaje `Failed to open\\.\EnergyDrv`, significa que no se
encontró el driver de Lenovo o que no funciona como se espera. Si no aparece
ningún mensaje, el programa funciona bien y el ventilador va a girar a
velocidad máxima.

![Captura del menú](github/menu-screenshot.jpg)

Al hacer clic en el ícono del programa en la bandeja se abre un menú. La
primera línea del menú indica el estado del ventilador, que puede ser uno de
estos tres:

1. `Low Speed`: el ventilador gira a velocidad mínima.
2. `High Speed`: el ventilador gira a velocidad máxima.
3. `Normal Speed`: el ventilador gira a velocidad normal.

Puedes hacer clic en `Low Speed` y `High Speed` en el menú, o usar los atajos
`Ctrl+Alt+F10` y `Ctrl+Alt+F11`, para mantener el ventilador en velocidad
mínima y máxima respectivamente. También puedes hacer clic en `Normal Speed`
o usar el atajo `Ctrl+Alt+F12` para devolver el ventilador a su velocidad
normal.

Por último, puedes hacer clic en `Exit` en el menú para cerrar el programa; el
ventilador vuelve entonces a velocidad normal.

Para elegir la velocidad inicial del ventilador, puedes ejecutar el programa
con el parámetro de línea de comandos `--low-speed`, `--normal-speed` o
`--high-speed`, que ponen el ventilador en velocidad baja, normal o alta al
iniciar, respectivamente. El comportamiento por defecto, sin ninguno de estos
parámetros, es velocidad alta. Por ejemplo, para mantener el ventilador en
velocidad baja al iniciar, ejecuta: `LenovoFanControl-x64.exe --low-speed`

**Nota:** la `Low Speed` manual desactiva el control automático por
temperatura y puede provocar temperaturas de hardware altas. El modo
automático nunca elige Velocidad baja por su cuenta.

# Cómo funciona

Normalmente, el ventilador de una laptop lo controla el Embedded Controller
(EC), un dispositivo encargado de suministrarles a las demás partes del
sistema el voltaje que necesitan. Por eso, si el EC le pasa más voltaje al
sistema de refrigeración, el ventilador gira a velocidad máxima. Para
controlar el EC se pueden cambiar sus registros y así lograr el
comportamiento que se busca, pero en algunos modelos esos registros
dedicados al ventilador no figuran en las hojas de datos. Por eso alguien
hizo ingeniería inversa de Lenovo Energy Manager para entender cómo funciona
realmente su función de eliminación de polvo, y construyó este programa a
partir de eso. Resultó que ese software se comunica con el EC a través del
driver de kernel `Lenovo ACPI-Compliant Virtual Power Controller`. Así es
exactamente como funciona este proyecto.

Si tienes instalado el driver `Lenovo ACPI-Compliant Virtual Power
Controller`, tu sistema tiene un dispositivo `\\.\EnergyDrv`. Este
dispositivo lo crea el driver de Lenovo para exponer una interfaz de
comunicación con otras aplicaciones, como Lenovo Energy Manager. El driver
ofrece una función de eliminación de polvo que puede controlar el
ventilador. Entonces, con la API de Win32, es fácil leer y escribir bytes
específicos del dispositivo para controlar el driver de Lenovo, que a su vez
controla el EC, que a su vez controla el ventilador. Ver el diagrama abajo.

![Diagrama](github/diagram.jpg)

Pero este enfoque tiene un problema: el ventilador gira de forma
intermitente. Después de pedirle al driver que ejecute la eliminación de
polvo, el ventilador gira a velocidad máxima unos 9 segundos, se detiene 2
segundos, y así hasta que pasan 2 minutos. La eliminación de polvo la
controla el propio EC de forma automática, y a veces se detiene de golpe en
medio de esos 9 segundos, lo que deja al ventilador parado entre 1 y 9
segundos.

Como workaround, primero le pedimos al driver que ejecute la eliminación de
polvo y esperamos 9 segundos. Después le pedimos manualmente que detenga el
procedimiento, para reiniciar el conteo. Por último, le pedimos que vuelva a
empezar el procedimiento justo antes de que el ventilador se detenga.
Esperamos otros 9 segundos, lo detenemos, y así en ciclo. Con esta
alternancia rápida entre inicio y parada, el ventilador no llega a
detenerse y gira a velocidad máxima casi todo el tiempo, salvo por una breve
caída de velocidad durante cada cambio.

# Descargo de responsabilidad

Este proyecto no se hace responsable de ningún daño posible; usalo bajo tu
propio riesgo.

# Referencias

- [IdeaFan][IdeaFan]
- [FanControl][FanControl]
- [Lenovo-IdeaPad-Z500-Fan-Controller][Lenovo-IdeaPad-Z500-Fan-Controller]
- [Windows Drivers Reverse Engineering Methodology][windows-drivers-reverse-engineering-methodology]

[IdeaFan]: https://www.allstone.lt/ideafan/
[FanControl]: https://github.com/bitrate16/FanControl
[Lenovo-IdeaPad-Z500-Fan-Controller]: https://github.com/Soberia/Lenovo-IdeaPad-Z500-Fan-Controller
[windows-drivers-reverse-engineering-methodology]: https://voidsec.com/windows-drivers-reverse-engineering-methodology/
