@ECHO OFF
IF "%PLATFORM%" == "x64" (
	cmake . -G "Visual Studio 14 2015 Win64" -DUSE_QT4=ON -DUSE_QT5=OFF
) ELSE (
	cmake . -G "Visual Studio 14 2015" -DUSE_QT4=ON -DUSE_QT5=OFF
)
