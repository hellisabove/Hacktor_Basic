#!/usr/bin/env bash
# =============================================================================
# Zephyr RTOS v4.3.99 (development) – Development Environment Setup for Arch Linux
# SDK: zephyr-sdk-0.17.4
# =============================================================================

set -euo pipefail

# ── Colors ────────────────────────────────────────────────────────────────────
RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'
CYAN='\033[0;36m'; BOLD='\033[1m'; NC='\033[0m'

info()    { echo -e "${CYAN}[INFO]${NC}  $*"; }
success() { echo -e "${GREEN}[OK]${NC}    $*"; }
warn()    { echo -e "${YELLOW}[WARN]${NC}  $*"; }
error()   { echo -e "${RED}[ERROR]${NC} $*" >&2; exit 1; }
step()    { echo -e "\n${BOLD}━━━ $* ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"; }

# ── Configuration ─────────────────────────────────────────────────────────────
ZEPHYR_VERSION="v4.3.0"   # git tag; 4.3.99 is an in-tree dev bump, not a real tag
SDK_VERSION="0.17.4"
SDK_URL="https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${SDK_VERSION}/zephyr-sdk-${SDK_VERSION}_linux-x86_64.tar.xz"
SDK_SHA_URL="https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${SDK_VERSION}/sha256.sum"

ZEPHYR_BASE="${HOME}/zephyrproject"
SDK_INSTALL_DIR="${HOME}/zephyr-sdk-${SDK_VERSION}"
VENV_DIR="${ZEPHYR_BASE}/.venv"

# ── Helpers ───────────────────────────────────────────────────────────────────
check_cmd() { command -v "$1" &>/dev/null; }

need_aur_helper() {
    if check_cmd yay; then AUR_HELPER="yay"
    elif check_cmd paru; then AUR_HELPER="paru"
    else
        warn "No AUR helper (yay/paru) found. Installing yay from source..."
        install_yay
    fi
}

install_yay() {
    sudo pacman -S --needed --noconfirm git base-devel
    local tmp; tmp=$(mktemp -d)
    git clone https://aur.archlinux.org/yay.git "$tmp/yay"
    (cd "$tmp/yay" && makepkg -si --noconfirm)
    rm -rf "$tmp"
    AUR_HELPER="yay"
}

# ── Steps ─────────────────────────────────────────────────────────────────────

step "1 · System update"
sudo pacman -Syu --noconfirm
success "System up to date"

step "2 · pacman dependencies"
PACMAN_PKGS=(
    git
    cmake
    ninja
    gperf
    ccache
    dfu-util
    dtc
    wget
    python
    python-pip
    python-setuptools
    python-wheel
    tk
    xz
    file
    make
    which
    openocd
    qemu-full          # optional – needed for emulation/testing
    picocom            # optional – serial console
)

sudo pacman -S --needed --noconfirm "${PACMAN_PKGS[@]}"
success "pacman packages installed"

step "3 · AUR packages"
need_aur_helper
AUR_PKGS=(
    python-west        # Zephyr's project meta-tool
)
"${AUR_HELPER}" -S --needed --noconfirm "${AUR_PKGS[@]}"
success "AUR packages installed"

step "4 · Python virtual environment"
python3 -m venv "${VENV_DIR}"
# shellcheck source=/dev/null
source "${VENV_DIR}/bin/activate"
pip install --upgrade pip
success "Virtual environment ready at ${VENV_DIR}"

step "5 · west & Zephyr Python requirements (inside venv)"
pip install west

info "Initialising west workspace for Zephyr ${ZEPHYR_VERSION}…"
mkdir -p "${ZEPHYR_BASE}"
cd "${ZEPHYR_BASE}"

if [ -d "${ZEPHYR_BASE}/.west" ]; then
    warn ".west workspace already exists – skipping west init"
else
    west init -m https://github.com/zephyrproject-rtos/zephyr \
              --mr "${ZEPHYR_VERSION}" "${ZEPHYR_BASE}"
fi

west update
west zephyr-export

