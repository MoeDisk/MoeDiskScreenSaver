#!/usr/bin/env bash
set -euo pipefail

find_macdeployqt() {
  local p prefix

  for p in \
    "$(type -p macdeployqt 2>/dev/null || true)" \
    "$(type -p macdeployqt6 2>/dev/null || true)"; do
    if [[ -n "${p}" && -x "${p}" ]]; then
      echo "${p}"
      return 0
    fi
  done

  for p in qmake qmake6; do
    if type -p "${p}" >/dev/null 2>&1; then
      prefix="$("${p}" -query QT_INSTALL_PREFIX 2>/dev/null || true)"
      if [[ -n "${prefix}" && -x "${prefix}/bin/macdeployqt" ]]; then
        echo "${prefix}/bin/macdeployqt"
        return 0
      fi
      if [[ -n "${prefix}" && -x "${prefix}/bin/macdeployqt6" ]]; then
        echo "${prefix}/bin/macdeployqt6"
        return 0
      fi
    fi
  done

  if command -v brew >/dev/null 2>&1; then
    for formula in qt qt@6 qt@5; do
      prefix="$(brew --prefix "${formula}" 2>/dev/null || true)"
      if [[ -n "${prefix}" && -x "${prefix}/bin/macdeployqt" ]]; then
        echo "${prefix}/bin/macdeployqt"
        return 0
      fi
    done
  fi

  return 1
}

if ! DEPLOY="$(find_macdeployqt)"; then
  cat >&2 <<'EOF'
macdeployqt not found.

Your project built with Qt, but deploy tools are not on PATH.
Conda (base) often hides Homebrew Qt — try:

  which qmake
  qmake -query QT_INSTALL_PREFIX
  ls "$(qmake -query QT_INSTALL_PREFIX)/bin/macdeployqt"

If qmake is missing, install Qt:

  brew install qt
  export PATH="$(brew --prefix qt)/bin:$PATH"

Then run this script again.
EOF
  exit 1
fi

APP="${1:-build-cmake/MoeDiskScreenSaver.app}"
if [[ ! -d "${APP}" ]]; then
  echo "Missing app bundle: ${APP}" >&2
  exit 1
fi

echo "Using: ${DEPLOY}"
"${DEPLOY}" "${APP}" -always-overwrite
echo "Deployed Qt into: ${APP}"
