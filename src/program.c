/*==========================================================================

  boilerplate
  program.c
  Copyright (c)2020 Kevin Boone
  Distributed under the terms of the GPL v3.0

==========================================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <wchar.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <microhttpd.h>
#include <signal.h>
#include "props.h" 
#include "program_context.h" 
#include "program.h" 
#include "request_handler.h" 
#include "httputil.h" 
#include "facade.h" 
#include "xine-server-x-api.h" 


/*============================================================================

  program_header_iterator

============================================================================*/
static int program_header_iterator (void *data, enum MHD_ValueKind kind, 
        const char *key, const char *value)
  {
  LOG_IN
  Props *headers = (Props *) data;
  props_put (headers, key, value);
  LOG_OUT
  return MHD_YES;
  }

/*============================================================================

  program_argument_iterator

============================================================================*/
static int program_argument_iterator (void *data, enum MHD_ValueKind kind, 
        const char *key, const char *value)
  {
  LOG_IN
  Props *headers = (Props *) data;
  if (value) // Avoid trying to store a null header value
    props_put (headers, key, value);
  LOG_OUT
  return MHD_YES;
  }

/*============================================================================

  program_handle_request 

============================================================================*/
int program_handle_request (void *_request_handler, 
      struct MHD_Connection *connection, const char *url,
      const char *method, const char *version, const char *upload_data,
      size_t *upload_data_size, void **con_cls)
  {
  LOG_IN
  int ret = MHD_YES;
  RequestHandler *request_handler = (RequestHandler*) _request_handler;

  log_debug ("request: %s", url);

  Props *headers = props_create();
  MHD_get_connection_values (connection, MHD_HEADER_KIND, 
       program_header_iterator, headers);

  Props *arguments = props_create();
  MHD_get_connection_values (connection, MHD_GET_ARGUMENT_KIND, 
       program_argument_iterator, arguments);

  if (strlen (url) == 0 || strcmp (url, "/") == 0)
    {
    const ProgramContext *context = request_handler_get_program_context 
        (request_handler);
    if (program_context_get (context, "index"))
      ret = program_handle_request (request_handler, 
        connection, "/gui/albums",
        method, version, upload_data,
        upload_data_size, con_cls);
    else
      ret = program_handle_request (request_handler, 
        connection, "/gui/files",
        method, version, upload_data,
        upload_data_size, con_cls);
    }
  else if (strncmp (url, EXT_FILE_BASE, 5) == 0) // TODO
    {
    time_t if_modified = 0;

    const char *s_if_modified = props_get (headers, "If-Modified-Since");
    if (s_if_modified)
      {
      httputil_parse_http_time (s_if_modified, &if_modified);
      }

    char *error = NULL;
    struct MHD_Response *response;
    size_t size;
    int code;
    int fd;
    time_t timestamp;
    char *content_type;
    if (request_handler_ext_file (request_handler, url + 4, &code, if_modified,
          &timestamp, &size, &fd, &content_type, &error))
      {
      response = MHD_create_response_from_fd (size, fd);
      MHD_add_response_header (response, "Content-Type", content_type);
      MHD_add_response_header (response, "Cache-Control", "max-age=0");
      MHD_add_response_header (response, "Cache-Control", "public");
      char last_modified [40];
      httputil_make_http_time (timestamp, last_modified, 
        sizeof (last_modified) - 1);
      MHD_add_response_header (response, "Last-Modified", last_modified);
      ret = MHD_queue_response (connection, code, response);
      free (content_type);
      MHD_destroy_response (response);
      }
    else
      {
      response = MHD_create_response_from_buffer (strlen (error),
         (void*) error, MHD_RESPMEM_MUST_FREE);
      MHD_add_response_header (response, "Content-Type", "text/plain");
      ret = MHD_queue_response (connection, code, response);
      MHD_destroy_response (response);
      }
    }
  else if (strncmp (url, API_BASE, 5) == 0) // TODO
    {
    struct MHD_Response *response;

    char *page;
    int code;
    request_handler_api (request_handler, url + 5, arguments, &code, &page);
    response = MHD_create_response_from_buffer (strlen (page),
         (void*) page, MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header (response, "Content-Type", 
            "application/json; charset=utf8");
    MHD_add_response_header (response, "Cache-Control", "no-cache");
    ret = MHD_queue_response (connection, code, response);
    MHD_destroy_response (response);
    }
  else if (strncmp (url, INT_FILE_BASE, 5) == 0) // TODO
    {
    time_t if_modified = 0;

    int code;
    time_t timestamp;
    char *content_type;
    struct MHD_Response *response;
    const uint8_t *buff;
    char *error;
    size_t size;

    const char *s_if_modified = props_get (headers, "If-Modified-Since");
    if (s_if_modified)
      {
      httputil_parse_http_time (s_if_modified, &if_modified);
      }

    if (request_handler_int_file (request_handler, url + 5, 
        &code, if_modified, &timestamp, &size, 
        &buff, &content_type, &error))
      {
      response = MHD_create_response_from_buffer (size,
           (void*) buff, MHD_RESPMEM_PERSISTENT);
      MHD_add_response_header (response, "Content-Type", content_type);
      MHD_add_response_header (response, "Cache-Control", "max-age=0");
      MHD_add_response_header (response, "Cache-Control", "public");
      char last_modified [40];
      httputil_make_http_time (timestamp, last_modified, 
        sizeof (last_modified) - 1);
      MHD_add_response_header (response, "Last-Modified", last_modified);
      ret = MHD_queue_response (connection, code, response);
      MHD_destroy_response (response);
      free (content_type);
      }
    else
      {
      response = MHD_create_response_from_buffer (strlen (error),
         (void*) error, MHD_RESPMEM_MUST_FREE);
      MHD_add_response_header (response, "Content-Type", "text/plain");
      ret = MHD_queue_response (connection, code, response);
      MHD_destroy_response (response);
      }
    }
  else if (strncmp (url, GUI_BASE, 5) == 0) // TODO
    {
    struct MHD_Response *response;
    char *buff;
    int code;

    request_handler_gui (request_handler, url + 5, arguments, &code, &buff);
    response = MHD_create_response_from_buffer (strlen (buff),
           (void*) buff, MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header (response, "Content-Type", 
            "text/html; charset=utf8");
    MHD_add_response_header (response, "Cache-Control", "no-cache");
    ret = MHD_queue_response (connection, code, response);
    MHD_destroy_response (response);
    }
  else
    {
    struct MHD_Response *response;
    const char *page = 
      "Bad request: URI must start with /int/, /ext/, or /api/\n";
    response = MHD_create_response_from_buffer (strlen (page),
       (void*) page, MHD_RESPMEM_PERSISTENT);

    ret = MHD_queue_response (connection, MHD_HTTP_BAD_REQUEST, response);
    MHD_destroy_response (response);
    }

  props_destroy (headers);
  props_destroy (arguments);
  LOG_OUT
  return ret;
  }


/*==========================================================================

  program_launch_xine_server

==========================================================================*/
void program_launch_xine_server (const char *command)
  {
  LOG_IN
  log_info ("Launching xine-server");

  String *s_command = string_create (command);
  List *args = string_tokenize (s_command);
  int argc = list_length (args);
  char **argv = malloc ((argc + 1) * sizeof (char *));
  
  for (int i = 0; i < argc; i++)
    {
    argv[i] = strdup (string_cstr (list_get (args, i)));
    }

  argv[argc] = NULL;

  int pid;
  if ((pid = fork()) == 0)
    {
    execvp (argv[0], argv);
    log_error ("Can't execute: %s", command);
    } 
  else
    {
    int wstatus;
    waitpid (pid, &wstatus, 0);
    }

  for (int i = 0; i < argc; i++)
    {
    free (argv[i]); 
    }

  free (argv);
  list_destroy (args);
  string_destroy (s_command);
  LOG_OUT
  }

/*==========================================================================

  program_run

  All the useful work starts here

==========================================================================*/
int program_run (ProgramContext *context)
  {
  const char *root = program_context_get (context, "root");
  const char *xshost = program_context_get (context, "xshost");
  if (!xshost) xshost = "localhost";
  int xsport = program_context_get_integer (context, "xsport", 
        XINESERVER_DEF_PORT);
  const char *gxsradio_dir = program_context_get (context, "gxsradio");
  if (gxsradio_dir == NULL)
    gxsradio_dir = GXSRADIO;

  const char *index = program_context_get (context, "index");
  if (!index)
    {
    log_warning ("No index file specified -- functionality will be limited");
    }


  const char *xslaunch = program_context_get (context, "xslaunch");
  if (xslaunch)
    program_launch_xine_server (xslaunch);

  if (!program_context_get_boolean (context, "debug", FALSE)) 
    { 
    daemon (0, 1); // TODO -- /dev/null when we have a log file 
    }

  log_info ("Using xine-server instance at %s:%d", xshost, xsport);

  facade_create (root, xshost, xsport, gxsradio_dir, index);

  RequestHandler *request_handler = request_handler_create (root, 
     context);

  sigset_t base_mask, waiting_mask;

  sigemptyset (&base_mask);
  sigaddset (&base_mask, SIGINT);
  sigaddset (&base_mask, SIGTSTP);
  sigaddset (&base_mask, SIGHUP);
  sigaddset (&base_mask, SIGQUIT);
  sigprocmask (SIG_SETMASK, &base_mask, NULL);
  // Ignore SIGCHLD, so when xsx spawns itself as the file scanner,
  // and the file scanner finishes, it doesn't leave a zombie
  // process. We don't want to _wait_ for the scanner -- we want
  // XSX to carry on, and ignore the scanner unless the user asks
  // for a status update.
  signal (SIGCHLD,SIG_IGN);

  log_info ("HTTP server starting");

  int xsxport = program_context_get_integer (context, "port", 
         XINESERVER_X_DEF_PORT);

  struct MHD_Daemon *daemon = MHD_start_daemon 
        (MHD_USE_THREAD_PER_CONNECTION, xsxport, NULL, NULL,
         program_handle_request, request_handler, MHD_OPTION_END);

  if (daemon)
    {
    while (!request_handler_shutdown_requested (request_handler))
       {
       usleep (1000000);
       sigpending (&waiting_mask);
       if (sigismember (&waiting_mask, SIGINT) ||
	   sigismember (&waiting_mask, SIGTSTP) ||
	   sigismember (&waiting_mask, SIGQUIT) ||
	   sigismember (&waiting_mask, SIGHUP))
	 {
	 log_warning ("Shutting down on signal");
	 request_handler_request_shutdown (request_handler);
	 }
       }

    log_info ("HTTP server stopping");

    if (xslaunch)
       facade_shut_down_xine_server ();

    MHD_stop_daemon (daemon);
    }
  else
   {
   log_error ("Can't start HTTP server (check port %d is not in use)", 
     xsxport);
   }

  request_handler_destroy (request_handler);
  facade_destroy();

  return 0;
  }

