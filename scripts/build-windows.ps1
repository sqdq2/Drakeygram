# Drakeygram — production Windows build (all-in-one)
# Run from "x64 Native Tools Command Prompt for VS" or Developer PowerShell for VS.
$ErrorActionPreference = "Stop"
$Root = Resolve-Path (Join-Path $PSScriptRoot "..")
Set-Location $Root

function Die([string]$msg) { Write-Host "[ERROR] $msg" -ForegroundColor Red; exit 1 }
function Info([string]$msg) { Write-Host "[INFO] $msg" -ForegroundColor Cyan }
function Ok([string]$msg) { Write-Host "[OK] $msg" -ForegroundColor Green }

Ok "=== Drakeygram Windows production build ==="
Info "Repo: $Root"

try {
  if (-not $env:TELEGRAM_API_ID) { $env:TELEGRAM_API_ID = Read-Host "Telegram API ID" }
  if (-not $env:TELEGRAM_API_HASH) { $env:TELEGRAM_API_HASH = Read-Host "Telegram API Hash" }
  if ($env:TELEGRAM_API_ID -notmatch '^\d+$') { Die "API ID must be numeric — https://my.telegram.org" }
  if ([string]::IsNullOrWhiteSpace($env:TELEGRAM_API_HASH)) { Die "API Hash required — https://my.telegram.org" }

  foreach ($cmd in @("cmake", "git")) {
    if (-not (Get-Command $cmd -ErrorAction SilentlyContinue)) { Die "Missing $cmd in PATH" }
  }

  if (Test-Path ".gitmodules") {
    Info "Updating submodules..."
    git submodule update --init --recursive
    if ($LASTEXITCODE -ne 0) { Info "Submodule update returned $LASTEXITCODE — continuing if possible" }
  }

  $Out = if ($env:OUT_DIR) { $env:OUT_DIR } else { Join-Path $Root "out" }
  $BuildType = if ($env:BUILD_TYPE) { $env:BUILD_TYPE } else { "Release" }
  New-Item -ItemType Directory -Force -Path $Out | Out-Null
  Set-Location $Out

  Info "Configuring CMake ($BuildType)..."
  $configured = $false
  try {
    & cmake "$Root" "-G" "Ninja" "-DCMAKE_BUILD_TYPE=$BuildType" `
      "-DTDESKTOP_API_ID=$($env:TELEGRAM_API_ID)" `
      "-DTDESKTOP_API_HASH=$($env:TELEGRAM_API_HASH)"
    if ($LASTEXITCODE -eq 0) { $configured = $true }
  } catch { }

  if (-not $configured) {
    Info "Ninja generator unavailable; trying default generator..."
    & cmake "$Root" "-DCMAKE_BUILD_TYPE=$BuildType" `
      "-DTDESKTOP_API_ID=$($env:TELEGRAM_API_ID)" `
      "-DTDESKTOP_API_HASH=$($env:TELEGRAM_API_HASH)"
    if ($LASTEXITCODE -ne 0) {
      Die "CMake configure failed. See https://github.com/telegramdesktop/tdesktop/blob/dev/docs/building-win-x64.md"
    }
  }

  Info "Building target Telegram ($BuildType)..."
  & cmake --build . --config $BuildType --target Telegram
  if ($LASTEXITCODE -ne 0) { Die "Compile failed. Scroll up for the first C++ error." }

  Ok "Build succeeded."
  Get-ChildItem -Path $Out -Recurse -Include "Drakeygram.exe","Telegram.exe" -ErrorAction SilentlyContinue |
    Select-Object -First 10 FullName
  Ok "Done."
} catch {
  Die $_.Exception.Message
}
