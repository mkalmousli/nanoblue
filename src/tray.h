#include <dbus/dbus.h>
#include <stddef.h>
#define TS 64 /* tray icon edge in pixels */
typedef struct { int visible, powered, scanning, connected; } TrayState;
/* one dbusmenu entry; id doubles as the action code sent back in tray_ev */
typedef struct { int id, parent, kind, on; char label[72], icon[28]; } Mi; /* kind: 0 item, 1 separator, 2 checkmark, 3 disabled label */
extern DBusConnection *tray_bus;
extern TrayState tray_st;
extern Mi tray_items[96];
extern int tray_nitems, tray_ev, menu_rev, tray_reg; /* tray_ev: menu id clicked, 1 = icon activated */
extern char icon[160];
extern unsigned char tray_pix[TS * TS * 4];

#define TRAY_RAISE 5 /* tray_ev: show and focus the window */
int tray_single(void);                 /* 1 if already running (raises that one) */
int tray_init(void);                   /* StatusNotifierItem on the session bus; -1 without a tray host */
void tray_update(const TrayState *s);  /* icon colour from state */
void tray_menu(const Mi *m, int n);    /* replaces the menu, signals hosts only if it changed */
void tray_process(void);
void tray_pixels(unsigned char *argb, unsigned col);
void tray_png(char *path, size_t n, const char *name, unsigned col);
unsigned tray_color(void);
void set_icon(void);

void vbasic(DBusMessageIter *it, int type, const void *v);
int sni_put(DBusMessageIter *it, const char *k);
DBusMessage *props_reply(DBusMessage *m, const char **names, int (*put)(DBusMessageIter *, const char *));
void menu_register(void);
void menu_changed(void);
DBusMessage *menu_layout(DBusMessage *m);
DBusMessage *menu_group(DBusMessage *m);
