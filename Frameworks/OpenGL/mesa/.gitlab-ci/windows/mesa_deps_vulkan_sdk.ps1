# Downloading specified vulkan sdk and vulkan runtime
# Touch this file needs update both WINDOWS_X64_BUILD_TAG WINDOWS_X64_TEST_TAG
# This file needs run in administrator mode

$env:VULKAN_SDK_VERSION="1.3.211.0"

$ProgressPreference = "SilentlyContinue"

# Save environment VULKAN_SDK_VERSION to system
[System.Environment]::SetEnvironmentVariable('VULKAN_SDK_VERSION', "$env:VULKAN_SDK_VERSION", [System.EnvironmentVariableTarget]::Machine)

$VULKAN_SDK_URL="https://sdk.lunarg.com/sdk/download/$env:VULKAN_SDK_VERSION/windows/VulkanSDK-$env:VULKAN_SDK_VERSION-Installer.exe"
Write-Host "Downloading Vulkan-SDK $VULKAN_SDK_URL at:"
Get-Date
Invoke-WebRequest -Uri "$VULKAN_SDK_URL" -OutFile "${env:TMP}\vulkan_sdk.exe" | Out-Null
Write-Host "Installing Vulkan-SDK at:"
Get-Date
Start-Process -NoNewWindow -Wait "${env:TMP}\vulkan_sdk.exe" -ArgumentList "--am --al -c in"
if (!$?) {
    Write-Host "Failed to install Vulkan SDK"
    Exit 1
}
Remove-Item "${env:TMP}\vulkan_sdk.exe" -Force

$VULKAN_RUNTIME_URL="https://sdk.lunarg.com/sdk/download/$env:VULKAN_SDK_VERSION/windows/VulkanRT-$env:VULKAN_SDK_VERSION-Installer.exe"
Write-Host "Downloading Vulkan-Runtime $VULKAN_RUNTIME_URL at:"
Get-Date
Invoke-WebRequest -Uri "$VULKAN_RUNTIME_URL" -OutFile "${env:TMP}\vulkan-runtime.exe" | Out-Null
Write-Host "Installing Vulkan-Runtime at:"
Get-Date
Start-Process -NoNewWindow -Wait "${env:TMP}\vulkan-runtime.exe" -ArgumentList '/S'
if (!$?) {
  Write-Host "Failed to install Vulkan-Runtime"
  Exit 1
}
Remove-Item "${env:TMP}\vulkan-runtime.exe" -Force

Write-Host "Installing Vulkan-Runtime finished at:"
Get-Date
