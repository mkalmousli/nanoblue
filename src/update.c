#include "version.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define RAW "https://raw.githubusercontent.com/" NB_REPO "/main/"
char upd_msg[96];

/* newer(a, b): dotted version a > b */
static int newer(const char *a, const char *b) {
	int x[3] = {0}, y[3] = {0};
	sscanf(a, "%d.%d.%d", x, x + 1, x + 2), sscanf(b, "%d.%d.%d", y, y + 1, y + 2);
	for (int i = 0; i < 3; i++) if (x[i] != y[i]) return x[i] > y[i];
	return 0;
}

static int latest(char *v, size_t n) {
	FILE *p = popen("curl -fsSL -m 8 https://api.github.com/repos/" NB_REPO "/releases/latest 2>/dev/null", "r");
	char b[8192] = "", *t;
	if (!p) return 0;
	if (fread(b, 1, sizeof b - 1, p) < 1) return pclose(p), 0;
	pclose(p);
	if (!(t = strstr(b, "\"tag_name\""))) return 0;
	return sscanf(t, "\"tag_name\"%*[: ]\"v%15[0-9.]\"", v) == 1 && n;
}

/* the installer replaces the running binary (NANOBLUE_DEST) */
static int install(const char *flags) {
	char dest[256] = "", cmd[600];
	ssize_t l = readlink("/proc/self/exe", dest, sizeof dest - 1);
	if (l > 0) dest[l] = 0;
	snprintf(cmd, sizeof cmd, "curl -fsSL " RAW "scripts/install.sh | NANOBLUE_DEST='%s' sh -s -- %s >/dev/null 2>&1", dest, flags);
	return system(cmd);
}

static void *check(void *u) {
	char v[16];
	if (!latest(v, sizeof v) || !newer(v, NB_VERSION)) return NULL;
	if (getenv("NANOBLUE_NO_UPDATE")) snprintf(upd_msg, sizeof upd_msg, "Update %s available - run: nanoblue --update", v);
	else snprintf(upd_msg, sizeof upd_msg, install("--bin-only") ? "Update %s failed - run: nanoblue --update" : "Updated to %s - restart to apply", v);
	return NULL;
}

void upd_start(void) {
	pthread_t t;
	if (!pthread_create(&t, NULL, check, NULL)) pthread_detach(t);
}

int cli(int argc, char **argv) {
	const char *a = argc > 1 ? argv[1] : "";
	char v[16];
	if (!strcmp(a, "--version")) return printf("nanoblue %s\n@" NB_HANDLE "\nhttps://github.com/" NB_REPO "\n", NB_VERSION), 0;
	if (!strcmp(a, "--update")) {
		if (!latest(v, sizeof v)) return fprintf(stderr, "cannot reach GitHub\n"), 1;
		if (!newer(v, NB_VERSION)) return printf("nanoblue %s is up to date\n", NB_VERSION), 0;
		printf("updating %s -> %s\n", NB_VERSION, v);
		return install("--bin-only") ? fputs("update failed (is the binary writable?)\n", stderr), 1 : 0;
	}
	if (!strcmp(a, "--uninstall")) return system("curl -fsSL " RAW "scripts/uninstall.sh | sh");
	if (!strcmp(a, "--help") || !strcmp(a, "-h"))
		return puts("usage: nanoblue [--demo | --version | --update | --uninstall]\n  --demo  fake devices, no Bluetooth needed\n"
		            "  env NANOBLUE_NO_UPDATE=1 disables automatic updates"), 0;
	return -1;
}
