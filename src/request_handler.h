/*============================================================================

  xine-server-x
  request_handler.h
  Copyright (c)2020 Kevin Boone, GPL v3.0

============================================================================*/

#pragma once

#include "defs.h"
#include "props.h"
#include "program_context.h"

struct _RequestHandler;
typedef struct _RequestHandler RequestHandler;

BEGIN_DECLS

RequestHandler *request_handler_create (const char *root,
                  const ProgramContext *content);

void            request_handler_destroy (RequestHandler *self);

/** Handle an external file. This method can return in one of two ways.
    In case of error, *error is set to a newly-allocated error string, and
    the return value if FALSE. If the file can/should be transerred, the *size,
    *fd, *content_type, and *timestamp are set, and the method returns TRUE. 
    In all cases, *code is set -- 200 for a good request, and 4XX otherwise */ 
BOOL request_handler_ext_file (RequestHandler *self, const char *uri, 
        int *code, time_t if_modified, time_t *timestamp, size_t *size, 
        int *fd, char **content_type, char **error);

BOOL request_handler_int_file (RequestHandler *self, const char *uri, 
        int *code, time_t if_modified, time_t *timestamp, size_t *size, 
        const uint8_t **buff, char **content_type, char **error);

void request_handler_api (RequestHandler *self, const char *uri, 
      const Props *arguments, int *code, char **buff);

void request_handler_gui (RequestHandler *self, const char *uri, 
       const Props *arguments, int *code, char **buff);

BOOL request_handler_shutdown_requested (const RequestHandler *self);

void request_handler_request_shutdown (RequestHandler *self);

const ProgramContext *request_handler_get_program_context 
        (const RequestHandler *self);

const char *request_handler_get_index_file
      (const RequestHandler *self);

const Path *request_handler_get_root
      (const RequestHandler *self);

END_DECLS


