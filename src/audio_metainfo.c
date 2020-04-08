/*============================================================================

  xine-server-x 
  audio-metainfo.c
  Copyright (c)2017 Kevin Boone, GPL v3.0

============================================================================*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>
#include <stdint.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include "string.h" 
#include "defs.h" 
#include "log.h" 
#include "path.h" 
#include "database.h" 
#include "audio_metainfo.h" 
#include "tag_reader.h" 
#include "mimebuffer.h" 

struct _AudioMetaInfo
  {
  char *composer;
  char *album;
  char *genre;
  char *artist;
  char *title;
  char *track;
  char *comment;
  char *year;
  time_t mtime;
  size_t size;
  MimeBuffer *cover;
  }; 


#define SAFE(x) (x != NULL ? (x) : "")

/*==========================================================================
  audio_metainfo_create
  Create a audio_metainfo from a copy of the data provided. The caller can, and
    probably should, free the data. This method is safe to call on
    static data, should the need arise.
*==========================================================================*/
AudioMetaInfo *audio_metainfo_create ()
  {
  LOG_IN
  AudioMetaInfo *self = malloc (sizeof (AudioMetaInfo));
  self->composer = NULL;
  self->artist = NULL;
  self->album = NULL;
  self->genre = NULL;
  self->title = NULL;
  self->track = NULL;
  self->comment= NULL;
  self->year = NULL;
  self->cover = NULL;
  self->mtime = 0;
  self->size = 0;
  LOG_OUT 
  return self;
  }


/*==========================================================================
  audio_metainfo_destroy
*==========================================================================*/
void audio_metainfo_destroy (AudioMetaInfo *self)
  {
  LOG_IN
  if (self)
    {
    if (self->composer) free (self->composer);
    if (self->album) free (self->album);
    if (self->genre) free (self->genre);
    if (self->artist) free (self->artist);
    if (self->title) free (self->title);
    if (self->track) free (self->track);
    if (self->comment) free (self->comment);
    if (self->year) free (self->year);
    if (self->cover) mimebuffer_destroy(self->cover);
    free (self);
    }
  LOG_OUT
  }

/*==========================================================================

  audio_metainfo_get_cover

==========================================================================*/
const MimeBuffer *audio_metainfo_get_cover (const AudioMetaInfo *self)
  {
  return self->cover;
  }

/*==========================================================================

  audio_metainfo_get_composer

==========================================================================*/
const char *audio_metainfo_get_composer (const AudioMetaInfo *self)
  {
  return self->composer;
  }

/*==========================================================================

  audio_metainfo_get_album

==========================================================================*/
const char *audio_metainfo_get_album (const AudioMetaInfo *self)
  {
  return self->album;
  }

/*==========================================================================

  audio_metainfo_get_genre

==========================================================================*/
const char *audio_metainfo_get_genre (const AudioMetaInfo *self)
  {
  return self->genre;
  }

/*==========================================================================

  audio_metainfo_get_artist

==========================================================================*/
const char *audio_metainfo_get_artist (const AudioMetaInfo *self)
  {
  return self->artist;
  }

/*==========================================================================

  audio_metainfo_get_title

==========================================================================*/
const char *audio_metainfo_get_title (const AudioMetaInfo *self)
  {
  return self->title;
  }

/*==========================================================================

  audio_metainfo_get_track

==========================================================================*/
const char *audio_metainfo_get_track (const AudioMetaInfo *self)
  {
  return self->track;
  }

/*==========================================================================

  audio_metainfo_get_comment

==========================================================================*/
const char *audio_metainfo_get_comment (const AudioMetaInfo *self)
  {
  return self->comment;
  }

/*==========================================================================

  audio_metainfo_get_year

==========================================================================*/
const char *audio_metainfo_get_year (const AudioMetaInfo *self)
  {
  return self->year;
  }

/*==========================================================================

  audio_metainfo_get_mtime

==========================================================================*/
time_t audio_metainfo_get_mtime (const AudioMetaInfo *self)
  {
  return self->mtime;
  }

/*==========================================================================

  audio_metainfo_get_size

==========================================================================*/
size_t audio_metainfo_get_size (const AudioMetaInfo *self)
  {
  return self->size;
  }

/*==========================================================================

  audio_metainfo_get_from_path

==========================================================================*/
BOOL audio_metainfo_get_from_path (AudioMetaInfo *self, const char *path,
      char **error)
  {
  LOG_IN
  BOOL ret;

  struct stat sb;
  if (stat (path, &sb) == 0)
    {
    ret = TRUE;
    self->mtime = sb.st_mtime;
    self->size = sb.st_size;

    TagData *tag_data = NULL;
    int r = tag_get_tags (path, &tag_data);
    if (r == TAG_OK)
      {
      self->album = strdup (SAFE((char *)tag_get_common 
	      (tag_data, TAG_COMMON_ALBUM)));
      self->artist = strdup (SAFE((char *)tag_get_common 
	      (tag_data, TAG_COMMON_ARTIST)));
      self->composer = strdup (SAFE((char *)tag_get_common 
	      (tag_data, TAG_COMMON_COMPOSER)));
      self->genre = strdup (SAFE((char *)tag_get_common 
	      (tag_data, TAG_COMMON_GENRE)));
      self->title = strdup (SAFE((char *)tag_get_common 
	      (tag_data, TAG_COMMON_TITLE)));
      self->track = strdup (SAFE((char *)tag_get_common 
	      (tag_data, TAG_COMMON_TRACK)));
      self->comment = strdup (SAFE((char *)tag_get_common 
	      (tag_data, TAG_COMMON_COMMENT)));
      self->year = strdup (SAFE((char *)tag_get_common 
	      (tag_data, TAG_COMMON_YEAR)));

      if (tag_data->cover)
        {
        self->cover = mimebuffer_create (tag_data->cover, tag_data->cover_len,
	  tag_data->cover_mime);
	}

      }
    if (tag_data) tag_free_tag_data (tag_data);
    }
  else
    {
    asprintf (error, "stat() failed for %s: %s", path, strerror (errno));
    ret = FALSE;
    }

  LOG_OUT
  return ret;
  }


/*==========================================================================

  audio_metainfo_get_from_database

==========================================================================*/
BOOL audio_metainfo_get_from_database (AudioMetaInfo *self, Database *db,
      const char *path, char **error)
  {
  LOG_IN
  BOOL ret;

  time_t mtime = 0;
  size_t size = 0;
  char *title = NULL, *album = NULL, *genre = NULL, *composer = NULL,
          *artist = NULL, *track = NULL, *comment = NULL, *year = NULL;
  
  if (database_get_by_path (db, path, &size, &mtime, &title,  &album,  
        &genre,  &composer,  &artist,  &track,  &comment,  &year,  error))
    {
    ret = TRUE;
    self->mtime = mtime;
    self->size = size;
    self->title = title;
    self->album = album;
    self->genre = genre;
    self->composer = composer;
    self->artist = artist;
    self->track = track;
    self->comment = comment;
    self->year = year;
    }
  else
    ret = FALSE;
    // error already set
    
  LOG_OUT
  return ret;
  }




