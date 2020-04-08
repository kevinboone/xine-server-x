/*============================================================================

  xine-server-x 
  searchconstraints.h
  Copyright (c)2020 Kevin Boone, GPL v3.0

============================================================================*/

#pragma once

#include <stdint.h>
#include "defs.h"
#include "props.h"

struct _SearchConstraints;
typedef struct _SearchConstraints SearchConstraints;

BEGIN_DECLS

SearchConstraints  *searchconstraints_create_from_args (const Props *args);

SearchConstraints  *searchconstraints_create_empty (void);

SearchConstraints  *searchconstraints_create_exhaustive (const char *term);

void                searchconstraints_destroy (SearchConstraints *self);

BOOL                searchconstraints_has_constraints 
                      (const SearchConstraints *self);

char               *searchconstraints_make_where 
                      (const SearchConstraints *self);

char               *searchconstraints_make_readable_where 
                      (const SearchConstraints *self);

END_DECLS

