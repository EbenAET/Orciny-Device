# V 1.0.0 (Show Release)
[CmdletBinding(SupportsShouldProcess = $true)]
param(
    [ValidateSet('Audit', 'Export')]
    [string]$Mode = 'Audit',

    [string]$McpUrl = 'http://127.0.0.1:27182/mcp',

    [string]$RepoPath = (Split-Path -Parent $PSScriptRoot),

    [string]$ExportRoot,

    [string]$Project,

    [string]$NameLike,

    [string]$ProjectMapPath = (Join-Path $PSScriptRoot 'fusion-project-map.json'),

    [switch]$DisableProjectMap,

    [ValidateSet('f3d', 'step', 'stl')]
    [string[]]$Formats = @('f3d', 'step'),

    [switch]$RunBoxSync,

    [switch]$IncludeStlOneFilePerBody,

    [string]$ReportPath
)

$ErrorActionPreference = 'Stop'

function Resolve-NormalizedPath {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path
    )

    return [System.IO.Path]::GetFullPath((Resolve-Path -LiteralPath $Path).Path)
}

function Invoke-McpJsonRpc {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Method,

        [Parameter()]
        [object]$Params = @{}
    )

    $id = [int](Get-Random -Minimum 1000 -Maximum 999999)
    $body = @{
        jsonrpc = '2.0'
        id = $id
        method = $Method
        params = $Params
    } | ConvertTo-Json -Depth 20

    $response = Invoke-RestMethod -Uri $McpUrl -Method Post -ContentType 'application/json' -Body $body
    if ($null -ne $response.error) {
        throw ('MCP error on method ' + $Method + ': ' + ($response.error | ConvertTo-Json -Depth 10))
    }

    return $response
}

function Initialize-Mcp {
    $response = Invoke-McpJsonRpc -Method 'initialize' -Params @{
        protocolVersion = '2024-11-05'
        clientInfo = @{
            name = 'fusion-archive-sync'
            version = '0.1.0'
        }
        capabilities = @{}
    }

    return $response.result
}

function Invoke-McpTool {
    param(
        [Parameter(Mandatory = $true)]
        [string]$ToolName,

        [Parameter(Mandatory = $true)]
        [hashtable]$Arguments
    )

    return (Invoke-McpJsonRpc -Method 'tools/call' -Params @{
            name = $ToolName
            arguments = $Arguments
        }).result
}

function ConvertFrom-McpToolTextJson {
    param(
        [Parameter(Mandatory = $true)]
        [object]$ToolResult
    )

    if (-not $ToolResult.content) {
        return $null
    }

    $textItem = $ToolResult.content | Where-Object { $_.type -eq 'text' } | Select-Object -First 1
    if (-not $textItem) {
        return $null
    }

    if ([string]::IsNullOrWhiteSpace($textItem.text)) {
        return $null
    }

    try {
        return $textItem.text | ConvertFrom-Json -Depth 30
    }
    catch {
        return [pscustomobject]@{
            success = $false
            rawText = $textItem.text
        }
    }
}

function New-SafeFileName {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Name
    )

    $invalid = [System.IO.Path]::GetInvalidFileNameChars()
    $safe = -join ($Name.ToCharArray() | ForEach-Object {
            if ($invalid -contains $_) {
                '_'
            }
            else {
                $_
            }
        })

    if ([string]::IsNullOrWhiteSpace($safe)) {
        return 'unnamed'
    }

    return $safe.Trim()
}

function Resolve-SafeRelativeSubfolder {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Subfolder
    )

    $trimmed = $Subfolder.Trim()
    if ([string]::IsNullOrWhiteSpace($trimmed)) {
        return ''
    }

    if ([System.IO.Path]::IsPathRooted($trimmed)) {
        throw ('Project map outputSubfolder must be relative: ' + $Subfolder)
    }

    $normalized = $trimmed.Replace('/', '\\').TrimStart('\\').TrimEnd('\\')
    $segments = $normalized.Split('\\')
    if ($segments -contains '..') {
        throw ('Project map outputSubfolder cannot contain .. segments: ' + $Subfolder)
    }

    return $normalized
}

