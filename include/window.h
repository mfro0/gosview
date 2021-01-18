#ifndef WINDOW_H
#define WINDOW_H

#include <gem.h>
#include <osbind.h>

class window
{
protected:
    short handle;
    long kind;
    bool word_aligned;

    GRECT rect;         /* current WF_SIZE */
    GRECT work;         /* = WF_WORKAREA */
    GRECT old;          /* previous rect */

    long left;          /* start of viewing rect in document */
    long top;

    long doc_width;
    long doc_height;

    long x_fac;         /* conversion factor document units <-> pixels in x */
    long y_fac;         /* conversion factor document units <-> pixels in y */

    bool is_open;
    bool is_topped;
    bool is_fulled;

    char name[200];		/* this is the XAAES max length, TOS allows only 80 characters */
    char info[200];		/* the window's info line */

public:
    window() {}
    window(short wi_kind, char *title);
    ~window();

    virtual void full(void);
    virtual void size(short x, short y, short w, short h);
    virtual void draw(short x, short y, short w, short h) = 0;
    virtual void redraw(short xc, short yc, short wc, short hc);
    virtual void open(short x, short y, short w, short h);
    virtual void clear(short x, short y, short w, short h);
    virtual void send_redraw(short x, short y, short w, short h);
    virtual void scroll(void);
    virtual void timer(void) = 0;
    virtual void event(short msgbuff[]);
    virtual void close(void);

    short work_in[10];
    short work_out[57];
    VdiHdl vdi_handle;
};

#define MIN_WIDTH  (10 * gl_wbox)
#define MIN_HEIGHT (10 * gl_hbox)

#endif /* WINDOW_H */

