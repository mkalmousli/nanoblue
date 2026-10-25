#include "tray.h"
#define N_ITEMS 4

static const char *label(int id) {
	return id == 1 ? (tray_st.visible ? "Hide window" : "Show window")
	     : id == 2 ? (tray_st.powered ? "Turn Bluetooth off" : "Turn Bluetooth on")
	     : id == 3 ? (tray_st.scanning ? "Stop scanning" : "Start scanning") : "Quit";
}

/* a{sv} with just the label */
static void iprops(DBusMessageIter *p, int id) {
	DBusMessageIter a, e;
	const char *k = "label", *l = label(id);
	dbus_message_iter_open_container(p, DBUS_TYPE_ARRAY, "{sv}", &a);
	dbus_message_iter_open_container(&a, DBUS_TYPE_DICT_ENTRY, NULL, &e);
	dbus_message_iter_append_basic(&e, DBUS_TYPE_STRING, &k);
	vbasic(&e, 's', &l);
	dbus_message_iter_close_container(&a, &e);
	dbus_message_iter_close_container(p, &a);
}

DBusMessage *menu_layout(DBusMessage *m) {
	DBusMessage *r = dbus_message_new_method_return(m);
	DBusMessageIter it, st, a, ch, v, c, e;
	dbus_uint32_t u = menu_rev;
	int id = 0;
	dbus_message_iter_init_append(r, &it);
	dbus_message_iter_append_basic(&it, DBUS_TYPE_UINT32, &u);
	dbus_message_iter_open_container(&it, DBUS_TYPE_STRUCT, NULL, &st);
	dbus_message_iter_append_basic(&st, DBUS_TYPE_INT32, &id);
	dbus_message_iter_open_container(&st, DBUS_TYPE_ARRAY, "{sv}", &a);
	dbus_message_iter_close_container(&st, &a);
	dbus_message_iter_open_container(&st, DBUS_TYPE_ARRAY, "v", &ch);
	for (id = 1; id <= N_ITEMS; id++) {
		dbus_message_iter_open_container(&ch, DBUS_TYPE_VARIANT, "(ia{sv}av)", &v);
		dbus_message_iter_open_container(&v, DBUS_TYPE_STRUCT, NULL, &c);
		dbus_message_iter_append_basic(&c, DBUS_TYPE_INT32, &id);
		iprops(&c, id);
		dbus_message_iter_open_container(&c, DBUS_TYPE_ARRAY, "v", &e);
		dbus_message_iter_close_container(&c, &e);
		dbus_message_iter_close_container(&v, &c);
		dbus_message_iter_close_container(&ch, &v);
	}
	dbus_message_iter_close_container(&st, &ch);
	dbus_message_iter_close_container(&it, &st);
	return r;
}

DBusMessage *menu_group(DBusMessage *m) {
	DBusMessage *r = dbus_message_new_method_return(m);
	DBusMessageIter it, a, st;
	dbus_message_iter_init_append(r, &it);
	dbus_message_iter_open_container(&it, DBUS_TYPE_ARRAY, "(ia{sv})", &a);
	for (int id = 1; id <= N_ITEMS; id++) {
		dbus_message_iter_open_container(&a, DBUS_TYPE_STRUCT, NULL, &st);
		dbus_message_iter_append_basic(&st, DBUS_TYPE_INT32, &id);
		iprops(&st, id);
		dbus_message_iter_close_container(&a, &st);
	}
	dbus_message_iter_close_container(&it, &a);
	return r;
}
