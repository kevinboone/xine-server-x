/*============================================================================

  xine-server-x 
  api_request_handler.c
  Copyright (c)2020 Kevin Boone
  Distributed under the terms of the GPL v3.0

============================================================================*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <wchar.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <microhttpd.h>
#include "defs.h" 
#include "log.h" 
#include "path.h" 
#include "request_handler.h" 
#include "api_request_handler.h" 
#include "xine-server-x-api.h" 
#include "wstring.h" 
#include "facade.h" 
#include "htmlutil.h" 
#include "searchconstraints.h" 

struct _APIRequestHandler
  {
  RequestHandler *request_handler;
  }; 

// Forward decls
void api_request_handler_status (APIRequestHandler *self, 
       const Props *arguments, int *code, char **result);
void api_request_handler_play_file (APIRequestHandler *self,  
       const Props *arguments, int *code, char **result);
void api_request_handler_add_file (APIRequestHandler *self,  
       const Props *arguments, int *code, char **result);
void api_request_handler_play_dir (APIRequestHandler *self,  
       const Props *arguments, int *code, char **result);
void api_request_handler_add_dir (APIRequestHandler *self,  
       const Props *arguments, int *code, char **result);
void api_request_handler_shutdown (APIRequestHandler *self, 
       const Props *arguments, int *code, char **result);
void api_request_handler_stop (APIRequestHandler *self, 
       const Props *arguments, int *code, char **result);
void api_request_handler_play (APIRequestHandler *self, 
       const Props *arguments, int *code, char **result);
void api_request_handler_pause (APIRequestHandler *self, 
       const Props *arguments, int *code, char **result);
void api_request_handler_next (APIRequestHandler *self, 
       const Props *arguments, int *code, char **result);
void api_request_handler_prev (APIRequestHandler *self, 
       const Props *arguments, int *code, char **result);
void api_request_handler_play_index (APIRequestHandler *self, 
       const Props *arguments, int *code, char **result);
void api_request_handler_set_volume (APIRequestHandler *self, 
       const Props *arguments, int *code, char **result);
void api_request_handler_play_station (APIRequestHandler *self, 
       const Props *arguments, int *code, char **result);
void api_request_handler_list_dirs (APIRequestHandler *self, 
       const Props *arguments, int *code, char **result);
void api_request_handler_list_station_lists (APIRequestHandler *self, 
       const Props *arguments, int *code, char **result);
void api_request_handler_list_station_names (APIRequestHandler *self, 
       const Props *arguments, int *code, char **result);
void api_request_handler_scanner_status (APIRequestHandler *self, 
       const Props *arguments, int *code, char **result);
void api_request_handler_full_scan (APIRequestHandler *self, 
       const Props *arguments, int *code, char **result);
void api_request_handler_quick_scan (APIRequestHandler *self, 
       const Props *arguments, int *code, char **result);
void api_request_handler_play_album (APIRequestHandler *self, 
       const Props *arguments, int *code, char **result);
void api_request_handler_list_albums (APIRequestHandler *self, 
       const Props *arguments, int *code, char **result);
void api_request_handler_add_matching (APIRequestHandler *self, 
       const Props *arguments, int *code, char **result);
void api_request_handler_play_matching (APIRequestHandler *self, 
       const Props *arguments, int *code, char **result);
void api_request_handler_clear (APIRequestHandler *self, 
       const Props *arguments, int *code, char **result);

// Definition of function table
typedef void (*ApiFn) (APIRequestHandler *self, const Props *arguments, 
  int *code, char **result);

typedef struct 
  {
  ApiFn apiFn;
  const char *name;
  } ApiFnData;

ApiFnData apiFnData[] = 
  {
  { api_request_handler_status, XINESERVER_X_FN_STATUS },
  { api_request_handler_play_dir, XINESERVER_X_FN_PLAY_DIR },
  { api_request_handler_add_dir, XINESERVER_X_FN_ADD_DIR },
  { api_request_handler_add_file, XINESERVER_X_FN_ADD_FILE },
  { api_request_handler_play_file, XINESERVER_X_FN_PLAY_FILE },
  { api_request_handler_shutdown, XINESERVER_X_FN_SHUTDOWN },
  { api_request_handler_stop, XINESERVER_X_FN_STOP },
  { api_request_handler_play, XINESERVER_X_FN_PLAY },
  { api_request_handler_pause, XINESERVER_X_FN_PAUSE },
  { api_request_handler_next, XINESERVER_X_FN_NEXT },
  { api_request_handler_prev, XINESERVER_X_FN_PREV },
  { api_request_handler_play_index, XINESERVER_X_FN_PLAY_INDEX },
  { api_request_handler_set_volume, XINESERVER_X_FN_SET_VOLUME },
  { api_request_handler_play_station, XINESERVER_X_FN_PLAY_STATION },
  { api_request_handler_list_dirs, XINESERVER_X_FN_LIST_DIRS },
  { api_request_handler_list_station_lists, XINESERVER_X_FN_LIST_STATION_LISTS },
  { api_request_handler_list_station_names, XINESERVER_X_FN_LIST_STATION_NAMES },
  { api_request_handler_scanner_status, XINESERVER_X_FN_SCANNER_STATUS },
  { api_request_handler_quick_scan, XINESERVER_X_FN_QUICK_SCAN },
  { api_request_handler_full_scan, XINESERVER_X_FN_FULL_SCAN },
  { api_request_handler_play_album, XINESERVER_X_FN_PLAY_ALBUM },
  { api_request_handler_list_albums, XINESERVER_X_FN_LIST_ALBUMS },
  { api_request_handler_add_matching, XINESERVER_X_FN_ADD_MATCHING },
  { api_request_handler_play_matching, XINESERVER_X_FN_PLAY_MATCHING },
  { api_request_handler_clear, XINESERVER_X_FN_CLEAR },
  { NULL, NULL }
  };

/*============================================================================

  api_request_handler_create

============================================================================*/
APIRequestHandler *api_request_handler_create 
      (RequestHandler *request_handler)
  {
  LOG_IN
  APIRequestHandler *self = malloc (sizeof (APIRequestHandler)); 
  self->request_handler = request_handler;
  LOG_OUT 
  return self;
  }

