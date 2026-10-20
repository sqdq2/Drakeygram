#!/usr/bin/env bash
# Drakeygram: one-command Linux setup + build (Docker based, like upstream).
#
#   curl -fsSL https://raw.githubusercontent.com/sqdq2/Drakeygram/main/scripts/build-linux.sh | bash
#   # or, from a checkout:
#   ./scripts/build-linux.sh [--debug] [--yes] [--dir PATH] [--jobs N]
#                            [--test-credentials] [--no-remote-cache]
#
# Your own Telegram API credentials are required (https://my.telegram.org):
#   export TDESKTOP_API_ID=...  TDESKTOP_API_HASH=...
# If they are not set the script asks for them. Nothing is stored on disk.
set -Eeuo pipefail

REPO_URL="${DRAKEYGRAM_REPO:-https://github.com/sqdq2/Drakeygram.git}"
BRANCH="${DRAKEYGRAM_BRANCH:-main}"
INSTALL_DIR="${DRAKEYGRAM_DIR:-$HOME/Drakeygram}"
CONFIG="${CONFIG:-Release}"
REMOTE_CACHE="ghcr.io/sqdq2/drakeygram-cache"
CACHE_HOME="${XDG_CACHE_HOME:-$HOME/.cache}/drakeygram"
IMAGE_TAG="drakeygram:env"
JOBS="$(nproc 2>/dev/null || echo 2)"
ASSUME_YES=0
USE_REMOTE_CACHE=1
TEST_CREDS=0
TOTAL_STEPS=7
STEP=0
T0=$SECONDS

# ---------- output helpers ----------
if [ -t 1 ]; then B=$'\033[1m'; G=$'\033[32m'; Y=$'\033[33m'; R=$'\033[31m'; N=$'\033[0m'; else B=; G=; Y=; R=; N=; fi
elapsed() { local s=$((SECONDS - ${1:-$T0})); printf '%02dh:%02dm:%02ds' $((s/3600)) $((s%3600/60)) $((s%60)); }
step()  { STEP=$((STEP + 1)); STEP_T=$SECONDS; printf '\n%s[%d/%d] %s%s  (total %s)\n' "$B" "$STEP" "$TOTAL_STEPS" "$1" "$N" "$(elapsed)"; }
done_step() { printf '%s  done in %s%s\n' "$G" "$(elapsed "$STEP_T")" "$N"; }
info()  { printf '  %s\n' "$*"; }
warn()  { printf '%s  warning: %s%s\n' "$Y" "$*" "$N" >&2; }
die()   { printf '%s  error: %s%s\n' "$R" "$*" "$N" >&2; exit 1; }
trap 'die "failed at line $LINENO (see the log above)"' ERR

confirm() { # confirm "question"  -> 0 if yes
  [ "$ASSUME_YES" = 1 ] && return 0
  [ -r /dev/tty ] || return 1
  local a; read -r -p "  $1 [y/N] " a < /dev/tty; [[ "$a" =~ ^[Yy] ]]
}

usage() { sed -n '2,11p' "$0" | sed 's/^# \{0,1\}//'; exit 0; }
while [ $# -gt 0 ]; do
  case "$1" in
    --debug) CONFIG=Debug ;;
    --yes|-y) ASSUME_YES=1 ;;
    --dir) INSTALL_DIR="$2"; shift ;;
    --jobs) JOBS="$2"; shift ;;
    --test-credentials) TEST_CREDS=1 ;;
    --no-remote-cache) USE_REMOTE_CACHE=0 ;;
    -h|--help) usage ;;
    *) die "unknown option: $1" ;;
  esac
  shift
done

SUDO=""; [ "$(id -u)" -ne 0 ] && SUDO="sudo"

pkg_install() { # pkg_install <names...> using whichever package manager exists
  if   command -v apt-get >/dev/null; then $SUDO apt-get update -y && $SUDO apt-get install -y "$@"
  elif command -v dnf     >/dev/null; then $SUDO dnf install -y "$@"
  elif command -v pacman  >/dev/null; then $SUDO pacman -Sy --noconfirm "$@"
  elif command -v zypper  >/dev/null; then $SUDO zypper --non-interactive install "$@"
  else die "no supported package manager found; install manually: $*"; fi
}

# ---------- 1. prerequisites ----------
step "Checking and installing prerequisites"
[ "$(uname -s)" = "Linux" ] || die "this script is for Linux; use build-macos.sh or build-windows.ps1"
for tool in git curl python3; do
  command -v "$tool" >/dev/null || { info "installing $tool"; pkg_install "$tool"; }
done
if ! command -v docker >/dev/null; then
  confirm "Docker is missing. Install it with Docker's official script (needs sudo)?" \
    || die "Docker is required: https://docs.docker.com/engine/install/"
  curl -fsSL https://get.docker.com | $SUDO sh
  $SUDO usermod -aG docker "$USER" || true
  warn "added $USER to the docker group; if the next check fails, log out and in again"
fi
docker info >/dev/null 2>&1 || die "cannot talk to the Docker daemon (is it running? are you in the docker group?)"
docker buildx version >/dev/null 2>&1 || { info "installing the Docker buildx plugin"; pkg_install docker-buildx-plugin || pkg_install docker-buildx || die "install docker buildx manually"; }
if ! command -v poetry >/dev/null; then
  info "installing poetry"
  curl -sSL https://install.python-poetry.org | python3 -
  export PATH="$HOME/.local/bin:$PATH"
fi
command -v poetry >/dev/null || die "poetry not found on PATH (try: export PATH=\$HOME/.local/bin:\$PATH)"
done_step

