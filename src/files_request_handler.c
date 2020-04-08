/*============================================================================

  xine-server-x 
  files_request_handler.c
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

/*============================================================================

 files_request_handler_make_img_html

============================================================================*/
char *files_request_handler_make_img_html (char *escaped_image_uri)
  {
  LOG_IN
  char *ret;
  asprintf (&ret, "<img class=\"dirlistimage\" src=\"%s\"/>", escaped_image_uri);
  LOG_OUT
  return ret;
  }

/*============================================================================

 files_request_handler_file_cell

 base -- the directory, relative to the filesystem media root
 name -- the name of the file within the base directory

============================================================================*/
static String *files_request_handler_file_cell (const Path *base, 
    const char *name) 
  {
  LOG_IN
  String *ret = string_create_empty();
  string_append (ret, "<div class=\"filelistcell\">");
  string_append (ret, name);
  
  // p2 is the file of interest, relative to the media root,
  //   that is, base + name
  Path *p2 = path_clone (base);
  path_append (p2, name);

  // s_p2 is base + name as a char *
  char *s_p2 = (char *)path_to_utf8 (p2);

  // js_p2 is base + name, with quote escaped to be used as a 
  //   javascript function argument
  char *js_p2 = htmlutil_escape_squote_js (s_p2);

  char *play_uri;
  asprintf (&play_uri, "javascript:cmd_play_file('%s')", js_p2);
  char *add_uri;
  asprintf (&add_uri, "javascript:cmd_add_file('%s')", js_p2);
  char *play_file_text_link = htmlutil_make_href (play_uri, "[play]");
  char *add_file_text_link = htmlutil_make_href (add_uri, "[add]");
  string_append (ret, " ");
  string_append (ret, play_file_text_link);
  string_append (ret, "&nbsp;");
  string_append (ret, add_file_text_link);

  string_append (ret, "<div>\n");

  free (play_uri);
  free (play_file_text_link);
  free (add_uri);
  free (add_file_text_link);
  free (js_p2);
  free (s_p2);
  path_destroy (p2);

  LOG_OUT
  return ret;
  }

/*============================================================================

 files_request_handler_dir_cell

 base -- the directory, relative to the filesystem media root
 name -- the name of the directory within the base directory

============================================================================*/
static String *files_request_handler_dir_cell (const Path *base, 
    const char *name) 
  {
  LOG_IN
  String *ret = string_create_empty();
  string_append (ret, "<div class=\"dirlistcell\">");

  // p2 is the directory of interest, relative to the media root,
  //   that is, base + name
  Path *p2 = path_clone (base);
  path_append (p2, name);

  // s_p2 is base + name as a char *
  char *s_p2 = (char *)path_to_utf8 (p2);

  // escaped_p2 is base + name, escaped for use in a URI
  char *escaped_p2 = htmlutil_escape (s_p2);

  // js_p2 is base + name, with quote escaped to be used as a 
  //   javascript function argument
  char *js_p2 = htmlutil_escape_squote_js (s_p2);

  // dir_expand_uri is the URI that the browser must use to expand
  //   the directory, with URI expansions in place
  char *dir_expand_uri;
  asprintf (&dir_expand_uri, "%sfiles?path=%s", GUI_BASE, escaped_p2); 
      
  // dir_expand_link is a text link that will raise the dir expand URI 
  char *dir_expand_text_link = htmlutil_make_href (dir_expand_uri, name);

  // play_dir_uri is the (javascript) URI the browser must invoke
  //   to play the folder
  char *play_dir_uri;
  asprintf (&play_dir_uri, "javascript:cmd_play_dir('%s')", js_p2);
  char *add_dir_uri;
  asprintf (&add_dir_uri, "javascript:cmd_add_dir('%s')", js_p2);
  // play_dir_text_link
  char *play_dir_text_link = htmlutil_make_href (play_dir_uri, "[play]");
  char *add_dir_text_link = htmlutil_make_href (add_dir_uri, "[add]");

  // Image URI is the URI of the associated folder image, if there
  //   is one. If there is not, we provide a default
  char *image_uri = facade_get_cover_image_for_dir (s_p2);
  if (!image_uri) 
     asprintf (&image_uri, "%s%s", INT_FILE_BASE, "default_cover.png");

  //char *escaped_image_uri = htmlutil_escape (image_uri);
  char *escaped_image_uri = strdup (image_uri); 
  char *imagehtml = files_request_handler_make_img_html (escaped_image_uri);

  char *dir_expand_image_link = htmlutil_make_href (dir_expand_uri, imagehtml);

  string_append (ret, "<span class=\"dirlisttext\">\n");
  string_append (ret, " ");
  string_append (ret, dir_expand_text_link);
  string_append (ret, " ");
  string_append (ret, play_dir_text_link);
  string_append (ret, " ");
  string_append (ret, add_dir_text_link);
  string_append (ret, "</span>");
  string_append (ret, "<p>\n");
  string_append (ret, dir_expand_image_link);
  string_append (ret, "</p>");
  string_append (ret, "</div>\n");
  
  free (play_dir_uri);
  free (add_dir_uri);
  free (js_p2);
  free (play_dir_text_link);
  free (add_dir_text_link);
  free (dir_expand_image_link);
  free (imagehtml);
  free (escaped_image_uri);
  free (image_uri);
  free (dir_expand_text_link);
  free (dir_expand_uri);
  free (escaped_p2);
  free (s_p2);
  path_destroy (p2);

  LOG_OUT
  return ret;
  }

