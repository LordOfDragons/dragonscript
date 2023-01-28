param (
    [Parameter(Mandatory=$true)][string]$SourceDir,
    [Parameter(Mandatory=$true)][string]$OutputDir
)

Import-Module "$PSScriptRoot\..\..\shared.psm1"

# application
$TargetDir = Join-Path -Path $OutputDir -ChildPath "$PathDistDataPackages\Utils"

#Write-Host "Package Utils: Copy Library to '$TargetDir'"
#Install-Files -Path (Join-Path -Path $OutputDir -ChildPath "Utils.dll") -Destination $TargetDir


Write-Host "Package Utils: Copy Scripts to '$TargetDir'"

Copy-Files -SourceDir (Join-Path -Path $SourceDir -ChildPath "src")` -TargetDir $TargetDir -Pattern "*.ds"


# debug
#$TargetDir = Join-Path -Path $OutputDir -ChildPath $PathDistPdbPackages
#Write-Host "Package Utils Debug: Copy PDBs to '$TargetDir'"

#Install-Files -Path (Join-Path -Path $OutputDir -ChildPath "Utils.pdb") -Destination $TargetDir
