param (
    [Parameter(Mandatory=$true)][string]$SourceDir,
    [Parameter(Mandatory=$true)][string]$OutputDir
)

Import-Module "$PSScriptRoot\..\..\shared.psm1"

# application
$TargetDir = Join-Path -Path $OutputDir -ChildPath "$PathDistRuntimePackages\Introspection"

Write-Host "Package Introspection: Copy Library to '$TargetDir'"
Install-Files -Path (Join-Path -Path $OutputDir -ChildPath "Introspection.dll") -Destination $TargetDir


Write-Host "Package Introspection: Copy Scripts to '$TargetDir'"

Copy-Files -SourceDir (Join-Path -Path $SourceDir -ChildPath "src")` -TargetDir $TargetDir -Pattern "*.ds"


# debug
$TargetDir = Join-Path -Path $OutputDir -ChildPath $PathDistPdbPackages
Write-Host "Package Introspection Debug: Copy PDBs to '$TargetDir'"

Install-Files -Path (Join-Path -Path $OutputDir -ChildPath "Introspection.pdb") -Destination $TargetDir
