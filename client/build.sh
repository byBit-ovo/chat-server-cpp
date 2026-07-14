#!/bin/bash
# ======================================================
# build.sh
# Build the ChatClient project on macOS
#
# Usage:
#   ./build.sh              - Build (release)
#   ./build.sh debug        - Build (debug)
#   ./build.sh clean        - Clean build
#   ./build.sh run          - Build and run
#   ./build.sh proto        - Only generate protobuf files
# ======================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
BUILD_TYPE="${1:-release}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

info()  { echo -e "${GREEN}[INFO]${NC}  $*"; }
warn()  { echo -e "${YELLOW}[WARN]${NC}  $*"; }
error() { echo -e "${RED}[ERROR]${NC} $*"; }

# Find Qt6 qmake
find_qmake() {
    if [ -n "${QMAKE}" ] && command -v "${QMAKE}" &>/dev/null; then
        echo "${QMAKE}"
        return
    fi

    # Try various paths
    for candidate in \
        "qmake6" \
        "qmake" \
        "/opt/homebrew/Cellar/qt/6*/bin/qmake" \
        "/usr/local/Cellar/qt/6*/bin/qmake" \
        "/opt/homebrew/bin/qmake" \
        "/usr/local/bin/qmake" \
        "$HOME/Qt/6*/macos/bin/qmake" \
        "/Applications/Qt/6*/macos/bin/qmake"; do

        # Expand globs
        for resolved in $candidate; do
            if [ -x "$resolved" ] && "$resolved" --version 2>/dev/null | grep -q "Qt version"; then
                echo "$resolved"
                return
            fi
        done
    done
}

# ======================================================
# Step 1: Check prerequisites
# ======================================================

info "Checking prerequisites..."

QMAKE_BIN=$(find_qmake)

if [ -z "${QMAKE_BIN}" ]; then
    error "qmake not found! Please install Qt6:"
    echo ""
    echo "  brew install qt6"
    echo ""
    echo "Then add it to your PATH:"
    echo "  export PATH=\"/opt/homebrew/opt/qt6/bin:\$PATH\""
    echo ""
    echo "Or download from: https://www.qt.io/download"
    exit 1
fi

info "Using qmake: ${QMAKE_BIN}"

# Check Qt version
QT_VERSION=$("${QMAKE_BIN}" --version 2>&1 | grep -oE "Qt version [0-9]+\.[0-9]+\.[0-9]+" | head -1)
info "Qt version: ${QT_VERSION}"

# ======================================================
# Step 2: Generate protobuf files
# ======================================================

info "Checking protobuf files..."

QPB_FILES=$(ls "${SCRIPT_DIR}/proto"/*.qpb.h 2>/dev/null | wc -l | tr -d ' ')

if [ "${QPB_FILES}" -eq 0 ]; then
    warn "Protobuf C++ files not found. Generating..."

    if [ -x "${SCRIPT_DIR}/generate_proto.sh" ]; then
        bash "${SCRIPT_DIR}/generate_proto.sh"
    else
        error "generate_proto.sh not found and not executable!"
        exit 1
    fi

    # Verify generation
    QPB_FILES=$(ls "${SCRIPT_DIR}/proto"/*.qpb.h 2>/dev/null | wc -l | tr -d ' ')
    if [ "${QPB_FILES}" -eq 0 ]; then
        warn "qtprotobufgen may not have generated files."
        warn "This could be because Qt's qmake protobuf support is included in the .pro file."
        warn "Continuing with build - qmake will handle proto generation if supported."
    else
        info "Generated ${QPB_FILES} protobuf header files."
    fi
else
    info "Found ${QPB_FILES} protobuf header files."
fi

# ======================================================
# Step 3: Handle special commands
# ======================================================

case "${BUILD_TYPE}" in
    proto)
        info "Proto generation complete. Exiting."
        exit 0
        ;;
    clean)
        info "Cleaning build directory..."
        rm -rf "${BUILD_DIR}"
        info "Cleaning generated proto C++ files..."
        rm -f "${SCRIPT_DIR}/proto"/*.qpb.h "${SCRIPT_DIR}/proto"/*.qpb.cpp
        info "Clean complete."
        exit 0
        ;;
esac

# ======================================================
# Step 4: Run qmake
# ======================================================

info "Running qmake..."
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Determine qmake config
if [ "${BUILD_TYPE}" = "debug" ]; then
    QMAKE_CONFIG="CONFIG+=debug CONFIG+=qml_debug"
else
    QMAKE_CONFIG="CONFIG+=release"
fi

"${QMAKE_BIN}" "${SCRIPT_DIR}/ChatClient.pro" ${QMAKE_CONFIG}

# ======================================================
# Step 5: Build with make
# ======================================================

info "Building..."

# Use number of CPU cores for parallel build
NPROC=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)
make -j"${NPROC}"

# ======================================================
# Step 6: Done
# ======================================================

info "Build successful!"
info "Executable: ${BUILD_DIR}/ChatClient.app/Contents/MacOS/ChatClient"
info "or:         ${BUILD_DIR}/ChatClient (if not building as app bundle)"

# ======================================================
# Step 7: Run (if requested)
# ======================================================

if [ "${BUILD_TYPE}" = "run" ]; then
    info "Running ChatClient..."

    if [ -d "${BUILD_DIR}/ChatClient.app" ]; then
        open "${BUILD_DIR}/ChatClient.app"
    elif [ -x "${BUILD_DIR}/ChatClient" ]; then
        "${BUILD_DIR}/ChatClient"
    else
        error "Cannot find executable to run."
        exit 1
    fi
fi
