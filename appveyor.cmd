@ECHO OFF
IF "%PLATFORM%" == "x64" (
	cmake . -G "Visual Studio 12 2013 Win64" -DQt5_DIR=C:/Qt/5.10.1/msvc2013_64 -DQt5LinguistTools_DIR=C:/Qt/5.10.1/msvc2013_64
) ELSE (
	cmake . -G "Visual Studio 12 2013" -DQt5_DIR=C:/Qt/5.7/msvc2013 -DQt5LinguistTools_DIR=C:/Qt/5.7/msvc2013
)
