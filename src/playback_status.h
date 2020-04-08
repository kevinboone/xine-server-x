/*============================================================================

  xine-server-x
  playback_status.h
  Copyright (c)2020 Kevin Boone, GPL v3.0

============================================================================*/

#pragma once

#include <stdint.h>
#include "defs.h"
#include "xine-server-x-api.h"

struct _PlaybackStatus;
typedef struct _PlaybackStatus PlaybackStatus;

BEGIN_DECLS

PlaybackStatus *playback_status_create (void);

void            playback_status_destroy (PlaybackStatus *self);

const char     *playback_status_get_composer (const PlaybackStatus *self);
const char     *playback_status_get_album (const PlaybackStatus *self);
const char     *playback_status_get_genre (const PlaybackStatus *self);
const char     *playback_status_get_artist (const PlaybackStatus *self);
const char     *playback_status_get_title (const PlaybackStatus *self);

BOOL            playback_status_is_seekable (const PlaybackStatus *self);

int             playback_status_get_bitrate (const PlaybackStatus *self);

int             playback_status_get_len (const PlaybackStatus *self);

int             playback_status_get_pos (const PlaybackStatus *self);

int             playback_status_get_playlist_index 
                   (const PlaybackStatus *self);

int             playback_status_get_playlist_length 
                   (const PlaybackStatus *self);

XSXTransportStatus playback_status_get_ts (const PlaybackStatus *self);

void            playback_status_set_composer (PlaybackStatus *self, 
                   const char *value);
void            playback_status_set_artist (PlaybackStatus *self, 
                   const char *value);
void            playback_status_set_album (PlaybackStatus *self, 
                   const char *value);
void            playback_status_set_genre (PlaybackStatus *self, 
                   const char *value);
void            playback_status_set_title (PlaybackStatus *self, 
                   const char *value);

void            playback_status_set_seekable (PlaybackStatus *self, BOOL value);

void            playback_status_set_bitrate (PlaybackStatus *self, int value);

void            playback_status_set_len (PlaybackStatus *self, int value);

void            playback_status_set_pos (PlaybackStatus *self, int value);

void            playback_status_set_playlist_index (PlaybackStatus *self, 
                   int value);

void            playback_status_set_playlist_length (PlaybackStatus *self, 
                   int value);

void            playback_status_set_ts (PlaybackStatus *self, 
                   XSXTransportStatus ts);

END_DECLS

