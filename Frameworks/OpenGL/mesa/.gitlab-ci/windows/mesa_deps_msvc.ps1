# we want more secure TLS 1.2 for most things
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12;

# VS17.x is 2022
$msvc_url = 'https://aka.ms/vs/17/release/vs_buildtools.exe'

Write-Host "Downloading Visual Studio 2022 build tools at:"
Get-Date
Invoke-WebRequest -Uri $msvc_url -OutFile C:\vs_buildtools.exe

Write-Host "Installing Visual Studio 2022 at:"
Get-Date
# Command line
# https://docs.microsoft.com/en-us/visualstudio/install/command-line-parameter-examples?view=vs-2022
# Component ids
# https://docs.microsoft.com/en-us/visualstudio/install/workload-component-id-vs-build-tools?view=vs-2022
# https://docs.microsoft.com/en-us/visualstudio/install/workload-component-id-vs-community?view=vs-2022
Start-Process -NoNewWindow -Wait -FilePath C:\vs_buildtools.exe `
-ArgumentList `
"--wait", `
"--quiet", `
"--norestart", `
"--nocache", `
"--installPath", "C:\BuildTools", `
"--add", "Microsoft.VisualStudio.Component.VC.Redist.14.Latest", `
"--add", "Microsoft.VisualStudio.Component.VC.ASAN", ` # MSVC 2022
"--add", "Microsoft.VisualStudio.Component.VC.ATL", `
"--add", "Microsoft.VisualStudio.Component.VC.ATLMFC", `
"--add", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64", `
"--add", "Microsoft.VisualStudio.Component.VC.14.29.16.11.ATL", ` # MSVC 2019
"--add", "Microsoft.VisualStudio.Component.VC.14.29.16.11.MFC", `
"--add", "Microsoft.VisualStudio.ComponentGroup.VC.Tools.142.x86.x64", `
"--add", "Microsoft.VisualStudio.Component.VC.Llvm.Clang", `
"--add", "Microsoft.VisualStudio.Component.Graphics.Tools", `
"--add", "Microsoft.VisualStudio.Component.Windows10SDK.20348"

if (!$?) {
  Write-Host "Failed to install Visual Studio tools"
  Exit 1
}
Remove-Item C:\vs_buildtools.exe -Force

Write-Host "Installing Visual Studio 2022 finished at:"
Get-Date

Exit 0
