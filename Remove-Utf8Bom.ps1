#Requires -Version 5.1
<#
.SYNOPSIS
    Finds files whose content starts with a UTF-8 BOM and optionally strips it.

.DESCRIPTION
    Recursively scans the target directory and reports every file that begins with
    the UTF-8 BOM byte sequence (EF BB BF). Files are skipped when they live in:
      * hidden directories at the root level whose name starts with '.' (e.g. .git, .vs)
      * Build, Illustrations, Libs, out, Runtime

    Files with a known binary extension (images, media, fonts, archives, build
    artifacts, ...) are skipped as well. Files without an extension are always
    scanned, and hidden files at the root level (names starting with '.') are
    still scanned.

    After the scan the script asks for confirmation before removing the BOM bytes.

.PARAMETER Root
    Directory to scan. Defaults to the directory containing this script.

.PARAMETER Yes
    Skip the confirmation prompt and remove the BOM without asking.

.PARAMETER IncludeBinary
    Also scan files whose extension is on the binary blacklist.

.EXAMPLE
    powershell -NoProfile -ExecutionPolicy Bypass -File .\Remove-Utf8Bom.ps1

.EXAMPLE
    powershell -NoProfile -ExecutionPolicy Bypass -File .\Remove-Utf8Bom.ps1 -Yes
#>
[CmdletBinding()]
param(
    [string]$Root,
    [switch]$Yes,
    [switch]$IncludeBinary
)

Set-StrictMode -Version 2.0
$ErrorActionPreference = 'Stop'

# Windows PowerShell 5.1 does not populate $PSScriptRoot before parameter binding,
# so the default is resolved here instead of in the param block.
if ([string]::IsNullOrWhiteSpace($Root)) {
    $Root = $PSScriptRoot
}
if ([string]::IsNullOrWhiteSpace($Root)) {
    $Root = (Get-Location).ProviderPath
}

# Directories that are never scanned (matched by name at any depth).
$IgnoredDirectoryNames = @('Build', 'Illustrations', 'Libs', 'out', 'Runtime')

# Non-text extensions that are never scanned unless -IncludeBinary is given.
$IgnoredExtensions = @(
    # images
    '.png', '.jpg', '.jpeg', '.gif', '.bmp', '.tga', '.tif', '.tiff', '.webp',
    '.ico', '.psd', '.dds', '.exr', '.hdr', '.ktx', '.ktx2', '.basis',
    # 3D models, fonts, audio and video
    '.fbx', '.glb', '.blend', '.max', '.ma', '.mb', '.ttf', '.otf', '.woff',
    '.woff2', '.mp3', '.wav', '.ogg', '.flac', '.mp4', '.mov', '.mkv', '.webm',
    # build artifacts and executables
    '.exe', '.dll', '.lib', '.pdb', '.idb', '.obj', '.o', '.a', '.so', '.dylib',
    '.ilk', '.exp', '.res', '.pyc', '.wasm',
    # archives
    '.zip', '.7z', '.rar', '.gz', '.bz2', '.xz', '.tar', '.jar', '.nupkg',
    # other binary data
    '.pdf', '.doc', '.docx', '.xls', '.xlsx', '.ppt', '.pptx', '.db', '.sqlite',
    '.dat', '.bin', '.pch', '.gch', '.blend1'
)

$script:SkippedBinaryCount = 0

function Test-BinaryExtension {
    param([string]$FileName, [string]$Extension)

    if ([string]::IsNullOrEmpty($Extension)) { return $false }
    # '.gitignore'-style names report the whole name as their extension.
    if ($Extension -eq $FileName) { return $false }

    return ($IgnoredExtensions -contains $Extension.ToLowerInvariant())
}

function Get-TargetFileList {
    param(
        [string]$Directory,
        [bool]$IsRoot
    )

    $result = New-Object System.Collections.Generic.List[string]

    try {
        $entries = @(Get-ChildItem -LiteralPath $Directory -Force -ErrorAction Stop)
    }
    catch {
        Write-Warning "Cannot enumerate '$Directory': $($_.Exception.Message)"
        return $result
    }

    foreach ($entry in $entries) {
        if (-not $entry.PSIsContainer) {
            if (-not $IncludeBinary -and (Test-BinaryExtension -FileName $entry.Name -Extension $entry.Extension)) {
                $script:SkippedBinaryCount++
                continue
            }
            $result.Add($entry.FullName)
        }
    }

    foreach ($entry in $entries) {
        if (-not $entry.PSIsContainer) { continue }
        if ($entry.Attributes -band [System.IO.FileAttributes]::ReparsePoint) { continue }
        if ($IsRoot -and $entry.Name.StartsWith('.')) { continue }
        if ($IgnoredDirectoryNames -contains $entry.Name) { continue }

        $childFiles = @(Get-TargetFileList -Directory $entry.FullName -IsRoot $false)
        foreach ($childFile in $childFiles) {
            $result.Add($childFile)
        }
    }

    return $result
}

