# Drakeygram — Windows all-in-one build (PowerShell)
# Run in "x64 Native Tools Command Prompt for VS" or Developer PowerShell for VS.
$ErrorActionPreference = "Stop"

function Die($msg) { Write-Host "ERROR: $msg" -ForegroundColor Red; exit 1 }
function Info($msg) { Write-Host $msg -ForegroundColor Cyan }
function Ok($msg) { Write-Host $msg -ForegroundColor Green }

$Root = Resolve-Path (Join-Path $PSScriptRoot "..")
Set-Location $Root
Ok "=== Drakeygram Windows build ==="

if (-not $env:TELEGRAM_API_ID -or -not $env:TELEGRAM_API_HASH) {
  $env:TELEGRAM_API_ID = Read-Host "Telegram API ID"
  $env:TELEGRAM_API_HASH = Read-Host "Telegram API Hash"
}
if ($env:TELEGRAM_API_ID -notmatch '^\d+$') { Die "API ID must be numeric (https://my.telegram.org)" }
if ([string]::IsNullOrWhiteSpace($env:TELEGRAM_API_HASH)) { Die "API Hash required" }

foreach ($cmd in @("cmake", "git")) {
  if (-not (Get-Command $cmd -ErrorAction SilentlyContinue)) { Die "Missing $cmd in PATH" }
}

if (Test-Path ".gitmodules") {
  Info "Updating submodules..."
  git submodule update --init --recursive
}

$Out = if ($env:OUT_DIR) { $env:OUT_DIR } else { Join-Path $Root "out" }
$BuildType = if ($env:BUILD_TYPE) { $env:BUILD_TYPE } else { "Release" }
New-Item -ItemType Directory -Force -Path $Out | Out-Null
Set-Location $Out

Info "Configuring CMake ($BuildType)..."
$cmakeArgs = @(
  "$Root",
  "-G", "Ninja",
  "-DCMAKE_BUILD_TYPE=$BuildType",
  "-DTDESKTOP_API_ID=$($env:TELEGRAM_API_ID)",
  "-DTDESKTOP_API_HASH=$($env:TELEGRAM_API_HASH)"
)
if ($env:CMAKE_EXTRA_ARGS) { $cmakeArgs += $env:CMAKE_EXTRA_ARGS.Split(" ") }

& cmake @cmakeArgs
if ($LASTEXITCODE -ne 0) {
  Info "Ninja generator failed; retrying with default generator..."
  & cmake "$Root" "-DCMAKE_BUILD_TYPE=$BuildType" "-DTDESKTOP_API_ID=$($env:TELEGRAM_API_ID)" "-DTDESKTOP_API_HASH=$($env:TELEGRAM_API_HASH)"
  if ($LASTEXITCODE -ne 0) { Die "CMake configure failed. See tdesktop Windows build docs." }
}

Info "Building..."
& cmake --build . --config $BuildType --target Telegram
if ($LASTEXITCODE -ne 0) { Die "Build failed" }

Ok "Build finished. Search under $Out for Drakeygram.exe or Telegram.exe"
Get-ChildItem -Path $Out -Recurse -Filter "Drakeygram.exe" -ErrorAction SilentlyContinue | Select-Object -First 5 FullName
Get-ChildItem -Path $Out -Recurse -Filter "Telegram.exe" -ErrorAction SilentlyContinue | Select-Object -First 5 FullName
