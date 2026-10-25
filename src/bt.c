#include "bt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Adp bt_ads[MAXA];
int bt_nads, bt_busy, bt_dirty;
char bt_status[128], bt_busy_path[96];
DBusConnection *bus;

/* NoInputNoOutput agent: accept everything with an empty reply */
static DBusHandlerResult agent(DBusConnection *c, DBusMessage *m, void *u) {
	if (dbus_message_get_type(m) != DBUS_MESSAGE_TYPE_METHOD_CALL) return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	DBusMessage *r = dbus_message_new_method_return(m);
	dbus_connection_send(c, r, NULL);
	dbus_message_unref(r);
	return DBUS_HANDLER_RESULT_HANDLED;
}
static const DBusObjectPathVTable vt = { .message_function = agent };

/* any BlueZ signal (property change, device added/removed) marks the model stale */
static DBusHandlerResult filt(DBusConnection *c, DBusMessage *m, void *u) {
	if (dbus_message_get_type(m) == DBUS_MESSAGE_TYPE_SIGNAL) bt_dirty = 1;
	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

static void agent_call(const char *m, int cap) {
	const char *p = "/nb/agent", *c = "NoInputNoOutput";
	DBusMessage *q = dbus_message_new_method_call(BZ, "/org/bluez", "org.bluez.AgentManager1", m), *r;
	dbus_message_append_args(q, DBUS_TYPE_OBJECT_PATH, &p, DBUS_TYPE_INVALID);
	if (cap) dbus_message_append_args(q, DBUS_TYPE_STRING, &c, DBUS_TYPE_INVALID);
	if ((r = dbus_connection_send_with_reply_and_block(bus, q, 2000, NULL))) dbus_message_unref(r);
	dbus_message_unref(q);
}

int bt_init(void) {
	if (!(bus = dbus_bus_get(DBUS_BUS_SYSTEM, NULL))) return -1;
	dbus_connection_register_object_path(bus, "/nb/agent", &vt, NULL);
	dbus_bus_add_match(bus, "type='signal',sender='org.bluez'", NULL);
	dbus_connection_add_filter(bus, filt, NULL, NULL);
	agent_call("RegisterAgent", 1);
	agent_call("RequestDefaultAgent", 0);
	return 0;
}

void bt_process(void) {
	if (!bus) return;
	dbus_connection_read_write(bus, 0);
	while (dbus_connection_dispatch(bus) == DBUS_DISPATCH_DATA_REMAINS);
}

static void done(DBusPendingCall *pc, void *label) {
	DBusMessage *r = dbus_pending_call_steal_reply(pc);
	DBusError e;
	dbus_error_init(&e);
	snprintf(bt_status, sizeof bt_status, "%s: %s", (char *)label, dbus_set_error_from_message(&e, r) ? e.message : "ok");
	dbus_error_free(&e);
	dbus_message_unref(r);
	dbus_pending_call_unref(pc);
	bt_busy--;
	bt_dirty = 1;
}

void bt_call(const char *path, const char *iface, const char *method, const char *arg) {
	DBusPendingCall *pc;
	DBusMessage *q;
	if (!bus) return;
	q = dbus_message_new_method_call(BZ, path, iface, method);
	if (arg) dbus_message_append_args(q, DBUS_TYPE_OBJECT_PATH, &arg, DBUS_TYPE_INVALID);
	snprintf(bt_status, sizeof bt_status, "%s...", method);
	snprintf(bt_busy_path, sizeof bt_busy_path, "%s", arg ? arg : path);
	if (dbus_connection_send_with_reply(bus, q, &pc, 60000) && pc) {
		bt_busy++;
		dbus_pending_call_set_notify(pc, done, strdup(method), free);
	}
	dbus_message_unref(q);
	dbus_connection_flush(bus);
}

void bt_set(const char *path, const char *iface, const char *prop, int type, const void *val) {
	if (!bus) return; /* demo mode */
	char sig[2] = { type, 0 };
	DBusMessageIter it, var;
	DBusMessage *q = dbus_message_new_method_call(BZ, path, "org.freedesktop.DBus.Properties", "Set");
	dbus_message_append_args(q, DBUS_TYPE_STRING, &iface, DBUS_TYPE_STRING, &prop, DBUS_TYPE_INVALID);
	dbus_message_iter_init_append(q, &it);
	dbus_message_iter_open_container(&it, DBUS_TYPE_VARIANT, sig, &var);
	dbus_message_iter_append_basic(&var, type, val);
	dbus_message_iter_close_container(&it, &var);
	dbus_connection_send(bus, q, NULL);
	dbus_message_unref(q);
	dbus_connection_flush(bus);
}

void bt_setb(const char *path, const char *iface, const char *prop, int v) {
	dbus_bool_t b = v;
	bt_set(path, iface, prop, 'b', &b);
}
