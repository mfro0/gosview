#include <stdio.h>
#include <mint/mintbind.h>

#define DEBUG
#ifdef DEBUG
#include "natfeats.h"
#define dbg(format, arg...) do { nf_printf("DEBUG: (%s):" format, __FUNCTION__, ##arg); } while (0)
#define out(format, arg...) do { nf_printf("" format, ##arg); } while (0)
#else
#define dbg(format, arg...) do { ; } while (0)
#endif /* DEBUG */

struct os_stat
{
    struct loadavg
    {
        float loadavg1, loadavg2, loadavg3;
        int running, tasks;
    } loadavg;
    struct stat
    {
        unsigned long user;
        unsigned long sys;
        unsigned long nice;
        unsigned long other;
    } stat;

    struct memstat
    {

    } memstat;
} gl_st;

void os_stat(void)
{
    char buff[512];
    FILE *fh;
    long sz;
    int last_pid;

    fh = Fopen("/kern/loadavg", 0);
    sz = Fread(fh, sizeof(buff), buff);
    buff[sz] = '\0';
    dbg("buff=%s\r\n", buff);
    sscanf(buff, "%f %f %f %d/%d %d\n",
           &gl_st.loadavg.loadavg1,
           &gl_st.loadavg.loadavg2,
           &gl_st.loadavg.loadavg3,
           &gl_st.loadavg.running,
           &gl_st.loadavg.tasks, &last_pid);
    Fclose(fh);

    dbg("loadavg: %f, %f, %f, running: %d tasks: %d\r\n",
        gl_st.loadavg.loadavg1,
        gl_st.loadavg.loadavg2,
        gl_st.loadavg.loadavg3,
        gl_st.loadavg.running, gl_st.loadavg.tasks);

    fh = Fopen("/kern/stat", 0);
    sz = Fread(fh, sizeof(buff), buff);
    buff[sz] = '\0';
    dbg("buff=%s\r\n", buff);
    sscanf(buff, "%s %ld %ld %ld %ld\n", buff,
           &gl_st.stat.user, &gl_st.stat.nice,
           &gl_st.stat.sys, &gl_st.stat.sys);
    Fclose(fh);
    dbg("stat: user: %ld, sys: %ld, nice: %ld, other: %ld\r\n",
        gl_st.stat.user, gl_st.stat.sys, gl_st.stat.nice, gl_st.stat.other);

#ifdef NOT_USED
    fp = fopen("/kern/meminfo", "r");
    sz = fread(buff, 1, sizeof(buff), fp);
    buff[sz] = '\0';
    dbg("meminfo: %s\r\n", buff);
#endif
}
