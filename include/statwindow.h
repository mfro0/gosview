#ifndef STATWINDOW_H
#define STATWINDOW_H

struct window *create_statwindow(short wi_kind, char *title);

#define STATWINDOW_WINELEMENTS SIZER|MOVER|FULLER|CLOSER|NAME
#define STATWINDOW_CLASS	0x2346L
#endif /* STATWINDOW_H_ */
