/*============================================================================
  boilerplate 
  numberformat.h 
  Copyright (c)2020 Kevin Boone, GPL v3.0
============================================================================*/

#pragma once

#include <stdint.h>

BEGIN_DECLS

char   *numberformat_space_64 (int64_t n, const char *sep);
char   *numberformat_size_64 (uint64_t n, const char *sep, BOOL binary);
BOOL    numberformat_read_integer (const char *s, uint64_t *v, BOOL strict);
BOOL    numberformat_read_double (const char *s, double *v, BOOL strict);

END_DECLS



