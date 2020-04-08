/*============================================================================

  xine-server-x 
  searchconstraints.c
  Copyright (c)2017 Kevin Boone, GPL v3.0

  Methods for storing and retrieving blocks of data. These methods are
    just thin wrappers around malloc() and free(), but the object stores
    the size of the data, which is often convenient when handling
    data blocks of variable size. The data is considered to be a block
    of BYTE, but the actual data is irrelevant to these methods.

============================================================================*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>
#include <stdint.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include "string.h" 
#include "defs.h" 
#include "log.h" 
#include "searchconstraints.h" 
#include "database.h" 

typedef enum _SCType
  {
  SC_TYPE_STRING = 0,
  SC_TYPE_NUMBER = 1
  } SCType;

typedef enum _SCTest
  {
  SC_TEST_IS = 0,
  SC_TEST_CONTAINS = 1,
  SC_TEST_LESSTHAN = 2 
  } SCTest;

typedef struct _Constraint
  {
  char *field;
  SCTest test;
  char *value;
  SCType type;
  } Constraint;

struct _SearchConstraints
  {
  List *constraints; // List of Constraint *
  BOOL disjunct;
  }; 

/*==========================================================================
  constraint_create
==========================================================================*/
Constraint *constraint_create (const char *field, SCTest test, 
     const char *value)
  {
  LOG_IN
  Constraint *self = malloc (sizeof (Constraint));
  self->value = strdup (value);
  self->field = strdup (field);
  self->test = test;
  self->type = SC_TYPE_STRING;
  // TODO identify numeric types
  LOG_OUT
  return self;
  }

/*==========================================================================
  constraint_destroy
==========================================================================*/
void constraint_destroy (Constraint *self)
  {
  LOG_IN
  if (self)
    {
    if (self->field) free (self->field);
    if (self->value) free (self->value);
    free (self);
    }
  LOG_OUT
  }

/*==========================================================================
  searchconstraints_parse_test
==========================================================================*/
SCTest constraint_parse_test (const char *test)
  {
  LOG_IN
  SCTest ret = -1;

  if (strcmp (test, "is") == 0)
    {
    ret = SC_TEST_IS;
    }
  else if (strcmp (test, "contains") == 0)
    {
    ret = SC_TEST_CONTAINS;
    }
  else if (strcmp (test, "lessthan") == 0)
    {
    ret = SC_TEST_LESSTHAN;
    }
  // TODO -- others

  LOG_OUT
  return ret;
  }

/*==========================================================================
  searchconstraints_parse_key_value
==========================================================================*/
static void searchconstraints_parse_key_value (SearchConstraints *self, 
     const char *key, const char *value)
  {
  LOG_IN
  if (strcmp (key, "disjunct") == 0)
    {
    if (value && atoi (value))
      self->disjunct = TRUE;
    }
  else
    {
    char *eq = strchr (key, '-');
    if (eq)
      {
      // Key contains an -, so this is potentially a search constraint
      char *key_ = strdup (key);
      eq = strchr (key_, '-');
      *eq = 0;
      const char *field = key_;
      const char* test = eq + 1;
      SCTest atest = constraint_parse_test (test);
      if ((int)atest >= 0)
	{
	Constraint *c = constraint_create (field, atest, value); 
	list_append (self->constraints, c);
	}
      else
	{
	log_error ("%s: unknown test '%s' (key=%s value=%s)",
	  __PRETTY_FUNCTION__, test, key, value); 
	}

      free (key_);
      }
    }
  LOG_OUT
  }

/*==========================================================================
  searchconstraints_create_empty
==========================================================================*/
SearchConstraints *searchconstraints_create_empty (void)
  {
  LOG_IN
  SearchConstraints *self = malloc (sizeof (SearchConstraints));
  self->constraints = list_create ((ListItemFreeFn) constraint_destroy);
  self->disjunct = FALSE;

  LOG_OUT 
  return self;
  }

/*==========================================================================
  searchconstraints_create_from_args
==========================================================================*/
SearchConstraints *searchconstraints_create_from_args (const Props *props)
  {
  LOG_IN
  SearchConstraints *self = malloc (sizeof (SearchConstraints));
  self->constraints = list_create ((ListItemFreeFn) constraint_destroy);
  self->disjunct = FALSE;

  List *args = props_get_keys (props);
  int l = list_length (args);
  for (int i = 0; i < l; i++)
    {
    const char *key = list_get (args, i);
    const char *val = props_get (props, key);

    searchconstraints_parse_key_value (self, key, val);
    }
  list_destroy (args);

  LOG_OUT 
  return self;
  }

