# Compiling Piglit

$ProgressPreference = "SilentlyContinue"
$MyPath = $MyInvocation.MyCommand.Path | Split-Path -Parent
. "$MyPath\mesa_init_msvc.ps1"

$source_dir = Join-Path -Path "$PWD" -ChildPath "src"
$waffle_source = Join-Path -Path "$source_dir" -ChildPath "waffle"
$waffle_install = Join-Path -Path "$PWD" -ChildPath "waffle"
$piglit_source = Join-Path -Path "$PWD" -ChildPath "Piglit"

Write-Host "Cloning Waffle at:"
Get-Date
New-Item -ItemType Directory -Path "$waffle_source" | Out-Null
Push-Location -Path $waffle_source
git init
git remote add origin https://gitlab.freedesktop.org/mesa/waffle.git
git fetch --depth 1 origin 950a1f35a718bc2a8e1dda75845e52651bb331a7 # of branch master
if (!$?) {
  Write-Host "Failed to fetch Waffle repository"
  Pop-Location
  Exit 1
}
git checkout FETCH_HEAD
Pop-Location

Write-Host "Compiling Waffle at:"
Get-Date
$waffle_build = Join-Path -Path "$source_dir" -ChildPath "waffle\build"
New-Item -ItemType Directory -Path "$waffle_build" | Out-Null
Push-Location -Path $waffle_build
meson setup `
--buildtype=release `
--default-library=static `
--prefix="$waffle_install" && `
ninja -j32 install
if (!$?) {
  Write-Host "Failed to compile or install Waffle"
  Pop-Location
  Exit 1
}
Pop-Location
Remove-Item -Recurse -Force -ErrorAction SilentlyContinue -Path "$waffle_build" | Out-Null

Write-Host "Downloading glext.h at:"
Get-Date
New-Item -ItemType Directory -Path "$source_dir\glext\GL" | Out-Null
Invoke-WebRequest -Uri 'https://github.com/KhronosGroup/OpenGL-Registry/raw/main/api/GL/glext.h' -OutFile "$source_dir\glext\GL\glext.h" | Out-Null

Write-Host "Cloning Piglit at:"
Get-Date
New-Item -ItemType Directory -Path "$piglit_source" | Out-Null
Push-Location -Path $piglit_source
git init
git remote add origin https://gitlab.freedesktop.org/mesa/piglit.git
git fetch --depth 1 origin b41accc83689966f91217fc5b57dbe06202b8c8c # of branch main
if (!$?) {
  Write-Host "Failed to fetch Piglit repository"
  Pop-Location
  Exit 1
}
git checkout FETCH_HEAD

Write-Host "Compiling Piglit at:"
Get-Date
cmake -S . -B . `
-GNinja `
-DCMAKE_BUILD_TYPE=Release `
-DPIGLIT_USE_WAFFLE=ON `
-DWaffle_INCLUDE_DIRS="$waffle_install\include\waffle-1" `
-DWaffle_LDFLAGS="$waffle_install\lib\libwaffle-1.a" `
-DGLEXT_INCLUDE_DIR="$source_dir\glext" && `
ninja -j32
if (!$?) {
  Write-Host "Failed to compile Piglit"
  Pop-Location
  Exit 1
}
Pop-Location

$depsInstallPath="C:\mesa-deps"
$piglit_bin = "$piglit_source\bin"

# Hard link Agility SDK into subfolder of piglit
$agility_dest = New-Item -ItemType Directory -Path $piglit_bin -Name 'D3D12'
New-Item -ItemType HardLink -path $agility_dest\D3D12Core.dll -Value $depsInstallPath\bin\D3D12\D3D12Core.dll
New-Item -ItemType HardLink -path $agility_dest\d3d12SDKLayers.dll -Value $depsInstallPath\bin\D3D12\d3d12SDKLayers.dll

# Hard link WARP next to piglit
New-Item -ItemType HardLink -path $piglit_bin\d3d10warp.dll -Value $depsInstallPath\bin\d3d10warp.dll

Remove-Item -Recurse -Force -ErrorAction SilentlyContinue -Path "$source_dir" | Out-Null
Remove-Item -Recurse -Force -ErrorAction SilentlyContinue -Path "$waffle_install" | Out-Null

# Cleanup piglit intermediate files
Get-ChildItem -Force -ErrorAction SilentlyContinue -Recurse "$piglit_source" | Where-Object {
  if($_.FullName -match "CMake|.git|.lib"){
    Remove-Item -Recurse -Force -ErrorAction SilentlyContinue -Path $_.FullName | Out-Null
  }
}

Write-Host "Compiling Piglit finished at:"
Get-Date
