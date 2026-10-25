#include "ui.h"
#include <math.h>
#include <string.h>
#define RH 54

enum { G_LINK, G_AUTO, G_TRUST, G_FILE, G_NET, G_TERM, G_PAIR, G_BLOCK, G_FORGET };

/* little pictogram in a tinted disc, drawn from primitives */
static void glyph(float x, float y, int g, Color c) {
	Vector2 o = {x, y};
	DrawCircleV(o, 17, ColorAlpha(c, 0.16f));
	DrawCircleLinesV(o, 17, ColorAlpha(c, 0.45f));
	switch (g) {
	case G_LINK: DrawRing((Vector2){x - 5, y}, 4, 6, 0, 360, 20, c), DrawRing((Vector2){x + 5, y}, 4, 6, 0, 360, 20, c); break;
	case G_AUTO: DrawRing(o, 6, 8, 40, 330, 20, c), DrawTriangle((Vector2){x + 4, y - 11}, (Vector2){x + 4, y - 3}, (Vector2){x + 11, y - 6}, c); break;
	case G_TRUST: DrawRing(o, 6, 8, 0, 360, 20, c), DrawLineEx((Vector2){x - 4, y}, (Vector2){x - 1, y + 3}, 2, c), DrawLineEx((Vector2){x - 1, y + 3}, (Vector2){x + 4, y - 3}, 2, c); break;
	case G_FILE: DrawRectangleRounded((Rectangle){x - 6, y - 8, 12, 16}, 0.2f, 4, c), DrawTriangle((Vector2){x + 6, y - 8}, (Vector2){x + 1, y - 8}, (Vector2){x + 6, y - 3}, C_PANEL); break;
	case G_NET: for (int i = 0; i < 3; i++) DrawRing((Vector2){x, y + 6}, 3 + i * 4, 5 + i * 4, 235, 305, 12, c); DrawCircle(x, y + 6, 2, c); break;
	case G_TERM: mtxt(">_", x - 8, y - 8, 16, c); break;
	case G_PAIR: DrawRing((Vector2){x - 4, y}, 5, 7, 0, 360, 20, c), DrawRing((Vector2){x + 4, y}, 5, 7, 0, 360, 20, c); break;
	case G_BLOCK: DrawRing(o, 7, 9, 0, 360, 20, c), DrawLineEx((Vector2){x - 6, y + 6}, (Vector2){x + 6, y - 6}, 2, c); break;
	default: DrawLineEx((Vector2){x - 6, y - 6}, (Vector2){x + 6, y + 6}, 2.5f, c), DrawLineEx((Vector2){x - 6, y + 6}, (Vector2){x + 6, y - 6}, 2.5f, c);
	}
}

/* one action: glyph, title, one-line hint, and either a switch (tog >= 0) or a chevron; true when clicked */
static int row(Rectangle r, int g, const char *t, const char *sub, Color c, int tog) {
	Vector2 m = GetMousePosition();
	int hot = CheckCollisionPointRec(m, r), hit = 0;
	DrawRectangleRounded(r, 0.22f, 8, hot ? C_HOT : ColorAlpha(C_HOT, 0.45f));
	DrawRectangleRounded((Rectangle){r.x, r.y + 10, 3, r.height - 20}, 1, 4, c);
	glyph(r.x + 32, r.y + r.height / 2, g, c);
	DrawText(t, r.x + 60, r.y + 9, 17, C_FG), DrawText(sub, r.x + 60, r.y + 31, 12, C_DIM);
	if (tog >= 0) hit = toggle((Rectangle){r.x + r.width - 54, r.y + r.height / 2 - 11, 40, 22}, tog, c);
	else DrawText(">", r.x + r.width - 24, r.y + r.height / 2 - 9, 18, hot ? c : C_DIM);
	return hit || (tog < 0 && hot && IsMouseButtonPressed(MOUSE_LEFT_BUTTON));
}

static const char *tag[] = {"AUDIO", "HID", "PHONE", "FILES", "NETWORK", "SERIAL"};
static const unsigned bit[] = {CAP_AUDIO, CAP_HID, CAP_PHONE, CAP_OPP, CAP_NAP, CAP_SPP};

/* what the device offers, as pills */
float chips(float x, float y, float w, const Dev *d) {
	float cx = x;
	for (int i = 0; i < 6; i++) {
		if (!(d->caps & bit[i])) continue;
		int tw_ = MeasureText(tag[i], 12) + 18;
		if (cx + tw_ > x + w) cx = x, y += 26;
		DrawRectangleRounded((Rectangle){cx, y, tw_, 20}, 1, 8, ColorAlpha(C_CYAN, 0.14f));
		DrawText(tag[i], cx + 9, y + 4, 12, C_CYAN);
		cx += tw_ + 6;
	}
	return cx == x ? y : y + 28;
}

float actions(float x, float y, float w, const Dev *d) {
	Rectangle r = {x, y, w, RH - 6};
#define NEXT (r.y += RH)
	if (row(r, G_LINK, d->connected ? "Disconnect" : "Connect", d->connected ? "Drop the link, keep the pairing" : "Open a link to this device", d->connected ? C_RED : C_GREEN, -1)) act(d, 'c');
	NEXT;
	if (d->paired && row(r, G_AUTO, "Auto-connect", "Reconnect whenever it is in range", C_GREEN, auto_on(d->addr))) auto_toggle(d->addr);
	if (d->paired) NEXT;
	if (row(r, G_TRUST, "Trusted", "Let it connect without asking", C_CYAN, d->trusted)) act(d, 't');
	NEXT;
	if (d->connected || d->caps & CAP_OPP) { if (row(r, G_FILE, "Send file", "Pick one, or drop it on the window", C_BLUE, -1)) xfer_pick(d); NEXT; }
	if (d->caps & CAP_NAP) { if (row(r, G_NET, "Use its network", "Tether through this device's internet", C_YEL, !!(d->caps & CAP_NETUP))) net_toggle(d); NEXT; }
	if (row(r, G_TERM, "Serial terminal", "Talk RFCOMM, send and read bytes", C_CYAN, -1)) term_open(d);
	NEXT;
	if (row(r, G_PAIR, d->paired ? "Re-pair" : "Pair", "Exchange keys with the device", C_BLUE, -1)) act(d, 'p');
	NEXT;
	if (row(r, G_BLOCK, d->blocked ? "Unblock" : "Block", "Refuse every connection from it", C_YEL, d->blocked)) act(d, 'b');
	NEXT;
	if (row(r, G_FORGET, "Forget device", "Remove it and its pairing", C_RED, -1)) act(d, 'x');
	return NEXT;
}
