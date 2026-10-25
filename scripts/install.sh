#!/bin/sh
# Nanoblue installer (Linux):  curl -fsSL https://raw.githubusercontent.com/mkalmousli/nanoblue/main/scripts/install.sh | sh
# --bin-only: only replace the binary (used by the auto updater).  NANOBLUE_DEST overrides the binary path.
set -e
REPO=mkalmousli/nanoblue
RAW=https://raw.githubusercontent.com/$REPO/main
[ "$(uname -s)" = Linux ] || { echo "Nanoblue needs Linux (it talks to BlueZ)"; exit 1; }
case $(uname -m) in
	x86_64|amd64) A=x86_64 ;; aarch64|arm64) A=aarch64 ;; armv7l|armv8l) A=armv7 ;;
	i?86) A=i386 ;; riscv64) A=riscv64 ;; *) echo "unsupported architecture: $(uname -m)"; exit 1 ;;
esac
DEST=${NANOBLUE_DEST:-$HOME/.local/bin/nanoblue}
D=${XDG_DATA_HOME:-$HOME/.local/share}
mkdir -p "$(dirname "$DEST")"
curl -fsSL -o "$DEST.new" "https://github.com/$REPO/releases/latest/download/nanoblue-linux-$A"
chmod 755 "$DEST.new" && mv -f "$DEST.new" "$DEST"
if [ "$1" != --bin-only ]; then
	mkdir -p "$D/applications" "$D/icons/hicolor/scalable/apps"
	curl -fsSL -o "$D/icons/hicolor/scalable/apps/nanoblue.svg" "$RAW/assets/logo.svg"
	cat > "$D/applications/nanoblue.desktop" <<EOT
[Desktop Entry]
Type=Application
Name=Nanoblue
Comment=Fast Bluetooth manager and serial terminal
Exec=$DEST
Icon=nanoblue
Categories=Utility;System;Settings;
Keywords=bluetooth;serial;rfcomm;
Terminal=false
EOT
	update-desktop-database "$D/applications" 2>/dev/null || true
	gtk-update-icon-cache -q -t "$D/icons/hicolor" 2>/dev/null || true
	echo "Installed Nanoblue to $DEST"
	case ":$PATH:" in *":$(dirname "$DEST"):"*) ;; *) echo "Add $(dirname "$DEST") to your PATH to run 'nanoblue' from a terminal." ;; esac
	if command -v ldd >/dev/null && ldd "$DEST" | grep -q "not found"; then
		echo "Missing libraries: $(ldd "$DEST" | awk '/not found/{print $1}' | tr '\n' ' ')"
		echo "Debian/Ubuntu: sudo apt install libdbus-1-3 libgl1 libx11-6   |  Fedora: sudo dnf install dbus-libs mesa-libGL libX11"
	fi
fi
