/*============================================================================

  xine-server-x 
  request_handler.c
  Copyright (c)2020 Kevin Boone
  Distributed under the terms of the GPL v3.0

============================================================================*/

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
#include "gui_request_handler.h" 
#include "wstring.h" 

#define ERR_NOT_FOUND    "Not found\n"
#define ERR_NOT_MODIFIED "Not changed\n"
#define ERR_DIR_LIST     "Directory listing not allowed\n"


struct _RequestHandler
  {
  Path *root;
  BOOL shutdown_requested;
  APIRequestHandler *api_request_handler;
  GUIRequestHandler *gui_request_handler;
  const ProgramContext *context;
  const char *index_file;
  time_t build_datetime;
  }; 


typedef struct 
  {
  const char *filename;
  const uint8_t *start;
  const uint8_t *end;
  } FileData;

extern uint8_t default_cover_png_start[] asm("_binary_docroot_default_cover_png_start");
extern uint8_t default_cover_png_end[]   asm("_binary_docroot_default_cover_png_end");
extern uint8_t logo_png_start[] asm("_binary_docroot_logo_png_start");
extern uint8_t logo_png_end[]   asm("_binary_docroot_logo_png_end");
extern uint8_t index_html_start[] asm("_binary_docroot_index_html_start");
extern uint8_t index_html_end[]   asm("_binary_docroot_index_html_end");
extern uint8_t favicon_ico_start[] asm("_binary_docroot_favicon_ico_start");
extern uint8_t favicon_ico_end[]   asm("_binary_docroot_favicon_ico_end");
extern uint8_t main_css_start[] asm("_binary_docroot_main_css_start");
extern uint8_t main_css_end[]   asm("_binary_docroot_main_css_end");
extern uint8_t functions_js_start[] asm("_binary_docroot_functions_js_start");
extern uint8_t functions_js_end[]   asm("_binary_docroot_functions_js_end");
extern uint8_t stopbutton_png_start[] asm("_binary_docroot_stopbutton_png_start");
extern uint8_t stopbutton_png_end[]   asm("_binary_docroot_stopbutton_png_end");
extern uint8_t playbutton_png_start[] asm("_binary_docroot_playbutton_png_start");
extern uint8_t playbutton_png_end[]   asm("_binary_docroot_playbutton_png_end");
extern uint8_t pausebutton_png_start[] asm("_binary_docroot_pausebutton_png_start");
extern uint8_t pausebutton_png_end[]   asm("_binary_docroot_pausebutton_png_end");
extern uint8_t nextbutton_png_start[] asm("_binary_docroot_nextbutton_png_start");
extern uint8_t nextbutton_png_end[]   asm("_binary_docroot_nextbutton_png_end");
extern uint8_t prevbutton_png_start[] asm("_binary_docroot_prevbutton_png_start");
extern uint8_t prevbutton_png_end[]   asm("_binary_docroot_prevbutton_png_end");
extern uint8_t caption_logo_png_start[] asm("_binary_docroot_caption_logo_png_start");
extern uint8_t caption_logo_png_end[]   asm("_binary_docroot_caption_logo_png_end");
extern uint8_t menu_icon_png_start[] asm("_binary_docroot_menu_icon_png_start");
extern uint8_t menu_icon_png_end[]   asm("_binary_docroot_menu_icon_png_end");
extern uint8_t spk_png_start[] asm("_binary_docroot_spk_png_start");
extern uint8_t spk_png_end[]   asm("_binary_docroot_spk_png_end");


static FileData fileData[] = 
  {
  { "default_cover.png", default_cover_png_start, default_cover_png_end },
  { "logo.png", logo_png_start, logo_png_end },
  { "index.html", index_html_start, index_html_end },
  { "favico.ico", favicon_ico_start, favicon_ico_end },
  { "main.css", main_css_start, main_css_end },
  { "functions.js", functions_js_start, functions_js_end },
  { "stopbutton.png", stopbutton_png_start, stopbutton_png_end },
  { "playbutton.png", playbutton_png_start, playbutton_png_end },
  { "pausebutton.png", pausebutton_png_start, pausebutton_png_end },
  { "nextbutton.png", nextbutton_png_start, nextbutton_png_end },
  { "prevbutton.png", prevbutton_png_start, prevbutton_png_end },
  { "caption_logo.png", caption_logo_png_start, caption_logo_png_end },
  { "menu_icon.png", menu_icon_png_start, menu_icon_png_end },
  { "spk.png", spk_png_start, spk_png_end },
  { NULL, NULL, NULL } 
  };

