#include "ui.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "tray.h"
#include "version.h"

void tray_sync(const Dev *d, int n, int sa, int shown);
int tray_act(int e, int *shown, int *sa);

void *glfwGetCurrentContext(void); /* bundled in libraylib.a; lets us veto the close button */
int glfwWindowShouldClose(void *);
void glfwSetWindowShouldClose(void *, int);

static const struct { int key; char what; } km[] = { {KEY_ENTER, 'c'}, {KEY_P, 'p'}, {KEY_T, 't'}, {KEY_B, 'b'}, {KEY_X, 'x'} };

/* unnamed devices only have their address as alias (AA-BB-...) */
static int unnamed(const Dev *d) {
	for (int i = 0; d->addr[i]; i++)
		if ((d->addr[i] == ':' ? '-' : d->addr[i]) != d->name[i]) return 0;
	return 1;
}

int demo_list(Dev *d, int max);

int main(int argc, char **argv) {
	Dev all[MAXD], v[MAXD];
	int nall = 0, nv, sa = 0, sel = 0, hide = 0, shown = 1, quit = 0;
	char want[20] = "";
	float scroll = 0, um = 1; /* um: ctrl +/- zoom on top of the automatic fit */
	double last = -9;
	int demo = argc > 1 && !strcmp(argv[1], "--demo"), rc = cli(argc, argv);
	if (rc >= 0) return rc;
	if (!demo && bt_init() < 0) return 1;
	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
	InitWindow(1400, 860, "Nanoblue"), SetWindowMinSize(320, 240);
	SetExitKey(KEY_NULL);
	fonts_init();
	if (!demo) tray_init(), upd_start();
	while (!quit) {
		double t = GetTime();
		if (glfwWindowShouldClose(glfwGetCurrentContext())) { /* checked first so closing never waits on BlueZ */
			glfwSetWindowShouldClose(glfwGetCurrentContext(), 0);
			if (tray_reg) shown = 0, SetWindowState(FLAG_WINDOW_HIDDEN); else break;
		}
		bt_process();
		if ((bt_dirty && t - last > 0.1) || t - last > 2) {
			int n = demo ? demo_list(all, MAXD) : bt_list(all, MAXD);
			if (n >= 0) nall = n;
			bt_dirty = 0, last = t;
		}
		int tm = term_on() || adp_panel_on();
		tray_process();
		if (!shown && !tray_reg) shown = 1, ClearWindowState(FLAG_WINDOW_HIDDEN); /* tray host gone: bring the window back */
		if (tray_ev) quit = tray_act(tray_ev, &shown, &sa), tray_ev = 0;
		tray_sync(all, nall, sa, shown);
		if (!shown) { PollInputEvents(); WaitTime(0.05); continue; }
		if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) {
			if (IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD)) um = um < 2 ? um + 0.1f : um;
			if (IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT)) um = um > 0.6f ? um - 0.1f : um;
		}
		if (!tm && IsKeyPressed(KEY_Q)) break;
		if (!tm && IsKeyPressed(KEY_E)) adp_panel_open(sa);
		if (!tm && IsKeyPressed(KEY_TAB) && bt_nads) sa = (sa + 1) % bt_nads;
		if (!tm && IsKeyPressed(KEY_F)) hide = !hide;
		if (sa >= bt_nads) sa = 0;
		nv = 0;
		for (int i = 0; i < nall; i++)
			if (all[i].adapter == sa && !(hide && unnamed(&all[i]))) v[nv++] = all[i];
		for (int i = 0; i < nv; i++)
			if (!strcmp(v[i].addr, want)) sel = i;
		if (sel >= nv) sel = nv ? nv - 1 : 0;
		if (!tm && IsKeyPressed(KEY_DOWN) && sel < nv - 1) sel++;
		if (!tm && IsKeyPressed(KEY_UP) && sel > 0) sel--;
		if (!tm && IsKeyPressed(KEY_S)) adp_act(sa, 's');
		if (!tm && IsKeyPressed(KEY_O)) adp_act(sa, 'o');
		for (int i = 0; i < 5 && nv && !tm; i++)
			if (IsKeyPressed(km[i].key)) act(&v[sel], km[i].what);
		if (!tm && IsKeyPressed(KEY_L)) term_open(NULL);
		if (!tm && nv && IsKeyPressed(KEY_R)) term_open(&v[sel]);
		ui_s = um * fminf(fmaxf(fminf((GetScreenWidth)() / 1400.f, (GetScreenHeight)() / 860.f), 0.45f), 2.5f); /* whole UI scales with the window */
		int W = GetScreenWidth(), H = GetScreenHeight();
		BeginDrawing();
		rlScalef(ui_s, ui_s, 1);
		backdrop(W, H);
		sidebar((Rectangle){0, 48, 220, H - 76}, &sa);
		if (term_on()) term_frame((Rectangle){220, 48, W - 220, H - 76});
		else if (adp_panel_on()) adp_panel((Rectangle){220, 48, W - 220, H - 76});
		else devlist((Rectangle){220, 48, W - 560, H - 76}, v, nv, &sel, &scroll), detail((Rectangle){W - 340, 48, 340, H - 76}, nv ? &v[sel] : NULL);
		chrome(W, H);
		EndDrawing();
		snprintf(want, sizeof want, "%s", nv ? v[sel].addr : "");
	}
	tray_update(&(TrayState){0});
	CloseWindow();
	return 0;
}