/*============================================================================

 files_request_handler_dirlist

============================================================================*/
static String *files_request_handler_dirlist (const char *path) 
  {
  LOG_IN
  String *ret = NULL;

  int error_code = 0;
  char *error = NULL;
  List *dirlist = facade_get_dir_list (path, &error_code, &error);

  if (dirlist)
    {
    // Directory list
    Path *base = path_create (path);
    ret = string_create ("<div class=\"dirlist\">\n");
    int l = list_length (dirlist); 
    for (int i = 0; i < l; i++)
      {
      char *p = list_get (dirlist, i);
      String *cell = files_request_handler_dir_cell (base, p);
      string_append (ret, string_cstr(cell));
      string_destroy (cell);
      }
    
    string_append (ret, "</div>\n");
    list_destroy (dirlist);
    path_destroy (base);
    }
  else
    {
    ret = string_create (error);
    free (error);
    }

  LOG_OUT
  return ret;
  }

/*============================================================================

 files_request_handler_filelist

  The has_files argument is set if the path has any playable files, in
  addition to creating the display. This is so that the caller can
  know without additional work whether to display the parts of the
  page that only make sense if the path is not empty of files

============================================================================*/
static String *files_request_handler_filelist (const char *path, 
     BOOL *has_files) 
  {
  LOG_IN
  
  String *ret = NULL;
  char *error = NULL;
  int error_code;
  *has_files = FALSE;
  List *filelist = facade_get_file_list (path, &error_code, &error);
  if (filelist)
    {
    ret = string_create ("<div class=\"filelist\">\n");
    Path *base = path_create (path);
    int l = list_length (filelist); 
    if (l > 0) *has_files = TRUE;
    for (int i = 0; i < l; i++)
      {
      char *p = list_get (filelist, i);
      String *cell = files_request_handler_file_cell (base, p);
      string_append (ret, string_cstr(cell));
      string_destroy (cell);
      }

    string_append (ret, "</div>\n");
    list_destroy (filelist);
    path_destroy (base);
    }
  else
    {
    ret = string_create (error);
    free (error);
    }

  LOG_OUT
  return ret;
  }

