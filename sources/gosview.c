#include <gem.h>
#include <stdlib.h>
#include <osbind.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "window.h"
#include "statwindow.h"
#include "bench.h"
#include "menu.h"
#include "rsrc.h"
#include "util.h"
#include "global.h"
#include "dialog.h"
#include "fontwindow.h"

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
bool quit;

OBJECT *popup_tree;

struct dialog_handler *about_dialog;
struct dialog_handler *prefs_dialog;

static short about_exit_handler(OBJECT *dialog, short exit_obj);
static short prefs_exit_handler(OBJECT *dialog, short exit_obj);
static short prefs_touchexit_handler(OBJECT *dialog, short exit_obj);
static short prefs_init_handler(OBJECT *dialog);

struct window *statwin = NULL;

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

    init_windows();
    statwin = create_statwindow(NAME | CLOSER | FULLER | MOVER | SIZER,
                            " GEM OSview ");
    open_window(statwin, 100, 100, 300, 200);
    multi();

    return 0;
}

static short about_exit_handler(OBJECT *dialog, short exit_obj)
{
    printf("exit_obj=%d\r\n", exit_obj);
    return 0;
}

static short prefs_init_handler(OBJECT *dial)
{
    /*
     * preset the button texts of the buttons that
     * activate the pulldown menus with the first entry of the pulldowns
     */
    dial[PREFS_PUPTEST1].ob_spec.free_string = popup_tree[POPUP_ITEM1].ob_spec.free_string;
    dial[PREFS_PUPTEST2].ob_spec.free_string = popup_tree[POPUP_ICON1].ob_spec.iconblk->ib_ptext;

    return 0;
}

static short prefs_exit_handler(OBJECT *dialog, short exit_obj)
{
    printf("exit_obj=%d\r\n", exit_obj);
    return 0;
}

static short find_checked(OBJECT *tree, short start_index)
{
    short index;

    for (index = start_index;
         tree[index].ob_next != tree[tree[index].ob_next].ob_tail;
         index = tree[index].ob_next)
    {
        short state = tree[index].ob_state;

        if (state & OS_CHECKED)
        {
            return index;
        }
    }
    tree[start_index].ob_state |= OS_CHECKED;
    return start_index;
}

static MENU mnu;

static short prefs_touchexit_handler(OBJECT *dialog, short touchexit_obj)
{

    short x;
    short y;
    short previous_item = NIL;
    short current_item;

    /*
     * get the onscreen coordinate of the button that triggered us
     * so we will be able to position and pop up our menu over it
     */
    objc_offset(dialog, touchexit_obj, &x, &y);

    /*
     * select the popup menu that belongs to the triggering button
     */
    if (touchexit_obj == PREFS_PUPTEST1)
    {
        mnu.mn_tree = popup_tree;
        mnu.mn_menu = POPUP_STRINGS;
        mnu.mn_item = previous_item = find_checked(popup_tree, POPUP_ITEM1);
        mnu.mn_scroll = POPUP_ITEM1;
        y -= popup_tree[POPUP_STRINGS].ob_height / 2;
    }
    else if (touchexit_obj == PREFS_PUPTEST2)
    {
        mnu.mn_tree = popup_tree;
        mnu.mn_menu = POPUP_ICONS;
        mnu.mn_item = previous_item = find_checked(popup_tree, POPUP_ICON1);
        mnu.mn_scroll = POPUP_ICON1;
        y -= popup_tree[POPUP_ICONS].ob_height / 2;
    }

    /*
     * bring the menu on screen
     */
    menu_popup(&mnu, x, y, &mnu);


    current_item = mnu.mn_item;
    if (current_item != previous_item)
    {
        popup_tree[previous_item].ob_state &= ~OS_CHECKED;
        popup_tree[current_item].ob_state |= OS_CHECKED;
        if (touchexit_obj == PREFS_PUPTEST1)
        {
            dialog[touchexit_obj].ob_spec.free_string = popup_tree[current_item].ob_spec.free_string;
        }
        else if (touchexit_obj == PREFS_PUPTEST2)
        {
            dialog[touchexit_obj].ob_spec.free_string = popup_tree[current_item].ob_spec.iconblk->ib_ptext;
        }
    }
    return touchexit_obj;
}

