param (
    [Parameter(Mandatory=$true)][string]$SourceDir,
    [Parameter(Mandatory=$true)][string]$OutputDir
)

Import-Module "$PSScriptRoot\..\shared.psm1"


# application
$TargetDir = Join-Path -Path $OutputDir -ChildPath $PathDistBin

Write-Host "DSI App: Copy Library to '$TargetDir'"
Install-Files -Path (Join-Path -Path $OutputDir -ChildPath "dsi.exe") -Destination $TargetDir


# debug
$TargetDir = Join-Path -Path $OutputDir -ChildPath $PathDistPdb
Write-Host "DSI Debug: Copy PDBs to '$TargetDir'"

Install-Files -Path (Join-Path -Path $OutputDir -ChildPath "dsi.pdb") -Destination $TargetDir
