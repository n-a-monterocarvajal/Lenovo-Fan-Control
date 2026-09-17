using System;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Reflection;
using System.Security.Principal;
using System.Threading;
using LibreHardwareMonitor.Hardware;
using Microsoft.Win32.SafeHandles;

internal static class Program
{
    private static bool IsAdministrator()
    {
        using (WindowsIdentity identity = WindowsIdentity.GetCurrent())
            return new WindowsPrincipal(identity).IsInRole(WindowsBuiltInRole.Administrator);
    }

    /// <summary>
    /// PawnIO's driver is what lets LibreHardwareMonitor read CPU temperature (MSR access).
    /// It ships as an embedded installer so the sensor stays a single file: silently
    /// install it once, as the fan control app already runs elevated for EnergyDrv.
    /// </summary>
    private static void EnsurePawnIoInstalled()
    {
        if (LibreHardwareMonitor.PawnIo.PawnIo.IsInstalled || !IsAdministrator()) return;
        string installerPath = Path.Combine(Path.GetTempPath(), "PawnIO_setup.exe");
        try
        {
            using (Stream resource = Assembly.GetExecutingAssembly().GetManifestResourceStream("PawnIO_setup.exe"))
            using (FileStream file = File.Create(installerPath))
                resource.CopyTo(file);
            using (Process installer = Process.Start(new ProcessStartInfo(installerPath, "-install -silent")
            {
                UseShellExecute = false,
                CreateNoWindow = true
            }))
                installer.WaitForExit(30000);
        }
        catch { /* CPU temperature simply stays unavailable; GPU/fan control are unaffected. */ }
        finally { try { File.Delete(installerPath); } catch { } }
    }

    private static float? Sample(IHardware hardware, bool cpu)
    {
        hardware.Update();
        float? hottest = null, package = null;
        foreach (ISensor sensor in hardware.Sensors)
        {
            // Headroom is a distance to a limit, not an actual temperature.
            if (sensor.SensorType != SensorType.Temperature ||
                sensor.Name.IndexOf("TjMax", StringComparison.OrdinalIgnoreCase) >= 0 ||
                sensor.Name.IndexOf("distance", StringComparison.OrdinalIgnoreCase) >= 0)
                continue;
            if (!cpu && sensor.Name != "GPU Core") continue;
            float? value = sensor.Value;
            if (!(value >= 0 && value <= 150)) continue;
            if (!hottest.HasValue || value > hottest) hottest = value;
            if (sensor.Name == "CPU Package" || sensor.Name == "Core (Tctl/Tdie)" ||
                sensor.Name == "Core (Tdie)") package = value;
        }
        foreach (IHardware child in hardware.SubHardware)
        {
            float? value = Sample(child, cpu);
            if (value.HasValue && (!hottest.HasValue || value > hottest)) hottest = value;
        }
        return package ?? hottest;
    }

    private static int Main(string[] args)
    {
        ulong handle, parentHandle;
        if (args.Length != 2 || !ulong.TryParse(args[0], out handle) ||
            !ulong.TryParse(args[1], out parentHandle)) return 1;
        using (var stop = new EventWaitHandle(false, EventResetMode.ManualReset))
        using (var parent = new EventWaitHandle(false, EventResetMode.ManualReset))
        {
            stop.SafeWaitHandle = new SafeWaitHandle(new IntPtr(unchecked((long)handle)), true);
            parent.SafeWaitHandle = new SafeWaitHandle(new IntPtr(unchecked((long)parentHandle)), true);
            var waits = new WaitHandle[] { stop, parent };
            var computer = new Computer { IsCpuEnabled = true, IsGpuEnabled = true };
            var monitoredGpus = new System.Collections.Generic.HashSet<string>();
            try
            {
                EnsurePawnIoInstalled();
                computer.Open();
                do
                {
                    float? cpu = null, gpu = null;
                    bool missing = false;
                    int gpuReadings = 0;
                    try
                    {
                        foreach (IHardware hardware in computer.Hardware)
                        {
                            bool isCpu = hardware.HardwareType == HardwareType.Cpu;
                            float? value = Sample(hardware, isCpu);
                            if (isCpu)
                            {
                                if (!value.HasValue) missing = true;
                                else if (!cpu.HasValue || value > cpu) cpu = value;
                            }
                            else if (value.HasValue)
                            {
                                monitoredGpus.Add(hardware.Identifier.ToString());
                                ++gpuReadings;
                                if (!gpu.HasValue || value > gpu) gpu = value;
                            }
                        }
                        missing |= !cpu.HasValue || gpuReadings < monitoredGpus.Count;
                        Console.WriteLine(missing ? "unavailable" : string.Format(
                            CultureInfo.InvariantCulture, "{0:F1} {1:F1}", cpu, gpu ?? -1));
                    }
                    catch { Console.WriteLine("unavailable"); }
                    Console.Out.Flush();
                } while (WaitHandle.WaitAny(waits, 2000) == WaitHandle.WaitTimeout);
                return 0;
            }
            catch { return 1; }
            finally { computer.Close(); }
        }
    }
}
