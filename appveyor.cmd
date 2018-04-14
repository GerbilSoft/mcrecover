@ECHO OFF
IF "%PLATFORM%" == "x64" (
	cmake . -G "Visual Studio 12 2013 Win64" -DCMAKE_PREFIX_PATH=C:/Qt/5.7/msvc2013_64
) ELSE (
	cmake . -G "Visual Studio 12 2013" -DCMAKE_PREFIX_PATH=C:/Qt/5.7/msvc2013
)