/*============================================================================

  api_request_handler_destroy

============================================================================*/
void api_request_handler_destroy (APIRequestHandler *self)
  {
  LOG_IN
  if (self)
    {
    free (self);
    }
  LOG_OUT 
  }

/*============================================================================

 api_request_handler_stock_error

============================================================================*/
void api_request_handler_stock_error (int code, const char *msg, char **result)
  {
  LOG_IN
  if (msg)
    asprintf (result, "{ \"status\": %d, \"message\": \"%s\" }", 
       code, msg);
  else
    asprintf (result, "{ \"status\": %d, \"message\": \"%s\" }",
       code, xineserver_x_perror (code));
  LOG_OUT
  }

/*============================================================================

 api_request_handler_stock_ok

============================================================================*/
void api_request_handler_stock_ok (char **result)
  {
  LOG_IN
  api_request_handler_stock_error (0, "OK", result);
  LOG_OUT
  }

/*============================================================================

 api_request_handler_json_sanitize 

============================================================================*/
char *api_request_handler_json_sanitize (const char *s)
  {
  LOG_IN
  char *ret;
  ret = strdup (s); 
  LOG_OUT
  return ret;
  }

/*============================================================================

 api_request_handler_status

============================================================================*/
void api_request_handler_status (APIRequestHandler *self, 
       const Props *arguments, int *code, char **result)
  {
  LOG_IN
  char *error_message = NULL;
  int error_code = 0;
  PlaybackStatus *status = NULL;
  facade_get_playback_status (&error_code, &error_message, &status);
  if (error_code == 0)
    {
    char *composer = api_request_handler_json_sanitize 
       (playback_status_get_composer (status));
    char *artist = api_request_handler_json_sanitize 
       (playback_status_get_artist (status));
    char *album = api_request_handler_json_sanitize 
       (playback_status_get_album (status));
    char *title = api_request_handler_json_sanitize 
       (playback_status_get_title (status));
    char *genre = api_request_handler_json_sanitize 
      (playback_status_get_genre (status));

    asprintf (result, "{ \"status\": 0, \"message\": \"OK\", "
       "\"pos\": %d, \"len\": %d, \"transport_status\": %d, "
       "\"playlist_index\": %d, \"playlist_length\": %d, "
       "\"bitrate\": %d, \"composer\": \"%s\", \"album\": \"%s\", "
       "\"artist\": \"%s\", \"genre\": \"%s\", \"title\": \"%s\", "
       "\"seekable\": %d "
       "}",
      playback_status_get_pos (status),
      playback_status_get_len (status),
      playback_status_get_ts (status),
      playback_status_get_playlist_index (status),
      playback_status_get_playlist_length (status),
      playback_status_get_bitrate (status),
      composer, album, artist, genre, title,
      playback_status_is_seekable (status)
      );
    playback_status_destroy (status);
    free (composer);
    free (album);
    free (artist);
    free (genre);
    free (title);
    }
  else
    {
    api_request_handler_stock_error (error_code, error_message, result);
    free (error_message);
    } 
  *code = 200;
  LOG_OUT
  }

