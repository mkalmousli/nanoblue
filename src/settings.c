#include "ui.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

static int open_, ai, editing;
static char buf[64];

int adp_panel_on(void) { return open_; }
void adp_panel_open(int a) { open_ = 1, ai = a, editing = 0; }

static void row(Rectangle r, float y, const char *k, const char *v) {
	DrawText(k, r.x + 24, y, 16, C_DIM);
	DrawText(v, r.x + 230, y, 16, C_FG);
}

static void flag(Rectangle r, float y, const char *k, const char *prop, int on, Adp *a) {
	DrawText(k, r.x + 24, y + 4, 18, C_FG);
	if (toggle((Rectangle){r.x + 230, y, 52, 28}, on, C_GREEN)) bt_setb(a->path, A1, prop, !on);
}

static void timeout(Rectangle r, float y, const char *k, const char *prop, unsigned v, Adp *a) {
	DrawText(k, r.x + 24, y + 4, 18, C_FG);
	DrawText(v ? TextFormat("%u s", v) : "never", r.x + 270, y + 4, 18, C_CYAN);
	if (button((Rectangle){r.x + 230, y, 30, 28}, "-", C_DIM)) v = v > 60 ? v - 60 : 0, bt_set(a->path, A1, prop, 'u', &v);
	if (button((Rectangle){r.x + 360, y, 30, 28}, "+", C_DIM)) v += 60, bt_set(a->path, A1, prop, 'u', &v);
}

void adp_panel(Rectangle r) {
	static int soft, hard, have;
	static float off;
	static double t;
	Adp *a;
	float y = r.y + 70;
	int c;
	if (ai >= bt_nads) { open_ = 0; return; }
	a = &bt_ads[ai];
	if (GetTime() - t > 1) have = rf_state(&soft, &hard), t = GetTime();
	DrawRectangleRounded((Rectangle){r.x + 8, r.y + 8, r.width - 16, r.height - 16}, 0.03f, 8, C_PANEL);
	scroll_begin(r, &off, 480);
	DrawText(TextFormat("ADAPTER %s SETTINGS", a->path + 11), r.x + 24, r.y + 22, 24, C_CYAN);
	if (button((Rectangle){r.x + r.width - 110, r.y + 20, 86, 32}, "CLOSE", C_RED)) open_ = 0;
	DrawText("Alias", r.x + 24, y + 4, 18, C_FG);
	Rectangle f = {r.x + 230, y, 260, 32};
	if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
		int in = CheckCollisionPointRec(GetMousePosition(), f);
		if (in && !editing) snprintf(buf, sizeof buf, "%s", a->name);
		editing = in;
	}
	DrawRectangleRounded(f, 0.2f, 6, C_BG);
	DrawRectangleRoundedLines(f, 0.2f, 6, editing ? C_CYAN : C_HOT);
	DrawText(editing ? TextFormat("%s_", buf) : a->name, f.x + 8, f.y + 6, 18, C_FG);
	while (editing && (c = GetCharPressed())) if (c >= 32 && c < 127 && strlen(buf) < 40) buf[strlen(buf)] = c;
	if (editing && IsKeyPressed(KEY_BACKSPACE) && *buf) buf[strlen(buf) - 1] = 0;
	if (editing && IsKeyPressed(KEY_ENTER)) { const char *s = buf; bt_set(a->path, A1, "Alias", 's', &s); editing = 0; }
	if (editing && IsKeyPressed(KEY_ESCAPE)) editing = 0;
	DrawText("press Enter to save", f.x + f.width + 12, y + 8, 13, C_DIM);
	flag(r, y += 48, "Powered", "Powered", a->powered, a);
	flag(r, y += 40, "Discoverable", "Discoverable", a->discoverable, a);
	flag(r, y += 40, "Pairable", "Pairable", a->pairable, a);
	timeout(r, y += 40, "Discoverable timeout", "DiscoverableTimeout", a->dto, a);
	timeout(r, y += 40, "Pairable timeout", "PairableTimeout", a->pto, a);
	DrawText("RF kill (radio)", r.x + 24, y += 40 + 4, 18, C_FG);
	if (!have) DrawText("no rfkill device", r.x + 230, y, 16, C_DIM);
	else if (button((Rectangle){r.x + 230, y - 4, 170, 30}, soft ? "UNBLOCK" : "BLOCK", soft ? C_GREEN : C_YEL)) {
		int e = rf_set(!soft);
		if (e) snprintf(bt_status, sizeof bt_status, "rfkill: %s (needs /dev/rfkill access)", strerror(e));
	}
	if (have) DrawText(hard ? "HARD BLOCKED" : soft ? "soft blocked" : "radio on", r.x + 410, y, 16, hard || soft ? C_RED : C_GREEN);
	y += 44;
	row(r, y, "Driver", TextFormat("%s (%s)", a->driver, a->bus));
	row(r, y += 26, "Address", a->addr);
	row(r, y += 26, "Class", TextFormat("0x%06X", a->cls));
	row(r, y += 26, "Modalias", a->modalias);
	scroll_end(r, off, 480);
}
