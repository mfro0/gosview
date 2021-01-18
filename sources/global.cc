#ifndef _GLOBAL_H_
#define _GLOBAL_H_

#include <stdint.h>
#include <stdbool.h>

#include "window.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

extern void init_global(void);
extern void free_global(void);

/*
 * GLOBAL VARIABLES
/*/

short phys_handle;	/* physical workstation handle */
short vdi_handle;	/* virtual workstation handle */
short contrl[12];
short shortin[128];
short ptsin[128];
short shortout[128];
short ptsout[128];	/* storage wasted for idiotic bindings */

short work_in[11];	/* Input to GSX parameter array */
short work_out[57];	/* Output from GSX parameter array */
short ext_out[57];
short pxyarray[10];	/* input poshort array */

short window_open_pos_x;	/* where to open new windows */
short window_open_pos_y;

short gl_wchar, gl_hchar, gl_wbox, gl_hbox;
short gl_nplanes;

OBJECT *gl_menu;
GRECT gl_desk;

bool quit;
#endif