/*============================================================================

 api_request_handler_add_dir

============================================================================*/
void api_request_handler_add_dir (APIRequestHandler *self,
       const Props *arguments, int *code, char **result)
  {
  LOG_IN
  const char *dir = props_get (arguments, "dir");
  if (dir)
    {
    log_debug ("%s: add dir %s", __PRETTY_FUNCTION__, dir);
    char *error_message = NULL;
    int error_code = 0;
    facade_add_dir (dir, &error_code, &error_message);
    if (error_code == 0)
      {
      api_request_handler_stock_ok (result);
      }
    else
      {
      api_request_handler_stock_error (error_code, error_message, result);
      free (error_message);
      } 
    }
  else
    {
    log_warning ("add_dir API call without dir argument");
    api_request_handler_stock_error (XINESERVER_X_ERR_ARG, NULL, result);
    *code = 400;
    }
  LOG_OUT
  }

/*============================================================================

 api_request_handler_play_dir

============================================================================*/
void api_request_handler_play_dir (APIRequestHandler *self,
       const Props *arguments, int *code, char **result)
  {
  LOG_IN
  const char *dir = props_get (arguments, "dir");
  if (dir)
    {
    log_debug ("%s: play dir %s", __PRETTY_FUNCTION__, dir);
    char *error_message = NULL;
    int error_code = 0;
    facade_play_dir (dir, &error_code, &error_message);
    if (error_code == 0)
      {
      api_request_handler_stock_ok (result);
      }
    else
      {
      api_request_handler_stock_error (error_code, error_message, result);
      free (error_message);
      } 
    }
  else
    {
    log_warning ("play_dir API call without dir argument");
    api_request_handler_stock_error (XINESERVER_X_ERR_ARG, NULL, result);
    *code = 400;
    }
  LOG_OUT
  }

/*============================================================================

 api_request_handler_add_file

============================================================================*/
void api_request_handler_add_file (APIRequestHandler *self,
       const Props *arguments, int *code, char **result)
  {
  LOG_IN
  const char *file = props_get (arguments, "file");
  if (file)
    {
    log_debug ("%s: add file %s", __PRETTY_FUNCTION__, file);
    char *error_message = NULL;
    int error_code = 0;
    facade_add_file (file, &error_code, &error_message);
    if (error_code == 0)
      {
      api_request_handler_stock_ok (result);
      }
    else
      {
      api_request_handler_stock_error (error_code, error_message, result);
      free (error_message);
      } 
    }
  else
    {
    log_warning ("add_file API call without file argument");
    api_request_handler_stock_error (XINESERVER_X_ERR_ARG, NULL, result);
    *code = 400;
    }
  LOG_OUT
  }

/*============================================================================

 api_request_handler_play_file

============================================================================*/
void api_request_handler_play_file (APIRequestHandler *self,
       const Props *arguments, int *code, char **result)
  {
  LOG_IN
  const char *file = props_get (arguments, "file");
  if (file)
    {
    log_debug ("%s: play file %s", __PRETTY_FUNCTION__, file);
    char *error_message = NULL;
    int error_code = 0;
    facade_play_file (file, &error_code, &error_message);
    if (error_code == 0)
      {
      api_request_handler_stock_ok (result);
      }
    else
      {
      api_request_handler_stock_error (error_code, error_message, result);
      free (error_message);
      } 
    }
  else
    {
    log_warning ("play_file API call without file argument");
    api_request_handler_stock_error (XINESERVER_X_ERR_ARG, NULL, result);
    *code = 400;
    }
  LOG_OUT
  }