/*============================================================================

  request_handler_create

============================================================================*/
RequestHandler *request_handler_create (const char *_root,
    const ProgramContext *context)
  {
  LOG_IN
  RequestHandler *self = malloc (sizeof (RequestHandler)); 
  self->root = path_create (_root);
  self->shutdown_requested = FALSE;
  self->api_request_handler = api_request_handler_create (self);
  self->gui_request_handler = gui_request_handler_create (self);
  self->context = context;
  self->index_file = program_context_get (context, "index");
  self->build_datetime = atol (BUILD_DATETIME);
  LOG_OUT 
  return self;
  }

/*============================================================================

  request_handler_destroy

============================================================================*/
void request_handler_destroy (RequestHandler *self)
  {
  LOG_IN
  if (self)
    {
    if (self->root) path_destroy (self->root);
    if (self->gui_request_handler) gui_request_handler_destroy 
      (self->gui_request_handler);
    if (self->api_request_handler) api_request_handler_destroy 
      (self->api_request_handler);
    free (self);
    }
  LOG_OUT 
  }

/*============================================================================

  request_handler_get_mime_type

============================================================================*/
const char *request_handler_get_mime_type (const char *path)
  {
  LOG_IN
  const char *ret = "application/octet-stream";
  char *p = strrchr (path, '.');
  if (p)
    {
    char *ext = strdup (p + 1); 
    int l = strlen (ext);
    for (int i = 0; i < l; i++)
      ext[i] = tolower (ext[i]); 
    if (strcmp (ext, "htm") == 0)
      ret = "text/html";
    else if (strcmp (ext, "html") == 0)
      ret = "text/html";
    else if (strcmp (ext, "gif") == 0)
      ret = "image/gif";
    else if (strcmp (ext, "png") == 0)
      ret = "image/png";
    else if (strcmp (ext, "jpeg") == 0)
      ret = "image/jpg";
    else if (strcmp (ext, "jpg") == 0)
      ret = "image/jpg";
    else if (strcmp (ext, "txt") == 0)
      ret = "text/plain";
    else if (strcmp (ext, "css") == 0)
      ret = "text/css";
    else if (strcmp (ext, "js") == 0)
      ret = "text/javascript";
    else if (strcmp (ext, "ico") == 0)
      ret = "image/x-icon";
    free (ext);
    }
  return ret;


  LOG_OUT
  }

/*============================================================================

  request_handler_get_int_file

============================================================================*/
BOOL request_handler_get_int_file (const char *uri, const uint8_t **buff, 
       size_t *size)
  { 
  LOG_IN
  *buff = NULL; 
  BOOL ret = FALSE;
  int i = 0;
  FileData *fd = &fileData[i];
  while (fd->filename && ret == FALSE) 
   {
   if (strcmp (uri, fd->filename) == 0)
     {
     ret = TRUE;
     *buff = fd->start;
     *size = fd->end - fd->start;
     }
   i++;
   fd = &fileData[i];
   }

  LOG_OUT
  return ret;
  }

/*============================================================================

  request_handler_int_file

============================================================================*/
BOOL request_handler_int_file (RequestHandler *self, const char *uri, 
        int *code, time_t if_modified, time_t *timestamp, size_t *size, 
        const uint8_t **buff, char **content_type, char **error)
  {
  LOG_IN
  log_debug ("Internal file request: %s", uri);

  BOOL ret = TRUE;
  if (request_handler_get_int_file (uri, buff, size))
    {
    if  (if_modified == 0 || if_modified <= self->build_datetime)
      {
      *content_type = strdup 
         (request_handler_get_mime_type ((char *)uri)); 
      *code = 200;
      *timestamp = time (NULL); // TODO
      }
    else
      {
      log_debug ("File not modified: %s", uri);
      *code = 304;
      *error = strdup (ERR_NOT_MODIFIED);
      ret = FALSE;
      }
    }
  else
    {
    *code = 404;
    *error = strdup (ERR_NOT_FOUND);
    ret = FALSE;
    }
  LOG_OUT
  return ret;
  }

