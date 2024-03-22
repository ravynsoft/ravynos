. .\_install\spirv2dxil_check.ps1 2>&1 | Set-Content -Path .\spirv2dxil_results.txt
$reference = Get-Content .\_install\spirv2dxil_reference.txt
$result = Get-Content .\spirv2dxil_results.txt
if (-Not ($reference -And $result)) {
    Exit 1
}
  
$diff = Compare-Object -ReferenceObject $reference -DifferenceObject $result
if (-Not $diff) {
    Exit 0
}

Write-Host "Unexpected change in results:"
Write-Output $diff | Format-Table -Property SideIndicator, InputObject -Wrap

Exit 1
