#!/bin/sh
# Nanoblue uninstaller:  curl -fsSL https://raw.githubusercontent.com/mkalmousli/nanoblue/main/scripts/uninstall.sh | sh
D=${XDG_DATA_HOME:-$HOME/.local/share}
pkill -x nanoblue 2>/dev/null || true
rm -f "${NANOBLUE_DEST:-$HOME/.local/bin/nanoblue}" "$D/applications/nanoblue.desktop" "$D/icons/hicolor/scalable/apps/nanoblue.svg" \
	"${XDG_RUNTIME_DIR:-/tmp}"/nanoblue-*.png
update-desktop-database "$D/applications" 2>/dev/null || true
echo "Nanoblue removed."
