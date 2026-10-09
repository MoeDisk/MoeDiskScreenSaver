#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD="${ROOT}/build-cmake"

qt_prefix_from_qmake() {
  local qmake="$1"
  if [[ -x "${qmake}" ]]; then
    "${qmake}" -query QT_INSTALL_PREFIX 2>/dev/null || true
  fi
}

find_qt_cmake_prefix() {
  local candidate prefix qmake

  if [[ -n "${CMAKE_PREFIX_PATH:-}" ]]; then
    echo "${CMAKE_PREFIX_PATH}"
    return 0
  fi

  if [[ -n "${QT_PREFIX_PATH:-}" ]]; then
    echo "${QT_PREFIX_PATH}"
    return 0
  fi

  while IFS= read -r qmake; do
    prefix="$(qt_prefix_from_qmake "${qmake}")"
    if [[ -n "${prefix}" && ( -f "${prefix}/lib/cmake/Qt6/Qt6Config.cmake" || -f "${prefix}/lib/cmake/Qt5/Qt5Config.cmake" ) ]]; then
      echo "${prefix}"
      return 0
    fi
  done < <(command -v qmake6 2>/dev/null; command -v qmake 2>/dev/null; true)

  if command -v brew >/dev/null 2>&1; then
    for formula in qt qt@6 qt@5; do
      prefix="$(brew --prefix "${formula}" 2>/dev/null || true)"
      if [[ -n "${prefix}" && ( -f "${prefix}/lib/cmake/Qt6/Qt6Config.cmake" || -f "${prefix}/lib/cmake/Qt5/Qt5Config.cmake" ) ]]; then
        echo "${prefix}"
        return 0
      fi
    done
  fi

  for candidate in \
    /opt/homebrew/opt/qt \
    /opt/homebrew/opt/qt@6 \
    /opt/homebrew/opt/qt@5 \
    /usr/local/opt/qt \
    /usr/local/opt/qt@6 \
    /usr/local/opt/qt@5 \
    "${HOME}/Qt/6.*/macos" \
    "${HOME}/Qt/5.*/macos"; do
    for prefix in ${candidate}; do
      if [[ -d "${prefix}" && ( -f "${prefix}/lib/cmake/Qt6/Qt6Config.cmake" || -f "${prefix}/lib/cmake/Qt5/Qt5Config.cmake" ) ]]; then
        echo "${prefix}"
        return 0
      fi
    done
  done

  return 1
}

if ! QT_PREFIX="$(find_qt_cmake_prefix)"; then
  cat >&2 <<'EOF'
Could not find Qt 5 or Qt 6 for CMake.

Install Qt, then re-run ./build-unix.sh

  macOS (Homebrew, recommended):
    brew install qt
    export PATH="$(brew --prefix qt)/bin:$PATH"

  Or Qt 5 only:
    brew install qt@5
    export PATH="$(brew --prefix qt@5)/bin:$PATH"

  Or point CMake manually:
    export CMAKE_PREFIX_PATH="/path/to/qt/prefix"
    ./build-unix.sh

  Qt online installer (example):
    export CMAKE_PREFIX_PATH="$HOME/Qt/6.8.0/macos"
EOF
  exit 1
fi

echo "Using Qt from: ${QT_PREFIX}"

mkdir -p "${BUILD}"
cd "${BUILD}"

cmake "${ROOT}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="${QT_PREFIX}"

cmake --build . --config Release -j"$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)"

if [[ "$(uname -s)" == "Darwin" && -x "${ROOT}/packaging/macos/deploy-qt.sh" ]]; then
  bash "${ROOT}/packaging/macos/deploy-qt.sh" "${BUILD}/MoeDiskScreenSaver.app" || true
fi

echo
if [[ "$(uname -s)" == "Darwin" ]]; then
  BIN="${BUILD}/MoeDiskScreenSaver.app/Contents/MacOS/MoeDiskScreenSaver"
  echo "App bundle: ${BUILD}/MoeDiskScreenSaver.app"
  echo "Binary: ${BIN}"
  echo "Run windowed: open \"${BUILD}/MoeDiskScreenSaver.app\""
  echo "Run screen saver: \"${BIN}\" --screensaver"
  echo "Optional .saver bundle:"
  echo "  bash \"${ROOT}/packaging/macos/quick-install.sh\""
else
  echo "Binary: ${BUILD}/MoeDiskScreenSaver"
fi
