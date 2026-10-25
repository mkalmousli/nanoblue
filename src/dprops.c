#include "tray.h"
#include <string.h>

void vbasic(DBusMessageIter *it, int type, const void *v) {
	char s[2] = { type, 0 };
	DBusMessageIter x;
	dbus_message_iter_open_container(it, DBUS_TYPE_VARIANT, s, &x);
	dbus_message_iter_append_basic(&x, type, v);
	dbus_message_iter_close_container(it, &x);
}

/* org.freedesktop.DBus.Properties Get / GetAll for a table of names */
DBusMessage *props_reply(DBusMessage *m, const char **nm, int (*put)(DBusMessageIter *, const char *)) {
	const char *i, *k = "";
	DBusMessage *r = dbus_message_new_method_return(m);
	DBusMessageIter it, a, e;
	dbus_message_iter_init_append(r, &it);
	if (!strcmp(dbus_message_get_member(m), "GetAll")) {
		dbus_message_iter_open_container(&it, DBUS_TYPE_ARRAY, "{sv}", &a);
		for (; *nm; nm++) {
			dbus_message_iter_open_container(&a, DBUS_TYPE_DICT_ENTRY, NULL, &e);
			dbus_message_iter_append_basic(&e, DBUS_TYPE_STRING, nm);
			put(&e, *nm);
			dbus_message_iter_close_container(&a, &e);
		}
		dbus_message_iter_close_container(&it, &a);
	} else if (!dbus_message_get_args(m, NULL, DBUS_TYPE_STRING, &i, DBUS_TYPE_STRING, &k, DBUS_TYPE_INVALID) || !put(&it, k)) {
		dbus_message_unref(r);
		r = dbus_message_new_error(m, "org.freedesktop.DBus.Error.UnknownProperty", k);
	}
	return r;
}
