#include <dbus/dbus.h>
#define MAXD 128
#define MAXA 4
#define BZ "org.bluez"
#define D1 "org.bluez.Device1"
#define A1 "org.bluez.Adapter1"

#define CAP_AUDIO 1
#define CAP_HID 2
#define CAP_PHONE 4
#define CAP_OPP 8   /* object push: can receive files */
#define CAP_NAP 16  /* offers network access (tethering) */
#define CAP_SPP 32
#define CAP_NETUP 64 /* its network is connected (bnep) */
typedef struct {
	char path[96], name[64], addr[20], icon[24], atype[12];
	int adapter, paired, connected, trusted, blocked, rssi, battery, uuids;
	unsigned cls, caps; /* caps: CAP_* bits from the UUIDs, plus network state */
} Dev;
typedef struct {
	char path[96], name[64], addr[20], modalias[48], driver[32], bus[16];
	int powered, scanning, discoverable, pairable;
	unsigned dto, pto, cls; /* discoverable / pairable timeouts in seconds */
} Adp;
typedef union { const char *s; int i; short n; unsigned char y; unsigned u; } V;

extern Adp bt_ads[MAXA];
extern int bt_nads, bt_busy, bt_dirty; /* busy = pending calls, dirty = BlueZ signalled a change */
extern char bt_status[128], bt_busy_path[96];
extern DBusConnection *bus;

int bt_init(void);
void bt_process(void);
int bt_list(Dev *d, int max); /* also fills bt_ads; -1 on error */
void bt_send(DBusConnection *c, DBusMessage *q, const char *label, const char *what); /* async call, reports into bt_status */
void bt_call(const char *path, const char *iface, const char *method, const char *arg);
void bt_set(const char *path, const char *iface, const char *prop, int type, const void *val); /* type: b s u */
void bt_setb(const char *path, const char *iface, const char *prop, int v);

/* props.c: D-Bus iterator helpers and property decoding */
DBusMessageIter sub(DBusMessageIter *i);
const char *kv(DBusMessageIter *e, DBusMessageIter *v);
void cp(char *d, size_t n, const char *s);
void dev_prop(Dev *x, const char *k, int t, V v);
void adp_prop(Adp *a, const char *k, int t, V v);
unsigned uuid_cap(const char *uuid); /* CAP_* bit for a 128-bit service UUID string */
void adp_sys(Adp *a); /* kernel driver + bus from sysfs */

/* rf.c: rfkill for the bluetooth radio */
int rf_state(int *soft, int *hard); /* 0 if no bluetooth rfkill */
int rf_set(int block);              /* 0 ok, else errno */
extern DBusConnection *tray_bus; /* session bus: tray, notifications, OBEX */
void notify(const char *title, const char *body, const char *icon); /* desktop notification */
void events_tick(const Dev *d, int n);                /* notify when a device connects, disconnects or pairs */
int auto_on(const char *addr);                        /* auto-connect flag, kept in ~/.config/nanoblue/auto */
void auto_toggle(const char *addr);
void auto_tick(const Dev *d, int n);                  /* reconnect flagged devices that dropped */
void xfer_send(const Dev *d, const char *file);       /* OBEX push to a device */
void xfer_pick(const Dev *d);                         /* file dialog, then xfer_send */
void xfer_init(void);
void xfer_tick(void);
void net_toggle(const Dev *d);                        /* tether through the device's network access point */
