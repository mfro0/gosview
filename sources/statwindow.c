#include "window.h"
#include "global.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>
#include "statwindow.h"
#include "os_stat.h"

//#define DEBUG
#ifdef DEBUG
#include "natfeats.h"
#define dbg(format, arg...) do { nf_printf("DEBUG: (%s):" format, __FUNCTION__, ##arg); } while (0)
#define out(format, arg...) do { nf_printf("" format, ##arg); } while (0)
#else
#define dbg(format, arg...) do { ; } while (0)
#endif /* DEBUG */

struct statwindow
{
    short pattern_type;
};

static void draw_statwindow(struct window *wi, short x, short y, short w, short h);
static void timer_statwindow(struct window *wi);
static void delete_statwindow(struct window *wi);

/*
 * create a new window and add it to the window list.
 */
struct window *create_statwindow(short wi_kind, char *title)
{
    struct window *wi = NULL;
    struct statwindow *sw;

    wi = create_window(wi_kind, title);

    if (wi != NULL)
    {
        wi->wclass = STATWINDOW_CLASS;
        wi->draw = draw_statwindow;
        wi->del = delete_statwindow;
        wi->timer = timer_statwindow;

        sw = malloc(sizeof(struct statwindow));
        wi->priv = sw;
        sw->pattern_type = 1;

        wi->top = 0;
        wi->left = 0;
        wi->doc_width = 0;
        wi->doc_height = 0;
        wi->x_fac = gl_wchar;	/* width of one character */
        wi->y_fac = gl_hchar;	/* height of one character */
    }
    return wi;
}

static void delete_statwindow(struct window *wi)
{
    if (wi && wi->priv) free(wi->priv);
    /* let the generic window code do the rest */
    delete_window(wi);

    exit(0);
}

/*
 * Draw testing code
 */
static void draw_statwindow(struct window *wi, short x, short y, short w, short h)
{
    struct statwindow *sw = (struct statwindow *) wi->priv;
    short bar_pxy[4] = { 0, 0, 99, 19 };
    int i;
    short wx, wy, ww, wh;
    short vh = wi->vdi_handle;

    wind_get(wi->handle, WF_WORKXYWH, &wx, &wy, &ww, &wh);

    wi->clear(wi, x, y, w, h);

    dbg("pattern type=%d\r\n", tw->pattern_type);

    for (i = 0; i < 5; i++)
    {
        short scoords[4];

        memcpy(scoords, bar_pxy, sizeof(scoords));
        scoords[0] += wx + i * 13;
        scoords[1] += wy + i * 20;
        scoords[2] += wx + i * 13;
        scoords[3] += wy + i * 20;

        vsf_interior(vh, FIS_PATTERN);
        vsf_style(vh, sw->pattern_type);
        vsf_perimeter(vh, 1);
        vsf_color(vh, G_BLACK);

        v_bar(vh, scoords);
    }
}


/*
 * react on timer events
 */
static void timer_statwindow(struct window *wi)
{
    static int r_counter = 0;
    struct statwindow *sw = (struct statwindow *) wi->priv;

    r_counter++;
    r_counter %= 20;

    if (r_counter == 19)
    {
        sw->pattern_type += 1;
        sw->pattern_type %= 24;

        dbg("pattern=%d\r\n", sw->pattern_type);

        do_redraw(wi, wi->work.g_x, wi->work.g_y, wi->work.g_w, wi->work.g_h);

        r_counter = 0;

        os_stat();
    }
}
