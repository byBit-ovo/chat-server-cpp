#!/bin/bash
# ======================================================
# generate_proto.sh
# Generate Qt protobuf C++ files (.qpb.h / .qpb.cpp)
# from .proto files using protoc + qtprotobufgen plugin.
#
# Prerequisites:
#   - Qt6 SDK installed (provides qtprotobufgen)
#   - Google protobuf installed (provides protoc)
#     brew install protobuf
# ======================================================

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROTO_DIR="${SCRIPT_DIR}/proto"
OUTPUT_DIR="${PROTO_DIR}"

# ---- Find protoc ----
if [ -n "${PROTOC}" ]; then
    PROTOC_BIN="${PROTOC}"
elif command -v protoc &>/dev/null; then
    PROTOC_BIN="protoc"
else
    echo "=============================================="
    echo " ERROR: protoc not found!"
    echo "=============================================="
    echo ""
    echo "Install Google protobuf first:"
    echo "  brew install protobuf"
    exit 1
fi

# ---- Find qtprotobufgen ----
# qtprotobufgen is in <Qt>/libexec/, not <Qt>/bin/
if [ -n "${QTPROTOBUFGEN}" ]; then
    QTPROTOBUFGEN_BIN="${QTPROTOBUFGEN}"
elif command -v qtprotobufgen &>/dev/null; then
    QTPROTOBUFGEN_BIN="qtprotobufgen"
elif [ -f "/Users/Zhuanz/Qt/6.11.1/macos/libexec/qtprotobufgen" ]; then
    QTPROTOBUFGEN_BIN="/Users/Zhuanz/Qt/6.11.1/macos/libexec/qtprotobufgen"
else
    # Try to find it under common Qt install locations
    QTPROTOBUFGEN_BIN=$(find /Users/Zhuanz/Qt -name qtprotobufgen -type f 2>/dev/null | head -1)
    if [ -z "${QTPROTOBUFGEN_BIN}" ]; then
        QTPROTOBUFGEN_BIN=$(find ~/Qt -name qtprotobufgen -type f 2>/dev/null | head -1)
    fi
fi

if [ ! -f "${QTPROTOBUFGEN_BIN}" ]; then
    echo "=============================================="
    echo " ERROR: qtprotobufgen not found!"
    echo "=============================================="
    echo ""
    echo "Install Qt6 with protobuf support via Qt Maintenance Tool."
    echo "Or set QTPROTOBUFGEN to the full path:"
    echo "  export QTPROTOBUFGEN=/path/to/qtprotobufgen"
    exit 1
fi

echo "Using protoc:         ${PROTOC_BIN}"
echo "Using qtprotobufgen:  ${QTPROTOBUFGEN_BIN}"
echo "Proto directory:      ${PROTO_DIR}"
echo "Output directory:     ${OUTPUT_DIR}"
echo ""

# Clean old generated files
echo "Cleaning old generated protobuf files..."
rm -f "${PROTO_DIR}"/*.qpb.h "${PROTO_DIR}"/*.qpb.cpp

# Proto files
PROTO_FILES=(
    "base.proto"
    "file.proto"
    "friend.proto"
    "gateway.proto"
    "message_storage.proto"
    "message_transmit.proto"
    "notify.proto"
    "speech_recognition.proto"
    "user.proto"
)

echo "Generating Qt protobuf C++ files..."

# Build protoc args
PROTO_ARGS=()
for pf in "${PROTO_FILES[@]}"; do
    PROTO_ARGS+=("${PROTO_DIR}/${pf}")
done

# Run protoc with qtprotobufgen plugin
# qtprotobufgen is a protoc plugin, not a standalone tool
"${PROTOC_BIN}" \
    --plugin=protoc-gen-qtprotobuf="${QTPROTOBUFGEN_BIN}" \
    --qtprotobuf_out="${OUTPUT_DIR}" \
    -I"${PROTO_DIR}" \
    "${PROTO_ARGS[@]}"

echo ""
echo "=============================================="
echo " Done! Generated files in: ${OUTPUT_DIR}"
echo "=============================================="
ls -la "${OUTPUT_DIR}"/*.qpb.h "${OUTPUT_DIR}"/*.qpb.cpp 2>/dev/null