function Test-Utf8Bom {
    param([string]$Path)

    $stream = $null
    try {
        $stream = [System.IO.File]::Open(
            $Path,
            [System.IO.FileMode]::Open,
            [System.IO.FileAccess]::Read,
            [System.IO.FileShare]::ReadWrite)

        if ($stream.Length -lt 3) { return $false }

        $buffer = New-Object byte[] 3
        $read = $stream.Read($buffer, 0, 3)

        return ($read -eq 3 -and
                $buffer[0] -eq 0xEF -and
                $buffer[1] -eq 0xBB -and
                $buffer[2] -eq 0xBF)
    }
    finally {
        if ($null -ne $stream) { $stream.Dispose() }
    }
}

function Remove-Utf8Bom {
    param([string]$Path)

    $bytes = [System.IO.File]::ReadAllBytes($Path)
    if ($bytes.Length -lt 3) { return }

    $remaining = New-Object byte[] ($bytes.Length - 3)
    if ($remaining.Length -gt 0) {
        [System.Array]::Copy($bytes, 3, $remaining, 0, $remaining.Length)
    }

    [System.IO.File]::WriteAllBytes($Path, $remaining)
}

$resolvedRoot = (Resolve-Path -LiteralPath $Root -ErrorAction Stop).ProviderPath
if (-not (Test-Path -LiteralPath $resolvedRoot -PathType Container)) {
    throw "'$resolvedRoot' is not a directory."
}

Write-Host "Scanning: $resolvedRoot"

$files = @(Get-TargetFileList -Directory $resolvedRoot -IsRoot $true)
$withBom = New-Object System.Collections.Generic.List[string]
$readErrors = 0

foreach ($file in $files) {
    try {
        if (Test-Utf8Bom -Path $file) {
            $withBom.Add($file)
        }
    }
    catch {
        $readErrors++
        Write-Warning "Cannot read '$file': $($_.Exception.Message)"
    }
}

Write-Host ("Scanned {0} file(s)." -f $files.Count)
if ($script:SkippedBinaryCount -gt 0) {
    Write-Host ("Skipped {0} non-text file(s) (use -IncludeBinary to scan them)." -f $script:SkippedBinaryCount) -ForegroundColor DarkGray
}
if ($readErrors -gt 0) {
    Write-Host ("{0} file(s) could not be read." -f $readErrors) -ForegroundColor DarkYellow
}

if ($withBom.Count -eq 0) {
    Write-Host "No file starts with a UTF-8 BOM." -ForegroundColor Green
    exit 0
}

Write-Host ""
Write-Host ("Files starting with a UTF-8 BOM ({0}):" -f $withBom.Count) -ForegroundColor Yellow
foreach ($file in $withBom) {
    $relative = $file.Substring($resolvedRoot.Length).TrimStart('\', '/')
    Write-Host "  $relative"
}
Write-Host ""

if (-not $Yes) {
    $answer = Read-Host ("Remove the UTF-8 BOM from these {0} file(s)? [y/N]" -f $withBom.Count)
    if ($answer -notmatch '^\s*(y|yes)\s*$') {
        Write-Host "Cancelled. No file was modified." -ForegroundColor Green
        exit 0
    }
}

$removed = 0
$failed = 0

foreach ($file in $withBom) {
    try {
        Remove-Utf8Bom -Path $file
        if (Test-Utf8Bom -Path $file) {
            $failed++
            Write-Warning "BOM still present after rewriting '$file'."
        }
        else {
            $removed++
        }
    }
    catch {
        $failed++
        Write-Warning "Cannot update '$file': $($_.Exception.Message)"
    }
}

Write-Host ""
Write-Host ("Done. BOM removed from {0} file(s)." -f $removed) -ForegroundColor Green
if ($failed -gt 0) {
    Write-Host ("{0} file(s) failed." -f $failed) -ForegroundColor Red
    exit 1
}