info "Installing Zephyr Python requirements…"
pip install -r "${ZEPHYR_BASE}/zephyr/scripts/requirements.txt"
success "west workspace and Python requirements ready"

step "6 · Zephyr SDK ${SDK_VERSION}"
if [ -d "${SDK_INSTALL_DIR}" ]; then
    warn "SDK already present at ${SDK_INSTALL_DIR} – skipping download"
else
    SDK_ARCHIVE="/tmp/zephyr-sdk-${SDK_VERSION}.tar.xz"

    info "Downloading Zephyr SDK ${SDK_VERSION}…"
    wget -c "${SDK_URL}" -O "${SDK_ARCHIVE}"

    info "Verifying checksum…"
    # sha256.sum entries use bare filenames, so we must cd to /tmp and rename
    # the archive to match before checking.
    EXPECTED_NAME="zephyr-sdk-${SDK_VERSION}_linux-x86_64.tar.xz"
    RENAMED="/tmp/${EXPECTED_NAME}"
    [ "${SDK_ARCHIVE}" != "${RENAMED}" ] && mv "${SDK_ARCHIVE}" "${RENAMED}"
    SDK_ARCHIVE="${RENAMED}"

    wget -qO- "${SDK_SHA_URL}" \
        | grep "${EXPECTED_NAME}" \
        | (cd /tmp && sha256sum --check --status) \
        || error "Checksum verification failed! Aborting."
    success "Checksum OK"

    info "Extracting SDK to ${HOME}…"
    tar -xf "${SDK_ARCHIVE}" -C "${HOME}"
    rm -f "${SDK_ARCHIVE}"
fi

info "Running SDK setup script…"
"${SDK_INSTALL_DIR}/setup.sh" -t all -h -c
success "SDK installed and registered"

step "7 · Environment variables"
ENV_BLOCK=$(cat <<EOF

# ── Zephyr v4.3.99 environment ───────────────────────────────────────────────────
export ZEPHYR_SDK_INSTALL_DIR="${SDK_INSTALL_DIR}"
export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
export ZEPHYR_BASE="${ZEPHYR_BASE}/zephyr"

# Activate the Zephyr Python venv
source "${VENV_DIR}/bin/activate"

# Source Zephyr environment
source "\${ZEPHYR_BASE}/zephyr-env.sh"
# ─────────────────────────────────────────────────────────────────────────────
EOF
)

append_to_rc() {
    local rc_file="$1"
    if [ -f "${rc_file}" ] && grep -q "Zephyr v4.3.99 environment" "${rc_file}"; then
        warn "Zephyr env block already present in ${rc_file} – skipping"
    else
        echo "${ENV_BLOCK}" >> "${rc_file}"
        success "Appended Zephyr env to ${rc_file}"
    fi
}

# Detect shell and append to the appropriate rc file
if [ -n "${ZSH_VERSION:-}" ] || [ "$(basename "$SHELL")" = "zsh" ]; then
    append_to_rc "${HOME}/.zshrc"
elif [ -n "${BASH_VERSION:-}" ] || [ "$(basename "$SHELL")" = "bash" ]; then
    append_to_rc "${HOME}/.bashrc"
else
    warn "Unknown shell – appending to ~/.profile"
    append_to_rc "${HOME}/.profile"
fi

step "8 · Verification"
info "cmake  : $(cmake --version | head -1)"
info "python : $(python3 --version)"
info "west   : $(west --version)"
info "dtc    : $(dtc --version | head -1)"
info "SDK    : ${SDK_INSTALL_DIR}"

echo ""
echo -e "${GREEN}${BOLD}✔  Zephyr v4.3.99 setup complete!${NC}"
echo ""
echo "  Next steps:"
echo "  1.  Restart your shell (or run:  source ~/.bashrc / source ~/.zshrc)"
echo "  2.  Build a sample:"
echo "        cd ${ZEPHYR_BASE}/zephyr"
echo "        west build -p always -b <your_board> samples/basic/blinky"
echo ""
