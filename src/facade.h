/*============================================================================

  xine-server-x
  facade.h
  Copyright (c)2020 Kevin Boone, GPL v3.0

============================================================================*/

#pragma once

#include <stdint.h>
#include <time.h>
#include "defs.h"
#include "list.h"
#include "playback_status.h"
#include "path.h"
#include "searchconstraints.h"
#include "audio_metainfo.h"

#define EXT_FILE_BASE          "/ext/"
#define API_BASE               "/api/"
#define GUI_BASE               "/gui/"
#define INT_FILE_BASE          "/int/"

/** Facade is a singleton class that handles all interation with 
    xine-server and the local filesystem. */
struct _Facade;
typedef struct _Facade Facade;

BEGIN_DECLS

void        facade_create (const char *root, const char *xshost, int xsport,
              const char *gxsradio_dir, const char *index_file);
Facade     *facade_get_instance (void);
void        facade_destroy (void);

/** Get a list of files (not directories) at the specific path, 
    relative to the media root. The list only includes playable
    audio files. Names are sorted into ASCII order, and returned as
    a List of char* */
List       *facade_get_file_list (const char  *path, int *error_code, 
                 char **error);

/** Get the playlust as a List of char* */
List       *facade_get_playlist (int *error_code, char **error);

/** Get a list of directories (not files) at the specific path, 
    relative to the media root. 
    Names are sorted into ASCII order, and returned as
    a List of char* */
List       *facade_get_dir_list (const char  *path, int *error_code, 
                 char **error);

/** Get a URI for the cover image associated with the specified
    directory path. The path is relative to the media root. The
    method looks for the usual -- folder.png, etc. If there is no
    cover image, returns NULL. The URI returned is relative to the
    server host:port, and will always begin with "/ext/", as it
    references a file outside the server itself. */
char       *facade_get_cover_image_for_dir (const char *path);

/** Get a URI for the cover image associated with the specified
    album name. Since cover images are associated with files, not
    albums, it's possible that the same album has multiple cover
    images. However, this function will only find (at most) one --
    the first one that turns up in a database search.
    The URI returned is relative to the
    server host:port, and will always begin with "/ext/", as it
    references a file outside the server itself. */
char       *facade_get_cover_image_for_album (const char *album);

/** Returns the URI of a cover image (beginning with /ext/) suitable
    for the specified path, which is relative to the media root.
    If there is no such image, returns NULL. Note that this function
    does not interrogate the file itself, which could be very slow --
    it just looks for image files (cover.*, folder.*) in the file's
    directory. */
char       *facade_get_cover_image_for_file (const char *path);

/** Tell xine-server to play the stream immediately, clearing the playlist.
   The stream can be a URI, or a local file, or anything that is acceptable
   to xine-server. If a local file, it must be an absolute path on the
   filesystem.  Error code returned is one of
   the XINESERVER_X_ERR_XXX values. error_message will be written if
   not null. If it is written, the caller must free it. */
void        facade_play_stream (const char *file, int *error_code, 
                char **error_message);

/** Tell xine-server to play the file files in the specified
   directory, clearing any existing playlist.
   The dir is relative to the media root. Error code returned is one of
   the XINESERVER_X_ERR_XXX values. error_message will be written if
   not null. If it is written, the caller must free it. */
void        facade_play_dir (const char *dir, int *error_code, 
                char **error_message);

/** Tell xine-server to add all files in the specified directory to the
   playlist. Existing playlist is not changed. 
   The dir is relative to the media root. Error code returned is one of
   the XINESERVER_X_ERR_XXX values. error_message will be written if
   not null. If it is written, the caller must free it. */
void       facade_add_dir (const char *dir, int *error_code, 
               char **error_message);

/** Tell xine-server to play from the specified item in the playlist */ 
void        facade_play_index (int index, int *error_code, 
                char **error_message);

/** Clear the playlist, add all the files from the specified album,
    and begin playback from the top */ 
void        facade_play_album (const char *album, int *error_code, 
                char **error_message);

/** Tell xine-server to add the file to the playlist, without clearing
    the playlist or changing the playback position. */
void        facade_add_file (const char *file, int *error_code, 
                char **error_message);

/** Tell xine-server to play the file immediately, clearing the playlist.
   The file is relative to the media root. Error code returned is one of
   the XINESERVER_X_ERR_XXX values. error_message will be written if
   not null. If it is written, the caller must free it. */
