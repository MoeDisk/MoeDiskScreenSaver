#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
APP="${1:-build-cmake/MoeDiskScreenSaver.app}"
CACHE="build-cmake/CMakeCache.txt"

if [[ ! -d "${APP}" ]]; then
  echo "Missing ${APP}. Run ./build-unix.sh first." >&2
  exit 1
fi

DEPLOY=""
if [[ -f "${CACHE}" ]]; then
  QT="$(sed -n 's/^CMAKE_PREFIX_PATH:[^=]*=//p' "${CACHE}" | head -n 1)"
  if [[ -n "${QT}" && -x "${QT}/bin/macdeployqt" ]]; then
    DEPLOY="${QT}/bin/macdeployqt"
  elif [[ -n "${QT}" && -x "${QT}/bin/macdeployqt6" ]]; then
    DEPLOY="${QT}/bin/macdeployqt6"
  fi
fi

if [[ -z "${DEPLOY}" ]]; then
  bash packaging/macos/deploy-qt.sh "${APP}"
  exit $?
fi

echo "Using ${DEPLOY}"
"${DEPLOY}" "${APP}" -always-overwrite

if [[ ! -f "${APP}/Contents/PlugIns/platforms/libqcocoa.dylib" ]]; then
  echo "ERROR: libqcocoa.dylib still missing after macdeployqt." >&2
  exit 1
fi
echo "OK: Qt plugins deployed."
