# Lenovo Fan Control 0.5

Fork de [jiarandiana0307/Lenovo-Fan-Control](https://github.com/jiarandiana0307/Lenovo-Fan-Control),
con activación automática de la velocidad alta según la temperatura.

Toda la operación se realiza desde el icono del área de notificaciones de Windows.
Las dos primeras líneas del menú muestran el modo de velocidad solicitado y las
temperaturas actuales de CPU y GPU, también en modo manual. No son una medición de RPM.

## Instalación y uso

1. Extrae **todo** el ZIP de tu arquitectura a una carpeta. Conserva el programa,
   `TemperatureMonitor.exe` y las DLL juntos.
2. El equipo debe ser compatible con el controlador **Lenovo ACPI-Compliant Virtual
   Power Controller (EnergyDrv)**. Para leer la CPU, instala una vez el controlador
   [PawnIO](https://pawnio.eu/). La biblioteca de sensores LibreHardwareMonitor viene
   incluida; no necesitas instalar las aplicaciones FanControl ni OHM. Se requiere
   .NET Framework 4.7.2 o posterior.
3. Ejecuta LenovoFanControl **como administrador** y abre su menú desde la bandeja.
4. En **Umbrales de temperatura...**, ajusta la temperatura para activar la velocidad
   alta (70 °C inicialmente) y para volver a la velocidad normal (65 °C inicialmente).
5. Activa **Automático por temperatura**. También puedes iniciar con `--auto`.

Si cualquiera de las temperaturas supera o alcanza el umbral alto, se solicita
velocidad alta. Cuando ambas bajan al umbral inferior o menos, se devuelve el control
normal al equipo. Entre ambos valores se mantiene el estado anterior. Los valores
son configurables: `20 ≤ Normal < Alta ≤ 100 °C`; no representan límites universales
de seguridad para el hardware.

Se usa CPU Package o la temperatura de die de AMD cuando está disponible; en su
defecto, la mayor temperatura absoluta de CPU disponible. Para GPU se usa GPU Core.
Se excluyen hotspot, memoria gráfica, discos, placa base y distancia a TjMax. Si hay
varias CPU/GPU se muestra el máximo de cada grupo. La CPU debe ser legible. Una GPU
sin sensor compatible se muestra como no disponible; si un sensor de GPU que estaba
funcionando deja de responder, se considera un fallo de lectura.

Las lecturas se actualizan aproximadamente cada dos segundos. En modo automático,
si faltan datos válidos o pasan diez segundos sin una lectura, se solicita velocidad
alta y se indica el problema. Si faltan temperaturas, comprueba PawnIO, los permisos
de administrador y los archivos del paquete. Desactiva y vuelve a activar Automático
para reiniciar el lector tras corregir el problema.

Elegir una velocidad manual o su atajo desactiva el modo automático, pero mantiene
la lectura de temperaturas. La velocidad baja manual **no** tiene protección por
umbral. Al desactivar Automático desde el menú se vuelve a la velocidad normal.
Al salir se solicita velocidad normal. Los ajustes se guardan en
`%LOCALAPPDATA%\LenovoFanControl\settings.ini`.

La interfaz usa español neutro en Windows configurado en español e inglés en los
demás idiomas, conservando también la traducción al chino. «Acerca de» incluye los
créditos del proyecto original, del fork y de la biblioteca de sensores.

## Compilación y verificación

Consulta [README.md](README.md#build-and-test-this-fork). `build.ps1 -Test` compila,
ejecuta pruebas con sensores y controlador simulados, y genera el ZIP distribuible.
Estas pruebas no verifican la refrigeración física del equipo: la compatibilidad
real debe comprobarse en el Lenovo de destino.

El programa conserva el método original de eliminación de polvo de Lenovo y sus
limitaciones. Úsalo bajo tu responsabilidad. [Dependencias y licencias](THIRD-PARTY-NOTICES.md).
