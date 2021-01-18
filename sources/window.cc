#include "window.h"
#include "util.h"
#include "global.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// #define DEBUG
#ifdef DEBUG
#include "natfeats.h"
#define dbg(format, arg...) do { nf_printf("DEBUG: (%s):" format, __FUNCTION__, ##arg); } while (0)
#define out(format, arg...) do { nf_printf("" format, ##arg); } while (0)
#else
#define dbg(format, arg...) do { ; } while (0)
#endif /* DEBUG */

#define WINDOW_CLASS	0x1234L

window::window(short wi_kind, char *title)
{
    handle = wind_create(wi_kind, gl_desk.g_x, gl_desk.g_y, gl_desk.g_w, gl_desk.g_h);

    if (handle < 1)
    {
        form_alert(1, "[1][cannot create window][OK]");
        return;
    }

    kind = wi_kind;
    is_fulled = false;

    is_topped = true;

    word_aligned = false;

    left = top = 0;		/* start display at top left corner of document */
    doc_width = doc_height = 0;

    v_opnvwk(work_in, &vdi_handle, work_out);
    wind_set_str(handle, WF_NAME, title);
    wind_set_str(handle, WF_INFO, "");
}

/*
 * resize window. Coordinates are frame coordinates
 */
void window::size(short x, short y, short w, short h)
{
    if (w < MIN_WIDTH)
    {
        w = MIN_WIDTH;
    }
    if (h < MIN_HEIGHT)
    {
        h = MIN_HEIGHT;
    }

    wind_set(handle, WF_CURRXYWH, x, y, w, h);
    wind_get(handle, WF_WORKXYWH, &work.g_x, &work.g_y, &work.g_w, &work.g_h);
    wind_get(handle, WF_CURRXYWH, &rect.g_x, &rect.g_y, &rect.g_w, &rect.g_h);
    scroll(); /* fix slider sizes and positions */
}

/*
 * open window
 */
void window::open(short x, short y, short w, short h)
{
    graf_growbox(/* desk_x + desk_w / 2 */ 10, /* desk_y + desk_h / 2 */ 10, gl_wbox, gl_hbox,
                 x, y, w, h);
    wind_open(handle, x, y, w, h);

    wind_get(handle, WF_WORKXYWH, &work.g_x, &work.g_y, &work.g_w, &work.g_h);
    wind_get(handle, WF_CURRXYWH, &rect.g_x, &rect.g_y, &rect.g_w, &rect.g_h);
}

/*
 * scroll window contents according to its slider settings
 */
void window::scroll(void)
{
    short xpos;
    short ypos;
    short sl_horiz;
    short sl_vert;
    short ret;
    short value;

    if ((kind & VSLIDE) || (kind & HSLIDE))
    {
        sl_horiz = (int)((float) work.g_w / x_fac / doc_width * 1000);
        sl_vert = (int)((float) work.g_h / y_fac / doc_height * 1000);

        xpos = (int)((float) left / (doc_width - work.g_w / x_fac) * 1000);
        ypos = (int)((float) top / (doc_height - work.g_h / y_fac) * 1000);

        /* wind_set is a costly operation. Set values only if we need to */
        wind_get(handle, WF_HSLIDE, &value, &ret, &ret, &ret);
        if (value != xpos)
            wind_set(handle, WF_HSLIDE, xpos, 0, 0, 0);

        wind_get(handle, WF_VSLIDE, &value, &ret, &ret, &ret);
        if (value != ypos)
            wind_set(handle, WF_VSLIDE, ypos, 0, 0, 0);

        wind_get(handle, WF_HSLSIZE, &value, &ret, &ret, &ret);
        if (value != sl_horiz)
            wind_set(handle, WF_HSLSIZE, sl_horiz, 0, 0, 0);

        wind_get(handle, WF_VSLSIZE, &value, &ret, &ret, &ret);
        if (value != sl_vert)
            wind_set(handle, WF_VSLSIZE, sl_vert, 0, 0, 0);
    }
}

void window::full(void)
{
    if (is_fulled)
    {
        wind_calc(WC_WORK, kind, old.g_x, old.g_y,
                  old.g_w, old.g_h,
                  &work.g_x, &work.g_y, &work.g_w, &work.g_h);
        wind_set(handle, WF_CURRXYWH, old.g_x, old.g_y, old.g_w, old.g_h);
    }
    else
    {
        wind_calc(WC_BORDER, kind, work.g_x, work.g_y,
                  work.g_w, work.g_h,
                  &old.g_x, &old.g_y, &old.g_w, &old.g_h);
        wind_calc(WC_WORK, kind, gl_desk.g_x, gl_desk.g_y, gl_desk.g_w, gl_desk.g_h,
                  &work.g_x, &work.g_y, &work.g_w, &work.g_h);
        wind_set(handle, WF_CURRXYWH, gl_desk.g_x, gl_desk.g_y, gl_desk.g_w, gl_desk.g_h);
    }
    wind_get(handle, WF_CURRXYWH, &rect.g_x, &rect.g_y, &rect.g_w, &rect.g_h);
    wind_get(handle, WF_WORKXYWH, &work.g_x, &work.g_y, &work.g_w, &work.g_h);
    is_fulled ^= 1;
}

