#include "bt.h"
#include <stdlib.h>
#include <string.h>
#define EACH(s, p) for (DBusMessageIter s = sub(p); dbus_message_iter_get_arg_type(&s) != DBUS_TYPE_INVALID; dbus_message_iter_next(&s))
#define IS(n) (!strcmp(k, n))

static int acmp(const void *a, const void *b) { return strcmp(((Adp *)a)->path, ((Adp *)b)->path); }
static int dcmp(const void *a, const void *b) {
	const Dev *x = a, *y = b;
	return (y->connected - x->connected) ?: (y->paired - x->paired) ?: strcmp(x->addr, y->addr);
}

int bt_list(Dev *d, int max) {
	DBusMessage *q = dbus_message_new_method_call(BZ, "/", "org.freedesktop.DBus.ObjectManager", "GetManagedObjects");
	DBusMessage *r = dbus_connection_send_with_reply_and_block(bus, q, 1000, NULL);
	DBusMessageIter it;
	Adp as[MAXA];
	int n = 0, na = 0;
	dbus_message_unref(q);
	if (!r) return -1;
	dbus_message_iter_init(r, &it);
	EACH(o, &it) {
		DBusMessageIter ifs;
		Dev x = { .battery = -1 };
		Adp a = {0};
		int isdev = 0, isad = 0;
		cp(x.path, sizeof x.path, kv(&o, &ifs));
		cp(a.path, sizeof a.path, x.path);
		EACH(i, &ifs) {
			DBusMessageIter props;
			const char *in = kv(&i, &props);
			int dv = !strcmp(in, D1), ad = !strcmp(in, A1), ba = !strcmp(in, "org.bluez.Battery1"), nw = !strcmp(in, "org.bluez.Network1");
			isdev |= dv, isad |= ad;
			if (!dv && !ad && !ba && !nw) continue;
			EACH(p, &props) {
				DBusMessageIter var;
				const char *k = kv(&p, &var);
				DBusMessageIter vv = sub(&var);
				V v = {0};
				int t = dbus_message_iter_get_arg_type(&vv);
				if (!dbus_type_is_basic(t)) {
					if (dv && t == 'a' && IS("UUIDs")) { EACH(u, &vv) { const char *s = ""; dbus_message_iter_get_basic(&u, &s), x.uuids++, x.caps |= uuid_cap(s); } }
					continue;
				}
				dbus_message_iter_get_basic(&vv, &v);
				if (dv) dev_prop(&x, k, t, v);
				else if (ba && IS("Percentage")) x.battery = v.y;
				else if (nw && IS("Connected") && v.i) x.caps |= CAP_NETUP;
				else if (ad) adp_prop(&a, k, t, v);
			}
		}
		if (isdev && n < max) d[n++] = x;
		if (isad && na < MAXA) adp_sys(&a), as[na++] = a;
	}
	dbus_message_unref(r);
	qsort(as, na, sizeof *as, acmp);
	memcpy(bt_ads, as, na * sizeof *as);
	bt_nads = na;
	for (int i = 0; i < n; i++)
		for (int j = d[i].adapter = -1; j < na; j++) {
			size_t len = j < 0 ? 0 : strlen(as[j].path);
			if (j >= 0 && !strncmp(d[i].path, as[j].path, len) && d[i].path[len] == '/') d[i].adapter = j;
		}
	qsort(d, n, sizeof *d, dcmp);
	return n;
}
