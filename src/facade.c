/*============================================================================

  xine-server-x 
  facade.c
  Copyright (c)2020 Kevin Boone
  Distributed under the terms of the GPL v3.0

============================================================================*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <ctype.h>
#include "defs.h" 
#include "log.h" 
#include "facade.h" 
#include "list.h" 
#include "path.h" 
#include "scanner.h" 
#include "xine-server-api.h" 
#include "xine-server-x-api.h" 
#include "searchconstraints.h" 
#include "database.h" 

struct _Facade
  {
  Path *root;   // Media file root
  char *xshost; // Host of xine-server
  int xsport;   // Port of xine-server
  char *gxsradio_dir; // Directory for radio station files
  char *index_file; // May be NULL
  }; 

static Facade *facade_instance = NULL;

/*============================================================================

  facade_create

============================================================================*/
void facade_create (const char *root, const char *xshost, int xsport,
    const char *gxsradio_dir, const char *index_file)
  {
  LOG_IN
  Facade *self = malloc (sizeof (Facade)); 
  facade_instance = self; 
  self->xshost = strdup (xshost);
  self->gxsradio_dir = strdup (gxsradio_dir);
  self->xsport = xsport;
  self->root = path_create (root);
  if (index_file)
    self->index_file = strdup (index_file);
  else
    self->index_file = NULL; 
  LOG_OUT 
  }

/*============================================================================

  facade_get_instance 

============================================================================*/
Facade *facade_get_instance (void)
  {
  LOG_IN
  LOG_OUT 
  return facade_instance;
  }

/*============================================================================

  facade_destroy

============================================================================*/
void facade_destroy (void)
  {
  LOG_IN
  Facade *self = facade_get_instance();
  if (self)
    {
    if (self->root) path_destroy (self->root);
    if (self->xshost) free (self->xshost);
    if (self->gxsradio_dir) free (self->gxsradio_dir);
    if (self->index_file) free (self->index_file);
    free (self);
    }
  LOG_OUT 
  }

/*============================================================================

  facade_is_playable

============================================================================*/
BOOL facade_path_is_playable (const Path *p2)
  {
  LOG_IN
  BOOL ret = FALSE;

  char *ext = (char *)path_get_extension_utf8 (p2);

  if (ext)
    {
    ret = xineserver_is_playable_ext (ext);
    free (ext);
    }

  LOG_OUT
  return ret;
  }


/*============================================================================

  facade_make_os_path_from_media_path

============================================================================*/
static char *facade_make_os_path_from_media_path (const char *path)
  {
  LOG_IN
  Facade *self = facade_get_instance();
  Path *ospath = path_clone (self->root);
  path_append (ospath, path);
  char *ret = (char *)path_to_utf8 (ospath);
  path_destroy (ospath);
  LOG_OUT
  return ret;
  }

/*============================================================================

  facade_get_file_list

============================================================================*/
static int facade_alpha_sort_fn (const void *p1, 
    const void*p2, void *user_data)
  {
  char *s1 = * (char *const *)p1;
  char *s2 = * (char *const *)p2;
  return strcmp (s1, s2);
  }

List *facade_get_file_list (const char *path, int *error_code, char **error)
  {
  LOG_IN
  List *ret = NULL;
  
  Facade *self = facade_instance;

  Path *p = path_clone (self->root);	
  path_append (p, path);
  char *s_path = (char *)path_to_utf8 (p);

  log_debug ("%s: Listing files in %s", __PRETTY_FUNCTION__, s_path); 

  DIR *d = opendir (s_path);
  if (d)
    {
    ret = list_create (free);
    struct dirent *de = readdir (d); 
    while (de)
      {
      const char *name = de->d_name;
      if (name[0] != '.')
	{
	Path *p2 = path_clone (p);
	path_append (p2, name);
	if (path_is_regular (p2))
	  {
	  if (facade_path_is_playable (p2))
	    {
	    list_append (ret, strdup (name));
	    }
	  } 
	path_destroy (p2); 
        }
      de = readdir (d);
      }
    closedir (d);
    list_sort (ret, facade_alpha_sort_fn, NULL);
    }
  else
    {
    asprintf (error, "Can't list directory %s: %s", s_path, strerror (errno));
    }

  free (s_path);
  path_destroy (p);

  LOG_OUT
  return ret; 
  }


/*============================================================================

  facade_get_dir_list

============================================================================*/
List *facade_get_dir_list (const char *path, int *error_code, char **error)
  {
  LOG_IN
  List *ret = NULL;
  
  Facade *self = facade_instance;

  Path *p = path_clone (self->root);	
  path_append (p, path);
  char *s_path = (char *)path_to_utf8 (p);

  log_debug ("%s: Listing dirs in %s", __PRETTY_FUNCTION__, s_path); 

  DIR *d = opendir (s_path);
  if (d)
    {
    ret = list_create (free);
    struct dirent *de = readdir (d); 
    while (de)
      {
      const char *name = de->d_name;
      if (name[0] != '.')
	{
	Path *p2 = path_clone (p);
	path_append (p2, name);
	if (path_is_directory (p2))
	  {
	  list_append (ret, strdup (name));
	  } 
	path_destroy (p2); 
        }
      de = readdir (d);
      }
    closedir (d);
    list_sort (ret, facade_alpha_sort_fn, NULL);
    }
  else
    {
    *error_code = XINESERVER_X_ERR_LIST_DIR;
    asprintf (error, "Can't list directory %s: %s", s_path, strerror (errno));
    }

  free (s_path);
  path_destroy (p);

  LOG_OUT
  return ret; 
  }