/*============================================================================

  request_handler_ext_file

============================================================================*/
BOOL request_handler_ext_file (RequestHandler *self, const char *uri, 
        int *code, time_t if_modified, time_t *timestamp, size_t *size, 
        int *fd, char **content_type, char **error)
  {
  LOG_IN
  BOOL ret = TRUE;
  static UTF32 empty[] = {0};
  static UTF32 dotdot[] = {'.', '.', 0};

  log_debug ("External file request: %s", uri);

  Path *path = path_clone (self->root);
  path_append (path, uri);
  Path *sani_path = (Path *) wstring_substitute_all 
    ((WString *)path, dotdot, empty);
  UTF8 *s_path = path_to_utf8 (sani_path);
  log_debug ("Internal path is: %s", uri);
  if (file_is_directory ((const char *)s_path))
    {
    log_debug ("Is a directory: %s", path);
    *code = 403;
    *error = strdup (ERR_DIR_LIST);
    ret = FALSE;
    }
  else
    {
    int f = open ((const char *)s_path, O_RDONLY);
    if (f >= 0)
      {
      struct stat sb;
      fstat (f, &sb);
      *timestamp = sb.st_mtime;
      if  (if_modified == 0 || if_modified < sb.st_mtime)
        {
        *size = sb.st_size;
        *content_type = strdup 
           (request_handler_get_mime_type ((char *)s_path)); 
        *code = 200;
        *fd = f;
        ret = TRUE;
        }
      else
        {
        close (f);
        log_debug ("File not modified: %s", path);
        *code = 304;
        *error = strdup (ERR_NOT_MODIFIED);
        ret = FALSE;
        }
      }
    else
      {
      log_debug ("File not found: %s", path);
      *code = 404;
      *error = strdup (ERR_NOT_FOUND);
      ret = FALSE;
      }
    }

  free (s_path);
  path_destroy (sani_path);
  path_destroy (path);

  LOG_OUT
  return ret;
  }

/*============================================================================

  request_handler_api

============================================================================*/
void request_handler_api (RequestHandler *self, const char *uri, 
       const Props* arguments, int *code, char **page)
  {
  LOG_IN
  log_debug ("API request: %s", uri);
  api_request_handler_handle (self->api_request_handler, uri, arguments,
          code, page);
  LOG_OUT
  }

/*============================================================================

  request_handler_gui

============================================================================*/
void request_handler_gui (RequestHandler *self, const char *uri, 
       const Props *arguments, int *code, char **page)
  {
  LOG_IN
  log_debug ("GUI request: %s", uri);
  //printf ("GUI request: %s\n", uri);
  gui_request_handler_handle (self->gui_request_handler, uri, arguments,
         code, page);
  LOG_OUT
  }

/*============================================================================

  request_handler_shutdown_requested

============================================================================*/
BOOL request_handler_shutdown_requested (const RequestHandler *self) 
  {
  LOG_IN
  BOOL ret = self->shutdown_requested;
  LOG_OUT
  return ret;
  }

/*============================================================================

  request_handler_request_shutdown

============================================================================*/
void request_handler_request_shutdown (RequestHandler *self)
  {
  LOG_IN
  log_info ("Shutdown requested");
  self->shutdown_requested = TRUE;
  LOG_OUT
  }


/*============================================================================

  request_handler_get_program_context

============================================================================*/
const ProgramContext *request_handler_get_program_context 
      (const RequestHandler *self)
  {
  LOG_IN
  const ProgramContext *ret = self->context;
  LOG_OUT
  return ret;
  }

/*============================================================================

  request_handler_get_root

============================================================================*/
const Path *request_handler_get_root
      (const RequestHandler *self)
  {
  LOG_IN
  const Path *ret = self->root; 
  LOG_OUT
  return ret;
  }

/*============================================================================

  request_handler_get_index_file
  CAUTION: may return NULL

============================================================================*/
const char *request_handler_get_index_file
      (const RequestHandler *self)
  {
  LOG_IN
  const char *ret = self->index_file; 
  LOG_OUT
  return ret;
  }


