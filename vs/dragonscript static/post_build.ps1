param (
    [Parameter(Mandatory=$true)][string]$SourceDir,
    [Parameter(Mandatory=$true)][string]$OutputDir
)

Import-Module "$PSScriptRoot\..\shared.psm1"

# sdk
$TargetDir = Join-Path -Path $OutputDir -ChildPath "$PathDistSdkInc\dscript"
if (Test-Path $TargetDir) {
    Remove-Item $TargetDir -Force -Recurse
}

$TargetDir = Join-Path -Path $OutputDir -ChildPath $PathDistSdkLib
Write-Host "DragonScript-Static SDK: Copy Libraries to '$TargetDir'"

Install-Files -Path (Join-Path -Path $OutputDir -ChildPath "libdscript-static.lib") -Destination $TargetDir


# debug
$TargetDir = Join-Path -Path $OutputDir -ChildPath $PathDistPdb
Write-Host "DragonScript-Static Debug: Copy PDBs to '$TargetDir'"

Install-Files -Path (Join-Path -Path $OutputDir -ChildPath "libdscript-static.pdb") -Destination $TargetDir
