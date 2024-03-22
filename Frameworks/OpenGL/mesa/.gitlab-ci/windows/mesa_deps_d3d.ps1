# Downloading specified D3D runtime
# Touch this file needs update both WINDOWS_X64_BUILD_TAG WINDOWS_X64_TEST_TAG
# This file needs run in administrator mode

$ProgressPreference = "SilentlyContinue"

$depsInstallPath="C:\mesa-deps"

Write-Host "Downloading DirectX 12 Agility SDK at:"
Get-Date
Invoke-WebRequest -Uri https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.610.2 -OutFile 'agility.zip'
Expand-Archive -Path 'agility.zip' -DestinationPath 'C:\agility'
# Copy Agility SDK into mesa-deps\bin\D3D12
New-Item -ErrorAction SilentlyContinue -ItemType Directory -Path $depsInstallPath\bin -Name 'D3D12'
Copy-Item 'C:\agility\build\native\bin\x64\*.dll' -Destination $depsInstallPath\bin\D3D12
Remove-Item 'agility.zip'
Remove-Item -Recurse 'C:\agility'

Write-Host "Downloading Updated WARP at:"
Get-Date
Invoke-WebRequest -Uri https://www.nuget.org/api/v2/package/Microsoft.Direct3D.WARP/1.0.7.1 -OutFile 'warp.zip'
Expand-Archive -Path 'warp.zip' -DestinationPath 'C:\warp'
# Copy WARP into mesa-deps\bin
Copy-Item 'C:\warp\build\native\amd64\d3d10warp.dll' -Destination $depsInstallPath\bin
Remove-Item 'warp.zip'
Remove-Item -Recurse 'C:\warp'

Write-Host "Downloading DirectXShaderCompiler release at:"
Get-Date
Invoke-WebRequest -Uri https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.7.2207/dxc_2022_07_18.zip -OutFile 'DXC.zip'
Expand-Archive -Path 'DXC.zip' -DestinationPath 'C:\DXC'
# No more need to get dxil.dll from the VS install
Copy-Item 'C:\DXC\bin\x64\*.dll' -Destination 'C:\Windows\System32'
Remove-Item -Recurse 'DXC.zip'
Remove-Item -Recurse 'C:\DXC'

Write-Host "Enabling developer mode at:"
Get-Date
# Create AppModelUnlock if it doesn't exist, required for enabling Developer Mode
$RegistryKeyPath = "HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\AppModelUnlock"
if (-not(Test-Path -Path $RegistryKeyPath)) {
    New-Item -Path $RegistryKeyPath -ItemType Directory -Force
}

# Add registry value to enable Developer Mode
New-ItemProperty -Path $RegistryKeyPath -Name AllowDevelopmentWithoutDevLicense -PropertyType DWORD -Value 1 -Force

Write-Host "Complete download D3D at:"
Get-Date
