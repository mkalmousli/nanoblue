#include "ui.h"

/* wide: sidebar | list | detail. Narrow (< 800 logical px): list or detail, one at a time, sidebar hidden (Tab switches adapter) */
void panes(int W, int H, Dev *v, int nv, int *sel, float *scroll, int *sa) {
	static int showd;
	int narrow = W < 800, x = narrow ? 0 : 220;
	Rectangle body = {x, 48, W - x, H - 76};
	if (!narrow) sidebar((Rectangle){0, 48, 220, H - 76}, sa);
	if (term_on()) return term_frame(body);
	if (adp_panel_on()) return adp_panel(body);
	if (!narrow) return devlist((Rectangle){220, 48, W - 560, H - 76}, v, nv, sel, scroll), detail((Rectangle){W - 340, 48, 340, H - 76}, nv ? &v[*sel] : NULL);
	if (showd && nv && !IsKeyPressed(KEY_ESCAPE)) {
		detail(body, &v[*sel]);
		if (button((Rectangle){W - 110, 56, 100, 32}, "< BACK", C_BLUE)) showd = 0;
	} else {
		showd = 0;
		int k = bt_nads + 2;
		float bw = (W - 8 - 6 * k) / k;
		for (int i = 0; i < bt_nads; i++)
			if (button((Rectangle){8 + i * (bw + 6), 52, bw, 34}, TextFormat("%.10s", bt_ads[i].name), i == *sa ? C_GREEN : C_BLUE)) *sa = i;
		if (bt_nads && button((Rectangle){8 + bt_nads * (bw + 6), 52, bw, 34}, bt_ads[*sa].scanning ? "STOP" : "SCAN", C_CYAN)) adp_act(*sa, 's');
		if (bt_nads && button((Rectangle){8 + (bt_nads + 1) * (bw + 6), 52, bw, 34}, "SET", C_YEL)) adp_panel_open(*sa);
		body.y += 40, body.height -= 40;
		devlist(body, v, nv, sel, scroll);
		if (nv && IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), (Rectangle){0, 88, W - 14, H - 116})) showd = 1;
	}
}
