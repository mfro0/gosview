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

/*
 * create a new window and add it to the window list.
 */
statwindow::statwindow(short wi_kind, char *title) : window(wi_kind, title)
{
    pattern_index = 1;

}

statwindow::~statwindow()
{
    exit(0);
}

/*
 * Draw testing code
 */
void statwindow::draw(short x, short y, short w, short h)
{
    short bar_pxy[4] = { 0, 0, 99, 19 };
    int i;
    short wx, wy, ww, wh;
    short vh = vdi_handle;

    wind_get(handle, WF_WORKXYWH, &wx, &wy, &ww, &wh);

    clear(x, y, w, h);

    dbg("pattern type=%d\r\n", tw->pattern_type);

    for (i = 0; i < 5; i++)
    {
        short scoords[4];

        memcpy(scoords, bar_pxy, sizeof(scoords));
        scoords[0] += wx + i * 13;
        scoords[1] += wy + i * 20;
        scoords[2] += wx + i * 13;
        scoords[3] += wy + i * 20;

        vsf_interior(vh, FIS_SOLID);
        vsf_color(vh, pattern_index);
        vsf_perimeter(vh, 1);

        v_bar(vh, scoords);
    }
}


/*
 * react on timer events
 */
void statwindow::timer(void)
{
    os_stat();
    pattern_index++;
    send_redraw(work.g_x, work.g_y, work.g_w, work.g_h);
}