static int timer_cb(struct window *wi)
{
    if (wi->timer) (*wi->timer)(wi);

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
    struct window *wi = NULL;

    do
    {
        event = evnt_multi(MU_MESAG | MU_BUTTON | MU_KEYBD | MU_TIMER,
                        0x103, 3, butdown,
                        0, 0, 0, 0, 0,
                        0, 0, 0, 0, 0,
                        msgbuff, msec, &mx, &my, &mbreturn, &keystate, &keyreturn, &ret);


        if (event & MU_MESAG)
        {
            wi = from_handle(msgbuff[3]);
            switch (msgbuff[0])
            {
                case WM_REDRAW:

                    do_redraw(wi, msgbuff[4], msgbuff[5], msgbuff[6], msgbuff[7]);
                    break;

                case WM_NEWTOP:
                case WM_ONTOP:
                case WM_TOPPED:
                    wind_set(wi->handle, WF_TOP, 0, 0, 0, 0);
                    wi->topped = true;
                    break;

                case WM_UNTOPPED:
                    wi->topped = false;
                    break;

                case WM_SIZED:
                case WM_MOVED:
                    if (wi->size) wi->size(wi, msgbuff[4], msgbuff[5], msgbuff[6], msgbuff[7]);
                    break;

                case WM_FULLED:
                    if (wi->full) wi->full(wi);
                    break;

                case WM_CLOSED:
                    if (wi->del) wi->del(wi);
                    break;

                case WM_ARROWED:
                    switch (msgbuff[4])
                    {
                        case WA_UPPAGE:
                            wi->top -= wi->work.g_h;
                            dbg("WA_UPPAGE\r\n");
                            break;

                        case WA_DNPAGE:
                            wi->top += wi->work.g_h;
                            dbg("WA_DNPAGE\r\n");
                            break;

                        case WA_UPLINE:
                            wi->top -= wi->y_fac;
                            dbg("WA_UPLINE\r\n");
                            break;

                        case WA_DNLINE:
                            wi->top += wi->x_fac;
                            dbg("WA_DNLINE\r\n");
                            break;

                        case WA_LFPAGE:
                            wi->left -= (wi->doc_width - wi->work.g_w);
                            dbg("WA_LEFT\r\n");
                            break;

                        case WA_RTPAGE:
                            wi->left += (wi->doc_width - wi->work.g_w);
                            dbg("WI_RTPAGE\r\n");
                            break;

                        case WA_LFLINE:
                            wi->left--;
                            dbg("WA_LFLINE\r\n");
                            break;

                        case WA_RTLINE:
                            wi->left++;
                            dbg("WA_RTLINE\r\n");
                            break;
                    } /* switch */
                    if (wi->top > wi->doc_height - wi->work.g_h / wi->y_fac)
                    {
                        wi->top = wi->doc_height - wi->work.g_h / wi->y_fac;
                    }
                    if (wi->top < 0) wi->top = 0;
                    if (wi->left > wi->doc_width - wi->work.g_w / wi->x_fac)
                    {
                        wi->left = wi->doc_width - wi->work.g_w / wi->x_fac;
                    }

                    if (wi->left < 0) wi->left = 0;
                    if (wi->scroll) wi->scroll(wi);
                    do_redraw(wi, wi->work.g_x, wi->work.g_y, wi->work.g_w, wi->work.g_h);
                    break;

                case WM_HSLID:
                    wi->left = (int)((float) msgbuff[4] / 1000.0 * wi->doc_width);

                    if (wi->scroll)
                    {
                        wi->scroll(wi);
                    }
                    dbg("WM_HSLID\r\n");
                    do_redraw(wi, wi->work.g_x, wi->work.g_y, wi->work.g_w, wi->work.g_h);
                    break;

                case WM_VSLID:
                    wi->top = (int)((float) msgbuff[4] / 1000.0 * wi->doc_height);
                    if (wi->scroll)
                    {
                        wi->scroll(wi);
                    }
                    dbg("WM_VSLID\r\n");
                    do_redraw(wi, wi->work.g_x, wi->work.g_y, wi->work.g_w, wi->work.g_h);
                    break;

                default:
                    dbg("unhandled event %x\n", msgbuff[0]);
                    break;
            } /* switch (msgbuff[0]) */
        }

        else if (event & MU_TIMER)
        {
            /*
             * we send a timer event to each window that requests regular scheduling services,
             * no matter if it's on top or not
             */
            foreach_window(timer_cb);
        }

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
