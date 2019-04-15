@echo off

REM this file generates the code needed for the ETW provider that acts as a logger

REM following are hints about modifying the ETW provider
REM 1) to change the provider, type ecmangen etwlogger.man in a Visual Studio developer command prompt.
REM 2) to add the provider to the system, administrative privileges are needed. type "wevtutil im block_storage_etw.man /rf:"d:\etw01_folder\ETW01.exe" /mf:"d:\etw01_folder\ETW01.exe". It will not work without full path name.
REM     a) errors might be due to user rights on the .exe, make sure that LocalService account has access right to the file.
REM 3) Event Viewer will list the ETW provider under "Application and Service Logs -> Microsoft-ServiceBus"

mc -um -h ../inc/azure_c_shared_utility -r ../res -v etwlogger.man