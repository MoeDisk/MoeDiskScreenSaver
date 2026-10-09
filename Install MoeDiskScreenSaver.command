#!/bin/zsh
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$PROJECT_ROOT"

bash packaging/macos/quick-install.sh

echo ""
echo "安装完成。请完全退出并重新打开“系统设置”，然后重新选择 MoeDiskScreenSaver。"
echo "如果系统仍显示旧缓存，请注销或重启 Mac 一次。"
echo "按回车键关闭窗口。"
read -r
