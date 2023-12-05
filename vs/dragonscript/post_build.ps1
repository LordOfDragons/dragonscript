param (
    [Parameter(Mandatory=$true)][string]$SourceDir,
    [Parameter(Mandatory=$true)][string]$OutputDir
)

Import-Module "$PSScriptRoot\..\shared.psm1"

# build
$TargetDir = Join-Path -Path $OutputDir -ChildPath "include\libdscript"
if (Test-Path $TargetDir) {
    Remove-Item $TargetDir -Force -Recurse
}

Write-Host "DragonScript: Copy Headers to '$TargetDir'"
Copy-Files -SourceDir $SourceDir -TargetDir $TargetDir -Pattern "*.h"


# sdk
$TargetDir = Join-Path -Path $OutputDir -ChildPath $PathDistBin

Write-Host "DragonScript SDK: Copy Library to '$TargetDir'"
Install-Files -Path (Join-Path -Path $OutputDir -ChildPath "libdscript.dll") -Destination $TargetDir


$TargetDir = Join-Path -Path $OutputDir -ChildPath "$PathDistInc\libdscript"
if (Test-Path $TargetDir) {
    Remove-Item $TargetDir -Force -Recurse
}

Write-Host "DragonScript SDK: Copy Headers to '$TargetDir'"
Copy-Files -SourceDir $SourceDir -TargetDir $TargetDir -Pattern "*.h"


$TargetDir = Join-Path -Path $OutputDir -ChildPath $PathDistLib
Write-Host "DragonScript SDK: Copy Libraries to '$TargetDir'"

Install-Files -Path (Join-Path -Path $OutputDir -ChildPath "libdscript.lib") -Destination $TargetDir
Install-Files -Path (Join-Path -Path $OutputDir -ChildPath "libdscript.exp") -Destination $TargetDir


$TargetDir = Join-Path -Path $OutputDir -ChildPath $PathDistPdb
Write-Host "DragonScript SDK: Copy PDBs to '$TargetDir'"

Install-Files -Path (Join-Path -Path $OutputDir -ChildPath "libdscript.pdb") -Destination $TargetDir
