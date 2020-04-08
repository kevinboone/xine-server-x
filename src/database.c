/*============================================================================

  xine-server-x
  database.c
  Copyright (c)2020 Kevin Boone, GPL v3.0

============================================================================*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <unistd.h>
#include <regex.h>
#include "string.h" 
#include "defs.h" 
#include "log.h" 
#include "database.h" 
#include "sqlite3.h" 
#include "searchconstraints.h" 

typedef struct _IteratePathsCallbackData
  {
  DBPathIteratorCallback callback;
  void *user_data;
  } IteratePathsCallbackData;

struct _Database
  {
  sqlite3 *sqlite;
  char *file;
  }; 

#define SAFE(x) (x != NULL ? (x) : "")

/*==========================================================================

  database_create

*==========================================================================*/
Database *database_create (const char *file)
  {
  LOG_IN
  Database *self = malloc (sizeof (Database));
  self->file = strdup (file);
  self->sqlite = NULL;
  LOG_OUT 
  return self;
  }


/*==========================================================================
  database_close
*==========================================================================*/
void database_close (Database *self)
  {
  LOG_IN
  if (self->sqlite) sqlite3_close (self->sqlite);
  self->sqlite = NULL;
  LOG_OUT
  }


/*==========================================================================
  database_destroy
*==========================================================================*/
void database_destroy (Database *self)
  {
  LOG_IN
  if (self)
    {
    database_close (self);
    if (self->file) free (self->file);
    free (self);
    }
  LOG_OUT
  }

/*==========================================================================
  database_escape_sql
*==========================================================================*/
char *database_escape_sql (const char *sql)
  {
  LOG_IN
  char *ret;
  if (strchr (sql, '\''))
    {
    int l = strlen (sql);
    int newlen = l;
    ret = malloc (newlen + 1);
    int p = 0;

    for (int i = 0; i < l; i++)
      {
      char c = sql[i];
      if (c == '\'')
	{
	newlen+=2;
	ret = realloc (ret, newlen + 1);
	ret[p++] = '\'';
	ret[p++] = '\'';
	}
      else
	{
	ret[p++] = c;
	}
      }
    ret[p] = 0;
    }
  else
    {
    ret = strdup (sql);
    }
  LOG_OUT
  return ret;
  }

/*==========================================================================

  database_exec

==========================================================================*/
BOOL database_exec (Database *self, const char *sql, char **error)
  {
  LOG_IN
  BOOL ret = FALSE;
  char *e = NULL;
  log_debug ("%s: executing SQL %s", __PRETTY_FUNCTION__, sql);
  sqlite3_exec (self->sqlite, sql, NULL, 0, &e); 
  if (e)
    {
    if (error) (*error) = strdup (e);
    sqlite3_free (e);
    ret = FALSE;
    }
  else
    ret = TRUE;
  LOG_OUT
  return ret;
  }

/*==========================================================================

  database_query

  Returns a List of char *, which may be empty. The list returns includes
  all rows and columns in row order, all concetenated into one long list.
  If the query selects a single column, then the list returned will be
  the same length as the number of matches, or 'limit' if that is
  smaller. If limit == 0, then no limit is applied

==========================================================================*/
List *database_query (Database *self, const char *sql, BOOL include_empty,
        int limit, char **error)
  {
  LOG_IN
  List *ret = NULL;
  char *e = NULL;
  char **result = NULL;
  int hits;
  int cols;
  log_debug ("%s: executing SQL %s", __PRETTY_FUNCTION__, sql);
  sqlite3_get_table (self->sqlite, sql, &result, &hits, &cols, &e); 
  if (e)
    {
    if (error) (*error) = strdup (e);
    sqlite3_free (e);
    ret = NULL;
    }
  else
    {
    ret = list_create (free); 

    for (int i = 0; i < hits && ((i < limit) || (limit == 0)); i++)
      {
      for (int j = 0; j < cols ; j++)
        {
        char *res = result[(i + 1) * cols + j];
        if (res[0] != 0 || include_empty)
          {
          list_append (ret, strdup (res));
          }
        }
      }
    sqlite3_free_table (result);
    }
  LOG_OUT
  return ret;
  }


