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
& cl.exe /nologo /std:c11 /utf-8 /W4 /D_CRT_SECURE_NO_WARNINGS /O2 /MT /Fo"$out\\" /Fe"$out\LenovoFanControl-$Architecture.exe" src/lenovo_fan_control.c src/fanctrl.c src/fan_worker.c src/auto_control.c src/temperature.c "$out\icon.res" "$out\sensor.res" /link /SUBSYSTEM:WINDOWS /MANIFEST:EMBED "/MANIFESTUAC:level='requireAdministrator' uiAccess='false'" user32.lib shell32.lib advapi32.lib
Check-Exit
Remove-Item "$out\TemperatureMonitor.exe","$out\TemperatureMonitor.exe.config","$out\TemperatureMonitor.pdb","$out\sensor.rc","$out\sensor.res" -ErrorAction SilentlyContinue
Copy-Item LICENSE,README.md,README.es.md,THIRD-PARTY-NOTICES.md $out
$licenseDir = Join-Path $out 'licenses'
New-Item -ItemType Directory -Force $licenseDir | Out-Null
Copy-Item licenses/* $licenseDir
$lock = Get-Content sensor/packages.lock.json -Raw | ConvertFrom-Json
foreach ($dependency in $lock.dependencies.'.NETFramework,Version=v4.7.2'.PSObject.Properties) {
    if ($dependency.Name -like 'Microsoft.NETFramework.ReferenceAssemblies*') { continue }
    $packageDir = Join-Path '.packages' "$($dependency.Name.ToLowerInvariant())/$($dependency.Value.resolved)"
    Copy-Item "$packageDir/*.nuspec" $licenseDir
    Get-ChildItem $packageDir -File -Filter '*license*' | ForEach-Object {
        Copy-Item $_.FullName (Join-Path $licenseDir "$($dependency.Name)-$($_.Name)")
    }
}
if ($Test) {
    $tests = Join-Path $PSScriptRoot 'test-results'
    New-Item -ItemType Directory -Force $tests | Out-Null
    & cl.exe /nologo /std:c11 /utf-8 /W4 /WX /MT /Fo"$tests\\" /Fe"$tests\auto_control_test.exe" tests/auto_control_test.c src/auto_control.c
    Check-Exit
    & "$tests\auto_control_test.exe"
    Check-Exit
    & cl.exe /nologo /std:c11 /utf-8 /W4 /WX /MT /Fo"$tests\\" /Fe"$tests\fan_worker_test.exe" tests/fan_worker_test.c src/fan_worker.c
    Check-Exit
    & "$tests\fan_worker_test.exe"
    Check-Exit
    & cl.exe /nologo /std:c11 /utf-8 /W4 /WX /MT /Fo"$tests\\" /Fe"$tests\TemperatureMonitor.exe" tests/fake_sensor.c
    Check-Exit
    & cl.exe /nologo /std:c11 /utf-8 /W4 /WX /D_CRT_SECURE_NO_WARNINGS /MT /Fo"$tests\\" /Fe"$tests\temperature_test.exe" tests/temperature_test.c src/temperature.c
    Check-Exit
    & "$tests\temperature_test.exe"
    Check-Exit
    & cl.exe /nologo /std:c11 /utf-8 /W4 /WX /D_CRT_SECURE_NO_WARNINGS /MT /Fo"$tests\\" /Fe"$tests\tray_ui_test.exe" tests/tray_ui_test.c src/fan_worker.c src/auto_control.c src/temperature.c "$out\icon.res" /link user32.lib shell32.lib advapi32.lib
    Check-Exit
    & "$tests\tray_ui_test.exe"
    Check-Exit
}
$packageFiles = @(Get-ChildItem $out -File | Where-Object {
    $_.Extension -in '.exe', '.dll', '.config', '.md' -or $_.Name -eq 'LICENSE'
} | Select-Object -ExpandProperty FullName) + @($licenseDir)
Compress-Archive -Path $packageFiles -DestinationPath "bin/LenovoFanControl-v0.5-$Architecture.zip" -Force
Write-Host "Built $out"
