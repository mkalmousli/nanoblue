#include "raylib.h"
#include "tray.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static float segdist(float px, float py, float ax, float ay, float bx, float by) {
	float dx = bx - ax, dy = by - ay, t = ((px - ax) * dx + (py - ay) * dy) / (dx * dx + dy * dy);
	t = t < 0 ? 0 : t > 1 ? 1 : t;
	return hypotf(px - ax - t * dx, py - ay - t * dy);
}

/* anti-aliased disc in `col` with the white bluetooth rune on top; TS x TS bytes A R G B */
void tray_pixels(unsigned char *o, unsigned col) {
	static const float p[6][2] = {{9, 11}, {23, 21}, {16, 27}, {16, 5}, {23, 11}, {9, 21}};
	const float k = TS / 32.f;
	for (int y = 0; y < TS; y++)
		for (int x = 0; x < TS; x++) {
			float fx = (x + .5f) / k, fy = (y + .5f) / k, d = hypotf(fx - 16, fy - 16), md = 99, w, a = d < 15 ? 1 : d < 16 ? 16 - d : 0;
			for (int i = 0; i < 5; i++) {
				float s = segdist(fx, fy, p[i][0], p[i][1], p[i + 1][0], p[i + 1][1]);
				md = s < md ? s : md;
			}
			w = md < 1.4f ? 1 : md < 2.2f ? (2.2f - md) / .8f : 0;
			unsigned char *q = o + (y * TS + x) * 4;
			q[0] = a * 255;
			q[1] = (col >> 16 & 255) * (1 - w) + 255 * w;
			q[2] = (col >> 8 & 255) * (1 - w) + 255 * w;
			q[3] = (col & 255) * (1 - w) + 255 * w;
		}
}

/* PNG file of the icon (many hosts, e.g. ayatana, ignore IconPixmap and want a path); also the window icon */
Image tray_image(unsigned col) {
	unsigned char *a = malloc(TS * TS * 4), *r = malloc(TS * TS * 4);
	tray_pixels(a, col);
	for (int i = 0; i < TS * TS; i++) r[i * 4] = a[i * 4 + 1], r[i * 4 + 1] = a[i * 4 + 2], r[i * 4 + 2] = a[i * 4 + 3], r[i * 4 + 3] = a[i * 4];
	free(a);
	return (Image){r, TS, TS, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
}

void tray_png(char *path, size_t n, const char *name, unsigned col) {
	const char *d = getenv("XDG_RUNTIME_DIR");
	Image im = tray_image(col);
	snprintf(path, n, "%s/nanoblue-%s.png", d ? d : "/tmp", name);
	ExportImage(im, path);
	UnloadImage(im);
}

char icon[160];
unsigned char tray_pix[TS * TS * 4];
unsigned tray_color(void) { return tray_st.connected ? 0x3CDC8C : tray_st.powered ? 0x4696FF : 0x787E96; }

void set_icon(void) {
	const char *d = getenv("XDG_RUNTIME_DIR");
	snprintf(icon, sizeof icon, "%s/nanoblue-%s.png", d ? d : "/tmp", tray_st.connected ? "connected" : tray_st.powered ? "on" : "off");
}
