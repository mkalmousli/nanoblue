#include "ui.h"
#include <string.h>
#define BG C_BG

static void rect(float x, float y, float w, float h, float u, Color c, float rd) {
	DrawRectangleRounded((Rectangle){x - w / 2 * u, y - h / 2 * u, w * u, h * u}, rd, 6, c);
}

/* flat glyph per BlueZ Icon name, drawn in a 40x40 design box scaled by r */
void dev_icon(float x, float y, float r, const char *ic, Color c) {
	Vector2 o = {x, y};
	float u = r / 20;
	DrawCircleV(o, r, ColorAlpha(c, 0.18f));
	if (strstr(ic, "head")) {
		DrawRing(o, 8 * u, 11 * u, 180, 360, 24, c);
		rect(x - 9.5f * u, y + 3 * u, 6, 11, u, c, 0.5f), rect(x + 9.5f * u, y + 3 * u, 6, 11, u, c, 0.5f);
	} else if (strstr(ic, "phone")) {
		rect(x, y, 12, 22, u, c, 0.25f), rect(x, y - 1.5f * u, 8, 14, u, BG, 0.1f), DrawCircle(x, y + 8.5f * u, 1.1f * u, BG);
	} else if (strstr(ic, "keyboard")) {
		rect(x, y, 26, 14, u, c, 0.2f);
		for (int i = 0; i < 6; i++) for (int j = 0; j < 2; j++) DrawRectangle(x + (-10 + i * 4) * u, y + (-4 + j * 4) * u, 2.5f * u, 2.5f * u, BG);
		rect(x, y + 4 * u, 12, 2, u, BG, 0.3f);
	} else if (strstr(ic, "mouse")) {
		rect(x, y, 14, 22, u, c, 0.9f), DrawRectangle(x - 0.5f * u, y - 11 * u, u, 9 * u, BG);
	} else if (strstr(ic, "gaming") || strstr(ic, "joystick")) {
		rect(x, y, 26, 13, u, c, 0.8f), DrawRectangle(x - 9 * u, y - u, 6 * u, 2 * u, BG), DrawRectangle(x - 7 * u, y - 3 * u, 2 * u, 6 * u, BG);
		DrawCircle(x + 6 * u, y - 2 * u, 1.6f * u, BG), DrawCircle(x + 9 * u, y + u, 1.6f * u, BG);
	} else if (strstr(ic, "computer") || strstr(ic, "laptop")) {
		rect(x, y - 2 * u, 24, 15, u, c, 0.1f), rect(x, y - 2 * u, 20, 11, u, BG, 0.05f), rect(x, y + 9 * u, 14, 2, u, c, 0.5f);
	} else if (strstr(ic, "audio") || strstr(ic, "speaker")) {
		rect(x, y, 14, 22, u, c, 0.2f), DrawCircle(x, y + 4 * u, 4 * u, BG), DrawCircle(x, y - 5 * u, 2 * u, BG);
	} else {
		static const float p[6][2] = {{-5, -5}, {5, 5}, {0, 10}, {0, -10}, {5, -5}, {-5, 5}};
		for (int i = 0; i < 5; i++)
			DrawLineEx((Vector2){x + p[i][0] * u, y + p[i][1] * u}, (Vector2){x + p[i + 1][0] * u, y + p[i + 1][1] * u}, 2 * u, c);
	}
}
