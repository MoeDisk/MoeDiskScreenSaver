#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
OUT="${1:-${PROJECT_ROOT}/build-macos/MoeDiskScreenSaver.saver}"
CONTENTS="${OUT}/Contents"
MACOS_DIR="${CONTENTS}/MacOS"
RESOURCES_DIR="${CONTENTS}/Resources"
EXECUTABLE="${MACOS_DIR}/MoeDiskScreenSaver"

rm -rf "${OUT}"
mkdir -p "${MACOS_DIR}" "${RESOURCES_DIR}"

xcrun clang \
  -fobjc-arc \
  -bundle \
  -arch arm64 \
  -arch x86_64 \
  -mmacosx-version-min=12.0 \
  -framework AppKit \
  -framework QuartzCore \
  -framework ScreenSaver \
  "${PROJECT_ROOT}/src/macos/MoeDiskScreenSaverView.m" \
  -o "${EXECUTABLE}"

cp "${SCRIPT_DIR}/ScreenSaver-Info.plist" "${CONTENTS}/Info.plist"
cp "${PROJECT_ROOT}/resources/DVDVideo360.png" "${RESOURCES_DIR}/DVDVideo360.png"

codesign --force --deep --sign - "${OUT}"
codesign --verify --deep --strict --verbose=2 "${OUT}"

echo "Created: ${OUT}"
