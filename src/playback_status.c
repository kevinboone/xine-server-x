/*============================================================================

  xine-server-x
  playback_status.c 
  Copyright (c)2020 Kevin Boone, GPL v3.0

============================================================================*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h" 
#include "log.h" 
#include "playback_status.h" 

struct _PlaybackStatus
  {
  int pos;
  int len;
  int playlist_index;
  int playlist_length;
  int bitrate;
  BOOL seekable;
  XSXTransportStatus ts;
  char *composer;
  char *album;
  char *genre;
  char *artist;
  char *title;
  }; 


/*==========================================================================

  playback_status_create

*==========================================================================*/
PlaybackStatus *playback_status_create (void)
  {
  LOG_IN
  PlaybackStatus *self = malloc (sizeof (PlaybackStatus));
  self->pos = -1;
  self->len = -1;
  self->playlist_index = -1;
  self->playlist_length = -1;
  self->ts = XINESERVER_X_TRANSPORT_STOPPED;
  self->bitrate = 0;
  self->seekable = FALSE;
  self->composer = NULL;
  self->album = NULL;
  self->genre = NULL;
  self->artist = NULL;
  self->title = NULL;
  LOG_OUT 
  return self;
  }


/*==========================================================================

  playback_status_destroy

*==========================================================================*/
void playback_status_destroy (PlaybackStatus *self)
  {
  LOG_IN
  if (self)
    {
    if (self->composer) free (self->composer);
    if (self->album) free (self->album);
    if (self->genre) free (self->genre);
    if (self->artist) free (self->artist);
    if (self->title) free (self->title);
    free (self);
    }
  LOG_OUT 
  }

/*==========================================================================

  playback_status_get_len

*==========================================================================*/
int playback_status_get_len (const PlaybackStatus *self)
  {
  LOG_IN
  int ret = self->len;
  LOG_OUT 
  return ret;
  }

/*==========================================================================

  playback_status_get_composer

*==========================================================================*/
const char *playback_status_get_composer (const PlaybackStatus *self)
  {
  LOG_IN
  const char *ret = self->composer;
  LOG_OUT 
  return ret;
  }

/*==========================================================================

  playback_status_get_album

*==========================================================================*/
const char *playback_status_get_album (const PlaybackStatus *self)
  {
  LOG_IN
  const char *ret = self->album;
  LOG_OUT 
  return ret;
  }

/*==========================================================================

  playback_status_get_genre

*==========================================================================*/
const char *playback_status_get_genre (const PlaybackStatus *self)
  {
  LOG_IN
  const char *ret = self->genre;
  LOG_OUT 
  return ret;
  }

/*==========================================================================

  playback_status_get_artist

*==========================================================================*/
const char *playback_status_get_artist (const PlaybackStatus *self)
  {
  LOG_IN
  const char *ret = self->artist;
  LOG_OUT 
  return ret;
  }

/*==========================================================================

  playback_status_get_title

*==========================================================================*/
const char *playback_status_get_title (const PlaybackStatus *self)
  {
  LOG_IN
  const char *ret = self->title;
  LOG_OUT 
  return ret;
  }

/*==========================================================================

  playback_status_get_bitrate

*==========================================================================*/
int playback_status_get_bitrate (const PlaybackStatus *self)
  {
  LOG_IN
  int ret = self->bitrate;
  LOG_OUT 
  return ret;
  }

/*==========================================================================

  playback_status_get_pos

*==========================================================================*/
int playback_status_get_pos (const PlaybackStatus *self)
  {
  LOG_IN
  int ret = self->pos;
  LOG_OUT 
  return ret;
  }

/*==========================================================================

  playback_status_get_playlist_index

*==========================================================================*/
int playback_status_get_playlist_index (const PlaybackStatus *self)
  {
  LOG_IN
  int ret = self->playlist_index;
  LOG_OUT 
  return ret;
  }

/*==========================================================================

  playback_status_get_playlist_length

*==========================================================================*/
int playback_status_get_playlist_length (const PlaybackStatus *self)
  {
  LOG_IN
  int ret = self->playlist_length;
  LOG_OUT 
  return ret;
  }

/*==========================================================================

  playback_status_get_ts

*==========================================================================*/
XSXTransportStatus playback_status_get_ts (const PlaybackStatus *self)
  {
  LOG_IN
  XSXTransportStatus ret = self->ts;
  LOG_OUT
  return ret;
  }


/*==========================================================================

  playback_status_is_seekable

*==========================================================================*/
BOOL playback_status_is_seekable (const PlaybackStatus *self)
  {
  LOG_IN
  BOOL ret = self->seekable;
  LOG_OUT
  return ret;
  }


/*==========================================================================

  playback_status_set_seekable

*==========================================================================*/
void playback_status_set_seekable (PlaybackStatus *self, BOOL value)
  {
  LOG_IN
  self->seekable = value;
  LOG_OUT 
  }

/*==========================================================================

  playback_status_set_title

*==========================================================================*/
void playback_status_set_title (PlaybackStatus *self, const char *value)
  {
  LOG_IN
  if (self->title) free (self->title);
  self->title = strdup (value);
  LOG_OUT 
  }

/*==========================================================================

  playback_status_set_artist

*==========================================================================*/
void playback_status_set_artist (PlaybackStatus *self, const char *value)
  {
  LOG_IN
  if (self->artist) free (self->artist);
  self->artist = strdup (value);
  LOG_OUT 
  }

/*==========================================================================

  playback_status_set_genre

*==========================================================================*/
void playback_status_set_genre (PlaybackStatus *self, const char *value)
  {
  LOG_IN
  if (self->genre) free (self->genre);
  self->genre = strdup (value);
  LOG_OUT 
  }

/*==========================================================================

  playback_status_set_album

*==========================================================================*/
void playback_status_set_album (PlaybackStatus *self, const char *value)
  {
  LOG_IN
  if (self->album) free (self->album);
  self->album = strdup (value);
  LOG_OUT 
  }

/*==========================================================================

  playback_status_set_composer

*==========================================================================*/
void playback_status_set_composer (PlaybackStatus *self, const char *value)
  {
  LOG_IN
  if (self->composer) free (self->composer);
  self->composer = strdup (value);
  LOG_OUT 
  }

/*==========================================================================

  playback_status_set_bitrate

*==========================================================================*/
void playback_status_set_bitrate (PlaybackStatus *self, int value)
  {
  LOG_IN
  self->bitrate = value;
  LOG_OUT 
  }

/*==========================================================================

  playback_status_set_len

*==========================================================================*/
void playback_status_set_len (PlaybackStatus *self, int value)
  {
  LOG_IN
  self->len = value;
  LOG_OUT 
  }

/*==========================================================================

  playback_status_set_pos

*==========================================================================*/
void playback_status_set_pos (PlaybackStatus *self, int value)
  {
  LOG_IN
  self->pos = value;
  LOG_OUT 
  }

/*==========================================================================

  playback_status_set_playlist_index

*==========================================================================*/
void playback_status_set_playlist_index (PlaybackStatus *self, int value)
  {
  LOG_IN
  self->playlist_index = value;
  LOG_OUT 
  }

/*==========================================================================

  playback_status_set_playlist_length

*==========================================================================*/
void playback_status_set_playlist_length (PlaybackStatus *self, int value)
  {
  LOG_IN
  self->playlist_length = value;
  LOG_OUT 
  }

/*==========================================================================

  playback_status_set_ts

*==========================================================================*/
void playback_status_set_ts (PlaybackStatus *self, XSXTransportStatus value)
  {
  LOG_IN
  self->ts = value;
  LOG_OUT 
  }



