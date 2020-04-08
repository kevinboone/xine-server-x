/*============================================================================
  boilerplate 
  scanner.h 
  Copyright (c)2020 Kevin Boone, GPL v3.0
============================================================================*/

#pragma once

#include "program_context.h"

#define SCANNER_STATUS_FILE    "/tmp/xsx_scan_status"

// Number of files scanned between writing totals to status file
#define SCANNER_FILE_INTERVAL  10

BEGIN_DECLS

int scanner_run (ProgramContext *context);

END_DECLS



