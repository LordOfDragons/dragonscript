param (
    [Parameter(Mandatory=$true)][string]$SourceDir,
    [Parameter(Mandatory=$true)][string]$OutputDir
)

Import-Module "$PSScriptRoot\..\..\shared.psm1"

# application
$TargetDir = Join-Path -Path $OutputDir -ChildPath "$PathDistRuntimePackages\Math"

Write-Host "Package Math: Copy Library to '$TargetDir'"
Install-Files -Path (Join-Path -Path $OutputDir -ChildPath "Math.dll") -Destination $TargetDir


# debug
$TargetDir = Join-Path -Path $OutputDir -ChildPath $PathDistPdbPackages
Write-Host "Package Math Debug: Copy PDBs to '$TargetDir'"

Install-Files -Path (Join-Path -Path $OutputDir -ChildPath "Math.pdb") -Destination $TargetDir
