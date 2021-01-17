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
#define err(format, arg...) do { fprintf(stderr, "" format, ##arg); } while (0)

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
    long res;
    short fh;
    long sz;
    int last_pid;

    res = Fopen("/kern/loadavg", 0);
    if (res > 0)
    {
        fh = (short) res;
        sz = Fread(fh, sizeof(buff), buff);
        buff[sz] = '\0';
        dbg("buff=%s", buff);

        sscanf(buff, "%f %f %f %d/%d %d\n",
           &gl_st.loadavg.loadavg1,
           &gl_st.loadavg.loadavg2,
           &gl_st.loadavg.loadavg3,
           &gl_st.loadavg.running,
           &gl_st.loadavg.tasks, &last_pid);
        Fclose(fh);
    }
    else
    {
        err("kern/loadavg cannot be opened\r\n");
        exit(1);
    }

    dbg("loadavg: %f, %f, %f, running: %d tasks: %d\r\n",
        gl_st.loadavg.loadavg1,
        gl_st.loadavg.loadavg2,
        gl_st.loadavg.loadavg3,
        gl_st.loadavg.running, gl_st.loadavg.tasks);

    res = Fopen("/kern/stat", 0);
    if (res > 0)
    {
        fh = (short) res;
        sz = Fread(fh, sizeof(buff), buff);
        buff[sz] = '\0';
        dbg("buff=%s", buff);
        sscanf(buff, "cpu %ld %ld %ld %ld",
           &gl_st.stat.user, &gl_st.stat.sys,
           &gl_st.stat.nice, &gl_st.stat.other);
        Fclose(fh);
        dbg("stat: user: %ld, sys: %ld, nice: %ld, other: %ld\r\n",
            gl_st.stat.user, gl_st.stat.sys, gl_st.stat.nice, gl_st.stat.other);
    }
    else
    {
        err("error opening /kern/stat\r\n");
        exit(1);
    }

#ifdef NOT_USED
    fp = fopen("/kern/meminfo", "r");
    sz = fread(buff, 1, sizeof(buff), fp);
    buff[sz] = '\0';
    dbg("meminfo: %s\r\n", buff);
#endif
}
