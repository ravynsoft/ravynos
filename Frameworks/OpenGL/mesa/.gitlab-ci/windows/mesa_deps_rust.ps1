# Installing rust compiler
# Touch this file needs update both WINDOWS_X64_BUILD_TAG WINDOWS_X64_TEST_TAG
# This file needs run in administrator mode

$ProgressPreference = "SilentlyContinue"

Write-Host "Installing rust at:"
Get-Date
$url = 'https://static.rust-lang.org/rustup/dist/x86_64-pc-windows-msvc/rustup-init.exe';
Write-Host ('Downloading {0} ...' -f $url);
Invoke-WebRequest -Uri $url -OutFile 'rustup-init.exe';
Write-Host "Installing rust toolchain"
.\rustup-init.exe -y;
Remove-Item rustup-init.exe;

Write-Host "Installing rust finished at:"
Get-Date