/*============================================================================

 files_request_handler_dir_image

============================================================================*/
static String *files_request_handler_dir_image (const char *path) 
  {
  LOG_IN
  String *ret = string_create_empty();

  char *image_uri = facade_get_cover_image_for_dir (path);
  if (!image_uri) 
       asprintf (&image_uri, "%s%s", INT_FILE_BASE, "default_cover.png");
  string_append (ret, "<img class=\"filepageimage\" src=\"");
  string_append (ret, image_uri);
  string_append (ret, "\">\n");
  free (image_uri);

  LOG_OUT
  return ret;
  }

/*============================================================================

 files_request_handler_play_all_link

============================================================================*/
static String *files_request_handler_play_all_link (const char *path) 
  {
  LOG_IN
  String *ret = string_create_empty();

  char *js_path = htmlutil_escape_squote_js (path);

  // play_dir_uri is the (javascript) URI the browser must invoke
  //   to play the folder
  char *play_dir_uri;
  asprintf (&play_dir_uri, "javascript:cmd_play_dir('%s')", js_path);
  // play_dir_text_link
  char *play_dir_text_link = htmlutil_make_href (play_dir_uri, "[play all]");
  string_append (ret, play_dir_text_link);

  free (play_dir_text_link);
  free (js_path);
  free (play_dir_uri);

  LOG_OUT
  return ret;
  }

/*============================================================================

 files_request_handler_add_all_link

============================================================================*/
static String *files_request_handler_add_all_link (const char *path) 
  {
  LOG_IN
  String *ret = string_create_empty();

  char *js_path = htmlutil_escape_squote_js (path);

  // play_dir_uri is the (javascript) URI the browser must invoke
  //   to play the folder
  char *add_dir_uri;
  asprintf (&add_dir_uri, "javascript:cmd_add_dir('%s')", js_path);
  // play_dir_text_link
  char *add_dir_text_link = htmlutil_make_href (add_dir_uri, "[add all]");
  string_append (ret, add_dir_text_link);

  free (add_dir_text_link);
  free (js_path);
  free (add_dir_uri);

  LOG_OUT
  return ret;
  }

/*============================================================================

  files_request_handler_page 

============================================================================*/
void files_request_handler_page (const Props *arguments, char **page)
  {
  LOG_IN
  const char *path = props_get (arguments, "path");
  if (!path)
    path = "/";

  String *generic = template_manager_get_template (TEMPLATE_FILES_HTML);

  template_manager_substitute_placeholder (generic, "path", path);
  
  /*
  String *body = files_request_handler_body (path);
  template_manager_substitute_placeholder (generic, "body", 
     string_cstr (body));
  string_destroy (body);
  */
  
  String *dirlist = files_request_handler_dirlist (path);
  template_manager_substitute_placeholder (generic, "dirlist", 
     string_cstr (dirlist));
  string_destroy (dirlist);

  BOOL has_files = FALSE;
  String *filelist = files_request_handler_filelist (path, &has_files);
  if (has_files)
    {
    template_manager_substitute_placeholder (generic, "filelist", 
       string_cstr (filelist));
  
    String *dirimage = files_request_handler_dir_image (path);
    template_manager_substitute_placeholder (generic, "dirimage", 
      string_cstr (dirimage));
    string_destroy (dirimage);
  
    String *playalllink = files_request_handler_play_all_link (path);
    template_manager_substitute_placeholder (generic, "playalllink", 
       string_cstr (playalllink));
    string_destroy (playalllink);
    String *addalllink = files_request_handler_add_all_link (path);
    template_manager_substitute_placeholder (generic, "addalllink", 
       string_cstr (addalllink));
    string_destroy (addalllink);
    }
  else
    {
    template_manager_substitute_placeholder (generic, "filelist", ""); 
    template_manager_substitute_placeholder (generic, "dirimage", ""); 
    template_manager_substitute_placeholder (generic, "playalllink", ""); 
    }
  if (filelist) string_destroy (filelist);

  template_manager_substitute_placeholder (generic, "title", "Files");
  // TODO others 

  *page = strdup (string_cstr (generic));
  string_destroy (generic);
  
  LOG_OUT
  }


