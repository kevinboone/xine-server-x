/*============================================================================

  xine-server-x
  request_handler.h
  Copyright (c)2020 Kevin Boone, GPL v3.0

============================================================================*/

#pragma once

#include <stdint.h>
#include <time.h>
#include "defs.h"
#include "props.h"
#include "request_handler.h"
#include "searchconstraints.h"

struct _GUIRequestHandler;
typedef struct _GUIRequestHandler GUIRequestHandler;

#define URI_PLAYLIST   "playlist"
#define URI_RADIO      "radio"
#define URI_ALBUMS     "albums"
#define URI_FILES      "files"
#define URI_ADMIN      "admin"
#define URI_SCANNER    "scanner"
#define URI_TRACKS     "tracks"
#define URI_ARTISTS    "artists"
#define URI_GENRES     "genres"
#define URI_COMPOSERS  "composers"
#define URI_BROWSE     "browse"
#define URI_SEARCH     "search"
#define URI_SEARCHRES  "searchres"

// Default number of albums in the albums page and search results
#define DEF_SEARCHRES         10
#define DEF_BROWSE_PER_PAGE   18

// MAX_LIMIT is the value of the limit URI argument if none is specified,
//   or it is specified as zero. This number needs to be much larger than
//   the largest number of anything in the database. This finagling is
//   only necessary because I forgot to account for the divide-by-zero
//   errors that would arise with a zero limit, and replicated the
//   same broker logic many times in the program :/
#define MAX_LIMIT             10000000

BEGIN_DECLS

GUIRequestHandler *gui_request_handler_create 
                      (RequestHandler *request_handler);

void               gui_request_handler_destroy (GUIRequestHandler *self);

void               gui_request_handler_handle (GUIRequestHandler *self, 
                    const char *uri, const Props* arguments, int *code, 
                    char **page);

void               gui_request_handler_error_page (const char *text, 
                     char **page);

String            *gui_request_handler_summary (const char *name, 
                     int from, int limit, const Props *arguments, int count, 
		     const SearchConstraints *sc);
String            *gui_request_handler_make_results_page_link 
                       (const char *uri, int from, int limit, 
		        const Props *arguments);

String            *gui_request_handler_listnav (const char *uri, const Props 
                       *arguments, int count, BOOL show_all);

END_DECLS