void        facade_play_file (const char *file, int *error_code, 
                char **error_message);

/** Tell xine-server to stop playback immediately */
void        facade_stop (int *error_code, char **error_message);

/** Tell xine-server to pause playback */
void        facade_pause (int *error_code, char **error_message);

/** If paused, resume. If there is something in the playlist, play. 
    Otherwise raise an error. */
void        facade_play (int *error_code, char **error_message);

/** Tell xine-server to play the next item in the playlist, if there
    is one. */
void        facade_next (int *error_code, char **error_message);

/** Tell xine-server to play the previous item in the playlist, if there
    is one. */
void        facade_prev (int *error_code, char **error_message);

/** Get playback status from xine-server. If error code is 0, 
      status is allocated, and the caller should free it. If error code
      is non-zero, error_message is written if it is non-null, and hte
      caller must free it */
void        facade_get_playback_status (int *error_code, char **error_message, 
               PlaybackStatus **status);

/** Set playback volume, in the range 0-100. */
void        facade_set_volume (int volume, int *error_code, 
               char **error_message);

/** Tell xine-server to shut down. */
void facade_shut_down_xine_server (void);

/** Get a list of gxsradio station files in the station file directory. 
    Only files ending in .gxsradio are returned, and this extension is
    _not_ included in the results.  Results are returned as a List of char* */ 
List       *facade_get_radio_station_lists (int *error_code, 
                 char **error);

/** Get a list of station names for a give station list. The startion lists
    are stored in .gxsradio files, but the name should not have this
    extension. */
List *facade_get_radio_station_names (const char *station_list_name, int 
        *error_code, char **error);

/** Play the specified station in the specified station list. */
void facade_play_station (const char *list, const char *name, 
        int *error_code, char **error_message);

/** Returns TRUE if the path has an extension that corresponds to one of the
    data types commonly associated with audio: .aac, .mp3, .m4a... */
BOOL facade_path_is_playable (const Path *path);

/** Gets the status of the file metainfo scanner. The function will return a
    failure if information is available, but malformed. Otherwise all the 
    data elements will be filled in, even if only with zeros. If the
    'running' field is FALSE, then all the others will, in fact, be zeros */
void facade_scanner_status (int *error_code, char **error_message, 
    BOOL *running, BOOL *scanned, BOOL *added, BOOL *modified, BOOL *deleted,
    BOOL *extracted);

/** Begin a quick file scan */
void facade_quick_scan (int *error_code, char **error_message);

/** Begin a full file scan */
void facade_full_scan (int *error_code, char **error_message);

List *facade_get_albums (int from, int limit, const SearchConstraints *sc, 
        int *match, int *error_code, char **error_message);

List *facade_get_artists (int from, int limit, const SearchConstraints *sc, 
        int *match, int *error_code, char **error_message);

List *facade_get_genres (int from, int limit, const SearchConstraints *sc, 
        int *match, int *error_code, char **error_message);

List *facade_get_composers (int from, int limit, const SearchConstraints *sc, 
        int *match, int *error_code, char **error_message);

/** Returns the path (relative to the media root) of the first track
    found in the datbase that matches the album. This function is
    mostly intended for finding the directory that contains the 
    album's files. There's nothing to stop an album being split
    across multiple directories, so there's no guarantee that the
    track found will allow the directory to be uniquely identified */
char *facade_get_first_track_for_album (const char *album, int *error_code, 
        char **error_message);

List *facade_get_paths (int from, int limit, const SearchConstraints *sc, 
        int *match, int *error_code, char **error_message);

AudioMetaInfo *facade_get_metainfo_from_database (const char *path, 
        int *error_code, char **error);

/** Add all tracks that match the search constraints to the playlist.
    Note that it is not an error if there are no matches, although
    it is an error if there is no index, or it cannot be read */
void facade_add_matching (const SearchConstraints *sc, int *error_code,
        char **error_message);

/** Add all tracks that match the search constraints to the playlist.
    Note that it is not an error if there are no matches, although
    it is an error if there is no index, or it cannot be read. Of
    course, if there are no matches, nothing will play. This function
    clears the current playlist and stops playback, whether anything
    new plays or not */
void facade_play_matching (const SearchConstraints *sc, int *error_code,
        char **error_message);

/** Clear playlist and stop playback */
void facade_clear (int *error_code, char **error_message);

END_DECLS

