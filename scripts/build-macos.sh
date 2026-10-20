#!/usr/bin/env bash
# Drakeygram: one-command macOS setup + build.
#
#   curl -fsSL https://raw.githubusercontent.com/sqdq2/Drakeygram/main/scripts/build-macos.sh | bash
#   # or, from a checkout:
#   ./scripts/build-macos.sh [--debug] [--yes] [--dir PATH] [--test-credentials]
#
# Needs full Xcode (App Store) and your own Telegram API credentials
# (https://my.telegram.org):  export TDESKTOP_API_ID=...  TDESKTOP_API_HASH=...
set -Eeuo pipefail

REPO_URL="${DRAKEYGRAM_REPO:-https://github.com/sqdq2/Drakeygram.git}"
BRANCH="${DRAKEYGRAM_BRANCH:-main}"
INSTALL_DIR="${DRAKEYGRAM_DIR:-$HOME/Drakeygram}"
CONFIG="${CONFIG:-Release}"
JOBS="$(sysctl -n hw.ncpu 2>/dev/null || echo 4)"
ASSUME_YES=0
TEST_CREDS=0
TOTAL_STEPS=7
STEP=0
T0=$SECONDS

if [ -t 1 ]; then B=$'\033[1m'; G=$'\033[32m'; Y=$'\033[33m'; R=$'\033[31m'; N=$'\033[0m'; else B=; G=; Y=; R=; N=; fi
elapsed() { local s=$((SECONDS - ${1:-$T0})); printf '%02dh:%02dm:%02ds' $((s/3600)) $((s%3600/60)) $((s%60)); }
step()  { STEP=$((STEP + 1)); STEP_T=$SECONDS; printf '\n%s[%d/%d] %s%s  (total %s)\n' "$B" "$STEP" "$TOTAL_STEPS" "$1" "$N" "$(elapsed)"; }
done_step() { printf '%s  done in %s%s\n' "$G" "$(elapsed "$STEP_T")" "$N"; }
info()  { printf '  %s\n' "$*"; }
warn()  { printf '%s  warning: %s%s\n' "$Y" "$*" "$N" >&2; }
die()   { printf '%s  error: %s%s\n' "$R" "$*" "$N" >&2; exit 1; }
trap 'die "failed at line $LINENO (see the log above)"' ERR
confirm() { [ "$ASSUME_YES" = 1 ] && return 0; [ -r /dev/tty ] || return 1; local a; read -r -p "  $1 [y/N] " a < /dev/tty; [[ "$a" =~ ^[Yy] ]]; }

while [ $# -gt 0 ]; do
  case "$1" in
    --debug) CONFIG=Debug ;;
    --yes|-y) ASSUME_YES=1 ;;
    --dir) INSTALL_DIR="$2"; shift ;;
    --test-credentials) TEST_CREDS=1 ;;
    -h|--help) sed -n '2,9p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
    *) die "unknown option: $1" ;;
  esac
  shift
done

step "Checking and installing prerequisites"
[ "$(uname -s)" = "Darwin" ] || die "this script is for macOS"
xcode-select -p >/dev/null 2>&1 || die "install Xcode from the App Store, open it once, then run: sudo xcode-select -s /Applications/Xcode.app/Contents/Developer"
[ -d /Applications/Xcode.app ] || die "full Xcode is required (Command Line Tools alone are not enough)"
if ! command -v brew >/dev/null; then
  confirm "Homebrew is missing. Install it?" || die "Homebrew is required: https://brew.sh"
  /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
  [ -x /opt/homebrew/bin/brew ] && eval "$(/opt/homebrew/bin/brew shellenv)"
fi
BREW_PKGS=(git automake libtool cmake wget pkg-config gnu-tar ninja nasm meson python3)
info "brew packages: ${BREW_PKGS[*]}"
brew install "${BREW_PKGS[@]}" 2>&1 | grep -v "already installed" || true
done_step

