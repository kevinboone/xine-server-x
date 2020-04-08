/*============================================================================

  xine-server-x
  request_handler.h
  Copyright (c)2020 Kevin Boone, GPL v3.0

============================================================================*/

#pragma once

#include "defs.h"
#include "props.h"

struct _APIRequestHandler;
typedef struct _APIRequestHandler APIRequestHandler;

BEGIN_DECLS

APIRequestHandler *api_request_handler_create 
                      (RequestHandler *request_handler);

void               api_request_handler_destroy (APIRequestHandler *self);

void               api_request_handler_handle (APIRequestHandler *self, 
                     const char *uri, const Props *arguments, int *code, 
                     char **page);

END_DECLS




