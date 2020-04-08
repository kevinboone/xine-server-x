/*============================================================================

  xine-server-x 
  gui_request_handler.c
  Copyright (c)2020 Kevin Boone
  Distributed under the terms of the GPL v3.0

============================================================================*/
#define _GNU_SOURCE
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
#include "files_request_handler.h" 
#include "albums_request_handler.h" 
#include "tracks_request_handler.h" 
#include "genres_request_handler.h" 
#include "composers_request_handler.h" 
#include "playlist_request_handler.h" 
#include "radio_request_handler.h" 
#include "artists_request_handler.h" 
#include "searchres_request_handler.h" 
#include "wstring.h" 
#include "template_manager.h" 
#include "facade.h" 
#include "htmlutil.h" 


struct _GUIRequestHandler
  {
  RequestHandler *request_handler;
  }; 

/*============================================================================

  gui_request_handler_create

============================================================================*/
GUIRequestHandler *gui_request_handler_create 
      (RequestHandler *request_handler)
  {
  LOG_IN
  GUIRequestHandler *self = malloc (sizeof (GUIRequestHandler)); 
  self->request_handler = request_handler;
  LOG_OUT 
  return self;
  }

/*============================================================================

  gui_request_handler_destroy

============================================================================*/
void gui_request_handler_destroy (GUIRequestHandler *self)
  {
  LOG_IN
  if (self)
    {
    free (self);
    }
  LOG_OUT 
  }

/*============================================================================

 gui_request_handler_error_page

============================================================================*/
void gui_request_handler_error_page (const char *text, char **page)
  {
  LOG_IN
  String *generic = template_manager_get_template (TEMPLATE_ERROR_HTML);
  template_manager_substitute_placeholder (generic, "body", text);
  template_manager_substitute_placeholder (generic, "title", "Error");
  // TODO others 
  *page = strdup (string_cstr(generic));
  string_destroy (generic);
  LOG_OUT
  }

/*============================================================================

 gui_request_handler_admin_page

============================================================================*/
void gui_request_handler_admin_page (GUIRequestHandler *self, 
           char **page)
  {
  LOG_IN
  String *generic = template_manager_get_template (TEMPLATE_ADMIN_HTML);
  //template_manager_substitute_placeholder (generic, "body", text);
  template_manager_substitute_placeholder (generic, "title", "Error");
  // TODO others 
  *page = strdup (string_cstr(generic));
  string_destroy (generic);
  LOG_OUT
  }

/*============================================================================

 gui_request_handler_search_page

============================================================================*/
void gui_request_handler_search_page (GUIRequestHandler *self, 
           char **page)
  {
  LOG_IN
  String *generic = template_manager_get_template (TEMPLATE_SEARCH_HTML);
  //template_manager_substitute_placeholder (generic, "body", text);
  template_manager_substitute_placeholder (generic, "title", "Search");
  // TODO others 
  *page = strdup (string_cstr(generic));
  string_destroy (generic);
  LOG_OUT
  }

/*============================================================================

 gui_request_handler_browse_page

============================================================================*/
void gui_request_handler_browse_page (GUIRequestHandler *self, 
           char **page)
  {
  LOG_IN
  String *generic = template_manager_get_template (TEMPLATE_BROWSE_HTML);
  //template_manager_substitute_placeholder (generic, "body", text);
  template_manager_substitute_placeholder (generic, "title", "Browse...");
  // TODO others 
  *page = strdup (string_cstr(generic));
  string_destroy (generic);
  LOG_OUT
  }

/*============================================================================

 gui_request_handler_scanner_page

============================================================================*/
void gui_request_handler_scanner_page (GUIRequestHandler *self, 
           char **page)
  {
  LOG_IN
  // TODO No index
  const Path *root = request_handler_get_root (self->request_handler);
  char *s_root = (char *)path_to_utf8 (root);
  const char *index_file = 
    request_handler_get_index_file (self->request_handler);
  String *generic;
  if (index_file)
    {
    generic = template_manager_get_template (TEMPLATE_SCANNER_HTML);
    template_manager_substitute_placeholder (generic, "title", "Scanner");
    template_manager_substitute_placeholder (generic, "root", s_root);
    template_manager_substitute_placeholder (generic, "index_file", 
       index_file);
    // TODO others 
    }
  else
    {
    generic = template_manager_get_template (TEMPLATE_NOSCANNER_HTML);
    template_manager_substitute_placeholder (generic, "title", "Scanner");
    // TODO others 
    }
  *page = strdup (string_cstr(generic));
  free (s_root);
  string_destroy (generic);
  LOG_OUT
  }


