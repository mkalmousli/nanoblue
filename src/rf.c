#include "bt.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int rd(int i, const char *f, char *out, int n) {
	char p[64];
	FILE *fp;
	snprintf(p, sizeof p, "/sys/class/rfkill/rfkill%d/%s", i, f);
	if (!(fp = fopen(p, "r"))) return 0;
	n = fgets(out, n, fp) != NULL;
	fclose(fp);
	return n;
}

int rf_state(int *soft, int *hard) {
	char b[32];
	for (int i = 0; i < 16; i++) {
		if (!rd(i, "type", b, sizeof b) || strncmp(b, "bluetooth", 9)) continue;
		*soft = rd(i, "soft", b, sizeof b) && b[0] == '1';
		*hard = rd(i, "hard", b, sizeof b) && b[0] == '1';
		return 1;
	}
	return 0;
}

/* struct rfkill_event: RFKILL_OP_CHANGE_ALL (3) for RFKILL_TYPE_BLUETOOTH (2) */
int rf_set(int block) {
	struct { unsigned idx; unsigned char type, op, soft, hard; } ev = { 0, 2, 3, block, 0 };
	int fd = open("/dev/rfkill", O_WRONLY), e = 0;
	if (fd < 0) return errno;
	if (write(fd, &ev, sizeof ev) != sizeof ev) e = errno;
	close(fd);
	return e;
}
