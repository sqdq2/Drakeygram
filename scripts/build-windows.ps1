<#
.SYNOPSIS
  Drakeygram: one-command Windows setup + build (PowerShell 5.1+).

.DESCRIPTION
  From a normal PowerShell window:
    irm https://raw.githubusercontent.com/sqdq2/Drakeygram/main/scripts/build-windows.ps1 -OutFile $env:TEMP\dg.ps1; powershell -ExecutionPolicy Bypass -File $env:TEMP\dg.ps1
  or from a checkout:
    powershell -ExecutionPolicy Bypass -File scripts\build-windows.ps1 [-Debug] [-Yes] [-Dir PATH] [-Arch x64|x86|arm64] [-TestCredentials]

  Needs your own Telegram API credentials (https://my.telegram.org):
    $env:TDESKTOP_API_ID = '...'; $env:TDESKTOP_API_HASH = '...'
  If they are not set the script asks for them. Nothing is stored on disk.
#>
[CmdletBinding()]
param(
  [switch]$Debug,
  [switch]$Yes,
  [string]$Dir = $(if ($env:DRAKEYGRAM_DIR) { $env:DRAKEYGRAM_DIR } else { Join-Path $HOME 'Drakeygram' }),
  [ValidateSet('x64', 'x86', 'arm64')][string]$Arch = 'x64',
  [switch]$TestCredentials
)
$ErrorActionPreference = 'Stop'
$RepoUrl = if ($env:DRAKEYGRAM_REPO) { $env:DRAKEYGRAM_REPO } else { 'https://github.com/sqdq2/Drakeygram.git' }
$Branch = if ($env:DRAKEYGRAM_BRANCH) { $env:DRAKEYGRAM_BRANCH } else { 'main' }
$Config = if ($Debug) { 'Debug' } else { 'Release' }
$TotalSteps = 7
$script:Step = 0
$T0 = Get-Date

function Format-Elapsed($from) { $s = (Get-Date) - $from; '{0:00}h:{1:00}m:{2:00}s' -f [int]$s.TotalHours, $s.Minutes, $s.Seconds }
function Start-Step($name) {
  $script:Step++; $script:StepT = Get-Date
  Write-Host ''; Write-Host ("[{0}/{1}] {2}  (total {3})" -f $script:Step, $TotalSteps, $name, (Format-Elapsed $T0)) -ForegroundColor Cyan
}
function Stop-Step { Write-Host ("  done in {0}" -f (Format-Elapsed $script:StepT)) -ForegroundColor Green }
function Info($m) { Write-Host "  $m" }
function Warn($m) { Write-Host "  warning: $m" -ForegroundColor Yellow }
function Fail($m) { Write-Host "  error: $m" -ForegroundColor Red; exit 1 }
function Confirm-Action($q) {
  if ($Yes) { return $true }
  return ((Read-Host "  $q [y/N]") -match '^[Yy]')
}
function Update-SessionPath {
  $env:Path = [Environment]::GetEnvironmentVariable('Path', 'Machine') + ';' + [Environment]::GetEnvironmentVariable('Path', 'User')
}
function Install-WithWinget($id, $label, $extra) {
  Info "installing $label (winget)"
  $wingetArgs = @('install', '--id', $id, '-e', '--accept-package-agreements', '--accept-source-agreements')
  if ($extra) { $wingetArgs += $extra }
  & winget @wingetArgs
  if ($LASTEXITCODE -ne 0) { Fail "winget could not install $label" }
  Update-SessionPath
}

# ---------- 1. prerequisites ----------
Start-Step 'Checking and installing prerequisites'
if ($env:OS -ne 'Windows_NT') { Fail 'this script is for Windows; use build-linux.sh or build-macos.sh' }
if (-not (Get-Command winget -ErrorAction SilentlyContinue)) {
  Fail 'winget is missing; install "App Installer" from the Microsoft Store, then run this again'
}
if (-not (Get-Command git -ErrorAction SilentlyContinue)) { Install-WithWinget 'Git.Git' 'Git' $null }
$py = Get-Command python -ErrorAction SilentlyContinue
if (-not $py -or ((& python --version 2>&1) -notmatch 'Python 3\.(9|1\d)')) {
  Install-WithWinget 'Python.Python.3.10' 'Python 3.10' $null
}
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
function Get-VsPath {
  if (-not (Test-Path $vswhere)) { return $null }
  $p = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
  if ($p) { return ($p | Select-Object -First 1) } else { return $null }
}
$vs = Get-VsPath
if (-not $vs) {
  if (-not (Confirm-Action 'Visual Studio with the C++ desktop workload is missing (about 10 GB). Install it?')) {
    Fail 'Visual Studio with "Desktop development with C++" and the Windows 11 SDK 10.0.26100 is required'
  }
  Install-WithWinget 'Microsoft.VisualStudio.Community' 'Visual Studio' @(
    '--override',
    '--passive --norestart --wait --add Microsoft.VisualStudio.Workload.NativeDesktop --add Microsoft.VisualStudio.Component.Windows11SDK.26100 --includeRecommended')
  $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
  $vs = Get-VsPath
  if (-not $vs) { Fail 'Visual Studio was installed but the C++ tools were not found; open the Visual Studio Installer and add "Desktop development with C++"' }
}
$archBat = @{ x64 = 'vcvars64.bat'; x86 = 'vcvars32.bat'; arm64 = 'vcvarsamd64_arm64.bat' }[$Arch]
$vcvars = Join-Path $vs "VC\Auxiliary\Build\$archBat"
if (-not (Test-Path $vcvars)) { Fail "$vcvars not found" }
# Upstream requires the v14.44 toolset (Windows 7 support); fall back to the default one if it is not installed.
& cmd /c "`"$vcvars`" -vcvars_ver=14.44 >nul 2>&1"
$VcArg = if ($LASTEXITCODE -eq 0) { '-vcvars_ver=14.44' } else { Warn 'toolset v14.44 not installed; using the default toolset (add "MSVC v143/v144 14.44" in the VS Installer if the build fails)'; '' }
Info "Visual Studio: $vs"
Stop-Step

# ---------- 2. resources ----------
Start-Step 'Checking resources'
$parent = Split-Path -Parent $Dir
if (-not (Test-Path $parent)) { New-Item -ItemType Directory -Path $parent | Out-Null }
$freeGb = [int]((Get-PSDrive -Name ((Resolve-Path $parent).Drive.Name)).Free / 1GB)
$memGb = [int]((Get-CimInstance Win32_ComputerSystem).TotalPhysicalMemory / 1GB)
Info "free disk: $freeGb GB, RAM: $memGb GB, CPU threads: $env:NUMBER_OF_PROCESSORS"
if ($freeGb -lt 60) { Fail "at least 60 GB of free disk space is needed (found $freeGb GB)" }
if ($memGb -lt 8) { Warn 'less than 8 GB of RAM; the build may be very slow' }
Stop-Step

# ---------- 3. credentials ----------
Start-Step 'Telegram API credentials'
if ($TestCredentials) {
  Warn "using Telegram's TEST-ONLY credentials: for compile checks only, not for real use"
  $env:TDESKTOP_API_ID = '17349'; $env:TDESKTOP_API_HASH = '344583e45741c457fe1862106095a5eb'
}
if (-not $env:TDESKTOP_API_ID -or -not $env:TDESKTOP_API_HASH) {
  Info 'Get your own api_id/api_hash at https://my.telegram.org (API development tools)'
  $env:TDESKTOP_API_ID = Read-Host '  api_id'
  $env:TDESKTOP_API_HASH = Read-Host '  api_hash'
}
if ($env:TDESKTOP_API_ID -notmatch '^\d+$') { Fail 'api_id must be a number' }
if ($env:TDESKTOP_API_HASH -notmatch '^[0-9a-fA-F]{32}$') { Fail 'api_hash must be 32 hex characters' }
$Hash = $env:TDESKTOP_API_HASH
Stop-Step

# ---------- 4. source ----------
Start-Step 'Getting the source'
$here = if ($PSScriptRoot) { $PSScriptRoot } else { $null }
if ($here -and (Test-Path (Join-Path $here '..\Telegram\CMakeLists.txt'))) {
  $Dir = (Resolve-Path (Join-Path $here '..')).Path
  Info "using the checkout at $Dir"
  & git -C $Dir submodule update --init --recursive --depth 1 --jobs 8
} elseif (Test-Path (Join-Path $Dir '.git')) {
  & git -C $Dir pull --ff-only
  & git -C $Dir submodule update --init --recursive --depth 1 --jobs 8
} else {
  & git clone --branch $Branch --depth 1 --recurse-submodules --shallow-submodules --jobs 8 $RepoUrl $Dir
}
if ($LASTEXITCODE -ne 0) { Fail 'git failed' }
Set-Location $Dir
$LogDir = Join-Path $Dir 'build-logs'
New-Item -ItemType Directory -Force -Path $LogDir | Out-Null
$Log = Join-Path $LogDir ("windows-{0}.log" -f (Get-Date -Format 'yyyyMMdd-HHmmss'))
Info "full log: $Log"
Stop-Step

function Invoke-InVs($commandLine) {
  # Runs in a cmd.exe with the Visual Studio environment; output is streamed, hidden-secret filtered and logged.
  $inner = "call `"$vcvars`" $VcArg >nul && $commandLine"
  # Windows PowerShell 5.1 turns any stderr line of a native command into a terminating
  # error under 'Stop' (compilers print warnings there), so relax it for this call only.
  $previous = $ErrorActionPreference
  $ErrorActionPreference = 'Continue'
  try {
    & cmd /c $inner 2>&1 |
      ForEach-Object { ($_.ToString()).Replace($Hash, '<hidden>') } |
      Tee-Object -FilePath $Log -Append
    $code = $LASTEXITCODE
  } finally { $ErrorActionPreference = $previous }
  if ($code -ne 0) { Fail "command failed (exit $code); see $Log" }
}