/*============================================================================

 gui_request_handler_handle 

============================================================================*/
void gui_request_handler_handle (GUIRequestHandler *self, const char *uri, 
      const Props *arguments, int *code,  char **page)
  {
  LOG_IN
  log_debug ("GUI request: %s", uri);

  if (strcmp (uri, URI_PLAYLIST) == 0)
    {
    playlist_request_handler_page (arguments, page);
    *code = 200;
    }
  else if (strcmp (uri, URI_RADIO) == 0)
    {
    radio_request_handler_page (arguments, page);
    *code = 200;
    }
  else if (strcmp (uri, URI_ALBUMS) == 0)
    {
    albums_request_handler_page (arguments, page);
    *code = 200;
    }
  else if (strcmp (uri, URI_FILES) == 0)
    {
    files_request_handler_page (arguments, page);
    *code = 200;
    }
  else if (strcmp (uri, URI_ADMIN) == 0)
    {
    gui_request_handler_admin_page (self, page);
    *code = 200;
    }
  else if (strcmp (uri, URI_BROWSE) == 0)
    {
    gui_request_handler_browse_page (self, page);
    *code = 200;
    }
  else if (strcmp (uri, URI_SCANNER) == 0)
    {
    gui_request_handler_scanner_page (self, page);
    *code = 200;
    }
  else if (strcmp (uri, URI_SEARCH) == 0)
    {
    gui_request_handler_search_page (self, page);
    *code = 200;
    }
  else if (strcmp (uri, URI_TRACKS) == 0)
    {
    tracks_request_handler_page (arguments, page);
    *code = 200;
    }
  else if (strcmp (uri, URI_ARTISTS) == 0)
    {
    artists_request_handler_page (arguments, page);
    *code = 200;
    }
  else if (strcmp (uri, URI_GENRES) == 0)
    {
    genres_request_handler_page (arguments, page);
    *code = 200;
    }
  else if (strcmp (uri, URI_COMPOSERS) == 0)
    {
    composers_request_handler_page (arguments, page);
    *code = 200;
    }
  else if (strcmp (uri, URI_SEARCHRES) == 0)
    {
    searchres_request_handler_page (self->request_handler, 
       arguments, page);
    *code = 200;
    }
  else
    {
    gui_request_handler_error_page ("Not found", page);
    *code = 404;
    }

  LOG_OUT
  }

/*============================================================================

  gui_request_handler_summary

============================================================================*/
String *gui_request_handler_summary (const char *name, int from, int limit, 
         const Props *arguments, int count, const SearchConstraints *sc)
  {
  LOG_IN;
  String *ret = string_create_empty();

  if (count > 0)
    {
    if (limit == 0)
      {
      string_append_printf (ret, "Showing all %d %s", count, name);
      }
    else
      string_append_printf (ret, "Showing %s %d to %d of %d", name, from + 1, 
        from + limit < count ? from + limit : count, count);
    }
  else
    string_append_printf (ret, "No matching %", name);

  char *where = searchconstraints_make_readable_where (sc);
  string_append (ret, where);
  free (where);

  LOG_OUT
  return ret;
  }

/*============================================================================

  gui_request_handler_make_results_page_link

============================================================================*/
String *gui_request_handler_make_results_page_link 
     (const char *uri, int from, int limit, const Props *arguments)
  {
  LOG_IN
  String *ret = string_create (GUI_BASE);
  string_append (ret, uri);
  string_append_printf (ret, "?from=%d&limit=%d", from, limit);

  List *args = props_get_keys (arguments);
  int l = list_length (args);
  for (int i = 0; i < l; i++)
    {
    const char *key = list_get (args, i);
    if (strcmp (key, "from") && strcmp (key, "limit"))
      {
      char *esc_val = htmlutil_escape (props_get (arguments, key));
      string_append_printf (ret, "&%s=%s", key, esc_val); 
      free (esc_val);
      }
    }
  list_destroy (args);

  LOG_OUT
  return ret;
  }

/*============================================================================

  gui_request_handler_listnav

============================================================================*/
String *gui_request_handler_listnav (const char *uri, const Props 
         *arguments, int count, BOOL show_all)
  {
  LOG_IN
  String *ret = string_create ("");

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

  if (limit == 0) limit = MAX_LIMIT;

  string_append (ret, 
    "<div class=\"listnav\">\nPage: ");

  int prevfrom = (from / limit - 1) * limit;
  if (prevfrom >= 0)
    {
    String *prevuri = gui_request_handler_make_results_page_link 
        (uri, prevfrom, limit, arguments);
    string_append_printf (ret, 
      "<a href=\"%s\"><span class=\"resultpagenoncurrent\">Prev</span></a>", 
      string_cstr (prevuri));
    string_destroy (prevuri);
    }

  int pages = count / limit + 1;
  for (int i = 0; i < pages; i++) 
    {
    int newfrom = i * limit;
    BOOL thispage = (newfrom == from);
    if (thispage)
      string_append_printf (ret, 
        "<span class=\"resultpagecurrent\">%d</span>\n", i + 1);
    else
      {
      string_append_printf (ret, "<a href=\"" GUI_BASE "%s", uri);
      string_append_printf (ret, "?from=%d&limit=%d", i * limit, limit);

      // Append all the existing URI arguments, other than from and
      //   limit. These will be search constraints
      List *args = props_get_keys (arguments);
      int l = list_length (args);
      for (int i = 0; i < l; i++)
        {
	const char *key = list_get (args, i);
	if (strcmp (key, "from") && strcmp (key, "limit"))
	  {
          char *esc_val = htmlutil_escape (props_get (arguments, key));
          string_append_printf (ret, "&%s=%s", key, esc_val); 
	  free (esc_val);
	  }
	}

      list_destroy (args);

      string_append_printf (ret, "\">");
      string_append_printf (ret, 
        "<span class=\"resultpagenoncurrent\">%d</span>\n", i + 1);
      string_append_printf (ret, "</a>\n");
      }
    }

  int nextfrom = (from / limit + 1) * limit;
  if (nextfrom < count)
    {
    String *nexturi = gui_request_handler_make_results_page_link 
        (uri, nextfrom, limit, arguments);
    string_append_printf (ret, 
      "<a href=\"%s\"><span class=\"resultpagenoncurrent\">Next</span></a>", 
      string_cstr (nexturi));
    string_destroy (nexturi);
    }

  if (limit != 0 && limit != MAX_LIMIT && show_all)
    {
    String *alluri = gui_request_handler_make_results_page_link 
        (uri, 0, 0, arguments);
    string_append_printf (ret, 
     "<a href=\"%s\"><span class=\"resultpagenoncurrent\">Show all</span></a>", 
      string_cstr (alluri));
    string_destroy (alluri);
    }

  string_append (ret, "</div>\n");

  LOG_OUT
  return ret;
  }



