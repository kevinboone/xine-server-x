/*============================================================================

  xine-server-x 
  radio_request_handler.c
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
#include "program_context.h" 
#include "facade.h" 
#include "template_manager.h" 
#include "htmlutil.h" 
#include "radio_request_handler.h" 


/*============================================================================

  radio_request_handler_stations

============================================================================*/
String *radio_request_handler_stations (const char *file)
  {
  LOG_IN
  String *ret = string_create_empty();

  int error_code = 0;
  char *error_message = NULL;
  List *entries = facade_get_radio_station_names
          (file, &error_code, &error_message);

  if (error_code == 0)
     {
     int l = list_length (entries);
     if (l > 0)
       {
       string_append (ret, "<table id=\"stationlisttable\">\n");

       for (int i = 0; i < l; i++)
         {
         const char *entry = list_get (entries, i);
	 string_append (ret, "<tr><td>");
         char *escaped_file = htmlutil_escape_sdquote_js (file); 
         char *escaped_entry = htmlutil_escape_sdquote_js (entry); 
	 string_append_printf (ret, 
             "<a href=\"javascript:cmd_play_station('%s','%s')\">", 
             escaped_file, escaped_entry);
	 string_append (ret, entry);
	 string_append (ret, "</a>");
         free (escaped_file);
         free (escaped_entry);
	 string_append (ret, "</td></tr>\n");
	 }

       string_append (ret, "</table>\n");
       }
     else
       {
       string_append (ret, "No radio station files");
       }
     list_destroy (entries);
     }
  else
     {
     string_append_printf (ret, "Can't show radio stations: %s\n", 
       error_message);
     free (error_message);
     }

  LOG_OUT
  return ret;
  }

/*============================================================================

  radio_request_handler_file_list

============================================================================*/
String *radio_request_handler_file_list ()
  {
  LOG_IN
  String *ret = string_create_empty();

  int error_code = 0;
  char *error_message = NULL;
  List *entries = facade_get_radio_station_lists
          (&error_code, &error_message);

  if (error_code == 0)
     {
     int l = list_length (entries);
     if (l > 0)
       {
       string_append (ret, "<p>Available station list files:</p>\n");
       string_append (ret, "<table id=\"stationfiletable\">\n");

       for (int i = 0; i < l; i++)
         {
         const char *entry = list_get (entries, i);
	 string_append (ret, "<tr><td>");
	 string_append (ret, "<a href=\"/gui/radio?file=");
         char *escaped_name = htmlutil_escape (entry); 
	 string_append_printf (ret, escaped_name); 
         free (escaped_name);
	 string_append (ret, "\">");
	 string_append (ret, entry);
	 string_append (ret, "</a>");
	 string_append (ret, "</td></tr>\n");
	 }

       string_append (ret, "</table>\n");
       }
     else
       {
       string_append (ret, "No radio station files");
       }
     list_destroy (entries);
     }
  else
     {
     string_append_printf (ret, "Can't show radio station files: %s\n", 
       error_message);
     free (error_message);
     }


  LOG_OUT
  return ret;
  }

/*============================================================================

  radio_request_handler_page 

============================================================================*/
void radio_request_handler_page (const Props *arguments, char **page)
  {
  LOG_IN

  String *generic = template_manager_get_template (TEMPLATE_RADIO_HTML);
  String *body;

  const char *file = props_get (arguments, "file");
  if (file)
    {
    // Display stations in the file
    body = radio_request_handler_stations (file);
    }
  else
    {
    body = radio_request_handler_file_list ();
    }

  template_manager_substitute_placeholder (generic, "body", 
    string_cstr (body)); 
  template_manager_substitute_placeholder (generic, "title", "Playlist");
  // TODO others 

  *page = strdup (string_cstr (generic));
  string_destroy (body);
  string_destroy (generic);
  
  LOG_OUT
  }


