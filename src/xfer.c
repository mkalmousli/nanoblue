#include "bt.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define OBEX "org.bluez.obex"

static FILE *dlg;
static Dev target;
static char picked[512];

/* second hop: the session exists, push the file through it */
static void session_up(DBusPendingCall *pc, void *file) {
	DBusMessage *r = dbus_pending_call_steal_reply(pc), *q;
	DBusError e;
	const char *path = "";
	dbus_error_init(&e);
	if (dbus_set_error_from_message(&e, r)) notify("Could not send file", e.message, "dialog-error");
	else if (dbus_message_get_args(r, NULL, DBUS_TYPE_OBJECT_PATH, &path, DBUS_TYPE_INVALID)) {
		q = dbus_message_new_method_call(OBEX, path, OBEX ".ObjectPush1", "SendFile");
		dbus_message_append_args(q, DBUS_TYPE_STRING, &file, DBUS_TYPE_INVALID);
		bt_send(tray_bus, q, "Send file", file);
		notify("Sending file", strrchr(file, '/') ? strrchr(file, '/') + 1 : file, "document-send");
	}
	dbus_error_free(&e);
	dbus_message_unref(r);
	dbus_pending_call_unref(pc);
}

void xfer_send(const Dev *d, const char *file) {
	if (!tray_bus || !d->path[0]) return;
	DBusMessage *q = dbus_message_new_method_call(OBEX, "/org/bluez/obex", OBEX ".Client1", "CreateSession");
	DBusMessageIter it, ar, en, v;
	const char *k = "Target", *opp = "opp", *dest = d->addr;
	DBusPendingCall *pc;
	dbus_message_append_args(q, DBUS_TYPE_STRING, &dest, DBUS_TYPE_INVALID);
	dbus_message_iter_init_append(q, &it);
	dbus_message_iter_open_container(&it, 'a', "{sv}", &ar), dbus_message_iter_open_container(&ar, 'e', NULL, &en);
	dbus_message_iter_append_basic(&en, 's', &k), dbus_message_iter_open_container(&en, 'v', "s", &v);
	dbus_message_iter_append_basic(&v, 's', &opp), dbus_message_iter_close_container(&en, &v);
	dbus_message_iter_close_container(&ar, &en), dbus_message_iter_close_container(&it, &ar);
	if (dbus_connection_send_with_reply(tray_bus, q, &pc, 60000) && pc) dbus_pending_call_set_notify(pc, session_up, strdup(file), free);
	dbus_message_unref(q);
	snprintf(bt_status, sizeof bt_status, "Sending to %s...", d->name);
}

/* zenity or kdialog, polled so the UI keeps running while the dialog is open */
void xfer_pick(const Dev *d) {
	if (dlg) return;
	target = *d, picked[0] = 0;
	if (!(dlg = popen("zenity --file-selection --title='Send file' 2>/dev/null || kdialog --getopenfilename \"$HOME\" 2>/dev/null", "r"))) return;
	fcntl(fileno(dlg), F_SETFL, O_NONBLOCK);
}

static DBusHandlerResult done(DBusConnection *c, DBusMessage *m, void *u) {
	DBusMessageIter it, ar;
	const char *iface = "";
	if (!dbus_message_is_signal(m, "org.freedesktop.DBus.Properties", "PropertiesChanged") || !dbus_message_iter_init(m, &it)) return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	dbus_message_iter_get_basic(&it, &iface), dbus_message_iter_next(&it);
	if (strcmp(iface, OBEX ".Transfer1")) return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
	for (dbus_message_iter_recurse(&it, &ar); dbus_message_iter_get_arg_type(&ar) == 'e'; dbus_message_iter_next(&ar)) {
		DBusMessageIter en, v;
		const char *k = "", *s = "";
		dbus_message_iter_recurse(&ar, &en), dbus_message_iter_get_basic(&en, &k), dbus_message_iter_next(&en), dbus_message_iter_recurse(&en, &v);
		if (strcmp(k, "Status") || dbus_message_iter_get_arg_type(&v) != 's') continue;
		dbus_message_iter_get_basic(&v, &s);
		if (!strcmp(s, "complete")) notify("File sent", "Transfer complete", "emblem-ok");
		else if (!strcmp(s, "error")) notify("Transfer failed", "The device rejected or lost the file", "dialog-error");
	}
	return DBUS_HANDLER_RESULT_HANDLED;
}

void xfer_init(void) {
	if (!tray_bus) return;
	dbus_bus_add_match(tray_bus, "type='signal',sender='" OBEX "',interface='org.freedesktop.DBus.Properties'", NULL);
	dbus_connection_add_filter(tray_bus, done, NULL, NULL);
}

void xfer_tick(void) {
	char b[256];
	size_t n, l;
	if (!dlg) return;
	errno = 0, n = fread(b, 1, sizeof b - 1, dlg), l = strlen(picked);
	if (n > 0 && l + n < sizeof picked) memcpy(picked + l, b, n), picked[l + n] = 0;
	if (n > 0) return;
	if (errno == EAGAIN) return clearerr(dlg);
	pclose(dlg), dlg = NULL;
	picked[strcspn(picked, "\n")] = 0;
	if (picked[0]) xfer_send(&target, picked);
}