/*==========================================================================
  searchconstraints_create_exhaustive
==========================================================================*/
SearchConstraints *searchconstraints_create_exhaustive (const char *term)
  {
  LOG_IN

  Props *p = props_create();
  props_put (p, "album-contains", term);
  props_put (p, "artist-contains", term);
  props_put (p, "composer-contains", term);
  props_put (p, "title-contains", term);
  props_put (p, "genre-contains", term);
  props_put (p, "disjunct", "1");
  SearchConstraints *self = searchconstraints_create_from_args (p);
  props_destroy (p);

  LOG_OUT 
  return self;
  }


/*==========================================================================
  searchconstraints_destroy
==========================================================================*/
void searchconstraints_destroy (SearchConstraints *self)
  {
  LOG_IN
  if (self)
    {
    if (self->constraints) list_destroy (self->constraints);
    free (self);
    }
  LOG_OUT
  }

/*==========================================================================

  searchconstraints_has_constraints

==========================================================================*/
BOOL searchconstraints_has_constraints (const SearchConstraints *self)
  {
  LOG_IN
  BOOL ret = FALSE;
  if (list_length (self->constraints) > 0) ret = TRUE;
  LOG_OUT
  return ret;
  }

/*==========================================================================

  searchconstraints_make_where

  Logically, this functionality should be in the Database class; but it's 
    just really fiddly to implement that way

==========================================================================*/
char *searchconstraints_make_where (const SearchConstraints *self)
  {
  LOG_IN
  String *where = string_create_empty ();
  int l = list_length (self->constraints); 
  if (l > 0)
    {
    string_append (where, " where ");
    for (int i = 0; i < l; i++)
      {
      Constraint *c = list_get (self->constraints, i);
      char *esc_value = database_escape_sql (c->value);
      switch (c->test)
        {
        case SC_TEST_IS:
          string_append (where, c->field);
          string_append (where, "=");
          if (c->type == SC_TYPE_NUMBER)
            string_append_printf (where, "%d", esc_value);
          else
            string_append_printf (where, "'%s'", esc_value);
          break;
        case SC_TEST_CONTAINS:
          // If using regular expressions is too burdensome for the
          //  system, replace the regular expression test with this
          //  compound 'like' expression
	  /*
          string_append (where, c->field);
          string_append (where, " like ");
          string_append_printf (where, " '%% %s %%' ", esc_value);
          string_append_printf (where, " or ");
          string_append (where, c->field);
          string_append (where, " like ");
          string_append_printf (where, " '%% %s' ", esc_value);
          string_append_printf (where, " or ");
          string_append (where, c->field);
          string_append (where, " like ");
          string_append_printf (where, " '%s %%' ", esc_value);
          string_append_printf (where, " or ");
          string_append (where, c->field);
          string_append (where, " like ");
          string_append_printf (where, " '%s,%%' ", esc_value);
	  */
          string_append (where, c->field);
          string_append (where, " regexp ");
          string_append_printf (where, "'\\b%s\\b'", esc_value);
          break;
        case SC_TEST_LESSTHAN:
	  // lessthan only applies to dates, so operates on the mtime
	  //  column
          string_append (where, "mtime > ");
	  long now = (long) time(NULL);
	  int days = atoi (c->value);
	  long days_sec = days * 24 * 3600;
	  int limit_mtime = (int) (now - days_sec);
	  string_append_printf (where, "%d", limit_mtime);
	  break;
        // TODO others
        }
      free (esc_value);
      if (i != l - 1)
	{ 
	if (self->disjunct)
	    string_append (where, " or ");
	else	
	    string_append (where, " and ");
	}
      }
    }
  char *ret = strdup (string_cstr (where));
  string_destroy (where);
  LOG_OUT
  return ret;  
  }

/*==========================================================================

  searchconstraints_make_readable where

==========================================================================*/
char *searchconstraints_make_readable_where (const SearchConstraints *self)
  {
  LOG_IN
  String *where = string_create_empty ();
  int l = list_length (self->constraints); 
  if (l > 0)
    {
    string_append (where, " where ");
    for (int i = 0; i < l; i++)
      {
      Constraint *c = list_get (self->constraints, i);
      string_append (where, c->field);
      string_append (where, " ");
      switch (c->test)
        {
        case SC_TEST_IS:
            string_append (where, " is ");
            break;
        case SC_TEST_CONTAINS:
            string_append (where, " contains ");
            break;
        case SC_TEST_LESSTHAN:
            string_append (where, " less than ");
            break;
        // TODO others
        }
      if (c->type == SC_TYPE_NUMBER)
          string_append_printf (where, "%d", c->value);
      else
          string_append_printf (where, "'%s'", c->value);
      if (i != l - 1)
	{ 
	if (self->disjunct)
	    string_append (where, " or ");
	else	
	    string_append (where, " and ");
	}
      }
    }
  char *ret = strdup (string_cstr (where));
  string_destroy (where);
  LOG_OUT
  return ret;  
  }




