# Download new TLS certs from Windows Update
Write-Host "Updating TLS certificate store at:"
Get-Date
Remove-Item -Recurse -Force -ErrorAction SilentlyContinue "_tlscerts" | Out-Null
$certdir = (New-Item -ItemType Directory -Name "_tlscerts")
certutil -syncwithWU "$certdir"
Foreach ($file in (Get-ChildItem -Path "$certdir\*" -Include "*.crt")) {
  Import-Certificate -FilePath $file -CertStoreLocation Cert:\LocalMachine\Root | Out-Null
}
Remove-Item -Recurse -Path $certdir

Write-Host "Installing graphics tools (DirectX debug layer) at:"
Get-Date
Set-Service -Name wuauserv -StartupType Manual
if (!$?) {
  Write-Host "Failed to enable Windows Update"
  Exit 1
}

For ($i = 0; $i -lt 5; $i++) {
  Dism /online /quiet /add-capability /capabilityname:Tools.Graphics.DirectX~~~~0.0.1.0
  $graphics_tools_installed = $?
  if ($graphics_tools_installed) {
    Break
  }
}

if (!$graphics_tools_installed) {
  Write-Host "Failed to install graphics tools"
  Get-Content C:\Windows\Logs\DISM\dism.log
  Exit 1
}

Write-Host "Installing Chocolatey at:"
Get-Date
Invoke-Expression ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
Import-Module "$env:ProgramData\chocolatey\helpers\chocolateyProfile.psm1"
# Add Chocolatey's native install path
Update-SessionEnvironment
Write-Host "Installing Chocolatey packages at:"
Get-Date

# Chocolatey tries to download winflexbison3 from github, which is not super reliable, and has no retry
# loop of its own - so we give it a helping hand here
For ($i = 0; $i -lt 5; $i++) {
  choco install --no-progress -y python3
  $python_install = $?
  choco install --allow-empty-checksums --no-progress -y cmake git git-lfs ninja pkgconfiglite winflexbison3 --installargs "ADD_CMAKE_TO_PATH=System"
  $other_install = $?
  $choco_installed = $other_install -and $python_install
  if ($choco_installed) {
    Break
  }
}

if (!$choco_installed) {
  Write-Host "Couldn't install dependencies from Chocolatey"
  Exit 1
}

# Add Chocolatey's newly installed package path
Update-SessionEnvironment

Start-Process -NoNewWindow -Wait git -ArgumentList 'config --global core.autocrlf false'

Write-Host "Upgrading pip at:"
Get-Date
python -m pip install --upgrade pip --progress-bar off
Write-Host "Installing python packages at:"
Get-Date
pip3 install packaging meson mako numpy --progress-bar off
if (!$?) {
  Write-Host "Failed to install dependencies from pip"
  Exit 1
}
Write-Host "Installing python packages finished at:"
Get-Date
