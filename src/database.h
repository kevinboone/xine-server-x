/*============================================================================

  xine-server-x
  database.h
  Copyright (c)2020 Kevin Boone, GPL v3.0

============================================================================*/

#pragma once

#include <stdint.h>
#include "defs.h"
#include "searchconstraints.h"

// Longest a field name will be in the database
#define DB_MAX_FIELD 20

typedef BOOL (*DBPathIteratorCallback) (const char *path, void *data);

struct _Database;
typedef struct _Database Database;

BEGIN_DECLS

Database   *database_create  (const char *file);

void        database_destroy (Database *self);

void        database_close   (Database *self);

/** Creates a new database files. Fails if the file already exists. */
BOOL        database_make    (Database *self, char **error);

/** Open an existing database file. Fails if the database does not
    exist. */
BOOL        database_open    (Database *self, char **error);

void database_insert (Database *database, const char *path, size_t size,
            time_t mtime, const char *title,  const char *album,  
	    const char *genre,  const char *composer,  const char *artist,  
	    const char *track,  const char *comment,  const char *year,  
	    char **error);

BOOL database_get_by_path (Database *db, const char *path, size_t *size, 
        time_t *mtime, char **title,  char **album,  char **genre,  
	char **composer,  char **artist,  char **track,  char **comment,  
	char **year,  char **error);

BOOL database_delete_path (Database *db, const char *path, char **error);

BOOL database_iterate_all_paths (Database *db, 
        DBPathIteratorCallback callback, void *user_data, char **error);

List *database_get_albums (Database *self, int from, int limit, 
        const SearchConstraints *sc, int *match, char **error);

List *database_get_artists (Database *self, int from, int limit, 
        const SearchConstraints *sc, int *match, char **error);

List *database_get_genres (Database *self, int from, int limit, 
        const SearchConstraints *sc, int *match, char **error);

List *database_get_composers (Database *self, int from, int limit, 
        const SearchConstraints *sc, int *match, char **error);

/** Returns the first file in the specified album. The file is as stored
    in the database, so relative to the media root */
char *database_get_first_track_for_album (Database *self, const char *album,
        char **error);

/** Returns a List of char* representing the paths in a specific
    album. The list might be empty, even if the operation succeeds.
    Paths are relative to the media root */
List *database_get_paths_by_album (Database *self, const char *album, 
        char **error);

List *database_get_paths (Database *self, int from, int limit, 
        const SearchConstraints *sc, int *match, char **error);

char *database_escape_sql (const char *sql);

END_DECLS

