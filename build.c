/* cc build.c -o build && ./build            builds bin/nanoblue (first run clones and builds third_party/raylib)
   ./build install                           also installs binary, icon and menu entry under ~/.local
   ./build uninstall                         removes them again */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int sh(const char *fmt, const char *a) {
	char c[1024];
	snprintf(c, sizeof c, fmt, a, a, a, a);
	return system(c);
}

static int install(void) {
	const char *h = getenv("HOME");
	FILE *f;
	char p[512];
	if (!h) return 1;
	sh("mkdir -p %s/.local/bin %s/.local/share/applications %s/.local/share/icons/hicolor/scalable/apps", h);
	if (sh("install -m755 bin/nanoblue %s/.local/bin/nanoblue", h)) return 1;
	sh("install -m644 assets/logo.svg %s/.local/share/icons/hicolor/scalable/apps/nanoblue.svg", h);
	snprintf(p, sizeof p, "%s/.local/share/applications/nanoblue.desktop", h);
	if (!(f = fopen(p, "w"))) return 1;
	fprintf(f, "[Desktop Entry]\nType=Application\nName=Nanoblue\nComment=Fast Bluetooth manager and serial terminal\n"
	           "Exec=%s/.local/bin/nanoblue\nIcon=nanoblue\nCategories=Utility;System;Settings;\nKeywords=bluetooth;serial;rfcomm;\nTerminal=false\n", h);
	fclose(f);
	sh("gtk-update-icon-cache -q -t %s/.local/share/icons/hicolor 2>/dev/null; update-desktop-database %s/.local/share/applications 2>/dev/null; true", h);
	printf("installed: %s/.local/bin/nanoblue (make sure it is in PATH)\n", h);
	return 0;
}

int main(int argc, char **argv) {
	if (argc > 1 && !strcmp(argv[1], "uninstall")) return system("sh scripts/uninstall.sh");
	mkdir("bin", 0755);
	if (access("third_party/raylib/src", F_OK) && system("git clone -q --depth 1 --branch 5.5 https://github.com/raysan5/raylib.git third_party/raylib")) return 1;
	if (access("third_party/raylib/src/libraylib.a", F_OK) && system("make -s -j8 -C third_party/raylib/src PLATFORM=PLATFORM_DESKTOP")) return 1;
	if (system("cc -O2 -Wall -Wextra -Wno-unused-parameter -Ithird_party/raylib/src -Isrc -o bin/nanoblue src/*.c "
	           "third_party/raylib/src/libraylib.a $(pkg-config --cflags --libs dbus-1) -lGL -lX11 -lm -lpthread -ldl -lrt")) return 1;
	return argc > 1 && !strcmp(argv[1], "install") ? install() : 0;
}