# ---------- 2. resources ----------
step "Checking resources"
mkdir -p "$(dirname "$INSTALL_DIR")" "$CACHE_HOME/ccache" "$CACHE_HOME/buildx"
free_gb=$(df -Pk "$(dirname "$INSTALL_DIR")" | awk 'NR==2{printf "%d", $4/1024/1024}')
mem_gb=$(awk '/MemTotal/{printf "%d", $2/1024/1024}' /proc/meminfo)
info "free disk: ${free_gb} GB, RAM: ${mem_gb} GB, CPU threads: ${JOBS}"
[ "$free_gb" -ge 60 ] || die "at least 60 GB of free disk space is needed (found ${free_gb} GB)"
[ "$mem_gb" -ge 8 ] || warn "less than 8 GB of RAM; linking may fail or swap heavily"
done_step

# ---------- 3. credentials ----------
step "Telegram API credentials"
if [ "$TEST_CREDS" = 1 ]; then
  warn "using Telegram's TEST-ONLY credentials: the result is for compile checks, not for real use"
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
info "credentials accepted (they are only passed to this one build)"
done_step

# ---------- 4. source ----------
step "Getting the source"
HERE="$(cd "$(dirname "${BASH_SOURCE[0]:-$0}")" 2>/dev/null && pwd || true)"
if [ -n "$HERE" ] && [ -f "$HERE/../Telegram/CMakeLists.txt" ]; then
  INSTALL_DIR="$(cd "$HERE/.." && pwd)"
  info "using the checkout at $INSTALL_DIR"
  git -C "$INSTALL_DIR" submodule update --init --recursive --depth 1 --jobs 8
elif [ -d "$INSTALL_DIR/.git" ]; then
  info "updating $INSTALL_DIR"
  git -C "$INSTALL_DIR" pull --ff-only
  git -C "$INSTALL_DIR" submodule update --init --recursive --depth 1 --jobs 8
else
  info "cloning $REPO_URL into $INSTALL_DIR"
  git clone --branch "$BRANCH" --depth 1 --recurse-submodules --shallow-submodules --jobs 8 "$REPO_URL" "$INSTALL_DIR"
fi
cd "$INSTALL_DIR"
LOG_DIR="$INSTALL_DIR/build-logs"; mkdir -p "$LOG_DIR"
LOG="$LOG_DIR/linux-$(date +%Y%m%d-%H%M%S).log"
info "full log: $LOG"
done_step

# ---------- 5. build image ----------
step "Building the library image (cached; first run without cache takes hours)"
ENV_DIR="Telegram/build/docker/centos_env"
( cd "$ENV_DIR" && poetry install --no-interaction >>"$LOG" 2>&1 && DEBUG= LTO= poetry run gen_dockerfile > Dockerfile )
docker buildx inspect drakeygram >/dev/null 2>&1 \
  || docker buildx create --name drakeygram --driver docker-container >/dev/null
CACHE_ARGS=(--cache-from "type=local,src=$CACHE_HOME/buildx" --cache-to "type=local,dest=$CACHE_HOME/buildx,mode=max")
if [ "$USE_REMOTE_CACHE" = 1 ]; then
  info "also trying the published library cache (ignored if unavailable)"
  for t in all qt webrtc ffmpeg; do CACHE_ARGS+=(--cache-from "type=registry,ref=$REMOTE_CACHE:$t"); done
fi
PROGRESS=plain; [ -t 1 ] && PROGRESS=tty
docker buildx build --builder drakeygram --progress "$PROGRESS" --load -t "$IMAGE_TAG" \
  "${CACHE_ARGS[@]}" "$ENV_DIR" 2>&1 | tee -a "$LOG"
done_step

# ---------- 6. compile ----------
step "Compiling Drakeygram ($CONFIG, $JOBS jobs); ninja prints [done/total] below"
TTY_ARGS=(); [ -t 1 ] && TTY_ARGS=(-t)
docker run --rm "${TTY_ARGS[@]}" \
  -u "$(id -u):$(id -g)" \
  -v "$PWD:/usr/src/tdesktop" \
  -v "$CACHE_HOME/ccache:/var/cache/ccache" \
  -e CONFIG="$CONFIG" \
  -e CCACHE_SLOPPINESS=pch_defines,time_macros \
  -e CMAKE_BUILD_PARALLEL_LEVEL="$JOBS" \
  "$IMAGE_TAG" \
  env -u CCACHE_DISABLE /usr/src/tdesktop/Telegram/build/docker/centos_env/build.sh \
  -D CMAKE_CONFIGURATION_TYPES="$CONFIG" \
  -D TDESKTOP_API_ID="$TDESKTOP_API_ID" \
  -D TDESKTOP_API_HASH="$TDESKTOP_API_HASH" \
  -D DESKTOP_APP_DISABLE_AUTOUPDATE=ON \
  2>&1 | sed -u "s/$TDESKTOP_API_HASH/<hidden>/g" | tee -a "$LOG"
done_step

# ---------- 7. result ----------
step "Result"
BIN="out/$CONFIG/Telegram"
[ -f "$BIN" ] || die "build finished but $BIN was not produced; see $LOG"
info "binary : $INSTALL_DIR/$BIN ($(du -h "$BIN" | cut -f1))"
info "sha256 : $(sha256sum "$BIN" | cut -d' ' -f1)"
info "log    : $LOG"
done_step
printf '\n%sAll done in %s.%s\n' "$G" "$(elapsed)" "$N"
