#include "ui.h"
#include <math.h>

static Color mix(Color a, Color b, float t) {
	return (Color){a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t, a.b + (b.b - a.b) * t, 255};
}

void shadow(Rectangle r, float d) {
	for (int i = 6; i > 0; i--) {
		float g = d * i / 6;
		DrawRectangleRounded((Rectangle){r.x - g + 1, r.y - g * .3f + d * .6f, r.width + 2 * g - 2, r.height + 2 * g}, 0.16f, 8, (Color){0, 0, 0, 14});
	}
}

/* surface lit from the top: dark base, lighter upper half, 1px highlight and 1px dark rim */
void card(Rectangle r, Color base, int raised) {
	float rd = 14 / fminf(r.width, r.height);
	if (raised) shadow(r, 7);
	DrawRectangleRounded(r, rd, 10, mix(base, BLACK, .35f));
	DrawRectangleRounded((Rectangle){r.x + 1, r.y + 1, r.width - 2, r.height - 2}, rd, 10, base);
	DrawRectangleRounded((Rectangle){r.x + 1, r.y + 1, r.width - 2, r.height * .5f}, rd, 10, (Color){255, 255, 255, raised ? 12 : 4});
	DrawRectangleRoundedLines((Rectangle){r.x + .5f, r.y + .5f, r.width - 1, r.height - 1}, rd, 10, (Color){255, 255, 255, raised ? 26 : 10});
}

void led(float x, float y, float r, Color c, int on) {
	Color b = on ? c : mix(c, BLACK, .75f);
	if (on) for (int i = 4; i > 0; i--) DrawCircle(x, y, r + i * 2.5f, ColorAlpha(c, .06f + .02f * sinf(GetTime() * 3)));
	DrawCircle(x, y, r + 1.5f, (Color){0, 0, 0, 160});
	DrawCircle(x, y, r, b);
	DrawCircleGradient(x - r * .3f, y - r * .35f, r * .7f, (Color){255, 255, 255, on ? 200 : 60}, BLANK);
}

void backdrop(int w, int h) {
	DrawRectangleGradientV(0, 0, w, h, (Color){20, 24, 36, 255}, (Color){9, 11, 17, 255});
	for (int y = 0; y < h; y += 4) DrawRectangle(0, y, w, 1, (Color){255, 255, 255, 3});
	DrawRectangleGradientH(0, 0, 60, h, (Color){0, 0, 0, 90}, BLANK);
	DrawRectangleGradientH(w - 60, 0, 60, h, BLANK, (Color){0, 0, 0, 90});
}

int button(Rectangle r, const char *t, Color c) {
	int hot = CheckCollisionPointRec(GetMousePosition(), r), dn = hot && IsMouseButtonDown(MOUSE_LEFT_BUTTON);
	Color top = mix(C_HOT, c, hot ? .3f : .14f), bot = mix(C_PANEL, c, hot ? .18f : .08f);
	if (!dn) shadow(r, 3);
	if (dn) r.y += 1;
	DrawRectangleRounded((Rectangle){r.x - 1, r.y - 1, r.width + 2, r.height + 2}, 0.3f, 8, (Color){0, 0, 0, 200});
	DrawRectangleRounded(r, 0.3f, 8, bot);
	DrawRectangleRounded((Rectangle){r.x, r.y, r.width, r.height * .55f}, 0.5f, 8, ColorAlpha(top, dn ? .5f : 1));
	DrawRectangleRoundedLines(r, 0.3f, 8, ColorAlpha(c, hot ? .9f : .45f));
	DrawText(t, r.x + (r.width - MeasureText(t, 16)) / 2 + 1, r.y + (r.height - 16) / 2 + 1, 16, BLACK);
	DrawText(t, r.x + (r.width - MeasureText(t, 16)) / 2, r.y + (r.height - 16) / 2, 16, hot ? C_FG : mix(c, C_FG, .35f));
	return hot && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
}

int toggle(Rectangle r, int on, Color c) {
	int hot = CheckCollisionPointRec(GetMousePosition(), r);
	float k = r.height / 2, x = r.x + (on ? r.width - k : k);
	DrawRectangleRounded((Rectangle){r.x - 1, r.y - 1, r.width + 2, r.height + 2}, 1, 12, (Color){0, 0, 0, 200});
	DrawRectangleRounded(r, 1, 12, on ? mix(c, BLACK, .35f) : (Color){16, 18, 26, 255});
	DrawRectangleRounded((Rectangle){r.x + 2, r.y + 2, r.width - 4, r.height * .4f}, 1, 12, (Color){0, 0, 0, 70});
	if (on) DrawCircle(r.x + k, r.y + k, 3, ColorAlpha(c, .9f));
	DrawCircle(x, r.y + k + 2, k - 2, (Color){0, 0, 0, 110});
	DrawCircleGradient(x, r.y + k, k - 2, hot ? C_FG : (Color){196, 202, 216, 255}, (Color){120, 128, 150, 255});
	DrawCircleGradient(x - 2, r.y + k - 3, k * .4f, (Color){255, 255, 255, 140}, BLANK);
	return hot && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
}
