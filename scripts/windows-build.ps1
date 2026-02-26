<#
.SYNOPSIS
    QShell Build and Package Script

.PARAMETER BuildOnly
    Build only, skip packaging

.PARAMETER Version
    Version string for ZIP filename (if not specified, will try to get from git tag)

.PARAMETER BuildType
    Build type: Release or Debug

.PARAMETER OutputDir
    Output directory

.EXAMPLE
    .\windows-build.ps1 -BuildOnly
    .\windows-build.ps1 -Version "1.0.0"
#>

param(
    [switch]$BuildOnly,
    [string]$Version,  # 移除默认值，让后续逻辑处理
    [ValidateSet("Release", "Debug")]
    [string]$BuildType = "Release",
    [string]$OutputDir = "deploy"
)

$ErrorActionPreference = "Stop"

function Write-Step {
    param([string]$Message)
    Write-Host ""
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host " $Message" -ForegroundColor Cyan
    Write-Host "========================================" -ForegroundColor Cyan
}

function Write-Success {
    param([string]$Message)
    Write-Host "[SUCCESS] $Message" -ForegroundColor Green
}

function Write-Info {
    param([string]$Message)
    Write-Host "[INFO] $Message" -ForegroundColor Yellow
}

function Write-Err {
    param([string]$Message)
    Write-Host "[ERROR] $Message" -ForegroundColor Red
}

# ==================== 版本号获取逻辑 ====================
# 优先使用传入参数，否则从 git tag 获取，最后使用默认值
if ($PSBoundParameters.ContainsKey('Version') -and $Version) {
    # 使用传入的参数
} else {
    # 尝试从 git tag 获取
    try {
        $gitTag = git describe --tags --abbrev=0 2>$null
        if ($LASTEXITCODE -eq 0 -and $gitTag) {
            # 移除开头的 v 或 V
            $Version = $gitTag -replace '^[vV]', ''
        } else {
            $Version = "1.0.0"
        }
    } catch {
        $Version = "1.0.0"
    }
}

Write-Host ""
Write-Info "Building version: $Version"

# Get project root directory
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir

Push-Location $ProjectRoot

try {
    # ==================== CMake Configure ====================
    Write-Step "CMake Configure"

    $BuildDir = "build"

    if (-not (Test-Path $BuildDir)) {
        New-Item -ItemType Directory -Path $BuildDir | Out-Null
    }

    cmake -B $BuildDir -S . -DAPP_VERSION=$Version -DCMAKE_BUILD_TYPE=$BuildType
    if ($LASTEXITCODE -ne 0) {
        throw "CMake configure failed"
    }
    Write-Success "CMake configure completed"

    # ==================== Build ====================
    Write-Step "Building ($BuildType)"

    cmake --build $BuildDir --config $BuildType --parallel
    if ($LASTEXITCODE -ne 0) {
        throw "Build failed"
    }
    Write-Success "Build completed"

    # Exit if build only
    if ($BuildOnly) {
        Write-Info "Build only mode, skipping package"
        Pop-Location
        exit 0
    }

    # ==================== Create Deploy Directory ====================
    Write-Step "Creating deploy directory"

    if (Test-Path $OutputDir) {
        Remove-Item -Path $OutputDir -Recurse -Force
    }
    New-Item -ItemType Directory -Path $OutputDir | Out-Null
    Write-Success "Deploy directory created: $OutputDir"

    # ==================== Copy Executable ====================
    Write-Step "Copying executable"

    $PossiblePaths = @(
        "build/src/$BuildType/qshell.exe",
        "build/src/qshell.exe",
        "build/$BuildType/qshell.exe",
        "build/qshell.exe"
    )

    $ExePath = $null
    foreach ($path in $PossiblePaths) {
        if (Test-Path $path) {
            $ExePath = $path
            break
        }
    }

    if ($null -eq $ExePath) {
        throw "Cannot find qshell.exe, please check build output path"
    }

    Write-Info "Found executable: $ExePath"
    Copy-Item -Path $ExePath -Destination "$OutputDir/" -Force
    Write-Success "Copied qshell.exe"

    # ==================== Deploy Qt Dependencies ====================
    Write-Step "Deploying Qt dependencies"

    $currentDir = Get-Location
    Set-Location $OutputDir

    if ($BuildType -eq "Debug") {
        windeployqt --debug --no-translations --no-system-d3d-compiler --no-opengl-sw qshell.exe
    } else {
        windeployqt --release --no-translations --no-system-d3d-compiler --no-opengl-sw qshell.exe
    }

    if ($LASTEXITCODE -ne 0) {
        Set-Location $currentDir
        throw "windeployqt failed"
    }

    Set-Location $currentDir
    Write-Success "Qt dependencies deployed"

    # ==================== Copy Additional Files ====================
    Write-Step "Copying additional files"

    if (Test-Path "config") {
        New-Item -ItemType Directory -Path "$OutputDir/config" -Force | Out-Null
        Copy-Item -Path "config/*" -Destination "$OutputDir/config/" -Recurse -Force
        Write-Info "Copied config directory"
    }

    if (Test-Path "resources") {
        Copy-Item -Path "resources/*" -Destination "$OutputDir/" -Recurse -Force
        Write-Info "Copied resources directory"
    }

    if (Test-Path "README.md") {
        Copy-Item -Path "README.md" -Destination "$OutputDir/" -Force
    }

    if (Test-Path "LICENSE") {
        Copy-Item -Path "LICENSE" -Destination "$OutputDir/" -Force
    }

    Write-Success "Additional files copied"

    # ==================== Create ZIP Archive ====================
    Write-Step "Creating ZIP archive"

    $ZipName = "qshell-$Version-win64.zip"

    if (Test-Path $ZipName) {
        Remove-Item -Path $ZipName -Force
    }

    Compress-Archive -Path "$OutputDir/*" -DestinationPath $ZipName -CompressionLevel Optimal
    Write-Success "ZIP archive created: $ZipName"

    $ZipInfo = Get-Item $ZipName
    $SizeMB = [math]::Round($ZipInfo.Length / 1MB, 2)
    Write-Info "File size: $SizeMB MB"

    # Set GitHub Actions environment variable
    if ($env:GITHUB_ENV) {
        "ZIP_NAME=$ZipName" | Out-File -FilePath $env:GITHUB_ENV -Append -Encoding utf8
    }

    # ==================== Done ====================
    Write-Step "Build and package completed!"
    Write-Host ""
    Write-Host "Output files:" -ForegroundColor Green
    Write-Host "  - $OutputDir/ (deploy directory)" -ForegroundColor White
    Write-Host "  - $ZipName (zip archive)" -ForegroundColor White
    Write-Host ""
}
catch {
    Write-Err $_.Exception.Message
    Pop-Location
    exit 1
}

Pop-Location
exit 0
