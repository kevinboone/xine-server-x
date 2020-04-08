/*============================================================================

  xine-server-x
  xine-server-x-api.c
  Copyright (c)2020 Kevin Boone
  Distributed under the terms of the GPL v3.0

============================================================================*/
#define _GNU_SOURCE
#include "xine-server-x-api.h" 

/*============================================================================

  xineserver_x_perror

============================================================================*/
const char *xineserver_x_perror (int error)
  {
  const char *ret = "Unknown error";
  switch (error)
    {
    case XINESERVER_X_ERR_OK: ret = "OK"; break;
    case XINESERVER_X_ERR_UNKNOWN_FN: 
      ret = "Unknown API function name"; break;
    case XINESERVER_X_ERR_ARG: 
      ret = "Bad or missing API argument"; break;
    case XINESERVER_X_ERR_LIST_DIR: 
      ret = "Can't list directory contents"; break;
    case XINESERVER_X_ERR_XS_SYNTAX:
      ret = "Internal error: bad syntax in communication with xine-server"; 
      break;
    case XINESERVER_X_ERR_PLAYBACK:
      ret = "Unspecified problem with audio playback"; break;
    case XINESERVER_X_ERR_NOFILE:
      ret = "Attempt to play a non-existent local file"; break;
    case XINESERVER_X_ERR_XS_BADCOMMAND:
      ret = "Internal error: bad command in communication with xine-server"; 
      break;
    case XINESERVER_X_ERR_XS_BADARG:
      ret = "Internal error: bad argument in communication with xine-server"; 
      break;
    case XINESERVER_X_ERR_PLAYLIST_INDEX:
      ret = "Playlist index out of range"; break;
    case XINESERVER_X_ERR_PLAYLIST_EMPTY:
      ret = "Playlist is empty"; break;
    case XINESERVER_X_ERR_PLAYLIST_END:
      ret = "Can't play beyond the end of the playlist"; break;
    case XINESERVER_X_ERR_PLAYLIST_START:
      ret = "Can't play beyond the start of the playlist"; break;
    case XINESERVER_X_ERR_XS_COMM:
      ret = "Can't communicate with xine-server"; break;
    case XINESERVER_X_ERR_XS_RESPONSE:
      ret = "Unexpected response from xine-server"; break;
    case XINESERVER_X_ERR_NO_FILES:
      ret = "No playable audio files in directory"; break;
    case XINESERVER_X_ERR_OPEN_STATION_LIST:
      ret = "Can't open station list"; break; 
    case XINESERVER_X_ERR_FIND_STATION:
      ret = "Can't find station name in station list"; break; 
    case XINESERVER_X_ERR_SCANNER_FILE:
      ret = "Can't interpret the scanner status file"; break; 
    case XINESERVER_X_ERR_NO_INDEX:
      ret = "Operation requires an index file, but none was specified"; break; 
    case XINESERVER_X_ERR_GEN_DATABASE:
      ret = "General database error (no more information available)"; break; 
    }  
  return ret;
  }

