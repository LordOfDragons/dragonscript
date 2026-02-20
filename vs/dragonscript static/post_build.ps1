param (
    [Parameter(Mandatory=$true)][string]$SourceDir,
    [Parameter(Mandatory=$true)][string]$OutputDir
)

Import-Module "$PSScriptRoot\..\shared.psm1"


# sdk
$TargetDir = Join-Path -Path $OutputDir -ChildPath $PathDistLib
Write-Host "DragonScript SDK: Copy Static Libraries to '$TargetDir'"

Install-Files -Path (Join-Path -Path $OutputDir -ChildPath "libdscript-static.lib") -Destination $TargetDir


$TargetDir = Join-Path -Path $OutputDir -ChildPath $PathDistPdb
Write-Host "DragonScript SDK: Copy PDBs to '$TargetDir'"

Install-Files -Path (Join-Path -Path $OutputDir -ChildPath "libdscript-static.pdb") -Destination $TargetDir
