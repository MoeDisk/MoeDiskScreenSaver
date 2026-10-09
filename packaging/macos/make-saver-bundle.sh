#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/../.." && pwd)"
DEFAULT_APP="${PROJECT_ROOT}/build-cmake/MoeDiskScreenSaver.app"

usage() {
  cat <<EOF
Usage: $(basename "$0") [PATH] [OUTPUT.saver]

PATH may be:
  - MoeDiskScreenSaver.app          (recommended, from build-cmake/)
  - .../MoeDiskScreenSaver.app/Contents/MacOS/MoeDiskScreenSaver
  - plain binary (must bundle Qt yourself)

With no PATH, uses: ${DEFAULT_APP}

Example (from project root):
  ./packaging/macos/make-saver-bundle.sh build-cmake/MoeDiskScreenSaver.app
  cp -R "packaging/macos/MoeDiskScreenSaver.saver" ~/Library/Screen\\ Savers/
EOF
}

resolve_input() {
  local arg="${1:-${DEFAULT_APP}}"
  if [[ "${arg}" == "-h" || "${arg}" == "--help" ]]; then
    usage
    exit 0
  fi
  if [[ ! -e "${arg}" ]]; then
    echo "Not found: ${arg}" >&2
    echo >&2
    usage >&2
    exit 1
  fi
  echo "${arg}"
}

deploy_qt_if_needed() {
  local app_path="$1"
  if [[ -d "${app_path}/Contents/Frameworks" ]]; then
    return 0
  fi
  if command -v macdeployqt >/dev/null 2>&1; then
    echo "Running macdeployqt on ${app_path} ..."
    macdeployqt "${app_path}" -always-overwrite
  else
    echo "Warning: no Qt Frameworks in .app. Install Qt tools and run:" >&2
    echo "  macdeployqt \"${app_path}\"" >&2
  fi
}

INPUT="$(resolve_input "${1:-}")"
NAME="MoeDiskScreenSaver.saver"
OUT="${2:-${SCRIPT_DIR}/${NAME}}"
MACOS_DIR="${OUT}/Contents/MacOS"

APP_PATH=""
BIN=""

if [[ -d "${INPUT}" && "${INPUT}" == *.app ]]; then
  APP_PATH="${INPUT}"
  deploy_qt_if_needed "${APP_PATH}"
  BIN="${APP_PATH}/Contents/MacOS/MoeDiskScreenSaver"
elif [[ -f "${INPUT}" ]]; then
  BIN="${INPUT}"
  if [[ "${BIN}" == */Contents/MacOS/* ]]; then
    APP_PATH="$(cd "$(dirname "${BIN}")/../.." && pwd)"
  fi
else
  echo "Unsupported path: ${INPUT}" >&2
  exit 1
fi

if [[ ! -f "${BIN}" ]]; then
  echo "Executable not found: ${BIN}" >&2
  exit 1
fi

rm -rf "${OUT}"
mkdir -p "${MACOS_DIR}"
cp -f "${BIN}" "${MACOS_DIR}/MoeDiskScreenSaver"
chmod +x "${MACOS_DIR}/MoeDiskScreenSaver"

if [[ -n "${APP_PATH}" && -d "${APP_PATH}/Contents/Frameworks" ]]; then
  rm -rf "${OUT}/Contents/Frameworks"
  cp -R "${APP_PATH}/Contents/Frameworks" "${OUT}/Contents/Frameworks"
fi

if [[ -n "${APP_PATH}" && -d "${APP_PATH}/Contents/PlugIns" ]]; then
  rm -rf "${OUT}/Contents/PlugIns"
  cp -R "${APP_PATH}/Contents/PlugIns" "${OUT}/Contents/PlugIns"
fi

cat > "${OUT}/Contents/Info.plist" <<'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleDevelopmentRegion</key>
	<string>en</string>
	<key>CFBundleExecutable</key>
	<string>MoeDiskScreenSaver</string>
	<key>CFBundleIdentifier</key>
	<string>com.moedisk.screensaver</string>
	<key>CFBundleInfoDictionaryVersion</key>
	<string>6.0</string>
	<key>CFBundleName</key>
	<string>MoeDiskScreenSaver</string>
	<key>CFBundlePackageType</key>
	<string>BNDL</string>
	<key>CFBundleShortVersionString</key>
	<string>1.0.1</string>
	<key>CFBundleVersion</key>
	<string>1.0.1</string>
	<key>NSHumanReadableCopyright</key>
	<string>Copyright (C) MoeDisk</string>
</dict>
</plist>
PLIST

echo "Created: ${OUT}"
echo
echo "Install:"
echo "  cp -R \"${OUT}\" ~/Library/Screen\\ Savers/"
echo
echo "Test without installing:"
echo "  \"${MACOS_DIR}/MoeDiskScreenSaver\" --screensaver"
echo "  open \"${APP_PATH:-${PROJECT_ROOT}/build-cmake/MoeDiskScreenSaver.app}\""