/*============================================================================

 api_request_handler_set_volume

============================================================================*/
void api_request_handler_set_volume (APIRequestHandler *self,
       const Props *arguments, int *code, char **result)
  {
  LOG_IN
  const char *s_volume = props_get (arguments, "volume");
  if (s_volume)
    {
    int volume = atoi (s_volume);
    log_debug ("%s: %d", __PRETTY_FUNCTION__, volume);
    char *error_message = NULL;
    int error_code = 0;
    facade_set_volume (volume, &error_code, &error_message);
    if (error_code == 0)
      {
      api_request_handler_stock_ok (result);
      }
    else
      {
      api_request_handler_stock_error (error_code, error_message, result);
      free (error_message);
      } 
    }
  else
    {
    log_warning ("play_index API call without index argument");
    api_request_handler_stock_error (XINESERVER_X_ERR_ARG, NULL, result);
    *code = 400;
    }
  LOG_OUT
  }

/*============================================================================

 api_request_handler_play_album

============================================================================*/
void api_request_handler_play_album (APIRequestHandler *self,
       const Props *arguments, int *code, char **result)
  {
  LOG_IN
  const char *album = props_get (arguments, "album");
  if (album)
    {
    log_debug ("%s: play album '%s#", __PRETTY_FUNCTION__, album);
    char *error_message = NULL;
    int error_code = 0;
    facade_play_album (album, &error_code, &error_message);
    if (error_code == 0)
      {
      api_request_handler_stock_ok (result);
      }
    else
      {
      api_request_handler_stock_error (error_code, error_message, result);
      free (error_message);
      } 
    }
  else
    {
    log_warning ("play_album API call without album argument");
    api_request_handler_stock_error (XINESERVER_X_ERR_ARG, NULL, result);
    *code = 400;
    }
  LOG_OUT
  }

/*============================================================================

 api_request_handler_play_index

============================================================================*/
void api_request_handler_play_index (APIRequestHandler *self,
       const Props *arguments, int *code, char **result)
  {
  LOG_IN
  const char *s_index = props_get (arguments, "index");
  if (s_index)
    {
    int index = atoi (s_index);
    log_debug ("%s: play index %d", __PRETTY_FUNCTION__, index);
    char *error_message = NULL;
    int error_code = 0;
    facade_play_index (index, &error_code, &error_message);
    if (error_code == 0)
      {
      api_request_handler_stock_ok (result);
      }
    else
      {
      api_request_handler_stock_error (error_code, error_message, result);
      free (error_message);
      } 
    }
  else
    {
    log_warning ("play_index API call without index argument");
    api_request_handler_stock_error (XINESERVER_X_ERR_ARG, NULL, result);
    *code = 400;
    }
  LOG_OUT
  }

/*============================================================================

 api_request_handler_scanner_status

============================================================================*/
void api_request_handler_scanner_status (APIRequestHandler *self,
       const Props *arguments, int *code, char **result)
  {
  LOG_IN

  char *error_message = NULL;
  int error_code = 0;
  BOOL running = FALSE;
  int scanned = 0, added = 0, modified = 0, deleted = 0, extracted = 0;
  facade_scanner_status (&error_code, &error_message, &running, &scanned, 
       &added, &modified, &deleted, &extracted);
  if (error_code == 0)
    {
    asprintf (result, "{ \"status\": 0, \"running\": %d, \"scanned\": %d, "
         "\"added\": %d, \"modified\": %d, \"deleted\": %d, \"extracted\": %d }", 
	 running, scanned, added, modified, deleted, extracted);
    }
  else
    {
    api_request_handler_stock_error (error_code, error_message, result);
    free (error_message);
    } 

  LOG_OUT
  }

/*============================================================================

 api_request_handler_shutdown

============================================================================*/
void api_request_handler_shutdown (APIRequestHandler *self,
       const Props *arguments, int *code, char **result)
  {
  LOG_IN
  request_handler_request_shutdown (self->request_handler);
  api_request_handler_stock_ok (result);
  *code = 200;
  LOG_OUT
  }

