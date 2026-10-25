#include "ui.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#define ROW 64

#define mix_(c) (Color){(c).r * 3 / 4 + 5, (c).g * 3 / 4 + 5, (c).b * 3 / 4 + 5, 255}

void devlist(Rectangle r, Dev *d, int n, int *sel, float *scroll) {
	static double lastclick;
	float max = n * ROW + 8 > r.height ? n * ROW + 8 - r.height : 0;
	Vector2 m = GetMousePosition();
	int inside = CheckCollisionPointRec(m, r);
	if (inside) *scroll -= GetMouseWheelMove() * 50;
	if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_DOWN)) {  /* keep keyboard selection in view */
		if (*sel * ROW < *scroll) *scroll = *sel * ROW;
		if ((*sel + 1) * ROW + 8 > *scroll + r.height) *scroll = (*sel + 1) * ROW + 8 - r.height;
	}
	*scroll = *scroll < 0 ? 0 : *scroll > max ? max : *scroll;
	BeginScissorMode(r.x, r.y, r.width, r.height);
	if (!n) DrawText("No devices - press S to scan", r.x + 30, r.y + 30, 20, C_DIM);
	for (int i = 0; i < n; i++) {
		Rectangle w = {r.x + 8, r.y + 8 + i * ROW - *scroll, r.width - 16, ROW - 6};
		Dev *x = &d[i];
		Color c = dev_color(x);
		float mid = w.y + w.height / 2, xr = w.x + w.width - 44;
		int hot = inside && CheckCollisionPointRec(m, w);
		if (w.y + w.height < r.y || w.y > r.y + r.height) continue;
		if (hot && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
			if (*sel == i && GetTime() - lastclick < 0.4) act(x, 'c');
			*sel = i, lastclick = GetTime();
		}
		card(w, i == *sel ? C_HOT : hot ? mix_(C_HOT) : C_PANEL, i == *sel);
		DrawRectangle(w.x, w.y + 8, 4, w.height - 16, c);
		dev_icon(w.x + 36, mid, 20, x->icon, c);
		if (x->connected) DrawCircleLinesV((Vector2){w.x + 36, mid}, 23 + 3 * sinf(GetTime() * 3), ColorAlpha(c, 0.5f));
		DrawText(TextFormat("%.26s", x->name), w.x + 66, w.y + 10, 20, C_FG);
		DrawText(TextFormat("%s  %s%s%s", x->addr, x->paired ? "PAIRED " : "", x->trusted ? "TRUSTED " : "",
		                    x->blocked ? "BLOCKED" : ""), w.x + 66, w.y + 36, 14, C_DIM);
		if (x->rssi) rssi_bars(xr, mid - 8, x->rssi), DrawText(TextFormat("%d", x->rssi), xr - 36, mid - 7, 14, C_DIM);
		if (x->battery >= 0) battery(xr - 80, mid - 7, x->battery);
		if (bt_busy && !strcmp(bt_busy_path, x->path)) spinner(xr - 120, mid, 10, C_YEL);
	}
	EndScissorMode();
}

/* last ~15 s of RSSI for the selected device, sampled at 4 Hz */
static void signal_graph(Rectangle g, const Dev *d) {
	static float h[60];
	static char who[20];
	static double t;
	if (strcmp(who, d->addr)) memset(h, 0, sizeof h), snprintf(who, sizeof who, "%s", d->addr);
	if (GetTime() - t > 0.25) memmove(h, h + 1, sizeof h - sizeof *h), h[59] = d->rssi, t = GetTime();
	card(g, C_BG, 0);
	for (int i = 1; i < 60; i++) {
		if (!h[i] || !h[i - 1]) continue;
		float a = (h[i - 1] + 100) / 70, b = (h[i] + 100) / 70;
		a = a < 0 ? 0 : a > 1 ? 1 : a, b = b < 0 ? 0 : b > 1 ? 1 : b;
		DrawLineEx((Vector2){g.x + (i - 1) * g.width / 59, g.y + g.height - 4 - a * (g.height - 8)},
		           (Vector2){g.x + i * g.width / 59, g.y + g.height - 4 - b * (g.height - 8)}, 2, b > .6f ? C_GREEN : b > .35f ? C_YEL : C_RED);
	}
	if (!d->rssi) DrawText("no signal (scan to update RSSI)", g.x + 10, g.y + 18, 14, C_DIM);
}

void detail(Rectangle r, const Dev *d) {
	DrawRectangleRec(r, C_PANEL);
	if (!d) { DrawText("Select a device", r.x + 20, r.y + 20, 20, C_DIM); return; }
	float x = r.x + 20, y = r.y + 20, bw = (r.width - 50) / 2;
	#define KV(k, ...) (DrawText(k, x, y, 15, C_DIM), DrawText(TextFormat(__VA_ARGS__), x + 90, y, 15, C_FG), y += 24)
	DrawText(TextFormat("%.20s", d->name), x, y, 26, C_FG);
	DrawText(d->blocked ? "BLOCKED" : d->connected ? "CONNECTED" : d->paired ? "PAIRED" : "DISCOVERED", x, y + 34, 16, dev_color(d));
	y += 70;
	KV("Address", "%s (%s)", d->addr, d->atype);
	KV("Type", "%s", d->icon[0] ? d->icon : "unknown");
	KV("Class", "0x%06X", d->cls);
	KV("RSSI", d->rssi ? "%d dBm  ~%.1f m" : "n/a", d->rssi, pow(10, (-59 - d->rssi) / 20.0));
	KV("Battery", d->battery >= 0 ? "%d%%" : "n/a", d->battery);
	KV("Services", "%d UUIDs", d->uuids);
	KV("Path", "%.26s", d->path + 11);
	signal_graph((Rectangle){x, y + 4, r.width - 40, 56}, d);
	y += 74;
	if (button((Rectangle){x, y, bw, 38}, d->connected ? "DISCONNECT" : "CONNECT", d->connected ? C_RED : C_GREEN)) act(d, 'c');
	if (button((Rectangle){x + bw + 10, y, bw, 38}, d->paired ? "RE-PAIR" : "PAIR", C_BLUE)) act(d, 'p');
	if (button((Rectangle){x, y + 48, bw, 38}, d->trusted ? "UNTRUST" : "TRUST", C_CYAN)) act(d, 't');
	if (button((Rectangle){x + bw + 10, y + 48, bw, 38}, d->blocked ? "UNBLOCK" : "BLOCK", C_YEL)) act(d, 'b');
	if (button((Rectangle){x, y + 96, r.width - 40, 38}, "REMOVE", C_RED)) act(d, 'x');
	if (button((Rectangle){x, y + 144, r.width - 40, 38}, "SERIAL TERMINAL", C_CYAN)) term_open(d);
}
