[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12

if (-not (Test-Path cmake-$Env:CMAKE_VERSION-win64-x64)) {
    Invoke-WebRequest `
        -Uri http://github.com/Kitware/CMake/releases/download/v$Env:CMAKE_VERSION/cmake-$Env:CMAKE_VERSION-win64-x64.zip `
        -OutFile cmake-$Env:CMAKE_VERSION-win64-x64.zip
    Expand-Archive cmake-$Env:CMAKE_VERSION-win64-x64.zip -DestinationPath .
}
$Env:Path="$Env:CI_PROJECT_DIR\cmake-$Env:CMAKE_VERSION-win64-x64\bin;$Env:Path"

if (-not (Test-Path 7za.exe)) {
    Invoke-WebRequest `
        -Uri https://www.7-zip.org/a/7z1900-extra.7z `
        -OutFile 7z1900-extra.7z
    cmake -E tar xf 7z1900-extra.7z 7za.exe
}

if (-not (Test-Path libxml2-build/xmlconf)) {
    Invoke-WebRequest `
        -Uri https://www.w3.org/XML/Test/xmlts20080827.tar.gz `
        -OutFile xmlts20080827.tar.gz ;
    .\7za.exe x xmlts20080827.tar.gz -olibxml2-build
}

cmake `
    -DBUILD_SHARED_LIBS="$Env:BUILD_SHARED_LIBS" `
    -DCMAKE_INSTALL_PREFIX=libxml2-install `
    -DLIBXML2_WITH_ICONV=OFF `
    -DLIBXML2_WITH_LZMA=OFF `
    -DLIBXML2_WITH_PYTHON=OFF `
    -DLIBXML2_WITH_ZLIB=OFF `
    -S . -B libxml2-build
cmake --build libxml2-build --config Debug --target install
cmake --build libxml2-build --config Release --target install
New-Item -ItemType Directory libxml2-install\share\libxml2
Copy-Item Copyright libxml2-install\share\libxml2

cd libxml2-build
ctest -C Debug -VV
if ($LastExitCode -ne 0) {
    throw "ctest failed"
}
ctest -C Release -VV
if ($LastExitCode -ne 0) {
    throw "ctest failed"
}
cd ..

.\7za.exe a libxml2-$Env:CI_COMMIT_SHORT_SHA-$Env:CMAKE_GENERATOR_TOOLSET-$Env:CMAKE_GENERATOR_PLATFORM-$Env:SUFFIX.7z .\libxml2-install\*
