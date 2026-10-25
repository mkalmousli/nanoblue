#include "bt.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#define IS(n) (!strcmp(k, n))

DBusMessageIter sub(DBusMessageIter *i) { DBusMessageIter s; dbus_message_iter_recurse(i, &s); return s; }

/* dict entry -> key; *v is left at the value */
const char *kv(DBusMessageIter *e, DBusMessageIter *v) {
	const char *k;
	*v = sub(e);
	dbus_message_iter_get_basic(v, &k);
	dbus_message_iter_next(v);
	return k;
}

void cp(char *d, size_t n, const char *s) { snprintf(d, n, "%.*s", (int)n - 1, s); }

void dev_prop(Dev *x, const char *k, int t, V v) {
	if (t == 's') {
		if (IS("Alias")) cp(x->name, sizeof x->name, v.s);
		else if (IS("Address")) cp(x->addr, sizeof x->addr, v.s);
		else if (IS("Icon")) cp(x->icon, sizeof x->icon, v.s);
		else if (IS("AddressType")) cp(x->atype, sizeof x->atype, v.s);
	} else if (t == 'b') {
		int *f = IS("Paired") ? &x->paired : IS("Connected") ? &x->connected
		       : IS("Trusted") ? &x->trusted : IS("Blocked") ? &x->blocked : NULL;
		if (f) *f = v.i;
	} else if (t == 'n' && IS("RSSI")) x->rssi = v.n;
	else if (t == 'u' && IS("Class")) x->cls = v.u;
}

void adp_prop(Adp *a, const char *k, int t, V v) {
	if (t == 's') {
		if (IS("Alias")) cp(a->name, sizeof a->name, v.s);
		else if (IS("Address")) cp(a->addr, sizeof a->addr, v.s);
		else if (IS("Modalias")) cp(a->modalias, sizeof a->modalias, v.s);
	} else if (t == 'b') {
		int *f = IS("Powered") ? &a->powered : IS("Discovering") ? &a->scanning
		       : IS("Discoverable") ? &a->discoverable : IS("Pairable") ? &a->pairable : NULL;
		if (f) *f = v.i;
	} else if (t == 'u') {
		unsigned *f = IS("DiscoverableTimeout") ? &a->dto : IS("PairableTimeout") ? &a->pto : IS("Class") ? &a->cls : NULL;
		if (f) *f = v.u;
	}
}

static void base(const char *link, char *out, size_t n) {
	char t[256];
	ssize_t l = readlink(link, t, sizeof t - 1);
	const char *b;
	if (l <= 0) { cp(out, n, "unknown"); return; }
	t[l] = 0;
	b = strrchr(t, '/');
	cp(out, n, b ? b + 1 : t);
}

void adp_sys(Adp *a) {
	char p[160];
	snprintf(p, sizeof p, "/sys/class/bluetooth/%s/device/driver", a->path + 11);
	base(p, a->driver, sizeof a->driver);
	snprintf(p, sizeof p, "/sys/class/bluetooth/%s/device/subsystem", a->path + 11);
	base(p, a->bus, sizeof a->bus);
}

unsigned uuid_cap(const char *u) {
	static const struct { const char *id; unsigned cap; } T[] = {
		{"110a", CAP_AUDIO}, {"110b", CAP_AUDIO}, {"110d", CAP_AUDIO}, {"1124", CAP_HID}, {"1812", CAP_HID},
		{"111e", CAP_PHONE}, {"111f", CAP_PHONE}, {"1112", CAP_PHONE}, {"1105", CAP_OPP}, {"1116", CAP_NAP}, {"1101", CAP_SPP},
	};
	for (unsigned i = 0; strlen(u) > 8 && i < sizeof T / sizeof *T; i++)
		if (!strncasecmp(u + 4, T[i].id, 4)) return T[i].cap;
	return 0;
}
