#include "tray.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

DBusConnection *tray_bus;
TrayState tray_st, tray_shown;
int tray_ev;
static unsigned char pix[TS * TS * 4];
static const char *names[] = {"Category", "Id", "Title", "Status", "IconName", "ItemIsMenu", "Menu", "WindowId", "IconPixmap", NULL};

static int put(DBusMessageIter *it, const char *k) {
	const char *s = !strcmp(k, "Category") ? "ApplicationStatus" : !strcmp(k, "Id") ? "nanoblue"
	              : !strcmp(k, "Title") ? "Nanoblue" : !strcmp(k, "Status") ? "Active" : !strcmp(k, "IconName") ? icon : NULL;
	const char *mp = "/MenuBar";
	const unsigned char *px = pix;
	dbus_bool_t f = 0;
	dbus_uint32_t u = 0;
	int wh = TS;
	DBusMessageIter v, a, st, b;
	if (s) vbasic(it, 's', &s);
	else if (!strcmp(k, "ItemIsMenu")) vbasic(it, 'b', &f);
	else if (!strcmp(k, "WindowId")) vbasic(it, 'u', &u);
	else if (!strcmp(k, "Menu")) vbasic(it, 'o', &mp);
	else if (!strcmp(k, "IconPixmap")) {
		dbus_message_iter_open_container(it, DBUS_TYPE_VARIANT, "a(iiay)", &v);
		dbus_message_iter_open_container(&v, DBUS_TYPE_ARRAY, "(iiay)", &a);
		dbus_message_iter_open_container(&a, DBUS_TYPE_STRUCT, NULL, &st);
		dbus_message_iter_append_basic(&st, DBUS_TYPE_INT32, &wh);
		dbus_message_iter_append_basic(&st, DBUS_TYPE_INT32, &wh);
		dbus_message_iter_open_container(&st, DBUS_TYPE_ARRAY, "y", &b);
		dbus_message_iter_append_fixed_array(&b, DBUS_TYPE_BYTE, &px, sizeof pix);
		dbus_message_iter_close_container(&st, &b);
		dbus_message_iter_close_container(&a, &st);
		dbus_message_iter_close_container(&v, &a);
		dbus_message_iter_close_container(it, &v);
	} else return 0;
	return 1;
}

static DBusHandlerResult handle(DBusConnection *c, DBusMessage *m, void *u) {
	const char *i = dbus_message_get_interface(m);
	DBusMessage *r = NULL;
	if (dbus_message_get_type(m) != DBUS_MESSAGE_TYPE_METHOD_CALL || !i) return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	if (!strcmp(i, "org.freedesktop.DBus.Properties")) r = props_reply(m, names, put);
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


int tray_init(void) {
	char nm[64], *p = nm;
	DBusMessage *q, *r;
	if (!(tray_bus = dbus_bus_get(DBUS_BUS_SESSION, NULL))) return -1;
	dbus_connection_set_exit_on_disconnect(tray_bus, 0);
	snprintf(nm, sizeof nm, "org.kde.StatusNotifierItem-%d-1", getpid());
	if (dbus_bus_request_name(tray_bus, nm, 0, NULL) < 0) return -1;
	dbus_connection_register_object_path(tray_bus, "/StatusNotifierItem", &vt, NULL);
	menu_register();
	tray_pixels(pix, tray_color());
	tray_png(icon, sizeof icon, "off", 0x787E96), tray_png(icon, sizeof icon, "on", 0x4696FF), tray_png(icon, sizeof icon, "connected", 0x3CDC8C);
	set_icon();
	q = dbus_message_new_method_call("org.kde.StatusNotifierWatcher", "/StatusNotifierWatcher",
	                                 "org.kde.StatusNotifierWatcher", "RegisterStatusNotifierItem");
	dbus_message_append_args(q, DBUS_TYPE_STRING, &p, DBUS_TYPE_INVALID);
	r = dbus_connection_send_with_reply_and_block(tray_bus, q, 1000, NULL);
	dbus_message_unref(q);
	if (!r) return tray_bus = NULL, -1; /* no host: closing the window must quit */
	dbus_message_unref(r);
	return 0;
}

void tray_update(const TrayState *s) {
	int icon = s->connected != tray_shown.connected || s->powered != tray_shown.powered;
	int menu = s->visible != tray_shown.visible || s->powered != tray_shown.powered || s->scanning != tray_shown.scanning;
	if (!tray_bus) return;
	tray_st = tray_shown = *s;
	if (icon) {
		DBusMessage *m = dbus_message_new_signal("/StatusNotifierItem", "org.kde.StatusNotifierItem", "NewIcon");
		tray_pixels(pix, tray_color());
		set_icon();
		dbus_connection_send(tray_bus, m, NULL);
		dbus_message_unref(m);
	}
	if (menu) menu_changed();
}

void tray_process(void) {
	if (!tray_bus) return;
	dbus_connection_read_write(tray_bus, 0);
	while (dbus_connection_dispatch(tray_bus) == DBUS_DISPATCH_DATA_REMAINS);
}
