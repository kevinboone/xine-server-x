/*============================================================================

  xine-server-x
  template_manager.h
  Copyright (c)2020 Kevin Boone, GPL v3.0

============================================================================*/

#pragma once

#include <stdint.h>
#include <time.h>
#include "defs.h"

#define TEMPLATE_GENERIC_HTML          0
#define TEMPLATE_ERROR_HTML            1
#define TEMPLATE_GENERIC_TOP_HTML      2
#define TEMPLATE_GENERIC_BOTTOM_HTML   3
#define TEMPLATE_FILES_HTML            4
#define TEMPLATE_TRANSPORT_HTML        5
#define TEMPLATE_CAPTIONMENU_HTML      6
#define TEMPLATE_ADMIN_HTML            7
#define TEMPLATE_PLAYLIST_HTML         8 
#define TEMPLATE_RADIO_HTML            9 
#define TEMPLATE_SCANNER_HTML          10 
#define TEMPLATE_SCANNER_TOP_HTML      11 
#define TEMPLATE_NOSCANNER_HTML        12 
#define TEMPLATE_ALBUMS_HTML           13 
#define TEMPLATE_TRACKS_HTML           14 
#define TEMPLATE_BROWSE_HTML           15 
#define TEMPLATE_SEARCH_HTML           16 
#define TEMPLATE_SEARCHRES_HTML        17 
#define TEMPLATE_ARTISTS_HTML          18 
#define TEMPLATE_GENRES_HTML           19 
#define TEMPLATE_COMPOSERS_HTML        20 

BEGIN_DECLS

String *template_manager_get_template (int template);

void template_manager_substitute_template (String *s);

void template_manager_substitute_placeholder (String *s, 
       const char *placeholder, const char *replace);

END_DECLS


