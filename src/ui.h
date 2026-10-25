#include "raylib.h"
#include "bt.h"
/* everything is laid out in logical pixels; ui_s scales the whole frame (ctrl +/-) */
extern float ui_s;
Vector2 ui_mouse(Vector2 v);
void rlScalef(float x, float y, float z);
#define DrawText(s, x, y, sz, c) txt(s, x, y, sz, c)
#define MeasureText(s, sz) tw(s, sz)
#define GetMousePosition() ui_mouse(GetMousePosition())
#define GetScreenWidth() ((int)(GetScreenWidth() / ui_s))
#define GetScreenHeight() ((int)(GetScreenHeight() / ui_s))
#define BeginScissorMode(x, y, w, h) BeginScissorMode((x) * ui_s, (y) * ui_s, (w) * ui_s, (h) * ui_s)
#define C_BG    (Color){12, 14, 20, 255}
#define C_PANEL (Color){22, 26, 38, 255}
#define C_HOT   (Color){36, 43, 64, 255}
#define C_FG    (Color){230, 235, 245, 255}
#define C_DIM   (Color){120, 130, 155, 255}
#define C_GREEN (Color){60, 220, 140, 255}
#define C_BLUE  (Color){70, 150, 255, 255}
#define C_RED   (Color){255, 85, 100, 255}
#define C_YEL   (Color){255, 200, 70, 255}
#define C_CYAN  (Color){60, 230, 240, 255}

int button(Rectangle r, const char *t, Color c);   /* true on click */
void spinner(float x, float y, float r, Color c);
void rssi_bars(float x, float y, int rssi);
void battery(float x, float y, int pct);
void radar(float x, float y, float r, Color c);
Color dev_color(const Dev *d);                     /* red blocked, green connected, blue paired */
void act(const Dev *d, char what);                 /* c connect, p pair, t trust, b block, x remove */
void adp_act(int adapter, char what);              /* s scan, o power */
void chrome(int W, int H);                         /* top bar, progress, status bar */
void sidebar(Rectangle r, int *sa);
void devlist(Rectangle r, Dev *d, int n, int *sel, float *scroll);
void detail(Rectangle r, const Dev *d);
void fonts_init(void);                              /* load TTFs after InitWindow */
void txt(const char *s, float x, float y, float sz, Color c);
int tw(const char *s, float sz);
void mtxt(const char *s, float x, float y, float sz, Color c); /* monospace */
int term_on(void);                                 /* serial terminal visible */
void term_open(const Dev *d);                      /* d: RFCOMM to device, NULL: local tty port */
void term_frame(Rectangle r);
int toggle(Rectangle r, int on, Color c);          /* switch, true on click */
void dev_icon(float x, float y, float r, const char *icon, Color c); /* glyph by BlueZ Icon name, rune if unknown */
int adp_panel_on(void);                            /* adapter settings visible */
void adp_panel_open(int adapter);
void adp_panel(Rectangle r);
void shadow(Rectangle r, float depth);              /* soft drop shadow under a raised surface */
void card(Rectangle r, Color base, int raised);     /* gradient surface, specular edge, bevel */
void led(float x, float y, float r, Color c, int on); /* glossy lens with glow */
void backdrop(int w, int h);                        /* brushed dark backdrop with vignette */
void brand(int W);                                  /* version pill + source link */
void scrollbar(Rectangle r, float off, float content);
void scroll_begin(Rectangle r, float *off, float content); /* wheel-scrolls everything drawn until scroll_end */
void scroll_end(Rectangle r, float off, float content);
float chips(float x, float y, float w, const Dev *d);   /* capability pills, returns the y below them */
float actions(float x, float y, float w, const Dev *d); /* visual action list, returns the y below it */
