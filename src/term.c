#include "ui.h"
#include "serial.h"
#include <glob.h>
#include <stdio.h>
#include <string.h>
#define NL 512
static struct { char t[160]; char tx; } ln[NL]; /* tx: 0 rx, 1 tx, 2 system */
static int n, mode, ch = 1, pi, bi = 4, hex, eol = 3, off, curn;
static char cur[160], in[200], addr[20], name[64];
static glob_t g;
static int have;
static const int bauds[] = {9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600};
static const char *eols[] = {"", "\r", "\n", "\r\n"}, *eoln[] = {"NONE", "CR", "LF", "CRLF"};

static void push(const char *s, int tx) { snprintf(ln[n % NL].t, 160, "%s", s), ln[n++ % NL].tx = tx; }
static void flush(void) { cur[curn] = 0, push(cur, 0), curn = 0; }

static void ingest(const unsigned char *b, int len) {
	for (int i = 0; i < len; i++) {
		if (hex) curn += snprintf(cur + curn, 160 - curn, "%02X ", b[i]);
		else if (b[i] == '\n') flush();
		else if (b[i] != '\r') cur[curn++] = b[i] >= 32 && b[i] < 127 ? b[i] : '.';
		if (curn >= (hex ? 48 : 120)) flush();
	}
}

static void ports(void) {
	if (have) globfree(&g);
	have = 1;
	glob("/dev/ttyUSB*", 0, NULL, &g), glob("/dev/ttyACM*", GLOB_APPEND, NULL, &g), glob("/dev/rfcomm*", GLOB_APPEND, NULL, &g);
	pi = g.gl_pathc ? pi % g.gl_pathc : 0;
}

int term_on(void) { return mode; }

void term_open(const Dev *d) {
	ser_close();
	n = curn = off = in[0] = 0, mode = d ? 1 : 2;
	if (d) snprintf(addr, sizeof addr, "%s", d->addr), snprintf(name, sizeof name, "%s", d->name), ch = 1, ser_rfcomm(addr, ch);
	else ports();
}

void term_frame(Rectangle r) {
	unsigned char b[512];
	int k, open, l = strlen(in), rows, total;
	float x = r.x + 16, y = r.y + 12, bx = x - 90, by = y + 56, y0 = by + 48, bot = r.y + r.height - 60;
	if (IsKeyPressed(KEY_ESCAPE)) { ser_close(), mode = 0; return; }
	while ((k = ser_poll((char *)b, sizeof b)) > 0) ingest(b, k);
	if (k < 0) push(TextFormat("-- %s --", ser_err), 2);
	open = ser.fd >= 0 && !ser.pending;
	for (int c; (c = GetCharPressed());) if (c >= 32 && c < 127 && l < 190) in[l++] = c, in[l] = 0;
	if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && l) in[--l] = 0;
	if (IsKeyPressed(KEY_ENTER) && open) {
		ser_write(in, l), ser_write(eols[eol], strlen(eols[eol])), push(in, 1), in[0] = 0;
	}
	DrawRectangleRec(r, C_PANEL);
	DrawText(mode == 1 ? TextFormat("RFCOMM  %.20s  %s  ch %d", name, addr, ser.fd >= 0 ? ser.ch : ch)
	                   : TextFormat("SERIAL  %s  %d baud", g.gl_pathc ? g.gl_pathv[pi] : "no ports found", bauds[bi]), x, y, 22, C_FG);
	DrawText(TextFormat("%s   RX %ld   TX %ld   %s", open ? "OPEN" : ser.pending ? "CONNECTING" : "CLOSED", ser.rx, ser.tx, ser_err),
	         x, y + 30, 14, open ? C_GREEN : ser.pending ? C_YEL : C_RED);
	#define B(t, c) button((Rectangle){bx += 90, by, 84, 34}, t, c)
	if (mode == 1) { if (B("CH -", C_DIM) && !open && ch > 1) ch--; if (B("CH +", C_DIM) && !open && ch < 30) ch++; }
	else { if (B("PORT", C_DIM)) ports(), pi = g.gl_pathc ? (pi + 1) % g.gl_pathc : 0; if (B("BAUD", C_DIM)) bi = (bi + 1) % 8; }
	if (B(open || ser.pending ? "CLOSE" : "OPEN", open || ser.pending ? C_RED : C_GREEN)) {
		if (ser.fd >= 0) ser_close();
		else if (mode == 1) ser_rfcomm(addr, ch);
		else if (g.gl_pathc) ser_tty(g.gl_pathv[pi], bauds[bi]);
	}
	if (B(hex ? "HEX" : "TEXT", C_CYAN)) hex = !hex;
	if (B(TextFormat("EOL %s", eoln[eol]), C_YEL)) eol = (eol + 1) % 4;
	if (B("CLEAR", C_DIM)) n = curn = 0;
	if (B("EXIT", C_RED)) { ser_close(), mode = 0; return; }
	rows = (bot - y0) / 20, total = n + (curn > 0);
	off += GetMouseWheelMove() * 3;
	off = off < 0 ? 0 : off > total - rows ? (total > rows ? total - rows : 0) : off;
	for (int i = 0; i < rows; i++) {
		int idx = total - rows - off + i, cr = idx == n;
		if (idx < 0 || idx >= total || idx < n - NL) continue;
		mtxt(cr ? cur : ln[idx % NL].t, x, y0 + i * 20, 16, cr || !ln[idx % NL].tx ? C_GREEN : ln[idx % NL].tx == 1 ? C_CYAN : C_YEL);
	}
	DrawRectangleRounded((Rectangle){x, bot + 8, r.width - 32, 38}, 0.2f, 8, C_BG);
	mtxt(TextFormat("> %s%s", in, (int)(GetTime() * 2) % 2 ? "_" : " "), x + 12, bot + 18, 16, C_FG);
}
