#include <gem.h>
#include <stdlib.h>
#include <osbind.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "window.h"
#include "statwindow.h"
#include "util.h"
#include "global.h"

//#define DEBUG
#ifdef DEBUG
#include "natfeats.h"
#define dbg(format, arg...) do { nf_printf("DEBUG: (%s):" format, __FUNCTION__, ##arg); } while (0)
#define out(format, arg...) do { nf_printf("" format, ##arg); } while (0)
#else
#define dbg(format, arg...) do { ; } while (0)
#endif /* DEBUG */


short msgbuff[8];    /* event message buffer */
short keycode;       /* keycode returned by event-keyboard */
short mx;
short my;            /* mouse x and y pos. */
short butdown;       /* button state tested for, UP/DOWN */
short ret;           /* dummy return variable */
extern bool quit;

statwindow statwin(NAME | CLOSER | FULLER | MOVER | SIZER, " GEM OSview ");

/*
 * forward function prototypes
 */
static void multi(void);

int main(int argc, char *argv[])
{
#ifdef DEBUG
    nf_init();
#endif /* DEBUG */

    gl_apid = appl_init();
    phys_handle = graf_handle(&gl_wchar, &gl_hchar, &gl_wbox, &gl_hbox);
    wind_get_grect(0, WF_WORKXYWH, &gl_desk);
    graf_mouse(ARROW, 0x0L);
    vdi_handle = open_vwork(work_out, ext_out);

    dbg("%dx%d resolution in %d planes\r\n",
        work_out[0], work_out[1], gl_nplanes);

    butdown = 1;
    quit = false;

    statwin.open(100, 100, 300, 200);
    multi();

    return 0;
}

static int timer_cb(window *wi)
{
    wi->timer();

    return 0;
}

/*
 *  AES event dispatcher
 */
static void multi(void)
{
    short event;
    short keystate;
    short keyreturn;
    short mbreturn;
    long msec = 100;


    do
    {
        event = evnt_multi(MU_MESAG | MU_BUTTON | MU_KEYBD | MU_TIMER,
                        0x103, 3, butdown,
                        0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0,
                        msgbuff, msec, &mx, &my, &mbreturn, &keystate, &keyreturn, &ret);


        if (event & MU_MESAG)
        {
            statwin.event(msgbuff);
        }
        else if (event & MU_TIMER)
            statwin.timer();
        else if (event & MU_BUTTON)
        {
            dbg("button event 0x%x\r\n", mbreturn);
            if (butdown)
            {
                butdown = 0;
            }
            else
            {
                butdown = 1;
            }
        }
        else if (event & MU_KEYBD)
        {
        }
    } while (!quit);
}