/*============================================================================

  facade_get_first_track_for_album

============================================================================*/
char *facade_get_first_track_for_album (const char *album, int *error_code, 
        char **error_message)
  {
  LOG_IN
  char *ret = NULL; 

  log_debug ("%d: album=%s", __PRETTY_FUNCTION__, album);

  Facade *self = facade_get_instance();

  if (self->index_file)
    {
    Database *db = database_create (self->index_file);
    
    if (database_open (db, error_message))
      {
      ret = database_get_first_track_for_album (db, album, error_message);
      database_close (db);
      } 
    else
      {
      *error_code = XINESERVER_X_ERR_GEN_DATABASE;
      }

    database_destroy (db);
    }
  else
    {
    *error_message = strdup (xineserver_x_perror (XINESERVER_X_ERR_NO_INDEX));
    *error_code = XINESERVER_X_ERR_NO_INDEX;
    }

  LOG_OUT
  return ret;
  }

/*============================================================================

  facade_get_cover_image_for_file

============================================================================*/
char *facade_get_cover_image_for_file (const char *_path)
  {
  LOG_IN
  char *ret = NULL;
  char *path = strdup (_path);
  char *p = strchr (path, PATH_SEPARATOR);
  if (p)
    {
    *p = 0;
    ret = facade_get_cover_image_for_dir (path);
    }
  else
    {
    ret = facade_get_cover_image_for_dir ("");
    }
  free (path);
  LOG_OUT
  return ret;
  }
   
/*============================================================================

  facade_get_cover_image_for_album

============================================================================*/
char *facade_get_cover_image_for_album (const char *album)
  {
  LOG_IN
  char *ret = NULL; 
  char *error;
  int error_code;

  log_debug ("%d: album=%s", __PRETTY_FUNCTION__, album);

  Facade *self = facade_get_instance();

  if (self->index_file)
    {
    char *path = facade_get_first_track_for_album (album, &error_code, &error);
    if (path)
      {
      ret = facade_get_cover_image_for_file (path);
      free (path);
      }
    else
      {
      log_warning (error);
      }
    }
  else
    {
    log_warning (xineserver_x_perror (XINESERVER_X_ERR_NO_INDEX));
    }

  LOG_OUT
  return ret;
  }

/*============================================================================

  facade_get_cover_image_for_dir

============================================================================*/
char *facade_get_cover_image_for_dir (const char *path)
  {
  LOG_IN
  char *ret = NULL;
  char *fspath = facade_make_os_path_from_media_path (path);

  DIR *d = opendir (fspath);
  if (d)
    {
    struct dirent *de = readdir (d);
    while (de && !ret)
      {
      const char *name = de->d_name;
      char *lcname = strdup (name);
      char *_lcname = lcname;
      while (*_lcname)
        {
        *_lcname = tolower (*_lcname);
	_lcname++;
	}
      
      BOOL iscover = FALSE;
      if (strcmp (lcname, "folder.jpg") == 0)
        iscover = TRUE;
      else if (strcmp (lcname, "folder.png") == 0)
        iscover = TRUE;
      else if (strcmp (lcname, "cover.jpg") == 0)
        iscover = TRUE;
      else if (strcmp (lcname, "cover.png") == 0)
        iscover = TRUE;
      else if (strcmp (lcname, ".folder.png") == 0)
        iscover = TRUE;

      if (iscover)
        {
        asprintf (&ret, EXT_FILE_BASE "%s/%s", path, name); 
	}

      de = readdir (d);
      free (lcname);
      }

    closedir (d);
    }

  free (fspath);
  LOG_OUT
  return ret;
  }

/*============================================================================

  facade_play_stream

============================================================================*/
void facade_play_stream (const char *stream, int *error_code, 
      char **error_message)
  {
  LOG_IN
  log_debug ("%s: %s", __PRETTY_FUNCTION__, stream);
  Facade *self = facade_instance;

  BOOL ok = TRUE;

  if (ok)
    ok = xineserver_clear (self->xshost, self->xsport, 
           error_code, error_message);

  if (ok)
    ok = xineserver_add_single (self->xshost, self->xsport, 
           stream, error_code, error_message);

  if (ok)
    ok = xineserver_play (self->xshost, self->xsport, 
           0, error_code, error_message);

  if (ok)
    {
    *error_message = NULL;
    *error_code = 0;
    }

  LOG_OUT
  }