step "Checking resources"
mkdir -p "$(dirname "$INSTALL_DIR")"
free_gb=$(df -k "$(dirname "$INSTALL_DIR")" | awk 'NR==2{printf "%d", $4/1024/1024}')
mem_gb=$(( $(sysctl -n hw.memsize) / 1024 / 1024 / 1024 ))
info "free disk: ${free_gb} GB, RAM: ${mem_gb} GB, CPU threads: ${JOBS}"
[ "$free_gb" -ge 60 ] || die "at least 60 GB of free disk space is needed (found ${free_gb} GB)"
[ "$mem_gb" -ge 8 ] || warn "less than 8 GB of RAM; the build may be very slow"
done_step

step "Telegram API credentials"
if [ "$TEST_CREDS" = 1 ]; then
  warn "using Telegram's TEST-ONLY credentials: for compile checks only, not for real use"
  TDESKTOP_API_ID=17349; TDESKTOP_API_HASH=344583e45741c457fe1862106095a5eb
fi
if [ -z "${TDESKTOP_API_ID:-}" ] || [ -z "${TDESKTOP_API_HASH:-}" ]; then
  [ -r /dev/tty ] || die "set TDESKTOP_API_ID and TDESKTOP_API_HASH (get them at https://my.telegram.org)"
  info "Get your own api_id/api_hash at https://my.telegram.org (API development tools)"
  read -r -p "  api_id: " TDESKTOP_API_ID < /dev/tty
  read -r -p "  api_hash: " TDESKTOP_API_HASH < /dev/tty
fi
[[ "$TDESKTOP_API_ID" =~ ^[0-9]+$ ]] || die "api_id must be a number"
[[ "$TDESKTOP_API_HASH" =~ ^[0-9a-fA-F]{32}$ ]] || die "api_hash must be 32 hex characters"
done_step

step "Getting the source"
HERE="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" 2>/dev/null && pwd || true)"
if [ -n "$HERE" ] && [ -f "$HERE/../Telegram/CMakeLists.txt" ]; then
  INSTALL_DIR="$(cd "$HERE/.." && pwd)"
  info "using the checkout at $INSTALL_DIR"
  git -C "$INSTALL_DIR" submodule update --init --recursive --depth 1 --jobs 8
elif [ -d "$INSTALL_DIR/.git" ]; then
  git -C "$INSTALL_DIR" pull --ff-only
  git -C "$INSTALL_DIR" submodule update --init --recursive --depth 1 --jobs 8
else
  git clone --branch "$BRANCH" --depth 1 --recurse-submodules --shallow-submodules --jobs 8 "$REPO_URL" "$INSTALL_DIR"
fi
cd "$INSTALL_DIR"
LOG_DIR="$INSTALL_DIR/build-logs"; mkdir -p "$LOG_DIR"
LOG="$LOG_DIR/macos-$(date +%Y%m%d-%H%M%S).log"
info "full log: $LOG"
done_step

step "Building third-party libraries (Qt, FFmpeg, ...; first run takes hours)"
./Telegram/build/prepare/mac.sh 2>&1 | tee -a "$LOG"
done_step

step "Generating the Xcode project"
( cd Telegram && ./configure.sh \
    -D TDESKTOP_API_ID="$TDESKTOP_API_ID" \
    -D TDESKTOP_API_HASH="$TDESKTOP_API_HASH" \
    -D DESKTOP_APP_DISABLE_AUTOUPDATE=ON ) 2>&1 | sed -u "s/$TDESKTOP_API_HASH/<hidden>/g" | tee -a "$LOG"
done_step

step "Compiling Drakeygram ($CONFIG, $JOBS jobs)"
PRETTY=cat; command -v xcpretty >/dev/null && PRETTY=xcpretty
xcodebuild -project out/Telegram.xcodeproj -scheme Telegram -configuration "$CONFIG" \
  -jobs "$JOBS" CODE_SIGNING_ALLOWED=NO build 2>&1 | tee -a "$LOG" | $PRETTY
APP="$(find out -maxdepth 4 -name 'Telegram.app' -path "*$CONFIG*" | head -1)"
[ -n "$APP" ] || die "build finished but no app bundle was found; see $LOG"
codesign --force --deep --sign - "$APP" >/dev/null 2>&1 || warn "ad-hoc signing failed; the app may not start"
info "app: $INSTALL_DIR/$APP"
info "log: $LOG"
done_step
printf '\n%sAll done in %s.%s\n' "$G" "$(elapsed)" "$N"
