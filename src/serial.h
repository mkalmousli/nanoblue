typedef struct { int fd, pending, ch; long rx, tx; } Ser;
extern Ser ser;       /* fd < 0: closed; pending: RFCOMM connect in flight */
extern char ser_err[96];
int ser_rfcomm(const char *addr, int ch); /* starts a non-blocking connect; retries up to ch 30 on ECONNREFUSED */
int ser_tty(const char *path, int baud);
void ser_close(void);
int ser_poll(char *buf, int n);           /* drives connect, returns bytes read; -1 once when the link ended */
void ser_write(const char *s, int n);
