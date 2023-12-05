param (
    [Parameter(Mandatory=$true)][string]$SourceDir
)

Import-Module "$PSScriptRoot\..\shared.psm1"

Write-Host "DragonScript: Init configuration header"

$Content = Get-Content -Raw -Path "dragonscript_config.h"
#$Content = $Content -creplace "%BuildVersion%","$BuildVersion"
Set-Content -Path (Join-Path -Path $SourceDir -ChildPath "dragonscript_config.h") -Value $Content