/*============================================================================

  facade_play_file

============================================================================*/
void facade_play_file (const char *file, int *error_code, 
      char **error_message)
  {
  LOG_IN
  log_debug ("%s: %s", __PRETTY_FUNCTION__, file);
  char *ospath = facade_make_os_path_from_media_path (file);
  facade_play_stream (ospath, error_code, error_message);
  free (ospath);
  LOG_OUT
  }

/*============================================================================

  facade_add_file

============================================================================*/
void facade_add_file (const char *file, int *error_code, 
      char **error_message)
  {
  LOG_IN
  log_debug ("%s: %s", __PRETTY_FUNCTION__, file);
  Facade *self = facade_get_instance();
  char *ospath = facade_make_os_path_from_media_path (file);
  xineserver_add_single (self->xshost, self->xsport, 
	ospath, error_code, error_message);
  free (ospath);
  LOG_OUT
  }

/*============================================================================

  facade_play_file

============================================================================*/
void facade_play_album (const char *album, int *error_code, 
                char **error)
  {
  LOG_IN
  log_debug ("%s: album=%s", __PRETTY_FUNCTION__, album);

  Facade *self = facade_get_instance();

  if (self->index_file)
    {
    Database *db = database_create (self->index_file);
    
    if (database_open (db, error))
      {
      List *list = database_get_paths_by_album (db, album, error);
      if (list)
        {
        int l = list_length (list);
        char **streams = malloc (l * sizeof (char **));

        for (int i = 0; i < l; i++)
          {
          Path *path = path_clone (self->root);
          char *file = list_get (list, i);
          path_append (path, file);
          streams[i] = (char *)path_to_utf8 (path); 
          path_destroy (path);
          }

        BOOL ok = xineserver_clear (self->xshost, self->xsport, 
              error_code, error);
        if (ok)
          {
          ok = xineserver_add (self->xshost, self->xsport, 
             l, (const char *const *)streams, error_code, error);
	  }
      
        if (ok)
          {
          ok = xineserver_play (self->xshost, self->xsport, 
             0, error_code, error);
	  }

        if (ok) *error_code = 0;

        for (int i = 0; i < l; i++) free (streams[i]);
        free (streams);
        list_destroy (list);
        }
      else
        {
        *error_code = XINESERVER_X_ERR_GEN_DATABASE; 
        }

      database_close (db);
      } 
    else
      {
      // Nothing to do -- error already set
      }

    database_destroy (db);
    }
  else
    {
    *error = strdup (xineserver_x_perror (XINESERVER_X_ERR_NO_INDEX));
    *error_code = XINESERVER_X_ERR_NO_INDEX; 
    }

  LOG_OUT
  }


/*============================================================================

  facade_play_dir

============================================================================*/
void facade_play_dir (const char *dir, int *error_code, 
      char **error_message)
  {
  LOG_IN
  log_debug ("%s: %s", __PRETTY_FUNCTION__, dir);
  Facade *self = facade_instance;

  char *ospath = facade_make_os_path_from_media_path (dir);
  //facade_play_stream (ospath, error_code, error_message);

  log_debug ("play_dir, OS dir=%s", ospath);

  List *files = NULL;
  if (file_expand_directory (ospath, FE_FILES | FE_PREPEND_FULL_PATH, 
          &files))
    {
    list_sort (files, string_alpha_sort_fn, NULL);

    // First scan the list for files that can actually be played
    int total = 0;
    int l = list_length (files);
    for (int i = 0; i < l; i++)
      {
      String *file = list_get (files, i);
      const char *s_file = string_cstr (file);
      char *p = strrchr (s_file, '.');
      if (p)
        {
        if (xineserver_is_playable_ext (p + 1))
          total++; 
	}
      }
    log_debug ("playable files=%d", total);

    if (total)
      {
      char **streams = malloc (total * sizeof (char **));

      for (int i = 0; i < total; i++)
        {
        String *file = list_get (files, i);
        const char *s_file = string_cstr (file);
        streams[i] = strdup (s_file);
        }

      BOOL ok = xineserver_clear (self->xshost, self->xsport, 
           error_code, error_message);
      if (ok)
        {
        ok = xineserver_add (self->xshost, self->xsport, 
           total, (const char *const *)streams, error_code, error_message);
	}
      
      if (ok)
        {
        ok = xineserver_play (self->xshost, self->xsport, 
           0, error_code, error_message);
	}

      for (int i = 0; i < total; i++) free (streams[i]);
      free (streams);

      if (ok)
        *error_code = 0;
      }
    else
      {
      *error_code = XINESERVER_X_ERR_NO_FILES;
      if (error_message)
        asprintf (error_message, "No playable audio files in directory %s", 
	  dir);
      }
    }
  else
    {
    *error_code = XINESERVER_X_ERR_LIST_DIR;
    if (error_message)
      asprintf (error_message, "Can't list directory %s", dir);
    }

  if (files) list_destroy (files);
  free (ospath);

  LOG_OUT
  }

