#include <dbus/dbus.h>
#include <stddef.h>
#define TS 64 /* tray icon edge in pixels */
typedef struct { int visible, powered, scanning, connected; } TrayState;
extern DBusConnection *tray_bus;
extern TrayState tray_st;
extern int tray_ev; /* set by tray: 1 toggle window, 2 power, 3 scan, 4 quit */

int tray_init(void);                   /* StatusNotifierItem on the session bus; 0 if a watcher took it */
void tray_update(const TrayState *s);  /* refreshes icon + menu labels when state changed */
void tray_process(void);
void tray_pixels(unsigned char *argb, unsigned col); /* TS x TS bluetooth rune, bytes A R G B */
void tray_png(char *path, size_t n, const char *name, unsigned col);
unsigned tray_color(void);

/* shared with tmenu.c */
void vbasic(DBusMessageIter *it, int type, const void *v);
DBusMessage *props_reply(DBusMessage *m, const char **names, int (*put)(DBusMessageIter *, const char *));
void menu_register(void);
void menu_changed(void);
DBusMessage *menu_layout(DBusMessage *m);
DBusMessage *menu_group(DBusMessage *m);
extern int menu_rev;
extern char icon[160];
void set_icon(void);
