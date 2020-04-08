/*============================================================================

  xine-server-x 
  searchres_request_handler.c
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
#include "tracks_request_handler.h" 
#include "searchres_request_handler.h" 
#include "searchconstraints.h" 
#include "audio_metainfo.h" 

/*============================================================================

  searchres_request_handler_page 

============================================================================*/
void searchres_request_handler_page (const RequestHandler *request_handler, 
        const Props *arguments, char **page)
  {
  LOG_IN

  if (request_handler_get_index_file (request_handler))
    {
    const char *search = props_get (arguments, "search");
    if (search)
      {
      char *esc_search = htmlutil_escape (search);
      String *generic = template_manager_get_template
         (TEMPLATE_SEARCHRES_HTML);
      template_manager_substitute_placeholder 
	 (generic, "title", "Search results");
      template_manager_substitute_placeholder 
	 (generic, "search", search);
      // TODO others 
     
      int dummy;
      int match;
      char *error_message = NULL;
       
      SearchConstraints *sc = searchconstraints_create_exhaustive (search);
      char *uri_args; 
      asprintf (&uri_args, "album-contains=%s&artist-contains=%s"
          "&title-contains=%s"
           "&genre-contains=%s&composer-contains=%s&disjunct=1", 
           esc_search, esc_search, esc_search, esc_search, esc_search);
      
      List *albumlist = facade_get_albums (0, DEF_SEARCHRES, sc, 
        &match, &dummy, &error_message);

      if (albumlist)
        {
        String *s_albumlist = albums_request_handler_albumlist (albumlist);
        template_manager_substitute_placeholder 
	   (generic, "album_matches", string_cstr(s_albumlist));
	string_destroy (s_albumlist);
        list_destroy (albumlist);
        if (match >= DEF_SEARCHRES)
          {
          char *album_search_uri;
          asprintf (&album_search_uri, GUI_BASE URI_ALBUMS "?%s",
            uri_args);
          char *album_morelink = htmlutil_make_href 
                  (album_search_uri, "More...");
          template_manager_substitute_placeholder 
	    (generic, "album_morelink", album_morelink);
          free (album_search_uri);
          free (album_morelink);
          }
        else
          template_manager_substitute_placeholder 
	    (generic, "album_morelink", "");
	}
      else
        {
        template_manager_substitute_placeholder 
	   (generic, "album_matches", error_message);
        template_manager_substitute_placeholder 
	   (generic, "album_morelink", "");
        log_error (error_message);
	free (error_message);
	}
      
      List *tracklist = facade_get_paths (0, DEF_SEARCHRES, sc, 
        &match, &dummy, &error_message);

      if (tracklist)
        {
        String *s_tracklist = tracks_request_handler_track_list (tracklist);
        template_manager_substitute_placeholder 
	   (generic, "track_matches", string_cstr(s_tracklist));
	string_destroy (s_tracklist);
        list_destroy (tracklist);
        if (match >= DEF_SEARCHRES)
          {
          char *track_search_uri;
          asprintf (&track_search_uri, GUI_BASE URI_TRACKS "?%s",
            uri_args);
          char *track_morelink = htmlutil_make_href 
                  (track_search_uri, "More...");
          template_manager_substitute_placeholder 
	    (generic, "track_morelink", track_morelink);
          free (track_search_uri);
          free (track_morelink);
          }
        else
          template_manager_substitute_placeholder 
	    (generic, "track_morelink", "");
	}
      else
        {
        template_manager_substitute_placeholder 
	   (generic, "track_matches", error_message);
        template_manager_substitute_placeholder 
	   (generic, "track_morelink", "");
        log_error (error_message);
	free (error_message);
	}
      
      *page = strdup (string_cstr (generic));
      string_destroy (generic);
      free (uri_args);
      free (esc_search);
      searchconstraints_destroy (sc);
      }
    else
      {
      String *generic = template_manager_get_template (TEMPLATE_SEARCH_HTML);
      template_manager_substitute_placeholder 
	 (generic, "title", "Search");
      *page = strdup (string_cstr (generic));
      string_destroy (generic);
      }
    }
  else
    {
    gui_request_handler_error_page 
       ("Search functionality requires an index to be specified", page);
    }

  LOG_OUT
  }

