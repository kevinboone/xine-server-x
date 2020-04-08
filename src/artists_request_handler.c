/*============================================================================

  xine-server-x 
  artists_request_handler.c
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
#include "artists_request_handler.h" 
#include "searchconstraints.h" 

/*============================================================================

  artists_request_handler_artist_cell

============================================================================*/
static String *artists_request_handler_album_cell (const char *artist)
  {
  LOG_IN
  char *escaped_artist = htmlutil_escape (artist);

  String *ret = string_create_empty ();
  string_append (ret, "<div class=\"artistlistcell\">");
  string_append (ret, "<span class=\"artistlisttext\">");
  string_append (ret, artist);
  string_append (ret, "</span>");
  string_append (ret, "<br/>\n");
  string_append (ret, "<a href=\"" GUI_BASE URI_ALBUMS "?artist-is=");
  string_append (ret, escaped_artist);
  string_append (ret, "\">");
  string_append (ret, "Albums");
  string_append (ret, "</a>");
  string_append (ret, " | ");
  string_append (ret, "<a href=\"" GUI_BASE URI_GENRES "?artist-is=");
  string_append (ret, escaped_artist);
  string_append (ret, "\">");
  string_append (ret, "Genres");
  string_append (ret, "</a>");
  string_append (ret, " | ");
  string_append (ret, "<a href=\"" GUI_BASE URI_TRACKS "?artist-is=");
  string_append (ret, escaped_artist);
  string_append (ret, "\">");
  string_append (ret, "Tracks");
  string_append (ret, "</a>");
  string_append (ret, "</div>\n");

  free (escaped_artist);

  LOG_OUT
  return ret;
  }


/*============================================================================

  artists_request_handler_artistlist

============================================================================*/
String *artists_request_handler_artistlist (List *list)
  {
  LOG_IN
  String *ret;
  int l = list_length (list);
  if (l > 0)
    {
    ret = string_create ("<div class=\"artistlist\">\n");
    for (int i = 0; i < l; i++)
       {
       const char *artist = list_get (list, i);
       String *cell = artists_request_handler_album_cell (artist);
       string_append (ret, string_cstr (cell));
       string_destroy (cell);
       } 
    string_append (ret, "</div>\n");
    }
  else
    {
    ret = string_create ("No matching artists");
    }
  LOG_OUT
  return ret;
  }


/*============================================================================

  artists_request_handler_page 

============================================================================*/
void artists_request_handler_page (const Props *arguments, char **page)
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
  List *list = facade_get_artists (from, limit, sc, &match, 
         &error_code, &error);
  if (list)
    {
    String *generic = template_manager_get_template (TEMPLATE_ARTISTS_HTML);
    template_manager_substitute_placeholder (generic, "title", "Artists");
    // TODO others 

    if (match > 0) 
      {
      String *artistlist = artists_request_handler_artistlist (list);
      String *listnav = gui_request_handler_listnav (URI_ARTISTS, 
        arguments, match, TRUE);

      template_manager_substitute_placeholder (generic, "list", 
        string_cstr (artistlist));
      template_manager_substitute_placeholder (generic, "listnav", 
        string_cstr (listnav));
      
      string_destroy (artistlist);
      string_destroy (listnav);
      }
    else
      {
      template_manager_substitute_placeholder (generic, "list", "");
      template_manager_substitute_placeholder (generic, "listnav", "");
      }

    String *summary = gui_request_handler_summary ("artists", 
       from, limit, arguments, match, sc);
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


