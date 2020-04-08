/*============================================================================

  xine-server-x 
  composers_request_handler.c
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
#include "composers_request_handler.h" 
#include "searchconstraints.h" 

/*============================================================================

  composers_request_handler_composer_cell

============================================================================*/
static String *composers_request_handler_album_cell (const char *composer)
  {
  LOG_IN
  char *escaped_composer = htmlutil_escape (composer);

  String *ret = string_create_empty ();
  string_append (ret, "<div class=\"composerlistcell\">");
  string_append (ret, "<span class=\"composerlisttext\">");
  string_append (ret, composer);
  string_append (ret, "</span>");
  string_append (ret, "<br/>\n");
  string_append (ret, "<a href=\"" GUI_BASE URI_ALBUMS "?composer-is=");
  string_append (ret, escaped_composer);
  string_append (ret, "\">");
  string_append (ret, "Albums");
  string_append (ret, "</a>");
  string_append (ret, " | ");
  string_append (ret, "<a href=\"" GUI_BASE URI_GENRES "?composer-is=");
  string_append (ret, escaped_composer);
  string_append (ret, "\">");
  string_append (ret, "Genres");
  string_append (ret, "</a>");
  string_append (ret, " | ");
  string_append (ret, "<a href=\"" GUI_BASE URI_TRACKS "?composer-is=");
  string_append (ret, escaped_composer);
  string_append (ret, "\">");
  string_append (ret, "Tracks");
  string_append (ret, "</a>");
  string_append (ret, "</div>\n");

  free (escaped_composer);

  LOG_OUT
  return ret;
  }


/*============================================================================

  composers_request_handler_composerlist

============================================================================*/
String *composers_request_handler_composerlist (List *list)
  {
  LOG_IN
  String *ret;
  int l = list_length (list);
  if (l > 0)
    {
    ret = string_create ("<div class=\"composerlist\">\n");
    for (int i = 0; i < l; i++)
       {
       const char *composer = list_get (list, i);
       String *cell = composers_request_handler_album_cell (composer);
       string_append (ret, string_cstr (cell));
       string_destroy (cell);
       } 
    string_append (ret, "</div>\n");
    }
  else
    {
    ret = string_create ("No matching composers");
    }
  LOG_OUT
  return ret;
  }

/*============================================================================

  composers_request_handler_page 

============================================================================*/
void composers_request_handler_page (const Props *arguments, char **page)
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
  List *list = facade_get_composers (from, limit, sc, &match, 
         &error_code, &error);
  if (list)
    {
    String *generic = template_manager_get_template (TEMPLATE_COMPOSERS_HTML);
    template_manager_substitute_placeholder (generic, "title", "Composers");
    // TODO others 

    if (match > 0)
      {
      String *composerlist = composers_request_handler_composerlist (list);
      String *listnav = gui_request_handler_listnav (URI_COMPOSERS, 
        arguments, match, TRUE);

      template_manager_substitute_placeholder (generic, "list", 
        string_cstr (composerlist));
      template_manager_substitute_placeholder (generic, "listnav", 
        string_cstr (listnav));

      string_destroy (composerlist);
      string_destroy (listnav);
      }
    else
      {
      template_manager_substitute_placeholder (generic, "list", "");
      template_manager_substitute_placeholder (generic, "listnav", "");
      template_manager_substitute_placeholder (generic, "playall", "");
      }

    String *summary = gui_request_handler_summary ("composers", 
      from, limit, arguments, match, sc);
    template_manager_substitute_placeholder (generic, "summary", 
      string_cstr (summary));

    *page = strdup (string_cstr (generic));
    list_destroy (list);
    string_destroy (summary);
    string_destroy (generic);
    }
  else
    {
    gui_request_handler_error_page (error, page);
    free (error);
    }

  searchconstraints_destroy (sc);

  LOG_OUT
  }


