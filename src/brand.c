#include "ui.h"
#include "version.h"
#define URL "https://github.com/" NB_REPO

/* top-right badge: version pill, then the maintainer handle linking to the source */
void brand(int W) {
	const char *v = upd_msg[0] ? upd_msg : "v" NB_VERSION, *h = "@" NB_HANDLE;
	int vw = MeasureText(v, 14) + 20, hw = MeasureText(h, 14) + 24;
	Rectangle pv = {W - 14 - hw - 8 - vw, 12, vw, 24}, ph = {W - 14 - hw, 12, hw, 24};
	int hot = CheckCollisionPointRec(GetMousePosition(), ph);
	DrawRectangleRounded(pv, 1, 12, upd_msg[0] ? ColorAlpha(C_YEL, 0.25f) : ColorAlpha(C_GREEN, 0.18f));
	DrawRectangleRoundedLinesEx(pv, 1, 12, 1, upd_msg[0] ? C_YEL : C_GREEN);
	DrawText(v, pv.x + 10, 16, 14, upd_msg[0] ? C_YEL : C_GREEN);
	DrawRectangleRounded(ph, 1, 12, hot ? ColorAlpha(C_BLUE, 0.35f) : ColorAlpha(C_BLUE, 0.15f));
	DrawRectangleRoundedLinesEx(ph, 1, 12, 1, hot ? C_CYAN : C_BLUE);
	DrawText(h, ph.x + 12, 16, 14, hot ? C_FG : C_CYAN);
	static int was;
	if (was && !hot) SetMouseCursor(MOUSE_CURSOR_DEFAULT);
	was = hot;
	if (hot) {
		SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
		DrawText("github.com/" NB_REPO, ph.x + ph.width - MeasureText("github.com/" NB_REPO, 12), 40, 12, C_DIM);
		if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) OpenURL(URL);
	}
}