function Resolve-FormatList {
    param(
        [Parameter()]
        [object]$SourceFormats,

        [Parameter(Mandatory = $true)]
        [string[]]$FallbackFormats,

        [Parameter(Mandatory = $true)]
        [string]$RuleLabel
    )

    $allowed = @('f3d', 'step', 'stl')
    $resolved = @()

    if ($null -eq $SourceFormats) {
        $resolved = @($FallbackFormats)
    }
    elseif ($SourceFormats -is [System.Array]) {
        foreach ($item in $SourceFormats) {
            $resolved += [string]$item
        }
    }
    else {
        $resolved = @([string]$SourceFormats)
    }

    $normalized = @()
    foreach ($fmt in $resolved) {
        $lower = $fmt.ToLowerInvariant()
        if ($allowed -notcontains $lower) {
            throw ('Unsupported format in ' + $RuleLabel + ': ' + $fmt)
        }
        if ($normalized -notcontains $lower) {
            $normalized += $lower
        }
    }

    if ($normalized.Count -eq 0) {
        throw ('No formats resolved in ' + $RuleLabel)
    }

    return $normalized
}

if (-not $ExportRoot) {
    $ExportRoot = Join-Path $RepoPath '3d Models\Archive\fusion_auto'
}

$RepoPath = Resolve-NormalizedPath -Path $RepoPath
if (-not (Test-Path -LiteralPath $ExportRoot)) {
    New-Item -ItemType Directory -Path $ExportRoot -Force | Out-Null
}
$ExportRoot = [System.IO.Path]::GetFullPath((Resolve-Path -LiteralPath $ExportRoot).Path)

$init = Initialize-Mcp
Write-Host ('MCP server: ' + $init.serverInfo.name + ' ' + $init.serverInfo.version)

$recentResult = Invoke-McpTool -ToolName 'fusion_mcp_read' -Arguments @{
    queryType = 'document'
    operation = 'recent'
}
$recent = ConvertFrom-McpToolTextJson -ToolResult $recentResult
if (-not $recent -or -not $recent.results) {
    throw 'Could not read recent documents from Fusion MCP.'
}

$documents = @($recent.results)

$projectMapResolvedPath = ''
$projectMapActive = $false
$projectMapRules = @()

if (-not $DisableProjectMap -and -not [string]::IsNullOrWhiteSpace($ProjectMapPath)) {
    $candidateMapPath = [System.IO.Path]::GetFullPath($ProjectMapPath)
    if (Test-Path -LiteralPath $candidateMapPath) {
        $projectMapResolvedPath = $candidateMapPath
        $projectMapRaw = Get-Content -LiteralPath $candidateMapPath -Raw | ConvertFrom-Json -Depth 30
        if ($projectMapRaw.rules) {
            $projectMapRules = @($projectMapRaw.rules)
        }

        if (-not $Project -and $projectMapRules.Count -gt 0) {
            $projectMapActive = $true
        }
    }
}

$workItems = [System.Collections.Generic.List[object]]::new()