/*==========================================================================

  database_regexp

==========================================================================*/
void database_regexp (sqlite3_context* context, int argc, 
      sqlite3_value** values) 
  {
  int ret;
  regex_t regex;
  char* reg = (char*)sqlite3_value_text(values[0]);
  char* text = (char*)sqlite3_value_text(values[1]);

  if ( argc != 2 || reg == 0 || text == 0) 
    {
    sqlite3_result_error (context, 
      "SQL function regexp() called with invalid arguments.\n", -1);
    return;
    }

  ret = regcomp (&regex, reg, REG_EXTENDED | REG_NOSUB | REG_ICASE);
  if (ret != 0) 
    {
    sqlite3_result_error (context, "error compiling regular expression", -1);
    return;
    }

  ret = regexec (&regex, text , 0, NULL, 0);
  regfree(&regex);

  sqlite3_result_int (context, (ret != REG_NOMATCH));
  }


/*==========================================================================

  database_open

==========================================================================*/
BOOL database_open (Database *self, char **error)
  {
  LOG_IN
  BOOL ret = FALSE;

  log_info ("Opening database file %s", self->file);
  
  if (access (self->file, W_OK) == 0)
    {
    int err = sqlite3_open (self->file, &self->sqlite);
    if (err == 0)
      {
      sqlite3_create_function(self->sqlite, "regexp", 2, SQLITE_ANY, 0,
        database_regexp, 0, 0);
      ret = TRUE;
      }
   else
      {
      log_error ("Can't open database file %s", self->file);
      ret = FALSE;
      }
    }
  else
    {
    asprintf (error, "Can't open database file '%s' in RW mode", self->file);
    ret = FALSE;
    }
  LOG_OUT
  return ret;
  }


/*==========================================================================
 
  database_make

*==========================================================================*/
BOOL database_make (Database *self, char **error)
  {
  LOG_IN
  BOOL ret = FALSE;

  log_info ("Creating database file %s", self->file);
  
  // TODO overwrite file

  int err = sqlite3_open (self->file, &self->sqlite);
  if (err == 0)
    {
    ret = database_exec (self, "create table files "
       "(path varchar not null, size integer, mtime integer, "
       "title varchar, album varchar, genre varchar, "
       "composer varchar, artist varchar, track varchar, "
       "comment varchar, year varchar, exist integer)", error);

    if (ret)   
      ret = database_exec (self, "create index albumindex on files (album)", 
       error);
    if (ret)   
      ret = database_exec (self, "create index artistindex on files (artist)", 
       error);
    if (ret)   
      ret = database_exec (self, 
        "create index composerindex on files (composer)", error);
    if (ret)   
      ret = database_exec (self, "create index pathindex on files (path)", 
       error);
    if (ret)   
      ret = database_exec (self, "create index genreindex on files (genre)", 
       error);
    }
 else
    {
    log_error ("Can't create database file %s", self->file);
    ret = FALSE;
    }

  LOG_OUT
  return ret;
  }


/*==========================================================================
 
  database_insert

*==========================================================================*/
void database_insert (Database *database, const char *path, size_t size,
    time_t mtime, const char *title,  const char *album,  const char *genre,  
    const char *composer,  const char *artist,  const char *track,  
    const char *comment,  const char *year,  char **error)
  {
  LOG_IN

  char *esc_path = database_escape_sql (SAFE(path));
  char *esc_title = database_escape_sql (SAFE(title));
  char *esc_album = database_escape_sql (SAFE(album));
  char *esc_genre = database_escape_sql (SAFE(genre));
  char *esc_composer = database_escape_sql (SAFE(composer));
  char *esc_artist = database_escape_sql (SAFE(artist));
  char *esc_track = database_escape_sql (SAFE(track));
  char *esc_comment = database_escape_sql (SAFE(comment));
  char *esc_year = database_escape_sql (SAFE(year));

  String *sql = string_create_empty ();
  string_append_printf (sql, "insert into files "
     "(path,size,mtime,title,album,genre,composer,"
     "artist,track,comment,year,exist) values "
     "('%s',%d,%d,'%s','%s','%s','%s','%s','%s','%s','%s',1)",
     esc_path,
     size,
     mtime,
     esc_title,
     esc_album,
     esc_genre,
     esc_composer,
     esc_artist,
     esc_track,
     esc_comment,
     esc_year);

  database_exec (database, string_cstr(sql), error);

  string_destroy (sql);
  free (esc_path);
  free (esc_title);
  free (esc_album);
  free (esc_genre);
  free (esc_composer);
  free (esc_artist);
  free (esc_track);
  free (esc_comment);
  free (esc_year);

  LOG_OUT
  }

