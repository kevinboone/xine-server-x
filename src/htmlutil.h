/*============================================================================

  xine-server-x
  htmlutil.h
  Copyright (c)2020 Kevin Boone, GPL v3.0

============================================================================*/

#pragma once

#include "defs.h"

BEGIN_DECLS

char *htmlutil_escape (const char *text);
char *htmlutil_make_href (const char *href, const char *text);
char *htmlutil_escape_squote_js (const char *str);
char *htmlutil_escape_dquote_js (const char *str);
char *htmlutil_escape_sdquote_js (const char *str);
char *htmlutil_escape_dquote_json (const char *str);

END_DECLS









