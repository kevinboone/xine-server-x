/*============================================================================

  xine-server-x
  searchres_request_handler.h
  Copyright (c)2020 Kevin Boone, GPL v3.0

============================================================================*/

#pragma once

#include "defs.h"
#include "props.h"
#include "request_handler.h"

BEGIN_DECLS

void searchres_request_handler_page (const RequestHandler *request_handler, 
        const Props *arguments, char **page);

END_DECLS