/*============================================================================

 api_request_handler_stop

============================================================================*/
void api_request_handler_stop (APIRequestHandler *self,
       const Props *arguments, int *code, char **result)
  {
  LOG_IN
  char *error_message = NULL;
  int error_code = 0;
  facade_stop (&error_code, &error_message);
  if (error_code == 0)
    {
    api_request_handler_stock_ok (result);
    }
  else
    {
    api_request_handler_stock_error (error_code, error_message, result);
    free (error_message);
    } 
  LOG_OUT
  }

/*============================================================================

 api_request_handler_clear

============================================================================*/
void api_request_handler_clear (APIRequestHandler *self,
       const Props *arguments, int *code, char **result)
  {
  LOG_IN
  char *error_message = NULL;
  int error_code = 0;
  facade_clear (&error_code, &error_message);
  if (error_code == 0)
    {
    api_request_handler_stock_ok (result);
    }
  else
    {
    api_request_handler_stock_error (error_code, error_message, result);
    free (error_message);
    } 
  LOG_OUT
  }

/*============================================================================

 api_request_handler_pause

============================================================================*/
void api_request_handler_pause (APIRequestHandler *self,
       const Props *arguments, int *code, char **result)
  {
  LOG_IN
  char *error_message = NULL;
  int error_code = 0;
  facade_pause (&error_code, &error_message);
  if (error_code == 0)
    {
    api_request_handler_stock_ok (result);
    }
  else
    {
    api_request_handler_stock_error (error_code, error_message, result);
    free (error_message);
    } 
  LOG_OUT
  }

/*============================================================================

 api_request_handler_next

============================================================================*/
void api_request_handler_next (APIRequestHandler *self,
       const Props *arguments, int *code, char **result)
  {
  LOG_IN
  char *error_message = NULL;
  int error_code = 0;
  facade_next (&error_code, &error_message);
  if (error_code == 0)
    {
    api_request_handler_stock_ok (result);
    }
  else
    {
    api_request_handler_stock_error (error_code, error_message, result);
    free (error_message);
    } 
  LOG_OUT
  }

/*============================================================================

 api_request_handler_full_scan

============================================================================*/
void api_request_handler_full_scan (APIRequestHandler *self,
       const Props *arguments, int *code, char **result)
  {
  LOG_IN
  char *error_message = NULL;
  int error_code = 0;
  facade_full_scan (&error_code, &error_message);
  if (error_code == 0)
    {
    api_request_handler_stock_ok (result);
    }
  else
    {
    api_request_handler_stock_error (error_code, error_message, result);
    free (error_message);
    } 
  LOG_OUT
  }

/*============================================================================

 api_request_handler_quick_scan

============================================================================*/
void api_request_handler_quick_scan (APIRequestHandler *self,
       const Props *arguments, int *code, char **result)
  {
  LOG_IN
  char *error_message = NULL;
  int error_code = 0;
  facade_quick_scan (&error_code, &error_message);
  if (error_code == 0)
    {
    api_request_handler_stock_ok (result);
    }
  else
    {
    api_request_handler_stock_error (error_code, error_message, result);
    free (error_message);
    } 
  LOG_OUT
  }

/*============================================================================

 api_request_handler_prev

============================================================================*/
void api_request_handler_prev (APIRequestHandler *self,
       const Props *arguments, int *code, char **result)
  {
  LOG_IN
  char *error_message = NULL;
  int error_code = 0;
  facade_prev (&error_code, &error_message);
  if (error_code == 0)
    {
    api_request_handler_stock_ok (result);
    }
  else
    {
    api_request_handler_stock_error (error_code, error_message, result);
    free (error_message);
    } 
  LOG_OUT
  }

/*============================================================================

 api_request_handler_play

============================================================================*/
void api_request_handler_play (APIRequestHandler *self,
       const Props *arguments, int *code, char **result)
  {
  LOG_IN
  char *error_message = NULL;
  int error_code = 0;
  facade_play (&error_code, &error_message);
  if (error_code == 0)
    {
    api_request_handler_stock_ok (result);
    }
  else
    {
    api_request_handler_stock_error (error_code, error_message, result);
    free (error_message);
    } 
  LOG_OUT
  }

