#include "tray.h"
#include "bt.h"

/* pushes adapter / connection state to the tray icon and menu */
void tray_sync(const Dev *d, int n, int sa, int shown) {
	int a = sa < bt_nads ? sa : 0;
	TrayState s = { shown, bt_nads && bt_ads[a].powered, bt_nads && bt_ads[a].scanning, 0 };
	for (int i = 0; i < n; i++) s.connected |= d[i].connected;
	tray_update(&s);
}