/*============================================================================

  facade_add_dir

============================================================================*/
void facade_add_dir (const char *dir, int *error_code, 
      char **error_message)
  {
  LOG_IN
  log_debug ("%s: %s", __PRETTY_FUNCTION__, dir);
  Facade *self = facade_instance;

  char *ospath = facade_make_os_path_from_media_path (dir);

  log_debug ("add_dir, OS dir=%s", ospath);

  List *files = NULL;
  if (file_expand_directory (ospath, FE_FILES | FE_PREPEND_FULL_PATH, 
          &files))
    {
    list_sort (files, string_alpha_sort_fn, NULL);

    // First scan the list for files that can actually be played
    int total = 0;
    int l = list_length (files);
    for (int i = 0; i < l; i++)
      {
      String *file = list_get (files, i);
      const char *s_file = string_cstr (file);
      char *p = strrchr (s_file, '.');
      if (p)
        {
        if (xineserver_is_playable_ext (p + 1))
          total++; 
	}
      }
    log_debug ("playable files=%d", total);

    if (total)
      {
      char **streams = malloc (total * sizeof (char **));

      for (int i = 0; i < total; i++)
        {
        String *file = list_get (files, i);
        const char *s_file = string_cstr (file);
        streams[i] = strdup (s_file);
        }

      BOOL ok = xineserver_add (self->xshost, self->xsport, 
             total, (const char *const *)streams, error_code, error_message);

      for (int i = 0; i < total; i++) free (streams[i]);
      free (streams);

      if (ok)
        *error_code = 0;
      }
    else
      {
      *error_code = XINESERVER_X_ERR_NO_FILES;
      if (error_message)
        asprintf (error_message, "No playable audio files in directory %s", 
	  dir);
      }
    }
  else
    {
    *error_code = XINESERVER_X_ERR_LIST_DIR;
    if (error_message)
      asprintf (error_message, "Can't list directory %s", dir);
    }

  if (files) list_destroy (files);
  free (ospath);

  LOG_OUT
  }

/*============================================================================

  facade_clear

============================================================================*/
void facade_clear (int *error_code, char **error_message)
  {
  LOG_IN

  log_debug ("%s", __PRETTY_FUNCTION__);
  Facade *self = facade_instance;

  BOOL ok = TRUE;

  if (ok)
    ok = xineserver_clear (self->xshost, self->xsport, 
           error_code, error_message);

  if (ok)
    {
    *error_message = NULL;
    *error_code = 0;
    }

  LOG_OUT
  }


/*============================================================================

  facade_stop

============================================================================*/
void facade_stop (int *error_code, char **error_message)
  {
  LOG_IN

  log_debug ("%s", __PRETTY_FUNCTION__);
  Facade *self = facade_instance;

  BOOL ok = TRUE;

  if (ok)
    ok = xineserver_stop (self->xshost, self->xsport, 
           error_code, error_message);

  if (ok)
    {
    *error_message = NULL;
    *error_code = 0;
    }

  LOG_OUT
  }


/*============================================================================

  facade_get_playback_status

============================================================================*/
void facade_get_playback_status (int *error_code, char **error_message, 
     PlaybackStatus **status)
  {
  LOG_IN

  log_debug ("%s", __PRETTY_FUNCTION__);
  Facade *self = facade_instance;

  XSStatus *xsstatus = NULL;

  if (xineserver_status (self->xshost, self->xsport, 
           &xsstatus, error_code, error_message))
    {
    XSMetaInfo *xsmi = NULL;

    if (xineserver_meta_info (self->xshost, self->xsport, &xsmi, 
      error_code, error_message))
      {
      PlaybackStatus *s = playback_status_create();
      // Beware the implicit (non-)converion of xine-server transport
      //   codes to XSX transport codes in the next line
      XSXTransportStatus ts = xsstatus_get_transport_status (xsstatus);

      playback_status_set_ts (s, ts);
      playback_status_set_len (s, xsstatus_get_length (xsstatus) / 1000);
      playback_status_set_pos (s, xsstatus_get_position (xsstatus) / 1000);
      playback_status_set_playlist_index 
         (s, xsstatus_get_playlist_index (xsstatus));
      playback_status_set_playlist_length
         (s, xsstatus_get_playlist_length (xsstatus));
  
      playback_status_set_bitrate (s, xsmetainfo_get_bitrate (xsmi));
      playback_status_set_composer (s, xsmetainfo_get_composer (xsmi));
      playback_status_set_album (s, xsmetainfo_get_album (xsmi));
      playback_status_set_genre (s, xsmetainfo_get_genre (xsmi));
      playback_status_set_artist (s, xsmetainfo_get_artist (xsmi));
      playback_status_set_title (s, xsmetainfo_get_title (xsmi));
      playback_status_set_seekable (s, xsmetainfo_is_seekable (xsmi));

      xsmetainfo_destroy (xsmi);
      *status = s;
      *error_code = 0;
      *error_message = NULL;
      }
    else
      {
      // Do nothing -- error already set
      }
    xsstatus_destroy (xsstatus);
    }
  else
    {
    // Do nothing -- error already set
    }

  LOG_OUT
  }

