#!/usr/bin/env bash
# =============================================================================
# Zephyr RTOS v4.3.99 (development) – Development Environment Setup for Ubuntu
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

# ── Steps ─────────────────────────────────────────────────────────────────────

step "1 · System update"
sudo apt-get update -y
sudo apt-get upgrade -y
success "System up to date"

step "2 · apt dependencies"
sudo apt-get install -y \
    git \
    cmake \
    ninja-build \
    gperf \
    ccache \
    dfu-util \
    device-tree-compiler \
    wget \
    python3 \
    python3-pip \
    python3-setuptools \
    python3-wheel \
    python3-venv \
    python3-tk \
    xz-utils \
    file \
    make \
    gcc \
    g++ \
    openocd \
    qemu-system \
    picocom

success "apt packages installed"

step "3 · Python virtual environment"
python3 -m venv "${VENV_DIR}"
# shellcheck source=/dev/null
source "${VENV_DIR}/bin/activate"
pip install --upgrade pip
success "Virtual environment ready at ${VENV_DIR}"

step "4 · west & Zephyr Python requirements (inside venv)"
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

step "5 · Zephyr SDK ${SDK_VERSION}"
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

step "6 · udev rules (flash boards as regular user)"
sudo cp "${SDK_INSTALL_DIR}/hosttools/sysroots/x86_64-pokysdk-linux/usr/share/openocd/contrib/60-openocd.rules" \
    /etc/udev/rules.d/ 2>/dev/null \
    && sudo udevadm control --reload \
    && success "udev rules installed" \
    || warn "Could not install udev rules – you may need to flash as root"

step "7 · Environment variables"
ENV_BLOCK=$(cat <<EOF

# ── Zephyr v4.3.99 environment ────────────────────────────────────────────────
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
