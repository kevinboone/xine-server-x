/*============================================================================

  xine-server-x
  composers_request_handler.h
  Copyright (c)2020 Kevin Boone, GPL v3.0

============================================================================*/

#pragma once

#include "defs.h"
#include "props.h"

BEGIN_DECLS

void    composers_request_handler_page (const Props *arguments, char **page);
String *composers_request_handler_albumlist (List *list);

END_DECLS








