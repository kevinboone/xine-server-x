/*============================================================================

  xine-server-x 
  tracks_request_handler.c
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
#include "tracks_request_handler.h" 
#include "searchconstraints.h" 
#include "audio_metainfo.h" 

/*============================================================================

  tracks_request_handler_make_results_page_link

============================================================================*/
String *tracks_request_handler_make_results_page_link 
     (int from, int limit, const Props *arguments)
  {
  LOG_IN
  String *ret = string_create (GUI_BASE URI_TRACKS);
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

  tracks_request_handler_make_track_html

============================================================================*/
char *tracks_request_handler_make_track_html (const char *path)
  {
  LOG_IN

  char *error = NULL;
  char *disp_title = NULL;
  char *title = NULL;
  char *genre = NULL;
  char *artist = NULL;
  char *album = NULL;
  char *composer = NULL;
  int error_code = 0;
  String *html = string_create_empty();

  AudioMetaInfo *ami = facade_get_metainfo_from_database (path, &error_code, 
     &error);
  if (ami)
    {
    const char *_title = audio_metainfo_get_title (ami);
    if (_title) title = strdup (_title);
    const char *_genre = audio_metainfo_get_genre (ami);
    if (_genre) genre = strdup (_genre);
    const char *_artist = audio_metainfo_get_artist (ami);
    if (_artist) artist = strdup (_artist);
    const char *_composer = audio_metainfo_get_composer (ami);
    if (_composer) composer = strdup (_composer);
    const char *_album = audio_metainfo_get_album (ami);
    if (_album) album = strdup (_album);
    audio_metainfo_destroy (ami);
    }
  else
    {
    log_warning (error);
    free (error);
    }

  if (title)
    {
    disp_title = strdup (title);
    }
  else
    {
    char *path_ = strdup (path);

    char *p = strrchr (path_, PATH_SEPARATOR);
    if (p)
      {
      disp_title = strdup (p + 1);
      }
    else
      disp_title = strdup (path_);

    free (path_);

    p = strrchr (disp_title, '.');
    if (p) *p = 0;
    }

  string_append (html, "<span class=\"tracklistname\">");
  string_append (html, disp_title);
  string_append (html, "</span>\n");
  
  char *js_path = htmlutil_escape_squote_js (path);
  char *play_uri;
  asprintf (&play_uri, "javascript:cmd_play_file('%s')", js_path);
  char *play_text_link = htmlutil_make_href (play_uri, "[play]");
  char *add_uri;
  asprintf (&add_uri, "javascript:cmd_add_file('%s')", js_path);
  char *add_text_link = htmlutil_make_href (add_uri, "[add]");

  string_append (html, " ");
  string_append (html, play_text_link);
  string_append (html, add_text_link);

  string_append (html, "<br/>\n");

  if (album)
    {
    string_append (html, "<span class=\"tracklistsupinfo\">");
    string_append (html, "Album: ");
    char *esc_album = htmlutil_escape (album);
    char *album_uri;
    asprintf (&album_uri, GUI_BASE URI_ALBUMS "?album-is=%s", esc_album);
    char *show_album_link = htmlutil_make_href (album_uri, album);
    string_append (html, show_album_link);
    string_append (html, "</span>");
    string_append (html, "&nbsp;");
    string_append (html, "&nbsp;");
    free (esc_album);
    free (album_uri);
    free (show_album_link);
    }

  if (genre)
    {
    string_append (html, "<span class=\"tracklistsupinfo\">");
    string_append (html, "Genre: ");
    char *esc_genre = htmlutil_escape (genre);
    char *genre_uri;
    asprintf (&genre_uri, GUI_BASE URI_ALBUMS "?genre-is=%s", esc_genre);
    char *show_genre_link = htmlutil_make_href (genre_uri, genre);
    string_append (html, show_genre_link);
    string_append (html, "</span>");
    string_append (html, "&nbsp;");
    string_append (html, "&nbsp;");
    free (esc_genre);
    free (genre_uri);
    free (show_genre_link);
    }

  if (artist)
    {
    string_append (html, "<span class=\"tracklistsupinfo\">");
    string_append (html, "Artist: ");
    char *esc_artist = htmlutil_escape (artist);
    char *artist_uri;
    asprintf (&artist_uri, GUI_BASE URI_ALBUMS "?artist-is=%s", esc_artist);
    char *show_artist_link = htmlutil_make_href (artist_uri, artist);
    string_append (html, show_artist_link);
    string_append (html, "</span>");
    string_append (html, "&nbsp;");
    string_append (html, "&nbsp;");
    free (esc_artist);
    free (artist_uri);
    free (show_artist_link);
    }

  if (composer)
    {
    string_append (html, "<span class=\"tracklistsupinfo\">");
    string_append (html, "Composer: ");
    char *esc_composer = htmlutil_escape (composer);
    char *composer_uri;
    asprintf (&composer_uri, GUI_BASE URI_ALBUMS "?composer-is=%s", esc_composer);
    char *show_composer_link = htmlutil_make_href (composer_uri, composer);
    string_append (html, show_composer_link);
    string_append (html, "</span>");
    string_append (html, "&nbsp;");
    string_append (html, "&nbsp;");
    free (esc_composer);
    free (composer_uri);
    free (show_composer_link);
    }

  char *ret = strdup (string_cstr (html));

  if (artist) free (artist);
  if (genre) free (genre);
  if (title) free (title);
  if (composer) free (composer);
  if (album) free (album);
  free (add_uri);
  free (add_text_link);
  free (play_uri);
  free (play_text_link);
  free (js_path);
  string_destroy (html);
  free (disp_title);

  LOG_OUT
  return ret;
  }


/*============================================================================

  tracks_request_handler_tracklist

============================================================================*/
String *tracks_request_handler_track_list (List *list)
  {
  LOG_IN
  String *ret = string_create_empty();
  int l = list_length (list);
  if (l > 0) 
    {
    string_append (ret, "<div class=\"tracklist\">\n");
    for (int i = 0; i < l; i++)
      {
      const char *path = list_get (list, i);
      char *trackhtml = tracks_request_handler_make_track_html (path);
      string_append (ret, "<div class=\"tracklistcell\">");
      string_append (ret, trackhtml); 
      string_append (ret, "</div>\n");
      free (trackhtml);
      }
    string_append (ret, "</div>\n");
    }
  else
    string_append (ret, "No matching tracks");
  LOG_OUT
  return ret;
  }


/*============================================================================

  tracks_request_handler_page 

============================================================================*/
void tracks_request_handler_page (const Props *arguments, char **page)
  {
  LOG_IN

  // TODO get and parse search restriction from albums

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
  List *list = facade_get_paths (from, limit, sc, &match, 
         &error_code, &error);

  if (list)
    {
    String *generic = template_manager_get_template (TEMPLATE_TRACKS_HTML);
    template_manager_substitute_placeholder (generic, "title", "Tracks");
    // TODO others 

    if (list_length (list) > 0)
      {
      if (searchconstraints_has_constraints (sc))
        {
	String *playall_args = string_create_empty();
	List *l = props_get_keys (arguments);
	int ll = list_length (l);
	for (int i = 0; i < ll; i++)
	  {
	  const char *key = list_get (l, i);
	  const char *value = props_get (arguments, key);
	  string_append_printf (playall_args, "%s=%s", key, value);
	  if (i != ll - 1)
	    string_append (playall_args, "&");
	  }
	list_destroy (l);
	char *s_playall_args = strdup (string_cstr (playall_args));
	string_destroy (playall_args);
	char *js_args = htmlutil_escape_dquote_js (s_playall_args);
	char *href1;
	asprintf (&href1, "javascript:cmd_play_matching(\'%s\')", js_args);
	char *play_all_html = htmlutil_make_href (href1, 
	  "[play all]");
	char *href2;
	asprintf (&href2, "javascript:cmd_add_matching(\'%s\')", js_args);
	char *add_all_html = htmlutil_make_href (href2, 
	  "[add all]");
	char *addplay_all_html;
	asprintf (&addplay_all_html, "<p>%s | %s</p>\n", 
	   add_all_html, play_all_html);
	free (s_playall_args);
	free (href1);
	free (href2);
	free (js_args);

	template_manager_substitute_placeholder (generic, "playall", 
	  addplay_all_html);
	free (addplay_all_html);
	free (play_all_html);
	free (add_all_html);
        }
      else
        {
        // The REST API won't allow /api/add_matching to be called without
        //   at least one search constraint, to avoid overwhelming
        //   the server's memory. So if the search is unconstrained, 
        //   remove the "add all" link
	template_manager_substitute_placeholder (generic, "playall", ""); 
        }
 
      String *tracklist = tracks_request_handler_track_list (list); 
      template_manager_substitute_placeholder (generic, "list", 
	string_cstr (tracklist));
      string_destroy (tracklist);

      String *listnav = gui_request_handler_listnav (URI_TRACKS, 
        arguments, match, FALSE);
      template_manager_substitute_placeholder (generic, "listnav", 
	string_cstr (listnav));
      string_destroy (listnav);
      }
    else
      {
      template_manager_substitute_placeholder (generic, "list", 
         "<div><p>No tracks found</p></div>\n");
      template_manager_substitute_placeholder (generic, "listnav", "");
      template_manager_substitute_placeholder (generic, "playall", ""); 
      }

    String *summary = gui_request_handler_summary ("tracks", from, limit, 
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


