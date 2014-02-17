<h2>Shell Extension</h2>

Here you can find an extension for context menu of Microsoft Windows OS.

<h4>Installation</h4>

<<<<<<< HEAD
To install/uninstall you may use register.cmd/unregister.cmd if you use x64 OS. Otherwise navigate to build folder,
=======
To install/uninstall you may use <b>register.cmd/unregister.cmd</b> if you use x64 OS. Otherwise navigate to build folder,
>>>>>>> 708489154a0bcdd7dba436aca7ea7b7864dc43c0
choose your version and type in command line: "Regsvr32.exe ShellExtension.dll". To uninstall use the same command
with "/u".

<h4>Usage</h4>

After installation select a group of files and choose "Calculate the sum" in context menu. There will appear "log.txt"
file, where calculated checksums are stored.

Boost library is used to calculate checksums (even though it's not so fast).

