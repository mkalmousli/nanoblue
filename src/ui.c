#include "ui.h"
#include <math.h>
#define P "/usr/share/fonts/truetype/"

float ui_s = 1.4f;
static Font ui_font, mono_font;
static const char *uip[] = {P "noto/NotoSans-Regular.ttf", P "ubuntu/Ubuntu-R.ttf", P "dejavu/DejaVuSans.ttf", P "liberation/LiberationSans-Regular.ttf", NULL};
static const char *monop[] = {P "dejavu/DejaVuSansMono.ttf", P "ubuntu/UbuntuMono-R.ttf", P "noto/NotoSansMono-Regular.ttf", NULL};

/* big atlas + mipmaps + trilinear: stays crisp at every size we draw */
static Font load(const char **p) {
	int cp[224];
	for (int i = 0; i < 224; i++) cp[i] = 32 + i;
	for (; *p; p++) {
		if (!FileExists(*p)) continue;
		Font f = LoadFontEx(*p, 64, cp, 224);
		GenTextureMipmaps(&f.texture);
		SetTextureFilter(f.texture, TEXTURE_FILTER_TRILINEAR);
		return f;
	}
	return GetFontDefault();
}

Image tray_image(unsigned col);
void fonts_init(void) { ui_font = load(uip), mono_font = load(monop), SetWindowIcon(tray_image(0x3D8BFF)); }
void txt(const char *s, float x, float y, float sz, Color c) { DrawTextEx(ui_font, s, (Vector2){(int)x, (int)y}, sz, 0, c); }
void mtxt(const char *s, float x, float y, float sz, Color c) { DrawTextEx(mono_font, s, (Vector2){(int)x, (int)y}, sz, 0, c); }
int tw(const char *s, float sz) { return MeasureTextEx(ui_font, s, sz, 0).x; }

void spinner(float x, float y, float r, Color c) {
	float a = GetTime() * 400;
	DrawRing((Vector2){x, y}, r - 3, r, a, a + 270, 32, c);
}

void rssi_bars(float x, float y, int rssi) {
	int n = !rssi ? 0 : rssi > -55 ? 4 : rssi > -65 ? 3 : rssi > -75 ? 2 : rssi > -90 ? 1 : 0;
	Color c = n >= 3 ? C_GREEN : n == 2 ? C_YEL : C_RED;
	for (int i = 0; i < 4; i++)
		DrawRectangle(x + i * 7, y + 16 - (i + 1) * 4, 5, (i + 1) * 4, i < n ? c : ColorAlpha(C_DIM, 0.3f));
}

void battery(float x, float y, int p) {
	Color c = p > 50 ? C_GREEN : p > 20 ? C_YEL : C_RED;
	DrawRectangleLines(x, y, 28, 14, C_DIM);
	DrawRectangle(x + 28, y + 4, 3, 6, C_DIM);
	DrawRectangle(x + 2, y + 2, 24 * p / 100, 10, c);
}

/* expanding pings plus a rotating sweep */
void radar(float x, float y, float r, Color c) {
	Vector2 o = {x, y};
	float a = GetTime() * 240;
	for (int i = 0; i < 3; i++) {
		float ph = fmodf(GetTime() * 0.8f + i / 3.0f, 1);
		DrawCircleLinesV(o, r * ph, ColorAlpha(c, 1 - ph));
	}
	DrawCircleSector(o, r, a, a + 50, 16, ColorAlpha(c, 0.35f));
	DrawCircleV(o, 3, c);
}

Color dev_color(const Dev *d) { return d->blocked ? C_RED : d->connected ? C_GREEN : d->paired ? C_BLUE : C_DIM; }

#undef GetMousePosition
Vector2 ui_mouse(Vector2 v) { return (Vector2){v.x / ui_s, v.y / ui_s}; }
