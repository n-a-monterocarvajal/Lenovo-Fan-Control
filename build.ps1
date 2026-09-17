param([ValidateSet('x64', 'x86')][string]$Architecture = 'x64', [switch]$Test)
$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (!$vs) { throw 'Install Visual Studio Build Tools with Desktop development with C++.' }
$envLines = & cmd.exe /d /c "`"$vs\Common7\Tools\VsDevCmd.bat`" -arch=$Architecture -host_arch=x64 >nul && set"
foreach ($line in $envLines) {
    if ($line -match '^([^=]+)=(.*)$') { [Environment]::SetEnvironmentVariable($matches[1], $matches[2], 'Process') }
}
function Check-Exit { if ($LASTEXITCODE -ne 0) { throw "Build command failed ($LASTEXITCODE)." } }
$out = Join-Path $PSScriptRoot "bin\$Architecture"
New-Item -ItemType Directory -Force $out | Out-Null
& rc.exe /nologo /i res /fo "$out\icon.res" res/icon.rc
Check-Exit
& dotnet build sensor/TemperatureMonitor.csproj -c Release --packages .packages -p:RuntimeIdentifier="win-$Architecture" -p:PlatformTarget=$Architecture -o "$out\"
Check-Exit
# Embed the (ILRepack-merged, single-file) sensor exe into the main exe as an RCDATA
# resource, so the release ships as one file instead of two (matches the pre-sensor packaging).
$sensorExe = (Resolve-Path "$out\TemperatureMonitor.exe").Path -replace '\\', '\\\\'
Set-Content -Path "$out\sensor.rc" -Value "109 RCDATA ""$sensorExe"""
& rc.exe /nologo /fo "$out\sensor.res" "$out\sensor.rc"
Check-Exit
& cl.exe /nologo /std:c11 /utf-8 /W4 /D_CRT_SECURE_NO_WARNINGS /O2 /MT /Fo"$out\\" /Fe"$out\LenovoFanControl-$Architecture.exe" src/lenovo_fan_control.c src/fanctrl.c src/fan_worker.c src/auto_control.c src/temperature.c "$out\icon.res" "$out\sensor.res" /link /SUBSYSTEM:WINDOWS /MANIFEST:NO user32.lib shell32.lib advapi32.lib
Check-Exit
Remove-Item "$out\TemperatureMonitor.exe","$out\TemperatureMonitor.exe.config","$out\TemperatureMonitor.pdb","$out\sensor.rc","$out\sensor.res" -ErrorAction SilentlyContinue
if ($Test) {
    $tests = Join-Path $PSScriptRoot 'test-results'
    New-Item -ItemType Directory -Force $tests | Out-Null
    # fake_sensor is built as TemperatureMonitor.exe for temperature_test; it is not a test itself.
    $suites = @(
        @{ Name = 'auto_control_test'; Args = @('tests/auto_control_test.c', 'src/auto_control.c') },
        @{ Name = 'fan_worker_test'; Args = @('tests/fan_worker_test.c', 'src/fan_worker.c') },
        @{ Name = 'TemperatureMonitor'; Args = @('tests/fake_sensor.c'); Fixture = $true },
        @{ Name = 'temperature_test'; Args = @('tests/temperature_test.c', 'src/temperature.c') },
        @{ Name = 'tray_ui_test'; Args = @('tests/tray_ui_test.c', 'src/fan_worker.c', 'src/auto_control.c', 'src/temperature.c', "$out\icon.res", '/link', 'user32.lib', 'shell32.lib', 'advapi32.lib') }
    )
    foreach ($suite in $suites) {
        & cl.exe /nologo /std:c11 /utf-8 /W4 /WX /D_CRT_SECURE_NO_WARNINGS /MT /Fo"$tests\\" /Fe"$tests\$($suite.Name).exe" @($suite.Args)
        Check-Exit
        if (!$suite.Fixture) { & "$tests\$($suite.Name).exe"; Check-Exit }
    }
}
Write-Host "Built $out\LenovoFanControl-$Architecture.exe"
