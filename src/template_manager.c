/*============================================================================

  xine-server-x 
  template_manager.c
  Copyright (c)2020 Kevin Boone
  Distributed under the terms of the GPL v3.0

============================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "defs.h" 
#include "log.h" 
#include "string.h" 
#include "template_manager.h" 

extern uint8_t generic_html_start[] asm("_binary_docroot_generic_html_start");
extern uint8_t generic_html_end[]   asm("_binary_docroot_generic_html_end");
extern uint8_t generic_top_html_start[] asm("_binary_docroot_generic_top_html_start");
extern uint8_t generic_top_html_end[]   asm("_binary_docroot_generic_top_html_end");
extern uint8_t generic_bottom_html_start[] asm("_binary_docroot_generic_bottom_html_start");
extern uint8_t generic_bottom_html_end[]   asm("_binary_docroot_generic_bottom_html_end");
extern uint8_t error_html_start[] asm("_binary_docroot_error_html_start");
extern uint8_t error_html_end[]   asm("_binary_docroot_error_html_end");
extern uint8_t files_html_start[] asm("_binary_docroot_files_html_start");
extern uint8_t files_html_end[]   asm("_binary_docroot_files_html_end");
extern uint8_t transport_html_start[] asm("_binary_docroot_transport_html_start");
extern uint8_t transport_html_end[]   asm("_binary_docroot_transport_html_end");
extern uint8_t captionmenu_html_start[] asm("_binary_docroot_captionmenu_html_start");
extern uint8_t captionmenu_html_end[]   asm("_binary_docroot_captionmenu_html_end");
extern uint8_t admin_html_start[] asm("_binary_docroot_admin_html_start");
extern uint8_t admin_html_end[]   asm("_binary_docroot_admin_html_end");
extern uint8_t playlist_html_start[] asm("_binary_docroot_playlist_html_start");
extern uint8_t playlist_html_end[] asm("_binary_docroot_playlist_html_end");
extern uint8_t radio_html_start[]   asm("_binary_docroot_radio_html_start");
extern uint8_t radio_html_end[]   asm("_binary_docroot_radio_html_end");
extern uint8_t scanner_html_start[]   asm("_binary_docroot_scanner_html_start");
extern uint8_t scanner_html_end[]   asm("_binary_docroot_scanner_html_end");
extern uint8_t scanner_top_html_start[]   asm("_binary_docroot_scanner_top_html_start");
extern uint8_t scanner_top_html_end[]   asm("_binary_docroot_scanner_top_html_end");
extern uint8_t noscanner_html_start[]   asm("_binary_docroot_noscanner_html_start");
extern uint8_t noscanner_html_end[]   asm("_binary_docroot_noscanner_html_end");
extern uint8_t albums_html_start[]   asm("_binary_docroot_albums_html_start");
extern uint8_t albums_html_end[]   asm("_binary_docroot_albums_html_end");
extern uint8_t tracks_html_start[]   asm("_binary_docroot_tracks_html_start");
extern uint8_t tracks_html_end[]   asm("_binary_docroot_tracks_html_end");
extern uint8_t browse_html_start[]   asm("_binary_docroot_browse_html_start");
extern uint8_t browse_html_end[]   asm("_binary_docroot_browse_html_end");
extern uint8_t searchres_html_start[]   asm("_binary_docroot_searchres_html_start");
extern uint8_t searchres_html_end[]   asm("_binary_docroot_searchres_html_end");
extern uint8_t search_html_start[]   asm("_binary_docroot_search_html_start");
extern uint8_t search_html_end[]   asm("_binary_docroot_search_html_end");
extern uint8_t artists_html_start[]   asm("_binary_docroot_artists_html_start");
extern uint8_t artists_html_end[]   asm("_binary_docroot_artists_html_end");
extern uint8_t genres_html_start[]   asm("_binary_docroot_genres_html_start");
extern uint8_t genres_html_end[]   asm("_binary_docroot_genres_html_end");
extern uint8_t composers_html_start[]   asm("_binary_docroot_composers_html_start");
extern uint8_t composers_html_end[]   asm("_binary_docroot_composers_html_end");

typedef struct 
  {
  int id;
  const char *tag;
  const uint8_t *start;
  const uint8_t *end;
  } TemplateData;

TemplateData templateData[] = 
  {
  { TEMPLATE_GENERIC_HTML, "generic.html", generic_html_start, generic_html_end },
  { TEMPLATE_FILES_HTML, "files.html", files_html_start, files_html_end },
  { TEMPLATE_GENERIC_TOP_HTML, "generic_top.html", generic_top_html_start, generic_top_html_end },
  { TEMPLATE_GENERIC_BOTTOM_HTML, "generic_bottom.html", generic_bottom_html_start, generic_bottom_html_end },
  { TEMPLATE_ERROR_HTML, "error.html", error_html_start, error_html_end },
  { TEMPLATE_TRANSPORT_HTML, "transport.html", transport_html_start, transport_html_end },
  { TEMPLATE_CAPTIONMENU_HTML, "captionmenu.html", captionmenu_html_start, captionmenu_html_end },
  { TEMPLATE_ADMIN_HTML, "admin.html", admin_html_start, admin_html_end },
  { TEMPLATE_PLAYLIST_HTML, "playlist.html", playlist_html_start, playlist_html_end },
  { TEMPLATE_RADIO_HTML, "radio.html", radio_html_start, radio_html_end },
  { TEMPLATE_ALBUMS_HTML, "albums.html", albums_html_start, albums_html_end },
  { TEMPLATE_TRACKS_HTML, "tracks.html", tracks_html_start, tracks_html_end },
  { TEMPLATE_BROWSE_HTML, "browse.html", browse_html_start, browse_html_end },
  { TEMPLATE_SCANNER_HTML, "scanner.html", scanner_html_start, scanner_html_end },
  { TEMPLATE_NOSCANNER_HTML, "noscanner.html", noscanner_html_start, noscanner_html_end },
  { TEMPLATE_SCANNER_TOP_HTML, "scanner_top.html", scanner_top_html_start, scanner_top_html_end },
  { TEMPLATE_SEARCH_HTML, "search.html", search_html_start, search_html_end },
  { TEMPLATE_ARTISTS_HTML, "artists.html", artists_html_start, artists_html_end },
  { TEMPLATE_GENRES_HTML, "genres.html", genres_html_start, genres_html_end },
  { TEMPLATE_COMPOSERS_HTML, "composers.html", composers_html_start, composers_html_end },
  { TEMPLATE_SEARCHRES_HTML, "searchres.html", searchres_html_start, searchres_html_end },
  { -1, NULL, NULL, NULL },
  };


/*============================================================================

 template_manager_get_template_id_by_tag

============================================================================*/
int template_manager_get_template_id_by_tag (const char * tag)
  {
  LOG_IN
  int ret = -1;

  int i = 0;
  TemplateData *td = &templateData[i];
  while (td->id >= 0 && ret < 0) 
   {
   if (strcmp (tag, td->tag) == 0)
     ret = td->id;
   i++;
   td = &templateData[i];
   }

  LOG_OUT
  return ret;
  }

