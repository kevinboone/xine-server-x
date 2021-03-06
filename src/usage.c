/*==========================================================================

  boilerplate
  usage.c
  Copyright (c)2020 Kevin Boone
  Distributed under the terms of the GPL v3.0

==========================================================================*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "usage.h" 


/*==========================================================================
  usage_show
==========================================================================*/
void usage_show (FILE *fout, const char *argv0)
  {
  fprintf (fout, "Usage: %s [options]\n", argv0);
  fprintf (fout, "  -d,--debug       stay in foreground\n");
  fprintf (fout, "  -g,--gxsradio=S  directory for radio station lists\n");
  fprintf (fout, "  -h,--help        show this message\n");
  fprintf (fout, "  -i,--index       index (database) file\n");
  fprintf (fout, "  -l,--log-level=N log level, 0-5 (default 2)\n");
  fprintf (fout, "  -p,--port=N      port number for this server (30000)\n");
  fprintf (fout, "  -q,--quickscan   scan changed files and build index\n");
  fprintf (fout, "  -r,--root=N      audio root directory\n");
  fprintf (fout, "  -s,--scan        scan files and build index\n");
  fprintf (fout, "  -v,--version     show version\n");
  fprintf (fout, "     --xsport=S    xine-server port (30001)\n");
  fprintf (fout, "     --xshost=S    xine-server host (localhost)\n");
  fprintf (fout, "  -x,--xslaunch=S  xine-server launch command\n");
  }

 
