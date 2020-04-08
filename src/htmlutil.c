/*============================================================================

  xine-server-x 
  htmlutil.c
  Copyright (c)2020 Kevin Boone
  Distributed under the terms of the GPL v3.0

============================================================================*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "defs.h" 
#include "log.h" 


/*============================================================================

  htmlutil_to_hex

============================================================================*/
static char htmlutil_to_hex(char code)
  {
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
  }

/*============================================================================

  htmlutil_escape_squote_js

============================================================================*/
char *htmlutil_escape_squote_js (const char *str)
  {
  int l = strlen (str);
  int newlen = l;
  char *ret = malloc (newlen + 1);
  int p = 0;

  for (int i = 0; i < l; i++)
    {
    char c = str[i];
    if (c == '\'')
      {
      newlen+=2;
      ret = realloc (ret, newlen + 1);
      ret[p++] = '\\';
      ret[p++] = '\'';
      }
    else
      {
      ret[p++] = c;
      }
    }
  ret[p] = 0;
  return ret;
  }

/*============================================================================

  htmlutil_escape_dquote_json

============================================================================*/
char *htmlutil_escape_dquote_json (const char *str)
  {
  int l = strlen (str);
  int newlen = l;
  char *ret = malloc (newlen + 1);
  int p = 0;

  for (int i = 0; i < l; i++)
    {
    char c = str[i];
    if (c == '\"')
      {
      newlen+=2;
      ret = realloc (ret, newlen + 1);
      ret[p++] = '\\';
      ret[p++] = '\"';
      }
    else
      {
      ret[p++] = c;
      }
    }
  ret[p] = 0;
  return ret;
  }

/*============================================================================

  htmlutil_escape_dquote_js

============================================================================*/
char *htmlutil_escape_dquote_js (const char *str)
  {
  int l = strlen (str);
  int newlen = l;
  char *ret = malloc (newlen + 1);
  int p = 0;

  for (int i = 0; i < l; i++)
    {
    char c = str[i];
    if (c == '\"')
      {
      newlen+=7;
      ret = realloc (ret, newlen + 1);
      ret[p++] = '&';
      ret[p++] = 'q';
      ret[p++] = 'u';
      ret[p++] = 'o';
      ret[p++] = 't';
      ret[p++] = ';';
      }
    else
      {
      ret[p++] = c;
      }
    }
  ret[p] = 0;
  return ret;
  }

/*============================================================================

  htmlutil_escape_sdquote_js

============================================================================*/
char *htmlutil_escape_sdquote_js (const char *str)
  {
  char *a1 = htmlutil_escape_squote_js (str);
  char *a2 = htmlutil_escape_dquote_js (a1);
  free (a1);
  return a2;
  }

/*============================================================================

  htmlutil_escape

============================================================================*/
char *htmlutil_escape (const char *str)
  {
  const char *pstr = str;
  char *buf = malloc (strlen(str) * 3 + 1); 
  char *pbuf = buf;
  while (*pstr)
    {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_'
      || *pstr == '.' || *pstr == '~')
      *pbuf++ = *pstr;
    else if (*pstr == ' ')
      *pbuf++ = '+';
    else 
      *pbuf++ = '%', *pbuf++ = htmlutil_to_hex (*pstr >> 4),
         *pbuf++ = htmlutil_to_hex (*pstr & 15);
    pstr++;
    }
  *pbuf = '\0';
  return buf;
  }

/*============================================================================

  htmlutil_make_href

============================================================================*/
char *htmlutil_make_href (const char *href, const char *text)
  {
  char *ret;
  asprintf (&ret, "<a href=\"%s\">%s</a>", href, text);
  return ret;
  }



