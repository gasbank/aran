
@echo off

echo Prerequisites...
xcopy ..\..\Prerequisites\*.* ..\..\Working\ /D /Y

echo Textures...
xcopy ..\..\Textures\*.* ..\..\Working\Textures\ /D /Y

echo Shaders...
xcopy ..\..\Src\Shaders\*.* ..\..\Working\Shaders\ /D /Y

echo Models...
xcopy ..\..\MaxAssets\Export\*.* ..\..\Working\Models\ /D /Y