/*============================================================================

  facadee_resume

============================================================================*/
void facade_resume (int *error_code, char **error_message)
  {
  LOG_IN

  log_debug ("%s", __PRETTY_FUNCTION__);
  Facade *self = facade_instance;

  BOOL ok = TRUE;

  if (ok)
    ok = xineserver_resume (self->xshost, self->xsport, 
           error_code, error_message);

  if (ok)
    {
    *error_message = NULL;
    *error_code = 0;
    }

  LOG_OUT
  }

/*============================================================================

  facade_pause

============================================================================*/
void facade_pause (int *error_code, char **error_message)
  {
  LOG_IN

  log_debug ("%s", __PRETTY_FUNCTION__);
  Facade *self = facade_instance;

  BOOL ok = TRUE;

  if (ok)
    ok = xineserver_pause (self->xshost, self->xsport, 
           error_code, error_message);

  if (ok)
    {
    *error_message = NULL;
    *error_code = 0;
    }

  LOG_OUT
  }

/*============================================================================

  facade_next

============================================================================*/
void facade_next (int *error_code, char **error_message)
  {
  LOG_IN

  log_debug ("%s", __PRETTY_FUNCTION__);
  Facade *self = facade_instance;

  BOOL ok = TRUE;

  if (ok)
    ok = xineserver_next (self->xshost, self->xsport, 
           error_code, error_message);

  if (ok)
    {
    *error_message = NULL;
    *error_code = 0;
    }

  LOG_OUT
  }

/*============================================================================

  facade_prev

============================================================================*/
void facade_prev (int *error_code, char **error_message)
  {
  LOG_IN

  log_debug ("%s", __PRETTY_FUNCTION__);
  Facade *self = facade_instance;

  BOOL ok = TRUE;

  if (ok)
    ok = xineserver_prev (self->xshost, self->xsport, 
           error_code, error_message);

  if (ok)
    {
    *error_message = NULL;
    *error_code = 0;
    }

  LOG_OUT
  }

/*============================================================================

  facade_play_index

============================================================================*/
void facade_play_index (int index, int *error_code, char **error_message)
  {
  LOG_IN

  log_debug ("%s", __PRETTY_FUNCTION__);
  Facade *self = facade_instance;

  BOOL ok = TRUE;

  if (ok)
    ok = xineserver_play (self->xshost, self->xsport, index,
           error_code, error_message);

  if (ok)
    {
    *error_message = NULL;
    *error_code = 0;
    }

  LOG_OUT
  }

/*============================================================================

  facade_play

============================================================================*/
void facade_play (int *error_code, char **error_message)
  {
  LOG_IN

  log_debug ("%s", __PRETTY_FUNCTION__);
  Facade *self = facade_instance;

  BOOL ok = TRUE;

  XSStatus *xsstatus = NULL;
  if (ok)
    {
    ok = xineserver_status (self->xshost, self->xsport, 
           &xsstatus, error_code, error_message);
    }

  if (ok)
    {
    XSXTransportStatus ts = xsstatus_get_transport_status (xsstatus);
    xsstatus_destroy (xsstatus);

    if (ts == XINESERVER_X_TRANSPORT_PAUSED)
       {
       // resume
       facade_resume (error_code, error_message);
       } 
    else if (ts == XINESERVER_X_TRANSPORT_STOPPED)
       {
       // play index 
       facade_play_index (0, error_code, error_message);
       }
    else
       {
       // nothing to do
       *error_code = 0;
       *error_message = NULL;
       }
    }

  LOG_OUT
  }

/*============================================================================

  facade_get_playlist

============================================================================*/
List *facade_get_playlist (int *error_code, char **error_message)
  {
  LOG_IN

  List *ret = NULL;

  log_debug ("%s", __PRETTY_FUNCTION__);
  Facade *self = facade_instance;

  BOOL ok = TRUE;
  XSPlaylist *playlist = NULL;

  ok = xineserver_playlist (self->xshost, self->xsport, &playlist,
           error_code, error_message);

  if (ok)
    {
    char *root_s = (char *)path_to_utf8 (self->root);
    int ls = strlen (root_s);
    int n = xsplaylist_get_nentries (playlist);
    char **const entries = xsplaylist_get_entries (playlist);
    ret = list_create (free);
    for (int i = 0; i < n; i++)
      {
      if (strstr (entries[i], root_s) == entries[i])
        list_append (ret, strdup (entries[i] + ls));
      else
        list_append (ret, strdup (entries[i]));
      }

    xsplaylist_destroy (playlist);
    *error_message = NULL;
    *error_code = 0;
    free (root_s);
    }

  LOG_OUT;
  return ret;
  }

/*============================================================================

  facade_set_volume

============================================================================*/
void facade_set_volume (int volume, int *error_code, 
               char **error_message)
  {
  LOG_IN

  log_debug ("%s", __PRETTY_FUNCTION__);
  Facade *self = facade_instance;

  BOOL ok = TRUE;

  if (ok)
    ok = xineserver_set_volume (self->xshost, self->xsport, volume,
           error_code, error_message);

  if (ok)
    {
    *error_message = NULL;
    *error_code = 0;
    }

  LOG_OUT
  }

