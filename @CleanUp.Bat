@Echo Off

rem -------------

attrib *.* -r -h -s +a
attrib AutoUpdate\*.* -r -h -s +a
attrib SQLEClient\*.* -r -h -s +a
attrib SQLEServer\*.* -r -h -s +a
attrib ServiceAgent\*.* -r -h -s +a
attrib ServiceAgent\win32\*.* -r -h -s +a
attrib ServiceAgent\x64\*.* -r -h -s +a
attrib zLib\*.* -r -h -s +a
attrib Bin\win32\*.* -r -h -s +a
attrib Bin\x64\*.* -r -h -s +a

del *.suo
del *.user
del *.ncb
del *.APS
Del *.sdf /Q

rem -------------

del "Bin\*.*" /Q
rd "Setup Files\Output" /S /Q

rem -------------

Del Bin\win32\*.* /Q
Del Bin\x64\*.* /Q

rem -------------

rd AutoUpdate\Win32 /S /Q
rd AutoUpdate\x64 /S /Q

rd SQLEClient\Win32 /S /Q
rd SQLEClient\x64 /S /Q

rd SQLEServer\Win32 /S /Q
rd SQLEServer\x64 /S /Q

rd ServiceAgent\Win32 /S /Q
rd ServiceAgent\x64 /S /Q

rd Bin\Win32
rd Bin\x64
rd Bin

rem -------------

del AutoUpdate\*.suo
del AutoUpdate\*.user
del AutoUpdate\*.ncb
del AutoUpdate\Resources\*.APS
Del AutoUpdate\*.sdf /Q

del SQLEClient\*.suo
del SQLEClient\*.user
del SQLEClient\*.ncb
del SQLEClient\Resources\*.APS
Del SQLEClient\*.sdf /Q

del SQLEServer\*.suo
del SQLEServer\*.user
del SQLEServer\*.ncb
del SQLEServer\Resources\*.APS
Del SQLEServer\*.sdf /Q

del ServiceAgent\*.suo
del ServiceAgent\*.user
del ServiceAgent\*.ncb
del ServiceAgent\Resources\*.APS
Del ServiceAgent\*.sdf /Q

rem -------------
