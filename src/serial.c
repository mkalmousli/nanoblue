#include "serial.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <termios.h>
#include <unistd.h>
#ifndef AF_BLUETOOTH
#define AF_BLUETOOTH 31
#endif
#define BTPROTO_RFCOMM 3

Ser ser = { .fd = -1 };
char ser_err[96];
static char peer[20];
static int dead; /* set when a link ends by itself, reported once by ser_poll */
static const struct { int baud; speed_t s; } rates[] = {
	{9600, B9600}, {19200, B19200}, {38400, B38400}, {57600, B57600},
	{115200, B115200}, {230400, B230400}, {460800, B460800}, {921600, B921600} };

static void end(const char *why, int err) {
	snprintf(ser_err, sizeof ser_err, "%s%s%s", why, err ? ": " : "", err ? strerror(err) : "");
	if (ser.fd >= 0) close(ser.fd);
	ser.fd = -1, ser.pending = 0, dead = 1;
}

void ser_close(void) {
	if (ser.fd >= 0) close(ser.fd);
	ser.fd = -1, ser.pending = 0, dead = 0, ser_err[0] = 0;
}

/* sockaddr_rc without libbluetooth: bdaddr is stored reversed */
static int dial(void) {
	struct { sa_family_t fam; unsigned char b[6]; unsigned char ch; } a = { AF_BLUETOOTH, {0}, ser.ch };
	unsigned x[6];
	if (sscanf(peer, "%x:%x:%x:%x:%x:%x", x, x + 1, x + 2, x + 3, x + 4, x + 5) != 6) return end("bad address", 0), -1;
	for (int i = 0; i < 6; i++) a.b[5 - i] = x[i];
	if ((ser.fd = socket(AF_BLUETOOTH, SOCK_STREAM | SOCK_NONBLOCK, BTPROTO_RFCOMM)) < 0) return end("socket", errno), -1;
	if (connect(ser.fd, (struct sockaddr *)&a, sizeof a) < 0 && errno != EINPROGRESS) return end("connect", errno), -1;
	ser.pending = 1;
	return 0;
}

int ser_rfcomm(const char *addr, int ch) {
	ser_close();
	snprintf(peer, sizeof peer, "%s", addr);
	ser.ch = ch, ser.rx = ser.tx = 0;
	return dial();
}

int ser_tty(const char *path, int baud) {
	struct termios t;
	speed_t s = B115200;
	ser_close();
	ser.rx = ser.tx = 0;
	for (int i = 0; i < 8; i++) if (rates[i].baud == baud) s = rates[i].s;
	if ((ser.fd = open(path, O_RDWR | O_NOCTTY | O_NONBLOCK)) < 0) return end(path, errno), -1;
	tcgetattr(ser.fd, &t);
	cfmakeraw(&t);
	cfsetspeed(&t, s);
	t.c_cflag |= CLOCAL | CREAD;
	tcsetattr(ser.fd, TCSANOW, &t);
	return 0;
}

int ser_poll(char *buf, int n) {
	int r;
	if (dead) { dead = 0; return -1; }
	if (ser.fd < 0) return 0;
	if (ser.pending) {
		struct pollfd p = { ser.fd, POLLOUT, 0 };
		int e = 0;
		socklen_t l = sizeof e;
		if (poll(&p, 1, 0) <= 0) return 0;
		getsockopt(ser.fd, SOL_SOCKET, SO_ERROR, &e, &l);
		if (e == ECONNREFUSED && ser.ch < 30) { close(ser.fd), ser.fd = -1, ser.ch++, dial(); return 0; }
		if (e) end("connect", e);
		else ser.pending = 0;
		return 0;
	}
	if ((r = read(ser.fd, buf, n)) > 0) { ser.rx += r; return r; }
	if (r < 0 && (errno == EAGAIN || errno == EINTR)) return 0;
	end(r ? "read" : "closed by peer", r ? errno : 0);
	return 0;
}

void ser_write(const char *s, int n) {
	int w;
	if (ser.fd >= 0 && !ser.pending && n > 0 && (w = write(ser.fd, s, n)) > 0) ser.tx += w;
}
