#include "bt.h"
#include <string.h>

void notify(const char *title, const char *body, const char *icon) {
	if (!tray_bus) return;
	DBusMessage *q = dbus_message_new_method_call("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", "Notify");
	DBusMessageIter it, ar, di;
	const char *app = "Nanoblue";
	dbus_uint32_t id = 0;
	dbus_int32_t ms = 5000;
	dbus_message_iter_init_append(q, &it);
	dbus_message_iter_append_basic(&it, 's', &app), dbus_message_iter_append_basic(&it, 'u', &id), dbus_message_iter_append_basic(&it, 's', &icon);
	dbus_message_iter_append_basic(&it, 's', &title), dbus_message_iter_append_basic(&it, 's', &body);
	dbus_message_iter_open_container(&it, 'a', "s", &ar), dbus_message_iter_close_container(&it, &ar);
	dbus_message_iter_open_container(&it, 'a', "{sv}", &di), dbus_message_iter_close_container(&it, &di);
	dbus_message_iter_append_basic(&it, 'i', &ms);
	dbus_connection_send(tray_bus, q, NULL);
	dbus_message_unref(q);
	dbus_connection_flush(tray_bus);
}

/* compares each paired device with the last pass; the first pass only records */
void events_tick(const Dev *d, int n) {
	static struct { char path[96]; int conn, paired; } p[MAXD];
	static int np, first = 1;
	typeof(*p) q[MAXD];
	for (int i = 0; i < n && i < MAXD; i++) {
		cp(q[i].path, sizeof q[i].path, d[i].path), q[i].conn = d[i].connected, q[i].paired = d[i].paired;
		for (int j = 0; j < np && !first; j++) {
			if (strcmp(p[j].path, d[i].path)) continue;
			if (d[i].paired && !p[j].conn && d[i].connected) notify(d[i].name, "Connected", d[i].icon[0] ? d[i].icon : "bluetooth");
			if (p[j].conn && !d[i].connected) notify(d[i].name, "Disconnected", "bluetooth-disabled");
			if (!p[j].paired && d[i].paired) notify(d[i].name, "Paired", "bluetooth-active");
		}
	}
	memcpy(p, q, n * sizeof *q), np = n < MAXD ? n : MAXD, first = 0;
}
