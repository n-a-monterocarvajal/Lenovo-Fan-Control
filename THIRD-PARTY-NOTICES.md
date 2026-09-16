# Sensor dependencies

The optional automatic mode includes **LibreHardwareMonitorLib 0.9.6**
(MPL-2.0), unmodified, and its NuGet dependencies. Source and license:
https://github.com/LibreHardwareMonitor/LibreHardwareMonitor/tree/v0.9.6
https://github.com/LibreHardwareMonitor/LibreHardwareMonitor/blob/v0.9.6/LICENSE

Dependency versions are recorded in `sensor/packages.lock.json`.
NuGet packages retain their license declarations and source links in `.nuspec`.
The build restores these packages; the sensor library is not part of Windows.

CPU sensor access requires the **PawnIO** driver. Its official signed installer
(`PawnIO_setup.exe`, downloaded from LibreHardwareMonitor's own embedded copy at
https://github.com/LibreHardwareMonitor/LibreHardwareMonitor/tree/master/LibreHardwareMonitor.Windows.Forms/Resources)
is embedded, unmodified, in the sensor helper and is run silently
(`-install -silent`) the first time the app runs elevated and PawnIO is not
already installed. PawnIO source/license: https://github.com/namazso/PawnIO .

The Lenovo EnergyDrv driver is still required to control the fan and is not
installed automatically. PawnIO only provides sensor access in this
application; it does not replace EnergyDrv.

Merging the sensor helper's managed dependencies into a single executable at
build time uses **ILRepack.Lib.MSBuild.Task** (Apache-2.0):
https://github.com/gluck/il-repack
