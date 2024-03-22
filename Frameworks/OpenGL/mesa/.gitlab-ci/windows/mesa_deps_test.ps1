# Compiling tests deps

$ProgressPreference = "SilentlyContinue"
$MyPath = $MyInvocation.MyCommand.Path | Split-Path -Parent
. "$MyPath\mesa_init_msvc.ps1"

$spirv_samples_source = Join-Path -Path "$PWD" -ChildPath "spirv-samples"

Write-Host "Cloning spirv-samples at:"
Get-Date
New-Item -ItemType Directory -Path "$spirv_samples_source" | Out-Null
Push-Location -Path $spirv_samples_source
git init
git remote add origin https://github.com/dneto0/spirv-samples.git
git fetch --depth 1 origin 36372636df06a24c4e2de1551beee055db01b91d # of branch main
if (!$?) {
  Write-Host "Failed to fetch spirv-samples repository"
  Pop-Location
  Exit 1
}
git checkout FETCH_HEAD
Pop-Location
Remove-Item -Recurse -Force -ErrorAction SilentlyContinue -Path "$spirv_samples_source\.git" | Out-Null

Write-Host "Cloning spirv-samples finished at:"
Get-Date

Write-Host "Complete Dockerfile_test at:"
Get-Date
