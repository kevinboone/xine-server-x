/*============================================================================

  xine-server-x 
  genres_request_handler.c
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
#include "genres_request_handler.h" 
#include "searchconstraints.h" 

/*============================================================================

  genres_request_handler_genre_cell

============================================================================*/
static String *genres_request_handler_album_cell (const char *genre)
  {
  LOG_IN
  char *escaped_genre = htmlutil_escape (genre);

  String *ret = string_create_empty ();
  string_append (ret, "<div class=\"genrelistcell\">");
  string_append (ret, "<span class=\"genrelisttext\">");
  string_append (ret, genre);
  string_append (ret, "</span>");
  string_append (ret, "<br/>\n");
  string_append (ret, "<a href=\"" GUI_BASE URI_ALBUMS "?genre-is=");
  string_append (ret, escaped_genre);
  string_append (ret, "\">");
  string_append (ret, "Albums");
  string_append (ret, "</a>");
  string_append (ret, " | ");
  string_append (ret, "<a href=\"" GUI_BASE URI_ARTISTS "?genre-is=");
  string_append (ret, escaped_genre);
  string_append (ret, "\">");
  string_append (ret, "Artists");
  string_append (ret, "</a>");
  string_append (ret, " | ");
  string_append (ret, "<a href=\"" GUI_BASE URI_TRACKS "?genre-is=");
  string_append (ret, escaped_genre);
  string_append (ret, "\">");
  string_append (ret, "Tracks");
  string_append (ret, "</a>");
  string_append (ret, "</div>\n");

  free (escaped_genre);

  LOG_OUT
  return ret;
  }


/*============================================================================

  genres_request_handler_genrelist

============================================================================*/
String *genres_request_handler_genrelist (List *list)
  {
  LOG_IN
  String *ret;
  int l = list_length (list);
  if (l > 0)
    {
    ret = string_create ("<div class=\"genrelist\">\n");
    for (int i = 0; i < l; i++)
       {
       const char *genre = list_get (list, i);
       String *cell = genres_request_handler_album_cell (genre);
       string_append (ret, string_cstr (cell));
       string_destroy (cell);
       } 
    string_append (ret, "</div>\n");
    }
  else
    {
    ret = string_create ("No matching genres");
    }
  LOG_OUT
  return ret;
  }


/*============================================================================

  genres_request_handler_page 

============================================================================*/
void genres_request_handler_page (const Props *arguments, char **page)
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
  List *list = facade_get_genres (from, limit, sc, &match, 
         &error_code, &error);
  if (list)
    {
    String *generic = template_manager_get_template (TEMPLATE_GENRES_HTML);
    template_manager_substitute_placeholder (generic, "title", "Genres");
    // TODO others 

    if (match > 0)
      {
      String *genrelist = genres_request_handler_genrelist (list);
      String *listnav = gui_request_handler_listnav 
        (URI_GENRES, arguments, match, TRUE);

      template_manager_substitute_placeholder (generic, "list", 
        string_cstr (genrelist));

      template_manager_substitute_placeholder (generic, "listnav", 
        string_cstr (listnav));

      string_destroy (genrelist);
      string_destroy (listnav);
      }
    else
      {
      template_manager_substitute_placeholder (generic, "list", "");

      template_manager_substitute_placeholder (generic, "listnav", "");
      }

    String *summary = gui_request_handler_summary ("genres", from, limit, 
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


