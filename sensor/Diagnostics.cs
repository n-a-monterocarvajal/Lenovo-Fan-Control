using System;
using System.Globalization;
using System.Security.Principal;
using System.Threading;
using LibreHardwareMonitor.Hardware;

internal static class Diagnostics
{
    internal static bool IsAdministrator()
    {
        using (WindowsIdentity identity = WindowsIdentity.GetCurrent())
            return new WindowsPrincipal(identity).IsInRole(WindowsBuiltInRole.Administrator);
    }

    private static void PrintHardware(IHardware hardware)
    {
        hardware.Update();
        Console.WriteLine("Hardware: {0} | {1}", hardware.HardwareType, hardware.Name);
        foreach (ISensor sensor in hardware.Sensors)
            if (sensor.SensorType == SensorType.Temperature)
                Console.WriteLine("  {0}: {1} C", sensor.Name, sensor.Value.HasValue
                    ? sensor.Value.Value.ToString("F1", CultureInfo.InvariantCulture) : "unavailable");
        foreach (IHardware child in hardware.SubHardware) PrintHardware(child);
    }

    internal static int Run()
    {
        var computer = new Computer { IsCpuEnabled = true, IsGpuEnabled = true };
        try
        {
            Console.WriteLine("Administrator: {0}", IsAdministrator());
            Console.WriteLine("64-bit process: {0}", Environment.Is64BitProcess);
            Console.WriteLine("LibreHardwareMonitor: {0}", typeof(Computer).Assembly.GetName().Version);
            Console.WriteLine("PawnIO installed: {0}; version: {1}",
                LibreHardwareMonitor.PawnIo.PawnIo.IsInstalled, LibreHardwareMonitor.PawnIo.PawnIo.Version);
            computer.Open();
            for (int i = 0; i < 3; ++i)
            {
                Console.WriteLine("Sample {0}", i + 1);
                foreach (IHardware hardware in computer.Hardware) PrintHardware(hardware);
                if (i < 2) Thread.Sleep(1000);
            }
            return 0;
        }
        catch (Exception ex) { Console.Error.WriteLine(ex); return 1; }
        finally { computer.Close(); }
    }
}
