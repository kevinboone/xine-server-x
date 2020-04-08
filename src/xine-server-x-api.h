/*============================================================================

  xine-server-x 
  xine-server-x-api.h
  Copyright (c)2020 Kevin Boone
  Distributed under the terms of the GPL v3.0

============================================================================*/
#pragma once

// Boolean -- define these if nobody else has

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE 
#define FALSE 0
#endif

#ifndef BOOL
typedef int BOOL;
#endif

// Default server port
#define XINESERVER_DEF_PORT 30001
#define XINESERVER_X_DEF_PORT 30000

// Error codes

// No error
#define XINESERVER_X_ERR_OK             0
// xine-server-x cannot connect to xine-server

// Error codes that mirror xine-server error code
// Since these codes are passed back from xine-server to XSX clients,
//   it is important that the error code numbers align with
//   those used by xine-server. We don't want clients of XSX to have
//   to include xine-server-api.h as well

// Syntax error in client command from XSX to xine-server
//  (This should never happen)
#define XINESERVER_X_ERR_XS_SYNTAX     1 
// General playback problem
#define XINESERVER_X_ERR_PLAYBACK      2 
// Tried to play a non-existent file 
#define XINESERVER_X_ERR_NOFILE        3 
// xine-server got an unrecognized command 
// This should never happen
#define XINESERVER_X_ERR_XS_BADCOMMAND 4 
// xineserver received a defective command argument
// This should never happen
#define XINESERVER_X_ERR_XS_BADARG      5 
// Playlist index is out of range 
#define XINESERVER_X_ERR_PLAYLIST_INDEX 6 
// Playlist is empty 
#define XINESERVER_X_ERR_PLAYLIST_EMPTY 7 
// Attempt to move past end of playlist 
#define XINESERVER_X_ERR_PLAYLIST_END   8 
// Attempt to move before start of playlist 
#define XINESERVER_X_ERR_PLAYLIST_START 9 
// Communication error with xinserver server 
#define XINESERVER_X_ERR_XS_COMM        10 
// Unexpected response from xine-server server 
#define XINESERVER_X_ERR_XS_RESPONSE    11 

// Error codes specific to XSX, not used by xine-server

// Unknown XSX API function
#define XINESERVER_X_ERR_UNKNOWN_FN     100 
// Bad or missing argument to XSX API 
#define XINESERVER_X_ERR_ARG            101 
// Can't list directory contents 
#define XINESERVER_X_ERR_LIST_DIR       102 
// Directory contains no playable audio files 
#define XINESERVER_X_ERR_NO_FILES       103 
// Can't find/open a station list fle
#define XINESERVER_X_ERR_OPEN_STATION_LIST 104
// Can't find station in station list
#define XINESERVER_X_ERR_FIND_STATION   105
// Can't parse the scanner status file
#define XINESERVER_X_ERR_SCANNER_FILE   106
// Operation required an index file, but none specified
#define XINESERVER_X_ERR_NO_INDEX       107
// General database error
#define XINESERVER_X_ERR_GEN_DATABASE   108


// Transport status values. To avoid a load of fiddly conversion,
//  these values must match the corresponding values returned
//  by xine-server (and defined in xine-server-api.h)

typedef enum _XSSTransportStatus
  {
  XINESERVER_X_TRANSPORT_STOPPED = 0,
  XINESERVER_X_TRANSPORT_PLAYING = 1,
  XINESERVER_X_TRANSPORT_PAUSED = 2,
  XINESERVER_X_TRANSPORT_BUFFERING = 3
  } XSXTransportStatus;

// API commands

#define XINESERVER_X_FN_SHUTDOWN       "shutdown"
#define XINESERVER_X_FN_STATUS         "status"
#define XINESERVER_X_FN_PLAY_FILE      "play_file"
#define XINESERVER_X_FN_ADD_FILE       "add_file"
#define XINESERVER_X_FN_PLAY_DIR       "play_dir"
#define XINESERVER_X_FN_ADD_DIR        "add_dir"
#define XINESERVER_X_FN_STOP           "stop"
#define XINESERVER_X_FN_PLAY           "play"
#define XINESERVER_X_FN_PAUSE          "pause"
#define XINESERVER_X_FN_NEXT           "next"
#define XINESERVER_X_FN_PREV           "prev"
#define XINESERVER_X_FN_PLAY_INDEX     "play_index"
#define XINESERVER_X_FN_SET_VOLUME     "set_volume"
#define XINESERVER_X_FN_PLAY_STATION   "play_station"
#define XINESERVER_X_FN_LIST_DIRS      "list_dirs"
#define XINESERVER_X_FN_LIST_STATION_LISTS      "list_station_lists"
#define XINESERVER_X_FN_LIST_STATION_NAMES      "list_station_names"
#define XINESERVER_X_FN_SCANNER_STATUS "scanner_status"
#define XINESERVER_X_FN_QUICK_SCAN     "quick_scan"
#define XINESERVER_X_FN_FULL_SCAN      "full_scan"
#define XINESERVER_X_FN_PLAY_ALBUM     "play_album"
#define XINESERVER_X_FN_LIST_ALBUMS    "list_albums"
#define XINESERVER_X_FN_ADD_MATCHING   "add_matching"
#define XINESERVER_X_FN_PLAY_MATCHING  "play_matching"
#define XINESERVER_X_FN_CLEAR          "clear"

#ifdef __cplusplus
exetern "C" { 
#endif

/** Convert an error code from an API function into a text string.
    This is a constant string, and should not be modified or freed */
const char *xineserver_x_perror (int error);

#ifdef __cplusplus
} 
#endif