/*============================================================================

  facade_shut_down_xine_server

============================================================================*/
void facade_shut_down_xine_server (void)
  {
  LOG_IN

  log_debug ("%s", __PRETTY_FUNCTION__);
  Facade *self = facade_instance;

  BOOL ok = TRUE;

  int error_code;
  char *error_message = NULL;
  if (ok)
    ok = xineserver_shutdown (self->xshost, self->xsport, 
           &error_code, &error_message);

  if (error_message)
    {
    log_warning (error_message);
    free (error_message);
    }

  LOG_OUT
  }


/*============================================================================

  facade_get_radio_station_files

============================================================================*/
List *facade_get_radio_station_lists (int *error_code, char **error)
  {
  LOG_IN

  List *ret = NULL;

  log_debug ("%s", __PRETTY_FUNCTION__);
  Facade *self = facade_instance;

  DIR *d = opendir (self->gxsradio_dir);
  if (d)
    {
    ret = list_create (free);
    struct dirent *de = readdir (d); 
    while (de)
      {
      const char *name = de->d_name;
      if (name[0] != '.')
	{
        const char *pos = strstr (name, ".gxsradio");
        if (pos)
          {
          int p = pos - name;
          char *name2 = strdup (name);
          name2[p] = 0;
          list_append (ret, name2);
          }
        }
      de = readdir (d);
      }
    closedir (d);
    list_sort (ret, facade_alpha_sort_fn, NULL);
    *error_code = 0;
    }
  else
    {
    asprintf (error, "Can't list directory %s: %s", self->gxsradio_dir,  
       strerror (errno));
    *error_code =XINESERVER_X_ERR_LIST_DIR; 
    }

  return ret;

  LOG_OUT
  }

/*============================================================================

  facade_get_radio_station_names

============================================================================*/
List *facade_get_radio_station_names (const char *station_list_name, int 
        *error_code, char **error)
  {
  LOG_IN
  List *ret = NULL;

  Facade *self = facade_instance;

  char *file;
  asprintf (&file, "%s%c%s.gxsradio", self->gxsradio_dir, PATH_SEPARATOR,
      station_list_name); 

  FILE *f = fopen (file, "r");
  if (f)
    {
    ret = list_create (free);
    int n = 0;
    do
      {
      char *line = NULL;
      size_t zero = 0;
      n = getline (&line, &zero, f); 
      if (n >= 2)
         {
         char *p = strchr (line, '\t');
         if (p)
           {
           *p = 0;
           list_append (ret, strdup (line));
           }
         }
      if (line) free (line);
      } while (n >= 2);
    fclose (f);
    }
  else
    {
    asprintf (error, "Can't open stations list file %s: %s", file,  
       strerror (errno));
    *error_code  = XINESERVER_X_ERR_OPEN_STATION_LIST; 
    }

  free (file);
  LOG_OUT
  return ret;
  }

/*============================================================================

  facade_play_station

============================================================================*/
void facade_play_station (const char *list, const char *name, 
        int *error_code, char **error_message)
  {
  LOG_IN

  char *uri = NULL;
  Facade *self = facade_instance;

  char *file;
  asprintf (&file, "%s%c%s.gxsradio", self->gxsradio_dir, PATH_SEPARATOR,
      list); 

  FILE *f = fopen (file, "r");
  if (f)
    {
    int n = 0;
    do
      {
      char *line = NULL;
      size_t zero = 0;
      n = getline (&line, &zero, f); 
      if (n >= 2)
         {
         char *p = strchr (line, '\t');
         if (p)
           {
           *p = 0;
           if (strcmp (name, line) == 0)
             {
             strtok (p + 1, "\t");
             uri = strdup (strtok (NULL, "\t"));
             }
           }
         }
      if (line) free (line);
      } while (n >= 2);
    fclose (f);

    if (uri)
      {
      facade_play_stream (uri, error_code, error_message);
      free (uri);
      }
    else
      {
      asprintf (error_message, "Can't find station %s in list %s", 
        name, list);  
      *error_code  = XINESERVER_X_ERR_FIND_STATION; 
      }
    }
  else
    {
    asprintf (error_message, "Can't open stations list file %s: %s", file,  
       strerror (errno));
    *error_code  = XINESERVER_X_ERR_OPEN_STATION_LIST; 
    }

  free (file);

  LOG_OUT
  }

