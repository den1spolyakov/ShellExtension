@echo off

CD /D %~dp0

Regsvr32.exe build/x64/ShellExtension.dll
