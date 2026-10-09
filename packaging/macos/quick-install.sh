#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/../.."
OUT="build-macos/MoeDiskScreenSaver.saver"

bash packaging/macos/build-native-saver.sh "${OUT}"

mkdir -p "${HOME}/Library/Screen Savers"
rm -rf "${HOME}/Library/Screen Savers/MoeDiskScreenSaver.saver"
ditto "${OUT}" "${HOME}/Library/Screen Savers/MoeDiskScreenSaver.saver"
echo "Installed: ${HOME}/Library/Screen Savers/MoeDiskScreenSaver.saver"
echo "Open System Settings → Screen Saver → choose MoeDiskScreenSaver"
