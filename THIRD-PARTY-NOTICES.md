# Sensor dependencies

The optional automatic mode includes **LibreHardwareMonitorLib 0.9.6**
(MPL-2.0), unmodified, and its NuGet dependencies. Source and license:
https://github.com/LibreHardwareMonitor/LibreHardwareMonitor/tree/v0.9.6
https://github.com/LibreHardwareMonitor/LibreHardwareMonitor/blob/v0.9.6/LICENSE

Dependency versions are recorded in `sensor/packages.lock.json`.
NuGet packages retain their license declarations and source links in `.nuspec`.
The build restores these packages; the sensor library is not part of Windows.

CPU sensor access requires the **PawnIO** driver, installed separately once
from https://pawnio.eu/ (official releases: https://github.com/namazso/PawnIO.Setup).
It is not installed automatically and is not bundled in this source repository.
PawnIO source/license: https://github.com/namazso/PawnIO .

The Lenovo EnergyDrv driver is still required to control the fan. PawnIO only
provides sensor access in this application; it does not replace EnergyDrv.
