/*==========================================================================

  boilerplate
  scanner.c
  Copyright (c)2020 Kevin Boone
  Distributed under the terms of the GPL v3.0

==========================================================================*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>
#include "props.h" 
#include "program_context.h" 
#include "xine-server-x-api.h" 
#include "database.h" 
#include "facade.h" 
#include "audio_metainfo.h" 
#include "scanner.h" 
#include "mimebuffer.h" 

typedef struct _ScannerIteratorContext 
  {
  const Path *root;
  List *list;
  } ScannerIteratorContext;


/*==========================================================================

  scanner_insert_db

==========================================================================*/
void scanner_insert_db (Database *database, const char *path, 
       const AudioMetaInfo *ami)
  {
  LOG_IN
  char *error = NULL;
  database_insert (database, 
    path,
    audio_metainfo_get_size (ami),
    audio_metainfo_get_mtime (ami),
    audio_metainfo_get_title (ami),
    audio_metainfo_get_album (ami),
    audio_metainfo_get_genre (ami),
    audio_metainfo_get_composer (ami),
    audio_metainfo_get_artist (ami),
    audio_metainfo_get_track (ami),
    audio_metainfo_get_comment (ami),
    audio_metainfo_get_year (ami),
    &error
    );

  if (error)
    {
    log_error (error);
    free (error);
    }
  LOG_OUT
  }

/*==========================================================================

  scanner_write_status

==========================================================================*/
static void scanner_write_status (int scanned, int added, 
        int modified, int deleted, int extracted)
   {
   FILE *f = fopen (SCANNER_STATUS_FILE, "w");
   if (f)
     {
     fprintf (f, "%d %d %d %d %d\n", scanned, added, modified, 
        deleted, extracted);
     fclose (f);
     }
   else
     log_error ("Can't write scanner status file %s\n", SCANNER_STATUS_FILE);
   }

/*==========================================================================

  scanner_write_cover

==========================================================================*/
BOOL scanner_write_cover (const MimeBuffer *cover, const Path *thispath, 
      char **error)
  {
  LOG_IN
  Path *coverpath = path_clone (thispath);
  const char *mime = mimebuffer_get_mime_type (cover);
  if (strcmp (mime, "image/png") == 0)
    path_append (coverpath, "cover.png");
  else if (strcmp (mime, "image/gif") == 0)
    path_append (coverpath, "cover.gif");
  else 
    path_append (coverpath, "cover.jpg");
  char *s_coverpath = (char *)path_to_utf8 (coverpath);

  BOOL ret = mimebuffer_write_to_file (cover, s_coverpath, error);
   
  free (s_coverpath);
  path_destroy (coverpath);
  LOG_OUT
  return ret;
  }