if ($projectMapActive) {
    $seenById = @{}
    $ruleIndex = 0
    foreach ($rule in $projectMapRules) {
        $ruleIndex++
        $ruleProject = [string]$rule.project
        $ruleProjectId = [string]$rule.projectId
        if ([string]::IsNullOrWhiteSpace($ruleProject) -and [string]::IsNullOrWhiteSpace($ruleProjectId)) {
            continue
        }

        $ruleLabel = 'map rule ' + $ruleIndex
        $ruleNameLike = [string]$rule.nameLike
        $ruleSubfolder = ''
        if ($rule.outputSubfolder) {
            $ruleSubfolder = Resolve-SafeRelativeSubfolder -Subfolder ([string]$rule.outputSubfolder)
        }
        elseif (-not [string]::IsNullOrWhiteSpace($ruleProject)) {
            $ruleSubfolder = New-SafeFileName -Name $ruleProject
        }
        else {
            $ruleSubfolder = New-SafeFileName -Name $ruleProjectId
        }

        $ruleFormats = Resolve-FormatList -SourceFormats $rule.formats -FallbackFormats $Formats -RuleLabel $ruleLabel

        $matchingDocs = @($documents | Where-Object {
                ((-not [string]::IsNullOrWhiteSpace($ruleProject)) -and $_.parentProject -eq $ruleProject) -or
                ((-not [string]::IsNullOrWhiteSpace($ruleProjectId)) -and $_.parentProjectId -eq $ruleProjectId)
            })

        if ($ruleNameLike) {
            $matchingDocs = @($matchingDocs | Where-Object { $_.name -like $ruleNameLike })
        }

        if ($NameLike) {
            $matchingDocs = @($matchingDocs | Where-Object { $_.name -like $NameLike })
        }

        foreach ($doc in $matchingDocs) {
            if ($seenById.ContainsKey($doc.id)) {
                continue
            }

            $seenById[$doc.id] = $true
            $workItems.Add([pscustomobject]@{
                    Document = $doc
                    OutputSubfolder = $ruleSubfolder
                    Formats = $ruleFormats
                    RuleLabel = $ruleLabel
                })
        }
    }
}
else {
    $filteredDocuments = @($documents)

    if ($Project) {
        $filteredDocuments = @($filteredDocuments | Where-Object {
                $_.parentProject -eq $Project -or $_.parentProjectId -eq $Project
            })
    }

    if ($NameLike) {
        $filteredDocuments = @($filteredDocuments | Where-Object { $_.name -like $NameLike })
    }

    $defaultFormats = Resolve-FormatList -SourceFormats $Formats -FallbackFormats $Formats -RuleLabel 'default format list'
    foreach ($doc in $filteredDocuments) {
        $workItems.Add([pscustomobject]@{
                Document = $doc
                OutputSubfolder = ''
                Formats = $defaultFormats
                RuleLabel = 'default'
            })
    }
}

$summaryLines = [System.Collections.Generic.List[string]]::new()
$summaryLines.Add('MODE=' + $Mode)
$summaryLines.Add('MCP_URL=' + $McpUrl)
$summaryLines.Add('REPO_PATH=' + $RepoPath)
$summaryLines.Add('EXPORT_ROOT=' + $ExportRoot)
$summaryLines.Add('PROJECT_FILTER=' + $Project)
$summaryLines.Add('NAMELIKE_FILTER=' + $NameLike)
$summaryLines.Add('FORMAT_FILTER=' + ($Formats -join ','))
$summaryLines.Add('PROJECT_MAP_PATH=' + $projectMapResolvedPath)
$summaryLines.Add('PROJECT_MAP_ACTIVE=' + $projectMapActive)
$summaryLines.Add('MATCH_COUNT=' + $workItems.Count)

Write-Host ('Matching Fusion docs: ' + $workItems.Count)
foreach ($item in $workItems) {
    $doc = $item.Document
    $summaryLines.Add('MATCH=' + $doc.name + ' | ' + $doc.id + ' | ' + $doc.parentProject + ' | ' + $doc.parentFolder + ' | subfolder=' + $item.OutputSubfolder + ' | formats=' + ($item.Formats -join ','))
}

if ($Mode -eq 'Audit') {
    if ($ReportPath) {
        Set-Content -LiteralPath $ReportPath -Value $summaryLines -Encoding UTF8
    }

    $summaryLines
    return
}

$timestamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$runRoot = Join-Path $ExportRoot ('run-' + $timestamp)
if (-not (Test-Path -LiteralPath $runRoot)) {
    New-Item -ItemType Directory -Path $runRoot -Force | Out-Null
}

$exported = [System.Collections.Generic.List[string]]::new()
$failed = [System.Collections.Generic.List[string]]::new()