/*============================================================================

 api_request_handler_play_station

============================================================================*/
void api_request_handler_play_station (APIRequestHandler *self,
       const Props *arguments, int *code, char **result)
  {
  LOG_IN

  const char *list = props_get (arguments, "list");
  const char *name = props_get (arguments, "name");
  
  if (list && name)
    {
    char *error_message = NULL;
    int error_code = 0;
    facade_play_station (list, name, &error_code, &error_message);
    if (error_code == 0)
      {
      api_request_handler_stock_ok (result);
      }
    else
      {
      api_request_handler_stock_error (error_code, error_message, result);
      free (error_message);
      }
    } 
  else
    {
    log_warning ("play_station API call without list and name arguments");
    api_request_handler_stock_error (XINESERVER_X_ERR_ARG, NULL, result);
    *code = 400;
    }

  LOG_OUT
  }

/*============================================================================

 api_request_handler_list_station_lists

============================================================================*/
void api_request_handler_list_station_lists (APIRequestHandler *self,
       const Props *arguments, int *code, char **result)
  {
  LOG_IN

  char *error_message = NULL;
  int error_code = 0;
  List *list = facade_get_radio_station_lists (&error_code, &error_message);
  if (error_code == 0)
    {
    String *response = string_create_empty();
    string_append (response, "{\"status\": 0, \"list\": [");
    int l = list_length (list);
    for (int i = 0; i < l; i++)
      {
      const char *name = list_get (list, i);
      string_append (response, "\"");
      char *escaped_name = htmlutil_escape_dquote_json (name);
      string_append (response, escaped_name);
      free (escaped_name);
      string_append (response, "\"");
      if (i != l - 1)
        string_append (response, ",");
      }
    string_append (response, "]}");
    list_destroy (list);
    *code = 200;
    *result = strdup (string_cstr (response)); 
    string_destroy (response);
    }
  else
    {
    api_request_handler_stock_error (error_code, error_message, result);
    free (error_message);
    }

  LOG_OUT
  }

/*============================================================================

 api_request_handler_list_station_names

============================================================================*/
void api_request_handler_list_station_names (APIRequestHandler *self,
       const Props *arguments, int *code, char **result)
  {
  LOG_IN

  const char *list_name = props_get (arguments, "list");
  
  if (list_name)
    {
    char *error_message = NULL;
    int error_code = 0;
    List *list = facade_get_radio_station_names (list_name, 
	    &error_code, &error_message);
    if (error_code == 0)
      {
      String *response = string_create_empty();
      string_append (response, "{\"status\": 0, \"list\": [");
      int l = list_length (list);
      for (int i = 0; i < l; i++)
	{
	const char *name = list_get (list, i);
	string_append (response, "\"");
	char *escaped_name = htmlutil_escape_dquote_json (name);
	string_append (response, escaped_name);
	free (escaped_name);
	string_append (response, "\"");
	if (i != l - 1)
	  string_append (response, ",");
	}
      string_append (response, "]}");
      list_destroy (list);
      *code = 200;
      *result = strdup (string_cstr (response)); 
      string_destroy (response);
      }
    else
      {
      api_request_handler_stock_error (error_code, error_message, result);
      free (error_message);
      }
    }
  else
    {
    log_warning ("list_station_names API call without list argument");
    api_request_handler_stock_error (XINESERVER_X_ERR_ARG, NULL, result);
    *code = 400;
    }

  LOG_OUT
  }

/*============================================================================

 api_request_handler_list_dirs

============================================================================*/
void api_request_handler_list_dirs (APIRequestHandler *self,
       const Props *arguments, int *code, char **result)
  {
  LOG_IN

  const char *dir = props_get (arguments, "dir");
  if (!dir) dir = "";
  
  char *error_message = NULL;
  int error_code = 0;
  List *list = facade_get_dir_list (dir, &error_code, &error_message);
  if (error_code == 0)
    {
    String *response = string_create_empty();
    string_append (response, "{\"status\": 0, \"list\": [");
    int l = list_length (list);
    for (int i = 0; i < l; i++)
      {
      const char *name = list_get (list, i);
      string_append (response, "\"");
      char *escaped_name = htmlutil_escape_dquote_json (name);
      string_append (response, escaped_name);
      free (escaped_name);
      string_append (response, "\"");
      if (i != l - 1)
        string_append (response, ",");
      }
    string_append (response, "]}");
    list_destroy (list);
    *code = 200;
    *result = strdup (string_cstr (response)); 
    string_destroy (response);
    }
  else
    {
    api_request_handler_stock_error (error_code, error_message, result);
    free (error_message);
    }

  LOG_OUT
  }