/*============================================================================

  facade_scanner_status

============================================================================*/
void facade_scanner_status (int *error_code, char **error_message, 
    BOOL *running, BOOL *scanned, BOOL *added, BOOL *modified, BOOL *deleted,
    BOOL *extracted)
  {
  LOG_IN

  *running = FALSE; *scanned = 0; *added = 0; *modified = 0; *deleted = 0;
  *error_code = 0, *extracted = 0;

  FILE *f = fopen (SCANNER_STATUS_FILE, "r");
  if (f)
    {
    int n = fscanf (f, "%d %d %d %d %d", scanned, added, modified, 
              deleted, extracted);
    if (n == 5)
      {
      *running = TRUE; 
      }
    else
      {
      *error_code = XINESERVER_X_ERR_SCANNER_FILE; 
      if (error_message) *error_message = 
           strdup ("Scanner status file did not contain five numerica values");
      }
    fclose (f);
    }
  else
    {
    // NOthing to do -- we already said it isn't running
    }

  LOG_OUT
  }

/*============================================================================

  facade_quick_scan

============================================================================*/
void facade_quick_scan (int *error_code, char **error_message)
  {
  LOG_IN

  Facade *self = facade_get_instance();
  if (self->index_file)
    {
    char argv0[256];
    int n = readlink ("/proc/self/exe", argv0, sizeof (argv0));
    argv0[n] = 0;
    char *s_root = (char *)path_to_utf8 (self->root);

    if (fork() == 0)
      {
      // Child
      execlp (argv0, argv0, "--root", s_root, "--index", self->index_file,
        "-q", NULL);
      }

    *error_code = 0;
    free (s_root);
    }
  else
    {
    *error_code = XINESERVER_X_ERR_NO_INDEX;
    *error_message = 
         strdup (xineserver_x_perror (*error_code));
    }

  LOG_OUT
  }

/*============================================================================

  facade_full_scan

============================================================================*/
void facade_full_scan (int *error_code, char **error_message)
  {
  LOG_IN

  Facade *self = facade_get_instance();
  if (self->index_file)
    {
    char argv0[256];
    int n = readlink ("/proc/self/exe", argv0, sizeof (argv0));
    argv0[n] = 0;
    char *s_root = (char *)path_to_utf8 (self->root);

    if (fork() == 0)
      {
      // Child
      int ret = execlp (argv0, argv0, "--root", s_root, 
           "--index", self->index_file, "-s", NULL);
      printf ("ret =%d\n", ret);
      }

    *error_code = 0;
    free (s_root);
    }
  else
    {
    *error_code = XINESERVER_X_ERR_NO_INDEX;
    *error_message = 
         strdup (xineserver_x_perror (*error_code));
    }
  LOG_OUT
  }

/*============================================================================

  facade_get_albums

============================================================================*/
List *facade_get_albums (int from, int limit, const SearchConstraints *sc, 
        int *match, int *error_code, char **error_message)
  {
  LOG_IN
  List *ret = NULL; 

  log_debug ("%s: from=%d limit=%d", __PRETTY_FUNCTION__, from, limit);

  Facade *self = facade_get_instance();

  if (self->index_file)
    {
    Database *db = database_create (self->index_file);
    
    if (database_open (db, error_message))
      {
      ret = database_get_albums (db, from, limit, sc, match, error_message); 
      database_close (db);
      } 
    else
      {
      *error_code = XINESERVER_X_ERR_GEN_DATABASE;
      }

    database_destroy (db);
    }
  else
    {
    *error_message = strdup (xineserver_x_perror (XINESERVER_X_ERR_NO_INDEX));
    *error_code = XINESERVER_X_ERR_NO_INDEX;
    }

  LOG_OUT
  return ret;
  }

/*============================================================================

  facade_get_genres

============================================================================*/
List *facade_get_genres (int from, int limit, const SearchConstraints *sc, 
        int *match, int *error_code, char **error_message)
  {
  LOG_IN
  List *ret = NULL; 

  log_debug ("%s: from=%d limit=%d", __PRETTY_FUNCTION__, from, limit);

  Facade *self = facade_get_instance();

  if (self->index_file)
    {
    Database *db = database_create (self->index_file);
    
    if (database_open (db, error_message))
      {
      ret = database_get_genres (db, from, limit, sc, match, error_message); 
      database_close (db);
      } 
    else
      {
      *error_code = XINESERVER_X_ERR_GEN_DATABASE;
      }

    database_destroy (db);
    }
  else
    {
    *error_message = strdup (xineserver_x_perror (XINESERVER_X_ERR_NO_INDEX));
    *error_code = XINESERVER_X_ERR_NO_INDEX;
    }

  LOG_OUT
  return ret;
  }

/*============================================================================

  facade_get_composers

============================================================================*/
List *facade_get_composers (int from, int limit, const SearchConstraints *sc, 
        int *match, int *error_code, char **error_message)
  {
  LOG_IN
  List *ret = NULL; 

  log_debug ("%s: from=%d limit=%d", __PRETTY_FUNCTION__, from, limit);

  Facade *self = facade_get_instance();

  if (self->index_file)
    {
    Database *db = database_create (self->index_file);
    
    if (database_open (db, error_message))
      {
      ret = database_get_composers (db, from, limit, sc, match, 
        error_message); 
      database_close (db);
      } 
    else
      {
      *error_code = XINESERVER_X_ERR_GEN_DATABASE;
      }

    database_destroy (db);
    }
  else
    {
    *error_message = strdup (xineserver_x_perror (XINESERVER_X_ERR_NO_INDEX));
    *error_code = XINESERVER_X_ERR_NO_INDEX;
    }

  LOG_OUT
  return ret;
  }

