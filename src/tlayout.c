#include "tray.h"
#include <string.h>

Mi tray_items[96];
int tray_nitems;

static int kids(int id) {
	for (int i = 0; i < tray_nitems; i++) if (tray_items[i].parent == id) return 1;
	return 0;
}

static void entry(DBusMessageIter *d, const char *k, int t, const void *v) {
	DBusMessageIter e;
	dbus_message_iter_open_container(d, DBUS_TYPE_DICT_ENTRY, NULL, &e);
	dbus_message_iter_append_basic(&e, DBUS_TYPE_STRING, &k);
	vbasic(&e, t, v);
	dbus_message_iter_close_container(d, &e);
}

/* a{sv} for one item */
static void props(DBusMessageIter *p, const Mi *m) {
	DBusMessageIter a;
	const char *sep = "separator", *chk = "checkmark", *sub = "submenu", *l = m->label, *ic = m->icon;
	dbus_bool_t off = 0;
	int on = m->on;
	dbus_message_iter_open_container(p, DBUS_TYPE_ARRAY, "{sv}", &a);
	if (m->kind == 1) entry(&a, "type", 's', &sep);
	else {
		entry(&a, "label", 's', &l);
		if (m->icon[0]) entry(&a, "icon-name", 's', &ic);
		if (m->kind == 3) entry(&a, "enabled", 'b', &off);
		if (m->kind == 2) entry(&a, "toggle-type", 's', &chk), entry(&a, "toggle-state", 'i', &on);
		if (kids(m->id)) entry(&a, "children-display", 's', &sub);
	}
	dbus_message_iter_close_container(p, &a);
}

/* (ia{sv}av) for id and, recursively, its children */
static void node(DBusMessageIter *it, int id) {
	DBusMessageIter st, ch, v;
	static const Mi root = { 0 };
	const Mi *m = &root;
	for (int i = 0; i < tray_nitems; i++) if (tray_items[i].id == id) m = &tray_items[i];
	dbus_message_iter_open_container(it, DBUS_TYPE_STRUCT, NULL, &st);
	dbus_message_iter_append_basic(&st, DBUS_TYPE_INT32, &id);
	props(&st, m);
	dbus_message_iter_open_container(&st, DBUS_TYPE_ARRAY, "v", &ch);
	for (int i = 0; i < tray_nitems; i++) {
		if (tray_items[i].parent != id) continue;
		dbus_message_iter_open_container(&ch, DBUS_TYPE_VARIANT, "(ia{sv}av)", &v);
		node(&v, tray_items[i].id);
		dbus_message_iter_close_container(&ch, &v);
	}
	dbus_message_iter_close_container(&st, &ch);
	dbus_message_iter_close_container(it, &st);
}

DBusMessage *menu_layout(DBusMessage *m) {
	DBusMessage *r = dbus_message_new_method_return(m);
	DBusMessageIter it, arg;
	dbus_uint32_t rev = menu_rev;
	int parent = 0;
	if (dbus_message_iter_init(m, &arg)) dbus_message_iter_get_basic(&arg, &parent);
	dbus_message_iter_init_append(r, &it);
	dbus_message_iter_append_basic(&it, DBUS_TYPE_UINT32, &rev);
	node(&it, parent);
	return r;
}

/* a(ia{sv}) for the requested ids (all when none given) */
DBusMessage *menu_group(DBusMessage *m) {
	DBusMessage *r = dbus_message_new_method_return(m);
	DBusMessageIter it, a, st, arg, ids;
	int want[96], nw = 0;
	if (dbus_message_iter_init(m, &arg) && dbus_message_iter_get_arg_type(&arg) == 'a')
		for (ids = (DBusMessageIter){0}, dbus_message_iter_recurse(&arg, &ids); dbus_message_iter_get_arg_type(&ids) == 'i' && nw < 96; dbus_message_iter_next(&ids))
			dbus_message_iter_get_basic(&ids, &want[nw++]);
	dbus_message_iter_init_append(r, &it);
	dbus_message_iter_open_container(&it, DBUS_TYPE_ARRAY, "(ia{sv})", &a);
	for (int i = 0; i < tray_nitems; i++) {
		int ok = !nw;
		for (int j = 0; j < nw; j++) ok |= want[j] == tray_items[i].id;
		if (!ok) continue;
		dbus_message_iter_open_container(&a, DBUS_TYPE_STRUCT, NULL, &st);
		dbus_message_iter_append_basic(&st, DBUS_TYPE_INT32, &tray_items[i].id);
		props(&st, &tray_items[i]);
		dbus_message_iter_close_container(&a, &st);
	}
	dbus_message_iter_close_container(&it, &a);
	return r;
}
