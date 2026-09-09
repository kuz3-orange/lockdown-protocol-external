[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release",

    [switch]$Clean
)

$ErrorActionPreference = "Stop"

$vswherePath = Join-Path ${env:ProgramFiles(x86)} "Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path -LiteralPath $vswherePath)) {
    throw "Visual Studio Installer was not found. Install Visual Studio 2022 with Desktop development with C++."
}

$installationPath = & $vswherePath `
    -latest `
    -version '[17.0,18.0)' `
    -products * `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    -property installationPath

if (-not $installationPath) {
    throw "A complete Visual Studio 2022 C++ toolchain was not found. Install MSVC v143 and a Windows 10/11 SDK."
}

$msbuildPath = Join-Path $installationPath "MSBuild\Current\Bin\MSBuild.exe"
$solutionPath = Join-Path $PSScriptRoot "protocol.sln"
$target = if ($Clean) { "Clean,Build" } else { "Build" }

& $msbuildPath $solutionPath "/t:$target" /m "/p:Configuration=$Configuration" /p:Platform=x64 /verbosity:minimal
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

$artifactPath = Join-Path $PSScriptRoot "x64\$Configuration\protocol.exe"
if (-not (Test-Path -LiteralPath $artifactPath)) {
    throw "MSBuild completed but the expected artifact was not found at $artifactPath."
}

Write-Host "Build completed: $artifactPath"
