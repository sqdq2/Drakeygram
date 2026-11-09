# Drakeygram — production Windows build
$ErrorActionPreference = "Stop"
$Root = Resolve-Path (Join-Path $PSScriptRoot ".."); Set-Location $Root
function Die($m){ Write-Host "[ERROR] $m" -ForegroundColor Red; exit 1 }
function Ok($m){ Write-Host "[OK] $m" -ForegroundColor Green }
function Info($m){ Write-Host "[INFO] $m" -ForegroundColor Cyan }
Ok "=== Drakeygram Windows production build ==="
try {
  if (-not $env:TELEGRAM_API_ID) { $env:TELEGRAM_API_ID = Read-Host "Telegram API ID" }
  if (-not $env:TELEGRAM_API_HASH) { $env:TELEGRAM_API_HASH = Read-Host "Telegram API Hash" }
  if ($env:TELEGRAM_API_ID -notmatch '^\d+$') { Die "API ID must be numeric — https://my.telegram.org" }
  if ([string]::IsNullOrWhiteSpace($env:TELEGRAM_API_HASH)) { Die "API Hash required" }
  foreach ($c in @("cmake","git")) { if (-not (Get-Command $c -EA SilentlyContinue)) { Die "Missing $c" } }
  if (Test-Path .gitmodules) { git submodule update --init --recursive }
  $Out = if ($env:OUT_DIR) { $env:OUT_DIR } else { Join-Path $Root "out" }
  $BuildType = if ($env:BUILD_TYPE) { $env:BUILD_TYPE } else { "Release" }
  New-Item -ItemType Directory -Force -Path $Out | Out-Null; Set-Location $Out
  Info "Configure $BuildType..."
  & cmake "$Root" "-G" "Ninja" "-DCMAKE_BUILD_TYPE=$BuildType" "-DTDESKTOP_API_ID=$($env:TELEGRAM_API_ID)" "-DTDESKTOP_API_HASH=$($env:TELEGRAM_API_HASH)"
  if ($LASTEXITCODE -ne 0) {
    & cmake "$Root" "-DCMAKE_BUILD_TYPE=$BuildType" "-DTDESKTOP_API_ID=$($env:TELEGRAM_API_ID)" "-DTDESKTOP_API_HASH=$($env:TELEGRAM_API_HASH)"
    if ($LASTEXITCODE -ne 0) { Die "CMake failed — see tdesktop Windows docs" }
  }
  Info "Build..."
  & cmake --build . --config $BuildType --target Telegram
  if ($LASTEXITCODE -ne 0) { Die "Compile failed" }
  Ok "Build succeeded."
  Get-ChildItem -Path $Out -Recurse -Include Drakeygram.exe,Telegram.exe -EA SilentlyContinue | Select-Object -First 10 FullName
} catch { Die $_.Exception.Message }