/*==========================================================================
 
  database_get_by_path

*==========================================================================*/
BOOL database_get_by_path (Database *db, const char *path, size_t *size, 
        time_t *mtime, char **title,  char **album,  char **genre,  
	char **composer,  char **artist,  char **track,  char **comment,  
	char **year,  char **error)
  {
  LOG_IN
  BOOL ret = FALSE;
  char *esc_path = database_escape_sql (path);

  char *sql;
  asprintf (&sql, "select size,mtime,title,album,genre,composer,artist,"
                    "track,comment,year from files where path='%s'", esc_path); 

  List *list = database_query (db, sql, TRUE, 0, error);
  if (list)
    {
    int l = list_length (list);
    if (l == 10)
      {
      *size = atoi (SAFE (list_get (list, 0)));
      *mtime = atoi (SAFE (list_get (list, 1)));
      *title = strdup (SAFE (list_get (list, 2)));
      *album = strdup (SAFE (list_get (list, 3)));
      *genre = strdup (SAFE (list_get (list, 4)));
      *composer = strdup (SAFE (list_get (list, 5)));
      *artist = strdup (SAFE (list_get (list, 6)));
      *track = strdup (SAFE (list_get (list, 7)));
      *comment = strdup (SAFE (list_get (list, 8)));
      *year = strdup (SAFE (list_get (list, 9)));
      ret = TRUE;
      }
    else
      {
      asprintf (error, "Path '%s' not in database", path);
      ret = FALSE;
      }
    list_destroy (list);
    }
  else
    {
    ret = FALSE;
    // Error already set
    }
  free (sql);
  free (esc_path);
  LOG_OUT
  return ret;
  }


/*==========================================================================
 
  database_delete_path

*==========================================================================*/
BOOL database_delete_path (Database *db, const char *path, char **error)
  {
  LOG_IN
  BOOL ret = FALSE;
  char *esc_path = database_escape_sql (path);

  char *sql;
  asprintf (&sql, "delete from files where path='%s'", esc_path);

  ret = database_exec (db, sql, error);

  free (sql);
  free (esc_path);
  LOG_OUT
  return ret;
  }


/*==========================================================================
 
  database_iterate_all_paths_callback

*==========================================================================*/
int database_iterate_all_paths_callback (void *data, int ncols, 
     char **cols, char **dummy)
  {
  BOOL ret = FALSE;
  IteratePathsCallbackData *ipcd = (IteratePathsCallbackData *)data;
  if (ncols == 1)
    {
    const char *path = cols[0];
    ret = ipcd->callback (path, ipcd->user_data);
    }
  else
    {
    // Should never happen
    log_error ("Internal error -- ncols != 1 in %s", __PRETTY_FUNCTION__);
    ret = FALSE;
    }
  
  if (ret) return 0;
  return -1;
  }


/*==========================================================================
 
  database_delete_path

*==========================================================================*/
BOOL database_iterate_all_paths (Database *db, 
        DBPathIteratorCallback callback, void *user_data, char **error)
  {
  LOG_IN
  BOOL ret = TRUE;

  IteratePathsCallbackData ipcd;
  ipcd.callback = callback;
  ipcd.user_data = user_data;

  char *e = NULL;
  sqlite3_exec(db->sqlite, 
    "select path from files", database_iterate_all_paths_callback, 
    &ipcd, &e); 

  if (e)
    {
    if (error) (*error) = strdup (e);
    sqlite3_free (e);
    ret = FALSE;
    }
  else
    ret = TRUE;

  LOG_OUT
  return ret;
  }

