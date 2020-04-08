/*============================================================================

  xine-server-x
  tracks_request_handler.h
  Copyright (c)2020 Kevin Boone, GPL v3.0

============================================================================*/

#pragma once

#include "defs.h"
#include "props.h"
#include "list.h"

BEGIN_DECLS

void    tracks_request_handler_page (const Props *arguments, char **page);
String *tracks_request_handler_track_list (List *list);

END_DECLS








