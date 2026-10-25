#include "bt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

/* one address per line */
static const char *file(void) {
	static char p[256];
	snprintf(p, sizeof p, "%s/.config/nanoblue/auto", getenv("HOME") ?: "");
	return p;
}

static int rw(const char *addr, int flip) {
	char line[40], all[4096] = "";
	int has = 0;
	FILE *f = fopen(file(), "r");
	while (f && fgets(line, sizeof line, f)) {
		line[strcspn(line, "\n")] = 0;
		if (!strcmp(line, addr)) has = 1;
		else if (line[0] && strlen(all) < sizeof all - 40) strcat(all, line), strcat(all, "\n");
	}
	if (f) fclose(f);
	if (!flip) return has;
	if (!has) strcat(all, addr), strcat(all, "\n");
	char dir[256];
	snprintf(dir, sizeof dir, "%s/.config/nanoblue", getenv("HOME") ?: ""), mkdir(dir, 0755);
	if ((f = fopen(file(), "w"))) fputs(all, f), fclose(f);
	return !has;
}

int auto_on(const char *addr) { return rw(addr, 0); }
void auto_toggle(const char *addr) { rw(addr, 1); }

void auto_tick(const Dev *d, int n) {
	static struct { char addr[20]; time_t t; } last[MAXD];
	time_t now = time(NULL);
	for (int i = 0; i < n && !bt_busy; i++) {
		if (!d[i].paired || d[i].connected || d[i].blocked || !bt_ads[d[i].adapter].powered || !auto_on(d[i].addr)) continue;
		int k = 0;
		while (k < MAXD - 1 && last[k].addr[0] && strcmp(last[k].addr, d[i].addr)) k++;
		if (last[k].addr[0] && now - last[k].t < 20) continue;
		cp(last[k].addr, sizeof last[k].addr, d[i].addr), last[k].t = now;
		bt_call(d[i].path, "org.bluez.Device1", "Connect", NULL);
	}
}