static long time;
long *_hz_200 = (long *) 0x4ba;

static long start_timer(void)
{
    return time = *_hz_200;
}

static long stop_timer(void)
{
    return *_hz_200 - time;
}

/*
 * find and redraw all clipping rectangles
 */
void window::redraw(short xc, short yc, short wc, short hc)
{
    GRECT t1, t2 = { xc, yc, wc, hc };
    short vh = vdi_handle;

    graf_mouse(M_OFF, 0);
    wind_update(BEG_UPDATE);

    Supexec(start_timer);

    wind_get(handle, WF_FIRSTXYWH, &t1.g_x, &t1.g_y, &t1.g_w, &t1.g_h);
    while (t1.g_w || t1.g_h)
    {
        if (rc_intersect(&t2, &t1))
        {
            set_clipping(vh, t1.g_x, t1.g_y, t1.g_w, t1.g_h, 1);
            dbg("redraw window contents (%d, %d) to (%d, %d)\n",
                t1.g_x, t1.g_y, t1.g_x + t1.g_w, t1.g_y + t1.g_h);
            draw(t1.g_x, t1.g_y, t1.g_w, t1.g_h);
        }
        wind_get(handle, WF_NEXTXYWH, &t1.g_x, &t1.g_y, &t1.g_w, &t1.g_h);
    }
    sprintf(info, "Time for redraw: %ld ms", Supexec(stop_timer) * 5);
    wind_set_str(handle, WF_INFO, info);

    wind_update(END_UPDATE);
    graf_mouse(M_ON, 0);
}

void window::event(short msgbuff[])
{
        switch (msgbuff[0])
        {
            case WM_REDRAW:

                redraw(msgbuff[4], msgbuff[5], msgbuff[6], msgbuff[7]);
                break;

            case WM_NEWTOP:
            case WM_ONTOP:
            case WM_TOPPED:
                wind_set(handle, WF_TOP, 0, 0, 0, 0);
                is_topped = true;
                break;

            case WM_UNTOPPED:
                is_topped = false;
                break;

            case WM_SIZED:
            case WM_MOVED:
                size(msgbuff[4], msgbuff[5], msgbuff[6], msgbuff[7]);
                break;

            case WM_FULLED:
                full();
                break;

            case WM_CLOSED:
                close();
                exit(0);
                break;

            case WM_ARROWED:
                switch (msgbuff[4])
                {
                    case WA_UPPAGE:
                        top -= work.g_h;
                        dbg("WA_UPPAGE\r\n");
                        break;

                    case WA_DNPAGE:
                        top += work.g_h;
                        dbg("WA_DNPAGE\r\n");
                        break;

                    case WA_UPLINE:
                        top -= y_fac;
                        dbg("WA_UPLINE\r\n");
                        break;

                    case WA_DNLINE:
                        top += x_fac;
                        dbg("WA_DNLINE\r\n");
                        break;

                    case WA_LFPAGE:
                        left -= (doc_width - work.g_w);
                        dbg("WA_LEFT\r\n");
                        break;

                    case WA_RTPAGE:
                        left += (doc_width - work.g_w);
                        dbg("WI_RTPAGE\r\n");
                        break;

                    case WA_LFLINE:
                        left--;
                        dbg("WA_LFLINE\r\n");
                        break;

                    case WA_RTLINE:
                        left++;
                        dbg("WA_RTLINE\r\n");
                        break;
                } /* switch */
                if (top > doc_height - work.g_h / y_fac)
                {
                    top = doc_height - work.g_h / y_fac;
                }
                if (top < 0) top = 0;
                if (left > doc_width - work.g_w / x_fac)
                {
                    left = doc_width - work.g_w / x_fac;
                }

                if (left < 0) left = 0;
                scroll();
                redraw(work.g_x, work.g_y, work.g_w, work.g_h);
                break;

            case WM_HSLID:
                left = (int)((float) msgbuff[4] / 1000.0 * doc_width);

                scroll();

                dbg("WM_HSLID\r\n");
                redraw(work.g_x, work.g_y, work.g_w, work.g_h);
                break;

            case WM_VSLID:
                top = (int)((float) msgbuff[4] / 1000.0 * doc_height);
                scroll();

                dbg("WM_VSLID\r\n");
                redraw(work.g_x, work.g_y, work.g_w, work.g_h);
                break;

            default:
                dbg("unhandled event %x\n", msgbuff[0]);
                break;
        } /* switch (msgbuff[0]) */
}

void window::close(void)
{
    wind_close(handle);
}

/*
 * clear window contents (to background color white)
 */
void window::clear(short x, short y, short w, short h)
{
    short pxy[4];
    short vh = vdi_handle;

    vsf_interior(vh, 1);
    vsf_style(vh, 0);
    vsf_color(vh, 0);
    pxy[0] = x;
    pxy[1] = y;
    pxy[2] = x + w - 1;
    pxy[3] = y + h - 1;
    v_bar(vh, pxy);  /* blank the interior */
}

/*
 * send a redraw event to our own application
 */
void window::send_redraw(short x, short y, short w, short h)
{
    short msg[8] = { WM_REDRAW, ap_id, 0, handle, x, y, w, h };

    appl_write(ap_id, sizeof(msg), msg);
}


