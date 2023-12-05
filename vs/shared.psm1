# Shared Functions Module
###########################


# Install files to directory
##############################

function Install-Files {
    param (
        [Parameter(Mandatory=$true)][string]$Path,
        [Parameter(Mandatory=$true)][string]$Destination,
        [Parameter(Mandatory=$false)][string]$Name
    )

    $Path = Resolve-Path $Path

    if (!(Test-Path $Destination)) {
        New-Item -ItemType Directory $Destination -ErrorAction SilentlyContinue | Out-Null
    }

    if (!$Name) {
        $Name = Split-Path -Path $Path -Leaf
    }

    Copy-Item -Path $Path -Destination (Join-Path -Path $Destination -ChildPath $Name)
}


# Copy files to directory
function Copy-Files {
    param (
        [Parameter(Mandatory=$true)][string]$SourceDir,
        [Parameter(Mandatory=$true)][string]$TargetDir,
        [Parameter(Mandatory=$true)][string]$Pattern
    )

    $SourceDir = Resolve-Path $SourceDir

    if (!(Test-Path $TargetDir)) {
        New-Item -ItemType Directory $TargetDir -ErrorAction SilentlyContinue | Out-Null
    }

    $CutLength = $SourceDir.Length + 1

    Get-ChildItem -Path (Join-Path -Path $SourceDir -ChildPath $Pattern) -Recurse | ForEach-Object {
        $RelativePath = $_.FullName.Substring($CutLength)
        $TargetPath = Join-Path -Path $TargetDir -ChildPath $RelativePath
        $ParentPath = Split-Path -Path $TargetPath -Parent
        # Write-Host $RelativePath
        if (!(Test-Path $ParentPath)) {
            New-Item -ItemType Directory $ParentPath -ErrorAction SilentlyContinue | Out-Null
        }
        Copy-Item -Path $_.FullName -Destination (Join-Path -Path $TargetDir -ChildPath $RelativePath)
    }
}


# Various path constants
##########################

New-Variable -Name PathDist -Value "Distribute\SDK" -Scope Global -Option ReadOnly -Force
New-Variable -Name PathDistRuntime -Value "$PathDist\dsinstall" -Scope Global -Option ReadOnly -Force
New-Variable -Name PathDistRuntimePackages -Value "$PathDistRuntime" -Scope Global -Option ReadOnly -Force
New-Variable -Name PathDistBin -Value "$PathDist\bin" -Scope Global -Option ReadOnly -Force
New-Variable -Name PathDistInc -Value "$PathDist\include" -Scope Global -Option ReadOnly -Force
New-Variable -Name PathDistLib -Value "$PathDist\lib" -Scope Global -Option ReadOnly -Force
New-Variable -Name PathDistPdb -Value "$PathDist\pdb" -Scope Global -Option ReadOnly -Force
New-Variable -Name PathDistPdbPackages -Value "$PathDistPdb\packages" -Scope Global -Option ReadOnly -Force
