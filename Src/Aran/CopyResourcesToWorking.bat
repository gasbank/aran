
@echo off

echo Prerequisites...
xcopy ..\..\Prerequisites\*.* . /D /Y

echo Textures...
xcopy ..\..\Textures\*.* Textures\ /D /Y

echo Shaders...
xcopy ..\Shaders\*.* Shaders\ /D /Y

echo Models...
xcopy ..\..\MaxAssets\Export\*.* Models\ /D /Y

pause