foreach ($item in $workItems) {
    $doc = $item.Document
    $safeName = New-SafeFileName -Name $doc.name

    $targetRoot = $runRoot
    if (-not [string]::IsNullOrWhiteSpace($item.OutputSubfolder)) {
        $targetRoot = Join-Path $runRoot $item.OutputSubfolder
    }

    if (-not (Test-Path -LiteralPath $targetRoot)) {
        New-Item -ItemType Directory -Path $targetRoot -Force | Out-Null
    }

    $docRoot = Join-Path $targetRoot $safeName

    if (-not (Test-Path -LiteralPath $docRoot)) {
        New-Item -ItemType Directory -Path $docRoot -Force | Out-Null
    }

    if (-not $PSCmdlet.ShouldProcess($doc.name, 'Open in Fusion and export archive files')) {
        continue
    }

    try {
        $null = Invoke-McpTool -ToolName 'fusion_mcp_execute' -Arguments @{
            featureType = 'document'
            object = @{
                operation = 'open'
                fileId = $doc.id
            }
        }

        $basePrefix = Join-Path $docRoot $safeName
        $basePrefixPy = $basePrefix.Replace('\\', '\\\\')

        $formatSet = @{}
        foreach ($fmt in $item.Formats) {
            $formatSet[$fmt.ToLowerInvariant()] = $true
        }

        $script = @"
import adsk.core
import adsk.fusion

def run(_context: str):
    app = adsk.core.Application.get()
    design = adsk.fusion.Design.cast(app.activeProduct)
    if not design:
        raise RuntimeError('No active Fusion design to export')

    export_manager = design.exportManager
    root_comp = design.rootComponent
    base = r'${basePrefixPy}'

    exports = []

    if ${([bool]$formatSet.ContainsKey('f3d')).ToString().ToLower()}:
        f3d_path = base + '.f3d'
        f3d_opts = export_manager.createFusionArchiveExportOptions(f3d_path)
        export_manager.execute(f3d_opts)
        exports.append(f3d_path)

    if ${([bool]$formatSet.ContainsKey('step')).ToString().ToLower()}:
        step_path = base + '.step'
        step_opts = export_manager.createSTEPExportOptions(step_path)
        export_manager.execute(step_opts)
        exports.append(step_path)

    if ${([bool]$formatSet.ContainsKey('stl')).ToString().ToLower()}:
        stl_path = base + '.stl'
        stl_opts = export_manager.createSTLExportOptions(root_comp, stl_path)
        stl_opts.sendToPrintUtility = False
        stl_opts.isBinaryFormat = True
        stl_opts.meshRefinement = adsk.fusion.MeshRefinementSettings.MeshRefinementMedium
        stl_opts.isOneFilePerBody = ${$IncludeStlOneFilePerBody.ToString().ToLower()}
        export_manager.execute(stl_opts)
        exports.append(stl_path)

    for item in exports:
        print('EXPORTED=' + item)
"@

        $execResult = Invoke-McpTool -ToolName 'fusion_mcp_execute' -Arguments @{
            featureType = 'script'
            object = @{
                script = $script
            }
        }

        $execText = ConvertFrom-McpToolTextJson -ToolResult $execResult
        if ($execText.rawText) {
            $summaryLines.Add('EXPORT_RAW=' + $doc.name + ' | ' + $execText.rawText)
        }

        $exported.Add($doc.name + ' -> ' + $item.OutputSubfolder)
    }
    catch {
        $failed.Add($doc.name + ' -> ' + $item.OutputSubfolder)
        $summaryLines.Add('ERROR=' + $doc.name + ' | ' + $_.Exception.Message)
    }
    finally {
        try {
            $null = Invoke-McpTool -ToolName 'fusion_mcp_execute' -Arguments @{
                featureType = 'document'
                object = @{
                    operation = 'close'
                    userConfirmedCloseWithoutSave = $true
                }
            }
        }
        catch {
            $summaryLines.Add('CLOSE_WARN=' + $doc.name + ' | ' + $_.Exception.Message)
        }
    }
}

$summaryLines.Add('EXPORTED_COUNT=' + $exported.Count)
$summaryLines.Add('FAILED_COUNT=' + $failed.Count)
foreach ($name in $exported) {
    $summaryLines.Add('EXPORTED=' + $name)
}
foreach ($name in $failed) {
    $summaryLines.Add('FAILED=' + $name)
}

if ($RunBoxSync) {
    $boxScript = Join-Path $PSScriptRoot 'sync-box-local.ps1'
    if (-not (Test-Path -LiteralPath $boxScript)) {
        throw ('Box sync script not found at ' + $boxScript)
    }

    if ($PSCmdlet.ShouldProcess($boxScript, 'Run RepoToBox sync after Fusion export')) {
        & $boxScript -Mode RepoToBox -RepoPath $RepoPath -PruneLegacyDuplicates
    }
}

if ($ReportPath) {
    Set-Content -LiteralPath $ReportPath -Value $summaryLines -Encoding UTF8
}

$summaryLines