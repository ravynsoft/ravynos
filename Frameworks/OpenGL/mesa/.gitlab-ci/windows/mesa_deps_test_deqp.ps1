# Compiling deqp

$ProgressPreference = "SilentlyContinue"
$MyPath = $MyInvocation.MyCommand.Path | Split-Path -Parent
. "$MyPath\mesa_init_msvc.ps1"

$source_dir = Join-Path -Path "$PWD" -ChildPath "src"
$deqp_source = Join-Path -Path "$source_dir" -ChildPath "VK-GL-CTS"
$deqp_build = Join-Path -Path "$PWD" -ChildPath "deqp"

Write-Host "Cloning Vulkan and GL Conformance Tests at:"
Get-Date
New-Item -ItemType Directory -Path "$deqp_source" | Out-Null
Push-Location -Path $deqp_source
git init
git remote add origin https://github.com/KhronosGroup/VK-GL-CTS.git
git fetch --depth 1 origin 56114106d860c121cd6ff0c3b926ddc50c4c11fd # of branch vulkan-cts-1.3.4
if (!$?) {
  Write-Host "Failed to fetch deqp repository"
  Pop-Location
  Exit 1
}
git checkout FETCH_HEAD

Write-Host "Fetch sources inside $deqp_source at:"
Get-Date
# --insecure is due to SSL cert failures hitting sourceforge for zlib and
# libpng (sigh).  The archives get their checksums checked anyway, and git
# always goes through ssh or https.
py .\external\fetch_sources.py --insecure
Pop-Location

Write-Host "Compiling deqp at:"
Get-Date
New-Item -ItemType Directory -Path "$deqp_build" | Out-Null
Push-Location -Path $deqp_build
cmake -S $($deqp_source) `
-B . `
-GNinja `
-DCMAKE_BUILD_TYPE=Release `
-DDEQP_TARGET=default && `
ninja -j32
if (!$?) {
  Write-Host "Failed to compile deqp"
  Pop-Location
  Exit 1
}
Pop-Location

# Copy test result templates
Copy-Item -Path "$($deqp_source)\doc\testlog-stylesheet\testlog.css" -Destination $deqp_build
Copy-Item -Path "$($deqp_source)\doc\testlog-stylesheet\testlog.xsl" -Destination $deqp_build

# Copy Vulkan must-pass list
$deqp_mustpass = New-Item -ItemType Directory -Path $deqp_build -Name "mustpass"
$root_mustpass = Join-Path -Path $deqp_source -ChildPath "external\vulkancts\mustpass\main"
$files = Get-Content "$($root_mustpass)\vk-default.txt"
foreach($file in $files) {
  Get-Content "$($root_mustpass)\$($file)" | Add-Content -Path "$($deqp_mustpass)\vk-main.txt"
}

Write-Host "Installing deqp-runner at:"
Get-Date
$env:Path += ";$($env:USERPROFILE)\.cargo\bin"
cargo install --git https://gitlab.freedesktop.org/anholt/deqp-runner.git --tag v0.16.1

$depsInstallPath="C:\mesa-deps"
$vk_cts_bin = "$deqp_build\external\vulkancts\modules\vulkan"

# Hard link Agility SDK into subfolder of Vulkan CTS
$agility_dest = New-Item -ItemType Directory -Path $vk_cts_bin -Name 'D3D12'
New-Item -ItemType HardLink -path $agility_dest\D3D12Core.dll -Value $depsInstallPath\bin\D3D12\D3D12Core.dll
New-Item -ItemType HardLink -path $agility_dest\d3d12SDKLayers.dll -Value $depsInstallPath\bin\D3D12\d3d12SDKLayers.dll

# Hard link WARP next to Vulkan CTS
New-Item -ItemType HardLink -path $vk_cts_bin\d3d10warp.dll -Value $depsInstallPath\bin\d3d10warp.dll

Remove-Item -Recurse -Force -ErrorAction SilentlyContinue -Path "$source_dir" | Out-Null

# Cleanup deqp intermediate files
Get-ChildItem -Force -ErrorAction SilentlyContinue -Recurse "$deqp_build" | Where-Object {
  if($_.FullName -match "CMake|.git|.lib"){
    Remove-Item -Recurse -Force -ErrorAction SilentlyContinue -Path $_.FullName | Out-Null
  }
}

Write-Host "Compiling deqp finished at:"
Get-Date
