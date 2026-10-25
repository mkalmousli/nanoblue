#include "bt.h"
#include <string.h>

/* `nanoblue --demo`: fake adapters and devices, for screenshots and trying the UI without hardware */
static const struct { const char *name, *addr, *icon; int paired, conn, rssi, bat; } D[] = {
	{"Sony WH-1000XM5", "38:18:4C:A1:02:7F", "audio-headphones", 1, 1, -48, 82},
	{"Pixel 9 Pro", "F4:F5:E8:11:90:3C", "phone", 1, 1, -57, 64},
	{"Keychron K3", "C8:2E:18:5B:44:D0", "input-keyboard", 1, 0, -63, 41},
	{"MX Master 3S", "E4:09:A3:6F:12:8B", "input-mouse", 1, 0, -71, 93},
	{"Xbox Wireless Controller", "98:B6:E9:30:55:21", "input-gaming", 0, 0, -66, 55},
	{"JBL Flip 6", "70:99:1C:E2:0A:B4", "audio-card", 0, 0, -82, -1},
	{"ThinkPad X1", "AC:67:5D:90:4E:12", "computer", 0, 0, -75, -1},
};

int demo_list(Dev *d, int max) {
	Adp *a = bt_ads;
	cp(a[0].path, sizeof a[0].path, "/org/bluez/hci0"), cp(a[0].name, sizeof a[0].name, "Laptop Bluetooth"), cp(a[0].addr, sizeof a[0].addr, "2C:DB:07:0A:E0:50");
	cp(a[0].driver, sizeof a[0].driver, "btusb"), cp(a[0].bus, sizeof a[0].bus, "usb"), cp(a[0].modalias, sizeof a[0].modalias, "usb:v8087p0033d0001");
	a[0].powered = a[0].scanning = a[0].pairable = 1, a[0].pto = 0, a[0].dto = 180, a[0].cls = 0x7c010c;
	cp(a[1].path, sizeof a[1].path, "/org/bluez/hci1"), cp(a[1].name, sizeof a[1].name, "USB Dongle"), cp(a[1].addr, sizeof a[1].addr, "68:8F:C9:56:27:ED");
	cp(a[1].driver, sizeof a[1].driver, "btusb"), cp(a[1].bus, sizeof a[1].bus, "usb"), a[1].powered = 1;
	bt_nads = 2;
	for (int i = 0; i < (int)(sizeof D / sizeof *D) && i < max; i++) {
		Dev x = { .adapter = 0, .paired = D[i].paired, .connected = D[i].conn, .trusted = D[i].paired, .rssi = D[i].rssi, .battery = D[i].bat, .uuids = 6 + i, .caps = i == 0 ? CAP_AUDIO | CAP_PHONE : i == 1 ? CAP_OPP | CAP_NAP | CAP_PHONE | CAP_NETUP : i == 2 || i == 3 ? CAP_HID : i == 4 ? CAP_HID | CAP_SPP : CAP_AUDIO };
		cp(x.name, sizeof x.name, D[i].name), cp(x.addr, sizeof x.addr, D[i].addr), cp(x.icon, sizeof x.icon, D[i].icon), cp(x.atype, sizeof x.atype, "public");
		cp(x.path, sizeof x.path, "/org/bluez/hci0/dev_demo");
		d[i] = x;
	}
	return sizeof D / sizeof *D;
}
