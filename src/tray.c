#include "tray.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

DBusConnection *tray_bus;
TrayState tray_st;
int tray_ev;
static const char *names[] = {"Category", "Id", "Title", "Status", "IconName", "ItemIsMenu", "Menu", "WindowId", "IconPixmap", NULL};

static DBusHandlerResult handle(DBusConnection *c, DBusMessage *m, void *u) {
	const char *i = dbus_message_get_interface(m);
	DBusMessage *r = NULL;
	if (dbus_message_get_type(m) != DBUS_MESSAGE_TYPE_METHOD_CALL || !i) return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	if (!strcmp(i, "org.freedesktop.DBus.Properties")) r = props_reply(m, names, sni_put);
	else if (!strcmp(i, "org.kde.StatusNotifierItem")) {
		tray_ev = !strcmp(dbus_message_get_member(m), "Activate") ? 1 : tray_ev;
		r = dbus_message_new_method_return(m);
	}
	if (!r) return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	dbus_connection_send(c, r, NULL);
	dbus_message_unref(r);
	return DBUS_HANDLER_RESULT_HANDLED;
}
static const DBusObjectPathVTable vt = { .message_function = handle };


int tray_reg; /* a tray host currently shows the icon */
static time_t tried;

static void reg(void) {
	char nm[64], *p = nm;
	DBusMessage *q, *r;
	tried = time(NULL);
	if (!dbus_bus_name_has_owner(tray_bus, "org.kde.StatusNotifierWatcher", NULL)) return;
	snprintf(nm, sizeof nm, "org.kde.StatusNotifierItem-%d-1", getpid());
	q = dbus_message_new_method_call("org.kde.StatusNotifierWatcher", "/StatusNotifierWatcher",
	                                 "org.kde.StatusNotifierWatcher", "RegisterStatusNotifierItem");
	dbus_message_append_args(q, DBUS_TYPE_STRING, &p, DBUS_TYPE_INVALID);
	r = dbus_connection_send_with_reply_and_block(tray_bus, q, 500, NULL);
	dbus_message_unref(q);
	if ((tray_reg = r != NULL)) dbus_message_unref(r);
}

/* tray host (re)started or died: register again, or let the window come back */
static DBusHandlerResult watch(DBusConnection *c, DBusMessage *m, void *u) {
	if (dbus_message_is_signal(m, "org.freedesktop.DBus", "NameOwnerChanged")) tray_reg = 0, tried = 0;
	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

int tray_init(void) {
	char nm[64];
	if (!tray_bus) return -1;
	snprintf(nm, sizeof nm, "org.kde.StatusNotifierItem-%d-1", getpid());
	if (dbus_bus_request_name(tray_bus, nm, 0, NULL) < 0) return tray_bus = NULL, -1;
	dbus_connection_register_object_path(tray_bus, "/StatusNotifierItem", &vt, NULL);
	dbus_bus_add_match(tray_bus, "type='signal',sender='org.freedesktop.DBus',member='NameOwnerChanged',arg0='org.kde.StatusNotifierWatcher'", NULL);
	dbus_connection_add_filter(tray_bus, watch, NULL, NULL);
	menu_register();
	tray_pixels(tray_pix, tray_color());
	tray_png(icon, sizeof icon, "off", 0x787E96), tray_png(icon, sizeof icon, "on", 0x4696FF), tray_png(icon, sizeof icon, "connected", 0x3CDC8C);
	set_icon();
	reg();
	return 0;
}

void tray_update(const TrayState *s) {
	int changed = s->connected != tray_st.connected || s->powered != tray_st.powered;
	if (!tray_bus) return;
	tray_st = *s;
	if (changed) {
		DBusMessage *m = dbus_message_new_signal("/StatusNotifierItem", "org.kde.StatusNotifierItem", "NewIcon");
		tray_pixels(tray_pix, tray_color());
		set_icon();
		dbus_connection_send(tray_bus, m, NULL);
		dbus_message_unref(m);
	}
}

void tray_menu(const Mi *m, int n) {
	if (!tray_bus || (n == tray_nitems && !memcmp(m, tray_items, n * sizeof *m))) return;
	memcpy(tray_items, m, n * sizeof *m), tray_nitems = n;
	menu_changed();
}

void tray_process(void) {
	if (!tray_bus) return;
	dbus_connection_read_write(tray_bus, 0);
	while (dbus_connection_dispatch(tray_bus) == DBUS_DISPATCH_DATA_REMAINS);
	if (!tray_reg && time(NULL) - tried >= 2) reg();
}