/*==========================================================================

  scanner_scan

==========================================================================*/
static void scanner_scan (Database *database, const Path *root, 
       const char *dir, BOOL full_scan, int *scanned, int *added,
       int *modified, int *extracted)
  {
  LOG_IN

  Path *thispath = path_clone (root);
  path_append (thispath, dir);
  char *s_thispath = (char *)path_to_utf8 (thispath);

  log_debug ("%s: Listing files in %s", __PRETTY_FUNCTION__, s_thispath); 

  DIR *d = opendir (s_thispath);
  if (d)
    {
    BOOL has_cover = FALSE;
    char *cover = facade_get_cover_image_for_dir (dir);
    if (cover)
      {
      log_debug ("This directory has a cover image: %s\n", cover);
      has_cover = TRUE;
      free (cover);
      }

    struct dirent *de = readdir (d); 
    while (de)
      {
      const char *name = de->d_name;
      if (name[0] != '.')
	{
        // p2 is the full filesystem path of the file under consideration
	Path *p2 = path_clone (thispath);
	path_append (p2, name);
	char *s_p2 = (char *) path_to_utf8 (p2);
        // p3 is the path of the file under consideration, relative to
        //   the media root directory
	Path *p3 = path_create (dir);
	path_append (p3, name);
	char *s_p3 = (char *) path_to_utf8 (p3);

	if (path_is_regular (p2))
	  {
	  if (facade_path_is_playable (p2))
	    {
	    struct stat sb;
	    if (path_stat (p2, &sb))
	      {
              BOOL update_db = FALSE;
	      if (full_scan) 
	        {
	        update_db = TRUE;
		(*added)++;
		}
	      else
	        {
                // In full scan mode, we always write a database row.
		// Otherwise, we check the modification time in the DB
		// against the file. Three possibilities:
		// 1. File is in the database with same date; 2. File is in
		// database with different date; 3. File is not in database

		AudioMetaInfo *ami = audio_metainfo_create(); 
                char *error = NULL;
                if (audio_metainfo_get_from_database (ami, database, 
                      s_p3, &error))
		  {
                  time_t db_mtime = audio_metainfo_get_mtime (ami);
                  time_t file_mtime = file_get_mtime (s_p2);
                  if (file_mtime == db_mtime)
		    {
                    update_db = FALSE;
		    }
                  else
                    {
                    update_db = TRUE;
		    (*modified)++;
		    char *error = NULL;
		    if (!database_delete_path (database, s_p3, &error))
		      {
		      log_error (error);
		      free (error);
		      }
                    }
                  }
                else
                  {
                  // Not in database
                  free (error);
                  update_db = TRUE;
		  (*added)++;
                  }
		audio_metainfo_destroy (ami); 
		}
	      if (update_db)
	        {
		char *error = NULL;
		AudioMetaInfo *ami = audio_metainfo_create(); 
                if (audio_metainfo_get_from_path (ami, s_p2, &error))
		  {
		  scanner_insert_db (database, s_p3, ami); 

                  if (!has_cover)
                     {
                     const MimeBuffer *cover = audio_metainfo_get_cover (ami);
		     if (cover)
		       {
		       char *error = NULL;
		       if (scanner_write_cover (cover, thispath, &error))
		         {
		         has_cover = TRUE;
		         (*extracted)++;
			 }
		       else
		         {
			 log_error (error);
			 free (error);
			 }
		       }
                     }
		  }
		else
		  {
                  log_error (error);
		  free (error);
		  }
		audio_metainfo_destroy (ami);
		}
	      }
	    else
	      {
	      log_error ("stat() failed for %s", s_p2);
	      }

            (*scanned)++;
            if ((*scanned) % SCANNER_FILE_INTERVAL == 0)
              {
              scanner_write_status (*scanned, *added, 
	         *modified, 0, *extracted);
              }
	    }
	  } 
	else if (path_is_directory (p2))
	  {
          scanner_scan (database, root, s_p3, full_scan, scanned, added,
	    modified, extracted);
	  }
	path_destroy (p2); 
	free (s_p2);
        path_destroy (p3);
        free (s_p3);
        }
      de = readdir (d);
      }
    closedir (d);
    }
  else
    {
    log_error ("Can't list directory %s: %s", s_thispath, 
      strerror (errno));
    }

  free (s_thispath);
  path_destroy (thispath);

  LOG_OUT
  }