# ---------- 5. libraries ----------
Start-Step 'Building third-party libraries (Qt, FFmpeg, ...; first run takes hours)'
Invoke-InVs 'Telegram\build\prepare\win.bat silent'
Stop-Step

# ---------- 6. configure ----------
Start-Step 'Generating the Visual Studio solution'
Invoke-InVs ("cd Telegram && configure.bat $Arch -D TDESKTOP_API_ID=$env:TDESKTOP_API_ID -D TDESKTOP_API_HASH=$Hash -D DESKTOP_APP_DISABLE_AUTOUPDATE=ON")
Stop-Step

# ---------- 7. compile ----------
Start-Step "Compiling Drakeygram ($Config, $Arch)"
Invoke-InVs "cmake --build out --config $Config --parallel -- /nologo /verbosity:minimal"
$exe = Get-ChildItem -Path (Join-Path $Dir 'out') -Recurse -Filter 'Telegram.exe' -ErrorAction SilentlyContinue |
  Where-Object { $_.FullName -match $Config } | Select-Object -First 1
if (-not $exe) { Fail "build finished but Telegram.exe was not found; see $Log" }
Info ("binary : {0} ({1:N1} MB)" -f $exe.FullName, ($exe.Length / 1MB))
Info ("sha256 : {0}" -f (Get-FileHash $exe.FullName -Algorithm SHA256).Hash)
Info "log    : $Log"
Stop-Step
Write-Host ''
Write-Host ("All done in {0}." -f (Format-Elapsed $T0)) -ForegroundColor Green
