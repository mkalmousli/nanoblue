#include "tray.h"
#include <string.h>
#define MI "com.canonical.dbusmenu"
int menu_rev = 1;
static const char *mn[] = {"Version", "TextDirection", "Status", NULL};

static int mput(DBusMessageIter *it, const char *k) {
	dbus_uint32_t v = 3;
	const char *s = !strcmp(k, "TextDirection") ? "ltr" : "normal";
	if (!strcmp(k, "Version")) vbasic(it, 'u', &v);
	else if (!strcmp(k, "TextDirection") || !strcmp(k, "Status")) vbasic(it, 's', &s);
	else return 0;
	return 1;
}

static DBusHandlerResult handle(DBusConnection *c, DBusMessage *m, void *u) {
	const char *i = dbus_message_get_interface(m), *me = dbus_message_get_member(m), *type;
	DBusMessage *r = NULL;
	DBusMessageIter it;
	int id;
	if (dbus_message_get_type(m) != DBUS_MESSAGE_TYPE_METHOD_CALL || !i) return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	if (!strcmp(i, "org.freedesktop.DBus.Properties")) r = props_reply(m, mn, mput);
	else if (!strcmp(i, MI)) {
		if (!strcmp(me, "GetLayout")) r = menu_layout(m);
		else if (!strcmp(me, "GetGroupProperties")) r = menu_group(m);
		else {
			r = dbus_message_new_method_return(m);
			if (!strcmp(me, "AboutToShow")) dbus_message_append_args(r, DBUS_TYPE_BOOLEAN, &(dbus_bool_t){0}, DBUS_TYPE_INVALID);
			if (!strcmp(me, "Event") && dbus_message_iter_init(m, &it)) {
				dbus_message_iter_get_basic(&it, &id);
				dbus_message_iter_next(&it);
				dbus_message_iter_get_basic(&it, &type);
				if (!strcmp(type, "clicked")) tray_ev = id;
			}
		}
	}
	if (!r) return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	dbus_connection_send(c, r, NULL);
	dbus_message_unref(r);
	return DBUS_HANDLER_RESULT_HANDLED;
}
static const DBusObjectPathVTable vt = { .message_function = handle };

void menu_register(void) { dbus_connection_register_object_path(tray_bus, "/MenuBar", &vt, NULL); }

void menu_changed(void) {
	DBusMessage *m = dbus_message_new_signal("/MenuBar", MI, "LayoutUpdated");
	dbus_uint32_t u = ++menu_rev;
	int parent = 0;
	dbus_message_append_args(m, DBUS_TYPE_UINT32, &u, DBUS_TYPE_INT32, &parent, DBUS_TYPE_INVALID);
	dbus_connection_send(tray_bus, m, NULL);
	dbus_message_unref(m);
}
