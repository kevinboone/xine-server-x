/*============================================================================

  xine-server-x 
  playlist_request_handler.c
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
#include "playlist_request_handler.h" 


/*============================================================================

  playlist_request_handler_page 

============================================================================*/
void playlist_request_handler_page (const Props *arguments, char **page)
  {
  LOG_IN

  String *generic = template_manager_get_template (TEMPLATE_PLAYLIST_HTML);

  int error_code = 0;
  char *error_message = NULL;
  List *entries = facade_get_playlist (&error_code, &error_message);
  String *table = string_create_empty();

  if (error_code == 0)
     {
     int l = list_length (entries);
     if (l > 0)
       {
       string_append (table, "<table id=\"playlisttable\">\n");

       for (int i = 0; i < l; i++)
         {
         const char *entry = list_get (entries, i);
	 string_append (table, "<tr><td>");
	 string_append (table, "<a href=\"");
	 string_append_printf (table, "javascript:cmd_play_index(%d)", i);
	 string_append (table, "\">");
	 string_append_printf (table, "[%d] ", i + 1);
	 string_append (table, entry);
	 string_append (table, "</a>");
	 string_append (table, "</td></tr>\n");
	 }

       string_append (table, "</table>\n");
       template_manager_substitute_placeholder (generic, "clearlink", 
         "<p><a href=\"javascript:cmd_clear()\">[clear]</a></p>\n"); 
       }
     else
       {
       string_append (table, "Playlist is empty");
       template_manager_substitute_placeholder (generic, "clearlink", ""); 
       }
     list_destroy (entries);
     }
  else
     {
     string_append_printf (table, "Can't show playlist: %s\n", error_message);
     free (error_message);
     }

  template_manager_substitute_placeholder (generic, "playlist", 
    string_cstr (table)); 
  template_manager_substitute_placeholder (generic, "title", "Playlist");
  // TODO others 

  *page = strdup (string_cstr (generic));
  string_destroy (table);
  string_destroy (generic);
  
  LOG_OUT
  }