/*==========================================================================
 
  database_get_field

*==========================================================================*/
List *database_get_field (Database *self, const char *field, 
    int from, int limit, const SearchConstraints *sc, 
    int *match, char **error)
  {
  LOG_IN
  List *ret = NULL;

  char *where = searchconstraints_make_where (sc);
  char *sql;
  asprintf (&sql, "select distinct %s from files %s order by %s",
    field, where, field);
  free (where);

  List *list = database_query (self, sql, FALSE, 0, error);
  if (list)
    {
    *match = list_length (list);
    ret = list_create (free);
    if (*match)
      {
      if (from < 0) from = 0;
      if (from >= *match) from = *match - limit;
      for (int i = from; (i < from + limit || limit == 0) && i < *match; i++)
	{
	list_append (ret, strdup (list_get (list, i)));
	}
      }
    list_destroy (list);
    }

  free (sql);

  LOG_OUT
  return ret;
  }


/*==========================================================================
 
  database_get_albums

*==========================================================================*/
List *database_get_albums (Database *self, int from, int limit, 
    const SearchConstraints *sc, int *match, char **error)
  {
  LOG_IN
  List *ret = database_get_field (self, "album", from, limit, sc, 
    match, error);
  LOG_OUT
  return ret;
  }

/*==========================================================================
 
  database_get_composers

*==========================================================================*/
List *database_get_composers (Database *self, int from, int limit, 
    const SearchConstraints *sc, int *match, char **error)
  {
  LOG_IN
  List *ret = database_get_field (self, "composer", from, limit, sc, 
    match, error);
  LOG_OUT
  return ret;
  }

/*==========================================================================
 
  database_get_genres

*==========================================================================*/
List *database_get_genres (Database *self, int from, int limit, 
    const SearchConstraints *sc, int *match, char **error)
  {
  LOG_IN
  List *ret = database_get_field (self, "genre", from, limit, sc, 
    match, error);
  LOG_OUT
  return ret;
  }

/*==========================================================================
 
  database_get_artists

*==========================================================================*/
List *database_get_artists (Database *self, int from, int limit, 
    const SearchConstraints *sc, int *match, char **error)
  {
  LOG_IN
  List *ret = database_get_field (self, "artist", from, limit, sc, 
    match, error);
  LOG_OUT
  return ret;
  }

/*==========================================================================
 
  database_get_first_track_for_album

*==========================================================================*/
char *database_get_first_track_for_album (Database *self, const char *album,
        char **error)
  {
  char *ret = NULL;
  LOG_IN

  char *esc_album = database_escape_sql (album);
  char *sql;
  asprintf (&sql, "select path from files where album='%s'", 
    esc_album);
  List *list = database_query (self, sql, TRUE, 1, error);
  if (list)
    {
    int l = list_length (list);
    if (l > 0)
      {
      ret = strdup (list_get (list, 0));
      }
    else
      {
      asprintf (error, "No tracks for album '%s'", album);
      }
    list_destroy (list);
    }
  else
    {
    // Do nothing -- error already set
    }

  free (esc_album);
  free (sql);
  LOG_OUT
  return ret;
  }


/*==========================================================================
 
  database_get_paths_by_album

*==========================================================================*/
List *database_get_paths_by_album (Database *self, const char *album, 
        char **error)
  {
  LOG_IN

  char *esc_album = database_escape_sql (album);
  char *sql;
  asprintf (&sql, 
    "select path from files where album='%s' "
    "order by cast (track as integer),title", 
    esc_album);
  List *ret = database_query (self, sql, TRUE, 0, error);
  free (esc_album);
  free (sql);

  LOG_OUT
  return ret;
  }

/*==========================================================================
 
  database_get_paths

*==========================================================================*/
List *database_get_paths (Database *self, int from, int limit, 
    const SearchConstraints *sc, int *match, char **error)
  {
  LOG_IN
  List *ret = NULL;

  char *sql;

  char *where = searchconstraints_make_where (sc);
  asprintf (&sql, 
     "select distinct path from files %s "
     "order by cast (track as integer),title,path", where);
  free (where);

  List *list = database_query (self, sql, TRUE, 0, error);
  if (list)
    {
    *match = list_length (list);
    ret = list_create (free);
    if (from < 0) from = 0;
    if (from >= *match) from = *match - limit;
    if (from < 0) from = 0;
    for (int i = from; (i < from + limit || limit == 0) && i < *match; i++)
      {
      list_append (ret, strdup (list_get (list, i)));
      }

    list_destroy (list);
    }

  free (sql);

  LOG_OUT
  return ret;
  }


