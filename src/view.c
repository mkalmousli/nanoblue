#include "ui.h"
#include <math.h>
#include "version.h"

void act(const Dev *d, char w) {
	if (w == 'c') bt_call(d->path, D1, d->connected ? "Disconnect" : "Connect", NULL);
	else if (w == 'p') bt_call(d->path, D1, "Pair", NULL);
	else if (w == 't') bt_setb(d->path, D1, "Trusted", !d->trusted);
	else if (w == 'b') bt_setb(d->path, D1, "Blocked", !d->blocked);
	else if (w == 'x') bt_call(bt_ads[d->adapter].path, A1, "RemoveDevice", d->path);
}

void adp_act(int i, char w) {
	if (i < 0 || i >= bt_nads) return;
	Adp *a = &bt_ads[i];
	if (w == 'o') bt_setb(a->path, A1, "Powered", !a->powered);
	else if (w == 's' && a->powered) bt_call(a->path, A1, a->scanning ? "StopDiscovery" : "StartDiscovery", NULL);
}

void chrome(int W, int H) {
	const char *hint = "UP/DN sel  ENTER conn  P pair  T trust  B block  X rm  R serial  L port  E adapter  S scan  O power  F named  TAB adp";
	DrawRectangleGradientH(0, 0, W, 48, C_PANEL, (Color){18, 44, 78, 255});
	DrawRectangle(0, 47, W, 1, (Color){255, 255, 255, 30});
	dev_icon(28, 24, 18, "", C_CYAN);
	DrawText("NANOBLUE", 56, 11, 26, C_FG);
	DrawText(upd_msg[0] ? upd_msg : "v" NB_VERSION "  by " NB_AUTHOR, W - 12 - MeasureText(upd_msg[0] ? upd_msg : "v" NB_VERSION "  by " NB_AUTHOR, 14), 16, 14, upd_msg[0] ? C_YEL : C_DIM);
	DrawText(TextFormat("%d adapter%s", bt_nads, bt_nads == 1 ? "" : "s"), 200, 18, 18, C_DIM);
	if (bt_busy) DrawRectangleGradientH(fmodf(GetTime() * 700, W + 240) - 240, 44, 240, 4, BLANK, C_CYAN);
	DrawRectangle(0, H - 28, W, 28, C_PANEL);
	DrawText(bt_status, 16, H - 22, 14, bt_busy ? C_YEL : C_DIM);
	DrawText(TextFormat("%d fps", GetFPS()), W - 70, H - 22, 14, C_DIM);
	DrawText(hint, W - 90 - MeasureText(hint, 12), H - 20, 12, C_DIM);
}

void sidebar(Rectangle r, int *sa) {
	for (int i = 0; i < bt_nads; i++) {
		Adp *a = &bt_ads[i];
		Rectangle c = {r.x + 10, r.y + 10 + i * 178, r.width - 20, 168};
		int sel = i == *sa;
		Color st = a->scanning ? C_CYAN : a->powered ? C_GREEN : C_RED;
		if (CheckCollisionPointRec(GetMousePosition(), c) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) *sa = i;
		card(c, sel ? C_HOT : C_PANEL, 1);
		DrawRectangle(c.x + 1, c.y + 14, 3, c.height - 28, sel ? C_CYAN : ColorAlpha(st, 0.6f));
		DrawText(TextFormat("%.14s", a->name), c.x + 16, c.y + 10, 20, C_FG);
		DrawText(TextFormat("%s  %s", a->path + 11, a->addr), c.x + 16, c.y + 36, 12, C_DIM);
		DrawText(TextFormat("%s via %s", a->driver, a->bus), c.x + 16, c.y + 52, 12, C_DIM);
		DrawText(a->scanning ? "SCANNING" : a->powered ? "IDLE" : "OFF", c.x + 16, c.y + 72, 14, st);
		if (a->scanning) radar(c.x + c.width - 34, c.y + 34, 24, C_CYAN);
		else led(c.x + c.width - 34, c.y + 34, 7, st, a->powered);
		if (button((Rectangle){c.x + 16, c.y + 98, 78, 30}, "POWER", a->powered ? C_GREEN : C_DIM)) adp_act(i, 'o');
		if (button((Rectangle){c.x + 102, c.y + 98, 78, 30}, "SCAN", a->scanning ? C_CYAN : C_DIM)) adp_act(i, 's');
		if (button((Rectangle){c.x + 16, c.y + 132, 164, 30}, "SETTINGS", C_BLUE)) adp_panel_open(i);
	}
	if (button((Rectangle){r.x + 10, r.y + r.height - 48, r.width - 20, 38}, "SERIAL PORT", C_CYAN)) term_open(NULL);
}