/*============================================================================

  facade_get_artists

============================================================================*/
List *facade_get_artists (int from, int limit, const SearchConstraints *sc, 
        int *match, int *error_code, char **error_message)
  {
  LOG_IN
  List *ret = NULL; 

  log_debug ("%s: from=%d limit=%d", __PRETTY_FUNCTION__, from, limit);

  Facade *self = facade_get_instance();

  if (self->index_file)
    {
    Database *db = database_create (self->index_file);
    
    if (database_open (db, error_message))
      {
      ret = database_get_artists (db, from, limit, sc, match, error_message); 
      database_close (db);
      } 
    else
      {
      *error_code = XINESERVER_X_ERR_GEN_DATABASE;
      }

    database_destroy (db);
    }
  else
    {
    *error_message = strdup (xineserver_x_perror (XINESERVER_X_ERR_NO_INDEX));
    *error_code = XINESERVER_X_ERR_NO_INDEX;
    }

  LOG_OUT
  return ret;
  }

/*============================================================================

  facade_get_paths

============================================================================*/
List *facade_get_paths (int from, int limit, const SearchConstraints *sc, 
        int *match, int *error_code, char **error_message)
  {
  LOG_IN
  List *ret = NULL;

  log_debug ("%s: from=%d limit=%d", __PRETTY_FUNCTION__, from, limit);

  Facade *self = facade_get_instance();

  if (self->index_file)
    {
    Database *db = database_create (self->index_file);
    
    if (database_open (db, error_message))
      {
      ret = database_get_paths (db, from, limit, sc, match, error_message); 
      database_close (db);
      } 
    else
      {
      *error_code = XINESERVER_X_ERR_GEN_DATABASE;
      }

    database_destroy (db);
    }
  else
    {
    *error_message = strdup (xineserver_x_perror (XINESERVER_X_ERR_NO_INDEX));
    *error_code = XINESERVER_X_ERR_NO_INDEX;
    }

  LOG_OUT
  return ret;
  }


/*============================================================================

  facade_get_metainfo_from_database

============================================================================*/
AudioMetaInfo *facade_get_metainfo_from_database (const char *path, 
        int *error_code, char **error_message)
  {
  LOG_IN
  AudioMetaInfo *ret = NULL;

  log_debug ("%s: path=%s", __PRETTY_FUNCTION__, path); 

  Facade *self = facade_get_instance();

  if (self->index_file)
    {
    Database *db = database_create (self->index_file);
    
    if (database_open (db, error_message))
      {
      ret = audio_metainfo_create();
      
      if (audio_metainfo_get_from_database (ret, db, path, error_message))
        {
        // nothing else to do -- fields all set
        }
      else
        {
        audio_metainfo_destroy (ret);
        ret = NULL;
        *error_code = XINESERVER_X_ERR_GEN_DATABASE;
        }

      database_close (db);
      } 
    else
      {
      *error_code = XINESERVER_X_ERR_GEN_DATABASE;
      }

    database_destroy (db);
    }
  else
    {
    *error_message = strdup (xineserver_x_perror (XINESERVER_X_ERR_NO_INDEX));
    *error_code = XINESERVER_X_ERR_NO_INDEX;
    }
  LOG_OUT
  return ret;
  }


/*============================================================================

  facade_add_matching

============================================================================*/
void facade_add_matching (const SearchConstraints *sc, int *error_code,
        char **error_message)
  {
  LOG_IN

  Facade *self = facade_get_instance();

  BOOL ok = TRUE;
  int dummy;
  List *list = facade_get_paths (0, 0, sc, 
      &dummy, error_code, error_message);
  if (list)
    {
    int l = list_length (list);
    if (l > 0)
      {
      char **streams = malloc (l * sizeof (char **));

      for (int i = 0; i < l; i++)
	{
	Path *path = path_clone (self->root);
	char *file = list_get (list, i);
	path_append (path, file);
	streams[i] = (char *)path_to_utf8 (path); 
	path_destroy (path);
	}

      ok = xineserver_add (self->xshost, self->xsport, 
	l, (const char *const *)streams, error_code, error_message);

      for (int i = 0; i < l; i++) free (streams[i]);
      free (streams);
      }
    list_destroy (list);
    if (ok) *error_code = 0;
    }

  LOG_OUT
  }

/*============================================================================

  facade_play_matching

============================================================================*/
void facade_play_matching (const SearchConstraints *sc, int *error_code,
        char **error_message)
  {
  LOG_IN

  *error_code = 0;
  facade_clear (error_code, error_message);

  if (*error_code == 0)
    facade_add_matching (sc, error_code, error_message);

  if (*error_code == 0)
    facade_play_index (0, error_code, error_message);

  LOG_OUT
  }





