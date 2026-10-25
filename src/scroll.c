#include "ui.h"
#undef GetMousePosition
void rlPushMatrix(void);
void rlPopMatrix(void);
void rlTranslatef(float x, float y, float z);

static float dy;       /* scroll offset of the area being drawn */
static Rectangle clip; /* visible part of it: the mouse is dead outside */
static int clipped;

/* mouse in logical pixels, in the coordinates of the scrolled content */
Vector2 ui_mouse(Vector2 v) {
	v = (Vector2){v.x / ui_s, v.y / ui_s};
	if (clipped && !CheckCollisionPointRec(v, clip)) return (Vector2){-1e4f, -1e4f};
	return (Vector2){v.x, v.y + dy};
}

void scrollbar(Rectangle r, float off, float content) {
	float th;
	if (content <= r.height) return;
	th = r.height * r.height / content;
	DrawRectangleRounded((Rectangle){r.x + r.width - 8, r.y + 2 + off / content * r.height, 5, th - 4}, 1, 4, ColorAlpha(C_CYAN, 0.45f));
}

/* wheel-scrolls everything drawn until scroll_end; content is its total height */
void scroll_begin(Rectangle r, float *off, float content) {
	float max = content > r.height ? content - r.height : 0;
	if (CheckCollisionPointRec(GetMousePosition(), r)) *off -= GetMouseWheelMove() * 50;
	*off = *off < 0 ? 0 : *off > max ? max : *off;
	BeginScissorMode(r.x, r.y, r.width, r.height);
	rlPushMatrix();
	rlTranslatef(0, -*off, 0);
	clip = r, clipped = 1, dy = *off;
}

void scroll_end(Rectangle r, float off, float content) {
	rlPopMatrix();
	EndScissorMode();
	clipped = 0, dy = 0;
	scrollbar(r, off, content);
}
