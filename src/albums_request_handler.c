/*============================================================================

  xine-server-x 
  albums_request_handler.c
  Copyright (c)2020 Kevin Boone
  Distributed under the terms of the GPL v3.0

============================================================================*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "defs.h" 
#include "log.h" 
#include "props.h" 
#include "path.h" 
#include "facade.h" 
#include "template_manager.h" 
#include "htmlutil.h" 
#include "gui_request_handler.h" 
#include "albums_request_handler.h" 
#include "searchconstraints.h" 

/*============================================================================

  albums_request_handler_make_img_html

============================================================================*/
static char *albums_request_handler_make_img_html (const char *image_uri)
  {
  LOG_IN
  char *ret;
  asprintf (&ret, "<img class=\"albumlistimage\" src=\"%s\"/>", 
     image_uri);
  LOG_OUT
  return ret;
  }

/*============================================================================

  albums_request_handler_album_cell

============================================================================*/
static String *albums_request_handler_album_cell (const char *album)
  {
  LOG_IN
  char *escaped_album = htmlutil_escape (album);

  char *album_expand_uri;
  asprintf (&album_expand_uri, "%s" URI_TRACKS "?album-is=%s", 
    GUI_BASE, escaped_album); 
      
  char *image_uri = facade_get_cover_image_for_album (album); 
  if (!image_uri) 
     asprintf (&image_uri, "%s%s", INT_FILE_BASE, "default_cover.png");
  char *imagehtml = albums_request_handler_make_img_html (image_uri);

  char *album_expand_image_link = htmlutil_make_href 
           (album_expand_uri, imagehtml);

  char *album_expand_text_link = htmlutil_make_href (album_expand_uri, album);

  char *js_album = htmlutil_escape_squote_js (album);
  char *play_album_uri;
  asprintf (&play_album_uri, "javascript:cmd_play_album('%s')", js_album);

  char *play_album_text_link = htmlutil_make_href (play_album_uri, "[play]");

  String *ret = string_create ("<div class=\"albumlistcell\">");
  string_append (ret, "<span class=\"albumlisttext\">\n");

  string_append (ret, album_expand_text_link);
  string_append (ret, " ");
  string_append (ret, play_album_text_link);

  string_append (ret, "</span>\n");
  string_append (ret, "<p>\n");
  string_append (ret, album_expand_image_link);
  string_append (ret, "</p>");

  string_append (ret, "</div>\n");

  free (js_album);
  free (play_album_uri);
  free (play_album_text_link);
  free (album_expand_text_link);
  free (image_uri);
  free (imagehtml);
  free (escaped_album);
  free (album_expand_uri);
  free (album_expand_image_link);

  LOG_OUT
  return ret;
  }


/*============================================================================

  albums_request_handler_albumlist

============================================================================*/
String *albums_request_handler_albumlist (List *list)
  {
  LOG_IN
  String *ret;
  int l = list_length (list);
  if (l > 0)
    {
    ret = string_create ("<div class=\"albumlist\">\n");
    for (int i = 0; i < l; i++)
       {
       const char *album = list_get (list, i);
       String *cell = albums_request_handler_album_cell (album);
       string_append (ret, string_cstr (cell));
       string_destroy (cell);
       } 
    string_append (ret, "</div>\n");
    }
  else
    {
    ret = string_create ("No matching albums");
    }
  LOG_OUT
  return ret;
  }

/*============================================================================

  albums_request_handler_page 

============================================================================*/
void albums_request_handler_page (const Props *arguments, char **page)
  {
  LOG_IN

  const char *s_from = props_get (arguments, "from");
  int from;
  if (s_from) 
    from = atoi (s_from);
  else
    from = 0;

  const char *s_limit = props_get (arguments, "limit");
  int limit;
  if (s_limit) 
    limit = atoi (s_limit);
  else
    limit = DEF_BROWSE_PER_PAGE; 

  SearchConstraints *sc = searchconstraints_create_from_args (arguments);
  
  char *error = NULL;
  int error_code = 0;
  int match = 0;
  List *list = facade_get_albums (from, limit, sc, &match, 
         &error_code, &error);
  if (list)
    {
    String *generic = template_manager_get_template (TEMPLATE_ALBUMS_HTML);
    template_manager_substitute_placeholder (generic, "title", "Albums");
    // TODO others 

    if (match > 0)
      {
      String *albumlist = albums_request_handler_albumlist (list);
      String *listnav = gui_request_handler_listnav (URI_ALBUMS, 
         arguments, match, TRUE);

      template_manager_substitute_placeholder (generic, "list", 
	string_cstr (albumlist));

      template_manager_substitute_placeholder (generic, "listnav", 
	string_cstr (listnav));

      string_destroy (albumlist);
      string_destroy (listnav);
      }
    else
      {
      template_manager_substitute_placeholder (generic, "list", 
        "<p>No albums found</p>");
      template_manager_substitute_placeholder (generic, "listnav", "");
      }

    String *summary = gui_request_handler_summary ("albums", from, limit, 
        arguments, match, sc);
    template_manager_substitute_placeholder (generic, "summary", 
      string_cstr (summary));
    string_destroy (summary);

    *page = strdup (string_cstr (generic));
    string_destroy (generic);
    list_destroy (list);
    }
  else
    {
    gui_request_handler_error_page (error, page);
    free (error);
    }

  searchconstraints_destroy (sc);

  LOG_OUT
  }


