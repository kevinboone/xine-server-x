/*============================================================================

  xine-server-x
  audio_metainfo.h
  Copyright (c)2020 Kevin Boone, GPL v3.0

============================================================================*/

#pragma once

#include <stdint.h>
#include "defs.h"
#include "mimebuffer.h"
#include "database.h"

struct _AudioMetaInfo;
typedef struct _AudioMetaInfo AudioMetaInfo;

BEGIN_DECLS

AudioMetaInfo    *audio_metainfo_create (void);

void              audio_metainfo_destroy (AudioMetaInfo *self);

BOOL              audio_metainfo_get_from_path (AudioMetaInfo *self, 
                    const char *path, char **error);

/** Note: 'path' is the relative path, as it would appear in the database,
 * _not_ the filesystem path */
BOOL              audio_metainfo_get_from_database (AudioMetaInfo *self, 
                    Database *db, const char *path, char **error);

const char       *audio_metainfo_get_title (const AudioMetaInfo *self);
const char       *audio_metainfo_get_artist (const AudioMetaInfo *self);
const char       *audio_metainfo_get_composer (const AudioMetaInfo *self);
const char       *audio_metainfo_get_album (const AudioMetaInfo *self);
const char       *audio_metainfo_get_genre (const AudioMetaInfo *self);
const char       *audio_metainfo_get_track (const AudioMetaInfo *self);
const char       *audio_metainfo_get_comment (const AudioMetaInfo *self);
const char       *audio_metainfo_get_year (const AudioMetaInfo *self);
size_t            audio_metainfo_get_size (const AudioMetaInfo *self);
time_t            audio_metainfo_get_mtime (const AudioMetaInfo *self);
const MimeBuffer *audio_metainfo_get_cover (const AudioMetaInfo *self);

END_DECLS

