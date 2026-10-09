#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
APP="build-cmake/MoeDiskScreenSaver.app"
SAVER="$HOME/Library/Screen Savers/MoeDiskScreenSaver.saver"
BIN_APP="${APP}/Contents/MacOS/MoeDiskScreenSaver"
BIN_SAVER="${SAVER}/Contents/MacOS/MoeDiskScreenSaver"

echo "=== MoeDiskScreenSaver diagnose ==="
echo "date: $(date)"
echo "uname: $(uname -a)"
echo

echo "--- Qt from build ---"
if [[ -f build-cmake/CMakeCache.txt ]]; then
  grep '^CMAKE_PREFIX_PATH' build-cmake/CMakeCache.txt || true
  QT="$(grep '^CMAKE_PREFIX_PATH:STATIC' build-cmake/CMakeCache.txt | sed 's/.*=//')"
  if [[ -n "${QT}" ]]; then
    echo "macdeployqt candidates:"
    ls -la "${QT}/bin/macdeployqt"* 2>/dev/null || echo "  (none under ${QT}/bin/)"
  fi
else
  echo "No build-cmake/CMakeCache.txt — run ./build-unix.sh first"
fi
echo

echo "--- .app bundle ---"
if [[ -d "${APP}" ]]; then
  ls -la "${BIN_APP}" 2>/dev/null || true
  echo "PlugIns/platforms:"
  ls -la "${APP}/Contents/PlugIns/platforms/" 2>/dev/null || echo "  MISSING (run deploy-qt.sh)"
  echo "Frameworks count:"
  ls "${APP}/Contents/Frameworks/" 2>/dev/null | wc -l || echo "0"
  echo "otool -L (first lines):"
  otool -L "${BIN_APP}" 2>/dev/null | head -8 || true
else
  echo "Missing ${APP}"
fi
echo

echo "--- .saver bundle ---"
if [[ -d "${SAVER}" ]]; then
  ls -la "${BIN_SAVER}" 2>/dev/null || true
  ls -la "${SAVER}/Contents/PlugIns/platforms/" 2>/dev/null || echo "  PlugIns MISSING"
  ls "${SAVER}/Contents/Frameworks/" 2>/dev/null | head -5 || echo "  Frameworks MISSING"
else
  echo "Not installed: ${SAVER}"
fi
echo

echo "--- Run test (5s, app, --screensaver) ---"
export MOEDISK_SAVER_DEBUG=1
if [[ -x "${BIN_APP}" ]]; then
  timeout 5 "${BIN_APP}" --screensaver 2>&1 || true
else
  echo "No app binary"
fi
echo
echo "Debug log (if any):"
cat /tmp/moedisk-screensaver.log 2>/dev/null || echo "(no /tmp/moedisk-screensaver.log)"
echo "=== end ==="
