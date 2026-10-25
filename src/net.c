#include "bt.h"

/* BlueZ Network1: join the device's access point as a PAN user; bnep0 comes up */
void net_toggle(const Dev *d) {
	const char *nap = "nap";
	int up = d->caps & CAP_NETUP;
	DBusMessage *q = dbus_message_new_method_call(BZ, d->path, "org.bluez.Network1", up ? "Disconnect" : "Connect");
	if (!up) dbus_message_append_args(q, DBUS_TYPE_STRING, &nap, DBUS_TYPE_INVALID);
	bt_send(bus, q, up ? "Network off" : "Network", d->path);
}