/*============================================================================

 api_request_handler_handle 

============================================================================*/
void api_request_handler_handle (APIRequestHandler *self, const char *uri, 
       const Props *arguments, int *code, char **page)
  {
  LOG_IN
  log_debug ("API request: %s", uri);

  int i = 0;
  BOOL done = FALSE;
  ApiFnData *afd = &apiFnData[i];
  while (afd->apiFn && !done) 
   {
   if (strcmp (uri, afd->name) == 0)
     {
     done = TRUE;
     *code = 200;
     afd->apiFn (self, arguments, code, page);
     }
   i++;
   afd = &apiFnData[i];
   }

  if (!done)
    {
    *code = 400;
    api_request_handler_stock_error (XINESERVER_X_ERR_UNKNOWN_FN, NULL, page); 
    }

  LOG_OUT
  }

/*============================================================================

 api_request_handler_list_albums

============================================================================*/
void api_request_handler_list_albums (APIRequestHandler *self,
       const Props *arguments, int *code, char **result)
  {
  LOG_IN

  char *error_message = NULL;
  int error_code = 0;
  SearchConstraints *sc = searchconstraints_create_from_args (arguments);
  int dummy;
  // TODO search constraints
  List *list = facade_get_albums (0, 0, sc, &dummy, 
         &error_code, &error_message);
  searchconstraints_destroy (sc);

  if (list)
    {
    String *response = string_create_empty();
    string_append (response, "{\"status\": 0, \"list\": [");
    int l = list_length (list);
    for (int i = 0; i < l; i++)
      {
      const char *name = list_get (list, i);
      string_append (response, "\"");
      char *escaped_name = htmlutil_escape_dquote_json (name);
      string_append (response, escaped_name);
      free (escaped_name);
      string_append (response, "\"");
      if (i != l - 1)
        string_append (response, ",");
      }
    string_append (response, "]}");
    list_destroy (list);
    *code = 200;
    *result = strdup (string_cstr (response)); 
    string_destroy (response);
    }
  else
    {
    api_request_handler_stock_error (error_code, error_message, result);
    free (error_message);
    }

  LOG_OUT
  }

/*============================================================================

 api_request_handler_add_matching

============================================================================*/
void api_request_handler_add_matching (APIRequestHandler *self,
       const Props *arguments, int *code, char **result)
  {
  LOG_IN

  *code = 0;

  SearchConstraints *sc = searchconstraints_create_from_args (arguments);
  
  if (searchconstraints_has_constraints (sc))
    {
    char *error_message = NULL;
    int error_code = 0;
    facade_add_matching (sc, &error_code, &error_message);
    if (error_code)
      {
      log_warning (error_message);
      api_request_handler_stock_error (error_code, error_message, result); 
      free (error_message);
      *code = 500;
      }
    else
      {
      api_request_handler_stock_ok (result);
      *code = 200;
      }
    }
  else
    {
    log_warning ("add_matching called without search constraints");
    api_request_handler_stock_error (XINESERVER_X_ERR_ARG, 
      "add_matching called without search constraints", result);
    *code = 400;
    }

  searchconstraints_destroy (sc);

  LOG_OUT;
  }

/*============================================================================

 api_request_handler_play_matching

============================================================================*/
void api_request_handler_play_matching (APIRequestHandler *self,
       const Props *arguments, int *code, char **result)
  {
  LOG_IN

  *code = 0;

  SearchConstraints *sc = searchconstraints_create_from_args (arguments);
  
  if (searchconstraints_has_constraints (sc))
    {
    char *error_message = NULL;
    int error_code = 0;
    facade_play_matching (sc, &error_code, &error_message);
    if (error_code)
      {
      log_warning (error_message);
      api_request_handler_stock_error (error_code, error_message, result); 
      free (error_message);
      *code = 500;
      }
    else
      {
      api_request_handler_stock_ok (result);
      *code = 200;
      }
    }
  else
    {
    log_warning ("play_matching called without search constraints");
    api_request_handler_stock_error (XINESERVER_X_ERR_ARG, 
      "play_matching called without search constraints", result);
    *code = 400;
    }

  searchconstraints_destroy (sc);

  LOG_OUT;
  }