/*==========================================================================

  scanner_scan_files

==========================================================================*/
int scanner_scan_files (ProgramContext *context)
  {
  const char *root = program_context_get (context, "root");
  // If index is not specified, we wont' even get this far, to
  //  no need to check again
  const char *index = program_context_get (context, "index");

  log_info ("Scanner pass 1 -- scanning filesystem");

  /*
  sigset_t base_mask, waiting_mask;

  sigemptyset (&base_mask);
  sigaddset (&base_mask, SIGINT);
  sigaddset (&base_mask, SIGTSTP);
  sigaddset (&base_mask, SIGHUP);
  sigaddset (&base_mask, SIGQUIT);
  sigprocmask (SIG_SETMASK, &base_mask, NULL);
  */

  facade_create (root, "", 0, "", index);

  char *dbfile = NULL;
  BOOL ok = TRUE;

  BOOL full_scan = program_context_get_boolean (context, "scan", FALSE);
  if (full_scan)
    {
    asprintf (&dbfile, "%s.temp", index);
    unlink (dbfile);
    }
  else
    dbfile = strdup (index);

  char *error = NULL; 

  Database *db = database_create (dbfile);
  
  if (full_scan)
    {
    if (database_make (db, &error))
      {
      ok = TRUE;
      }
    else
      {
      log_error ("Can't create database: %s", error);
      free (error);
      ok = FALSE;
      }
    }
  else
    { 
    if (database_open (db, &error))
      {
      ok = TRUE;
      }
    else
      {
      log_error ("Can't open database: %s", error);
      free (error);
      ok = FALSE;
      }
    } 

  if (ok)
    {
    // Database open -- do the scan
    Path *rootpath = path_create (root);
    int scanned = 0;
    int added = 0;
    int modified = 0;
    int extracted = 0;
    scanner_scan (db, rootpath, "", full_scan, &scanned, &added,
      &modified, &extracted);
    path_destroy (rootpath);
    log_info ("Files scanned: %d", scanned);
    log_info ("Entries added to index: %d", added);
    log_info ("Index entries updated: %d", modified);
    log_info ("Cover images extracted: %d", extracted);
    scanner_write_status (scanned, added, modified, 0, extracted);
    }

  database_close (db);
  database_destroy (db);

  if (full_scan && ok)
    {
    char* cmd;
    asprintf (&cmd, "mv \"%s\" \"%s\"", dbfile, index);
    log_debug ("Moving temporary index %s to main index %s",
       dbfile, index);
    system (cmd);
    free (cmd);
    }

  free (dbfile);

  facade_destroy();

  log_info ("Scanner pass 1 done");

  return 0;
  }

/*==========================================================================

  scanner_check_file_exists
  Called by database_iterate_all_paths, to build a list of database
  entries that have no corresponding file on the filesystem. Presumably
  these files have been deleted. This function appends the (database)
  path to a list, which is then used to purge the database

==========================================================================*/
BOOL scanner_check_file_exists (const char *path, void *data)
  {
  ScannerIteratorContext *sic = (ScannerIteratorContext *)data;

  Path *test = path_clone (sic->root);
  path_append (test, path);
  if (!path_is_regular (test))
    {
    List *list = sic->list;
    list_append (list, strdup (path));
    }
   
  path_destroy (test);

  return TRUE;
  }

/*==========================================================================

  scanner_scan_db

==========================================================================*/
int scanner_scan_db (ProgramContext *context)
  {
  log_info ("Scanner pass 2 -- scanning index for missing files");

  const char *root = program_context_get (context, "root");
  // If index is not specified, we wont' even get this far, to
  //  no need to check again
  const char *index = program_context_get (context, "index");

  facade_create (root, "", 0, "", index);

  Database *db = database_create (index);
  
  char *error = NULL;
  if (database_open (db, &error))
    {
    ScannerIteratorContext sic;
    Path *root_path = path_create (root);
    sic.root = root_path; 
    sic.list = list_create (free);

    char *error = NULL;
    // Determine which database entries have no file
    if (!database_iterate_all_paths (db, scanner_check_file_exists, &sic, 
           &error))
      {
      log_error (error);
      free (error);
      }
    int l = list_length (sic.list);
    for (int i = 0; i < l; i++)
      {
      const char *s = list_get (sic.list, i);
      //delete database entries with no file
      char *error = NULL;
      database_delete_path (db, s, &error);
      if (error)
        {
        log_error (error);
        free (error);
        }
      }
    log_info ("Entries deleted from index: %d", l);
    list_destroy (sic.list);
    path_destroy (root_path);
    database_close (db);
    }
  else
    {
    log_error (error);
    free (error);
    }

  database_destroy (db);

  facade_destroy();

  log_info ("Scanner pass 2 done");

  return 0;
  }


/*==========================================================================

  scanner_run

  All the useful work starts here

==========================================================================*/
int scanner_run (ProgramContext *context)
  {
  unlink (SCANNER_STATUS_FILE);
  scanner_scan_files (context);
  BOOL full_scan = program_context_get_boolean (context, "scan", FALSE);
  // If this is _not_ a full scan, we don't need to check the correspondence
  //   between the database and the filesystem, because we've just built
  //   one from the other. In a quick scan, we do need to check whether
  //   the DB and filesystem are in sync
  if (!full_scan)
    scanner_scan_db (context);
  unlink (SCANNER_STATUS_FILE);
  return 0;
  }


