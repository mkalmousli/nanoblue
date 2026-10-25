#include "ui.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "tray.h"

#define ND 8 /* devices listed in the tray menu */
enum { SHOW = 1, QUIT = 9, ADP = 100, DEVP = 500, DEV = 1000 }; /* menu id bases: adapter a -> ADP + a*10 + op, device k -> DEV + k*10 + op */
static Mi m[96];
static int n;
static Dev td[ND]; /* devices as listed, so a click maps back to the right one */
static int ntd;

static void add(int id, int parent, int kind, int on, const char *ic, const char *fmt, ...) {
	va_list ap;
	if (n >= 96) return;
	memset(&m[n], 0, sizeof m[n]);
	m[n].id = id, m[n].parent = parent, m[n].kind = kind, m[n].on = on;
	snprintf(m[n].icon, sizeof m[n].icon, "%s", ic);
	va_start(ap, fmt);
	vsnprintf(m[n].label, sizeof m[n].label, fmt, ap);
	va_end(ap);
	n++;
}

static void adapters(void) {
	for (int a = 0; a < bt_nads; a++) {
		const Adp *p = &bt_ads[a];
		int P = bt_nads > 1 ? 10 + a : 0, b = ADP + a * 10;
		if (P) add(P, 0, 0, 0, "bluetooth", "%s  (%s)", p->name, p->scanning ? "scanning" : p->powered ? "on" : "off");
		else add(10, 0, 3, 0, "", "%s  %s", p->name, p->addr);
		add(b, P, 2, p->powered, "", "Powered");
		add(b + 1, P, 2, p->scanning, "", "Scan for devices");
		add(b + 2, P, 2, p->discoverable, "", "Discoverable");
		add(b + 3, P, 2, p->pairable, "", "Pairable");
		add(b + 4, P, 0, 0, "preferences-system", "Adapter settings...");
	}
}

static void devices(const Dev *d, int cnt) {
	for (int pass = 0; pass < 2; pass++)
		for (int i = 0; i < cnt && ntd < ND; i++)
			if ((pass ? !d[i].connected : d[i].connected) && d[i].paired) td[ntd++] = d[i];
	for (int k = 0; k < ntd; k++) {
		const Dev *x = &td[k];
		int P = DEVP + k, b = DEV + k * 10;
		add(P, 0, 0, 0, x->connected ? "bluetooth-active" : "bluetooth-paired", "%s%s", x->connected ? "● " : "", x->name);
		add(b, P, 0, 0, "", x->connected ? "Disconnect" : "Connect");
		add(b + 1, P, 2, x->trusted, "", "Trusted");
		add(b + 2, P, 0, 0, "utilities-terminal", "Serial terminal");
		add(b + 3, P, 0, 0, "edit-delete", "Forget device");
		if (x->rssi) add(b + 9, P, 3, 0, "", "%d dBm  %d%%", x->rssi, x->battery);
	}
	if (!ntd) add(20, 0, 3, 0, "", "No paired devices");
}

/* pushes adapter / connection state to the tray icon and menu */
void tray_sync(const Dev *d, int cnt, int sa, int shown) {
	int a = sa < bt_nads ? sa : 0;
	TrayState s = { shown, bt_nads && bt_ads[a].powered, bt_nads && bt_ads[a].scanning, 0 };
	for (int i = 0; i < cnt; i++) s.connected |= d[i].connected;
	tray_update(&s);
	if (!tray_bus) return;
	n = ntd = 0;
	add(SHOW, 0, 0, 0, shown ? "window-minimize" : "window-new", shown ? "Hide window" : "Show window");
	add(2, 0, 1, 0, "", "");
	adapters();
	add(3, 0, 1, 0, "", "");
	devices(d, cnt);
	add(4, 0, 1, 0, "", "");
	add(QUIT, 0, 0, 0, "application-exit", "Quit Nanoblue");
	tray_menu(m, n);
}

static void raise_window(int *shown) { *shown = 1, ClearWindowState(FLAG_WINDOW_HIDDEN); }

/* handles a tray click (1 = icon, else menu id); returns 1 to quit */
int tray_act(int e, int *shown, int *sa) {
	if (e == QUIT) return 1;
	if (e == TRAY_RAISE) return raise_window(shown), SetWindowFocused(), 0;
	if (e == 1) *shown ? SetWindowState(FLAG_WINDOW_HIDDEN) : ClearWindowState(FLAG_WINDOW_HIDDEN), *shown = !*shown;
	else if (e >= ADP && e < ADP + 10 * MAXA) {
		int a = (e - ADP) / 10, op = (e - ADP) % 10;
		if (a >= bt_nads) return 0;
		if (op == 0) adp_act(a, 'o');
		else if (op == 1) adp_act(a, 's');
		else if (op < 4) bt_setb(bt_ads[a].path, A1, op == 2 ? "Discoverable" : "Pairable", !(op == 2 ? bt_ads[a].discoverable : bt_ads[a].pairable));
		else *sa = a, adp_panel_open(a), raise_window(shown);
	} else if (e >= DEV && e < DEV + 10 * ND && (e - DEV) / 10 < ntd) {
		const Dev *x = &td[(e - DEV) / 10];
		int op = (e - DEV) % 10;
		if (op == 0) act(x, 'c');
		else if (op == 1) act(x, 't');
		else if (op == 2) term_open(x), raise_window(shown);
		else if (op == 3) act(x, 'x');
	}
	return 0;
}