/*============================================================================

 template_manager_get_template_by_tag

============================================================================*/
String *template_manager_get_template_by_tag (const char *tag)
  {
  LOG_IN
  String *ret = NULL;
  int id = template_manager_get_template_id_by_tag (tag);
  if (id >= 0)
    ret = template_manager_get_template (id);
  LOG_OUT
  return ret;
  }


/*============================================================================

 template_manager_get_fisrt_tag
 The returned start and end values are such that start-end is the length
 of the tag, including the starting and ending @@ characters. So 'end'
 indexes the first character _after_ the closing @@, which might be the
 end of the template.

============================================================================*/
static BOOL template_manager_get_first_tag (const String *s, int *start, 
        int *end)
  {
  LOG_IN
  BOOL ret = FALSE;
  const char *ss = string_cstr (s);
  const char *p = strstr (ss, "@@");
  if (p)
    {
    const char *q = strstr (p + 2, "@@");
    if (q)
      {
      *start = p - ss;
      *end = q - ss + 2;
      ret = TRUE;
      }
    else
      {
      log_warning ("Internal error: unmatched @@ in template substitution");
      }
    }
  LOG_OUT
  return ret;
  }

/*============================================================================

 template_manager_substitute_template

============================================================================*/
void template_manager_substitute_template (String *s)
  {
  int start, end;
  BOOL got_tag;
  do 
    {
    got_tag = template_manager_get_first_tag (s, &start, &end);
    if (got_tag)
      {
      const char *ss = string_cstr (s);
      int taglen = end - start;
      char *tag = malloc (end - start + 1 - 4);
      memcpy (tag, ss + start + 2, taglen - 4);
      tag [taglen - 4] = 0; 
      string_delete (s, start, (end - start));
      String *replace = template_manager_get_template_by_tag (tag);
      string_insert (s, start, string_cstr (replace));
      string_destroy (replace);
      free (tag);
      }
    } while (got_tag);
  }
 
/*============================================================================

 template_manager_get_template_data_by_id

============================================================================*/
static const TemplateData *template_manager_get_template_data_by_id (int id)
  {
  LOG_IN
  const TemplateData *ret = NULL; 

  int i = 0;
  TemplateData *td = &templateData[i];
  while (td->id >= 0 && ret == NULL) 
   {
   if (id == td->id)
     ret = td;
   i++;
   td = &templateData[i];
   }

  LOG_OUT
  return ret;
  }

/*============================================================================

 template_manager_get_template

============================================================================*/
String *template_manager_get_template (int template)
  {
  LOG_IN
  String *ret = NULL; 
  log_debug ("%s: %d", __PRETTY_FUNCTION__, template);

  const TemplateData *td = template_manager_get_template_data_by_id (template);
  const uint8_t *buff = td->start; 
  int size = td->end - td->start;;
  ret = string_create_with_len ((char *)buff, size);
  template_manager_substitute_template (ret);

  LOG_OUT
  return ret;
  }

/*============================================================================

  template_manager_substitute_placeholder

============================================================================*/
void template_manager_substitute_placeholder (String *s, 
       const char *placeholder, const char *replace)
  {
  int pl_len = strlen (placeholder) + 5;
  char *pl = malloc (pl_len);
  strcpy (pl, "%%");
  strcat (pl, placeholder);
  strcat (pl, "%%"); 
  string_substitute_all_in_place (s, pl, replace); 
  free (pl);
  }


