
@echo off

echo == Copying Textures...
xcopy Textures\*.* Working\Textures\ /D /Y

echo == Copying Shaders...
xcopy Src\Shaders\*.* Working\Shaders\ /D /Y

echo == Copying Models...
xcopy MaxAssets\Export\*.* Working\Models\ /D /Y

pause
