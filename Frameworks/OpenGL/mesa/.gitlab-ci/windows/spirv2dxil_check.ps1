$exec_mode_to_stage = @{ Fragment = "fragment"; Vertex = "vertex"; GLCompute = "compute" }

$spvasm_files = (Get-ChildItem C:\spirv-samples\spvasm\*.spvasm) | Sort-Object Name
foreach ($spvasm in $spvasm_files) {
    $test_name = "Test:$($spvasm.Name):"
    $spvfile = ($spvasm -replace '\.spvasm$', '.spv')
    $content = Get-Content $spvasm
    $spv_version = "1.0"
    if ($content | Where-Object { $_ -match 'Version:\s(\d+\.\d+)' }) {
        $spv_version = $Matches[1]
    }
    
    $as_output = . "$env:VULKAN_SDK\Bin\spirv-as.exe" --target-env spv$spv_version --preserve-numeric-ids -o $spvfile $spvasm 2>&1 | % { if ($_ -is [System.Management.Automation.ErrorRecord]) { $_.Exception.Message } else { $_ } }  | Out-String
    if ($LASTEXITCODE -ne 0) {
        Write-Output "$test_name Skip: Unable to assemble shader"
        Write-Output "$as_output`n"
        continue
    }

    $entry_points = $content | Select-String -Pattern '^OpEntryPoint\s(\w+)[^"]+"(\w+)"' | Select-Object -ExpandProperty Matches -First 1
    if ($entry_points.Count -eq 0) {
        Write-Output "$test_name Skip"
        Write-Output "No OpEntryPoint not found`n"
        continue
    }

    foreach ($match in $entry_points) {
        $exec_mode, $entry_point = $match.Groups[1].Value, $match.Groups[2].Value
        $subtest = "$test_name$entry_point|${exec_mode}:"
        $stage = $exec_mode_to_stage[$exec_mode]
        if ($stage -eq '') {
            Write-Output "$subtest Fail: Unknown shader type ($exec_mode)"
            continue
        }
        
        $s2d_output = .\_install\bin\spirv2dxil.exe -v -e "$entry_point" -s "$stage" -o NUL $spvfile 2>&1 | ForEach-Object { if ($_ -is [System.Management.Automation.ErrorRecord]) { $_.Exception.Message } else { $_ } }  | Out-String
        if ($LASTEXITCODE -eq 0) {
            Write-Output "$subtest Pass"
        }
        else {
            Write-Output "$subtest Fail"
            $sanitized_output = $s2d_output -replace ', file .+, line \d+' -replace '    In file .+:\d+'
            Write-Output "$sanitized_output`n"
        }
    }
}
