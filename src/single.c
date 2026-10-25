#include "tray.h"
#define SINGLE "io.github.mkalmousli.Nanoblue"

/* another instance calls Raise on the first, which shows and focuses its window */
static DBusHandlerResult raise_handle(DBusConnection *c, DBusMessage *m, void *u) {
	DBusMessage *r;
	if (!dbus_message_is_method_call(m, SINGLE, "Raise")) return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	tray_ev = TRAY_RAISE;
	r = dbus_message_new_method_return(m);
	dbus_connection_send(c, r, NULL);
	dbus_message_unref(r);
	return DBUS_HANDLER_RESULT_HANDLED;
}
static const DBusObjectPathVTable rvt = { .message_function = raise_handle };

/* 1 when an instance already runs (it has been told to raise its window) */
int tray_single(void) {
	DBusMessage *q;
	if (!(tray_bus = dbus_bus_get(DBUS_BUS_SESSION, NULL))) return 0;
	dbus_connection_set_exit_on_disconnect(tray_bus, 0);
	if (dbus_bus_request_name(tray_bus, SINGLE, DBUS_NAME_FLAG_DO_NOT_QUEUE, NULL) == DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
		dbus_connection_register_object_path(tray_bus, "/org/nanoblue", &rvt, NULL);
		return 0;
	}
	q = dbus_message_new_method_call(SINGLE, "/org/nanoblue", SINGLE, "Raise");
	dbus_connection_send_with_reply_and_block(tray_bus, q, 1000, NULL);
	dbus_message_unref(q);
	return 1;
}
