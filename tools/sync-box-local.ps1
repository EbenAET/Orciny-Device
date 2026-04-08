[CmdletBinding(SupportsShouldProcess = $true)]
param(
    [ValidateSet('Audit', 'BoxToRepo', 'RepoToBox')]
    [string]$Mode = 'Audit',

    [string]$BoxPath = (Join-Path ([Environment]::GetFolderPath('UserProfile')) 'Box\Orciny Device'),

    [string]$RepoPath = (Split-Path -Parent $PSScriptRoot),

    [string]$ReportPath,

    [switch]$DeleteDestinationOnly,

    [switch]$PruneLegacyDuplicates
)

$ErrorActionPreference = 'Stop'

$ignoredFileNames = @('.DS_Store', 'Thumbs.db')
$legacyDuplicatePaths = @(
    'circuit\Adafruit_CAD_Parts',
    'circuit\Adafruit-Eagle-Library',
    'circuit\orciny_blocks-backups',
    'circuit\orciny_device-backups'
)

function Resolve-NormalizedPath {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path
    )

    return [System.IO.Path]::GetFullPath((Resolve-Path -LiteralPath $Path).Path)
}

function Test-IsLegacyDuplicatePath {
    param(
        [Parameter(Mandatory = $true)]
        [string]$RelativePath
    )

    foreach ($prefix in $legacyDuplicatePaths) {
        if ($RelativePath.Equals($prefix, [System.StringComparison]::OrdinalIgnoreCase)) {
            return $true
        }

        if ($RelativePath.StartsWith($prefix + '\', [System.StringComparison]::OrdinalIgnoreCase)) {
            return $true
        }
    }

    return $false
}

function Get-SyncFileMap {
    param(
        [Parameter(Mandatory = $true)]
        [string]$RootPath
    )

    $rootLength = $RootPath.Length
    $map = @{}

    Get-ChildItem -LiteralPath $RootPath -Recurse -File -Force |
        Where-Object {
            $_.FullName -notmatch '\\.git(\\|$)' -and
            $ignoredFileNames -notcontains $_.Name
        } |
        ForEach-Object {
            $relativePath = $_.FullName.Substring($rootLength).TrimStart('\\')
            if (Test-IsLegacyDuplicatePath -RelativePath $relativePath) {
                return
            }

            $map[$relativePath] = [pscustomobject]@{
                RelativePath = $relativePath
                FullName = $_.FullName
                LastWriteUtc = $_.LastWriteTimeUtc
                Size = $_.Length
            }
        }

    return $map
}

function Get-SyncPlan {
    param(
        [Parameter(Mandatory = $true)]
        [string]$SourcePath,

        [Parameter(Mandatory = $true)]
        [string]$DestinationPath
    )

    $sourceMap = Get-SyncFileMap -RootPath $SourcePath
    $destinationMap = Get-SyncFileMap -RootPath $DestinationPath
    $allRelativePaths = @($sourceMap.Keys + $destinationMap.Keys | Sort-Object -Unique)

    $sourceOnly = [System.Collections.Generic.List[object]]::new()
    $destinationOnly = [System.Collections.Generic.List[object]]::new()
    $sourceNewer = [System.Collections.Generic.List[object]]::new()
    $destinationNewer = [System.Collections.Generic.List[object]]::new()
    $same = [System.Collections.Generic.List[object]]::new()
    $conflicts = [System.Collections.Generic.List[object]]::new()

    foreach ($relativePath in $allRelativePaths) {
        $sourceItem = $sourceMap[$relativePath]
        $destinationItem = $destinationMap[$relativePath]

        if ($sourceItem -and -not $destinationItem) {
            $sourceOnly.Add($sourceItem)
            continue
        }

        if ($destinationItem -and -not $sourceItem) {
            $destinationOnly.Add($destinationItem)
            continue
        }

        if ($sourceItem.LastWriteUtc -gt $destinationItem.LastWriteUtc) {
            $sourceNewer.Add([pscustomobject]@{
                RelativePath = $relativePath
                Source = $sourceItem
                Destination = $destinationItem
            })
            continue
        }

        if ($sourceItem.LastWriteUtc -lt $destinationItem.LastWriteUtc) {
            $destinationNewer.Add([pscustomobject]@{
                RelativePath = $relativePath
                Source = $sourceItem
                Destination = $destinationItem
            })
            continue
        }

        if ($sourceItem.Size -eq $destinationItem.Size) {
            $same.Add([pscustomobject]@{
                RelativePath = $relativePath
                Source = $sourceItem
                Destination = $destinationItem
            })
            continue
        }

        $conflicts.Add([pscustomobject]@{
            RelativePath = $relativePath
            Source = $sourceItem
            Destination = $destinationItem
        })
    }

    return [pscustomobject]@{
        SourcePath = $SourcePath
        DestinationPath = $DestinationPath
        SourceOnly = $sourceOnly
        DestinationOnly = $destinationOnly
        SourceNewer = $sourceNewer
        DestinationNewer = $destinationNewer
        Same = $same
        Conflicts = $conflicts
    }
}

function Copy-SyncItem {
    param(
        [Parameter(Mandatory = $true)]
        [object]$SourceItem,

        [Parameter(Mandatory = $true)]
        [string]$DestinationRoot
    )

    $destinationPath = Join-Path $DestinationRoot $SourceItem.RelativePath
    $destinationDirectory = Split-Path -Parent $destinationPath

    if (-not (Test-Path -LiteralPath $destinationDirectory)) {
        New-Item -ItemType Directory -Path $destinationDirectory -Force | Out-Null
    }

    Copy-Item -LiteralPath $SourceItem.FullName -Destination $destinationPath -Force
    (Get-Item -LiteralPath $destinationPath).LastWriteTimeUtc = $SourceItem.LastWriteUtc
}

function Remove-DestinationOnlyItem {
    param(
        [Parameter(Mandatory = $true)]
        [object]$DestinationItem,

        [Parameter(Mandatory = $true)]
        [string]$DestinationRoot
    )

    $destinationPath = Join-Path $DestinationRoot $DestinationItem.RelativePath

    if (Test-Path -LiteralPath $destinationPath) {
        Remove-Item -LiteralPath $destinationPath -Force
    }
}

function Remove-LegacyDuplicatePaths {
    param(
        [Parameter(Mandatory = $true)]
        [string]$RootPath,

        [Parameter(Mandatory = $true)]
        [string]$Label
    )

    foreach ($relativePath in $legacyDuplicatePaths) {
        $targetPath = Join-Path $RootPath $relativePath
        if ((Test-Path -LiteralPath $targetPath) -and $PSCmdlet.ShouldProcess($targetPath, 'Remove legacy duplicate path from ' + $Label)) {
            Remove-Item -LiteralPath $targetPath -Recurse -Force
        }
    }
}

function Write-PlanSummary {
    param(
        [Parameter(Mandatory = $true)]
        [object]$Plan,

        [Parameter(Mandatory = $true)]
        [string]$ModeName
    )

    $lines = [System.Collections.Generic.List[string]]::new()
    $lines.Add('MODE=' + $ModeName)
    $lines.Add('SOURCE=' + $Plan.SourcePath)
    $lines.Add('DESTINATION=' + $Plan.DestinationPath)
    $lines.Add('SOURCE_ONLY=' + $Plan.SourceOnly.Count)
    $lines.Add('DESTINATION_ONLY=' + $Plan.DestinationOnly.Count)
    $lines.Add('SOURCE_NEWER=' + $Plan.SourceNewer.Count)
    $lines.Add('DESTINATION_NEWER=' + $Plan.DestinationNewer.Count)
    $lines.Add('SAME=' + $Plan.Same.Count)
    $lines.Add('CONFLICTS=' + $Plan.Conflicts.Count)

    foreach ($item in $Plan.SourceOnly | Select-Object -First 20) {
        $lines.Add('SOURCE_ONLY_ITEM=' + $item.RelativePath)
    }

    foreach ($item in $Plan.SourceNewer | Select-Object -First 20) {
        $lines.Add('SOURCE_NEWER_ITEM=' + $item.RelativePath)
    }

    foreach ($item in $Plan.DestinationOnly | Select-Object -First 20) {
        $lines.Add('DESTINATION_ONLY_ITEM=' + $item.RelativePath)
    }

    foreach ($item in $Plan.DestinationNewer | Select-Object -First 20) {
        $lines.Add('DESTINATION_NEWER_ITEM=' + $item.RelativePath)
    }

    foreach ($item in $Plan.Conflicts | Select-Object -First 20) {
        $lines.Add('CONFLICT_ITEM=' + $item.RelativePath)
    }

    if ($ReportPath) {
        Set-Content -LiteralPath $ReportPath -Value $lines -Encoding UTF8
    }

    $lines
}

$BoxPath = Resolve-NormalizedPath -Path $BoxPath
$RepoPath = Resolve-NormalizedPath -Path $RepoPath

switch ($Mode) {
    'Audit' {
        $plan = Get-SyncPlan -SourcePath $BoxPath -DestinationPath $RepoPath
        Write-PlanSummary -Plan $plan -ModeName $Mode
        break
    }
    'BoxToRepo' {
        $plan = Get-SyncPlan -SourcePath $BoxPath -DestinationPath $RepoPath
        Write-PlanSummary -Plan $plan -ModeName $Mode

        if ($PruneLegacyDuplicates) {
            Remove-LegacyDuplicatePaths -RootPath $RepoPath -Label 'repo'
        }

        foreach ($item in $plan.SourceOnly) {
            if ($PSCmdlet.ShouldProcess($item.RelativePath, 'Copy Box file to repo')) {
                Copy-SyncItem -SourceItem $item -DestinationRoot $RepoPath
            }
        }

        foreach ($item in $plan.SourceNewer) {
            if ($PSCmdlet.ShouldProcess($item.RelativePath, 'Overwrite repo file with newer Box copy')) {
                Copy-SyncItem -SourceItem $item.Source -DestinationRoot $RepoPath
            }
        }

        if ($DeleteDestinationOnly) {
            foreach ($item in $plan.DestinationOnly) {
                if ($PSCmdlet.ShouldProcess($item.RelativePath, 'Delete repo-only file')) {
                    Remove-DestinationOnlyItem -DestinationItem $item -DestinationRoot $RepoPath
                }
            }
        }

        break
    }
    'RepoToBox' {
        $plan = Get-SyncPlan -SourcePath $RepoPath -DestinationPath $BoxPath
        Write-PlanSummary -Plan $plan -ModeName $Mode

        if ($PruneLegacyDuplicates) {
            Remove-LegacyDuplicatePaths -RootPath $BoxPath -Label 'Box'
        }

        foreach ($item in $plan.SourceOnly) {
            if ($PSCmdlet.ShouldProcess($item.RelativePath, 'Copy repo file to Box')) {
                Copy-SyncItem -SourceItem $item -DestinationRoot $BoxPath
            }
        }

        foreach ($item in $plan.SourceNewer) {
            if ($PSCmdlet.ShouldProcess($item.RelativePath, 'Overwrite Box file with newer repo copy')) {
                Copy-SyncItem -SourceItem $item.Source -DestinationRoot $BoxPath
            }
        }

        if ($DeleteDestinationOnly) {
            foreach ($item in $plan.DestinationOnly) {
                if ($PSCmdlet.ShouldProcess($item.RelativePath, 'Delete Box-only file')) {
                    Remove-DestinationOnlyItem -DestinationItem $item -DestinationRoot $BoxPath
                }
            }
        }

        break
    }
}