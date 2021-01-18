#ifndef STATWINDOW_H
#define STATWINDOW_H

#include "window.h"

class statwindow : public window
{
public:
    statwindow(short wi_kind, char *title);
    ~statwindow();

    // virtual void open(short x, short y, short w, short h);
    virtual void draw(short x, short y, short w, short h);
    virtual void timer(void);
private:
    short pattern_type;
};

#define STATWINDOW_WINELEMENTS SIZER|MOVER|FULLER|CLOSER|NAME
#define STATWINDOW_CLASS	0x2346L
#endif /* STATWINDOW_H_ */
