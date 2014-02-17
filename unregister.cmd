@echo off

CD /D %~dp0

Regsvr32.exe /u build/x64/ShellExtension.dll
