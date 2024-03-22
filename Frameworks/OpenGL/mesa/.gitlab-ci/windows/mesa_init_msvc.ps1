$vsInstallPath=& "C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -version 16.0  -property installationpath
Write-Output "vswhere.exe installPath: $vsInstallPath"
$vsInstallPath =  if ("$vsInstallPath" -eq "" ) { "C:\BuildTools" } else { "$vsInstallPath" }
Write-Output "Final installPath: $vsInstallPath"
Import-Module (Join-Path $vsInstallPath "Common7\Tools\Microsoft.VisualStudio.DevShell.dll")
# https://en.wikipedia.org/wiki/Microsoft_Visual_C%2B%2B
# VS2015 14.0 use -vcvars_ver=14.0
# VS2017 14.1 to 14.16 use -vcvars_ver=14.16
# VS2019 14.20 to 14.29 use -vcvars_ver=14.29
# VS2022 14.30 to 14.38 (not finished yet) use -vcvars_ver=14 to choose the newest version

$vcvars_ver_arg=$args
if ($null -eq $vcvars_ver_arg[0]) {
  $vcvars_ver_arg="-vcvars_ver=14.29"
}

Enter-VsDevShell -VsInstallPath $vsInstallPath -SkipAutomaticLocation -DevCmdArguments "$vcvars_ver_arg -arch=x64 -no_logo -host_arch=amd64"
