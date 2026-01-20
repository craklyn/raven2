/* ************************************************************************
*   File: multi.c                                       Part of RavenMUD *
*  Usage: Player-level god commands and other goodies                    *
************************************************************************ */
 
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "util/utils.h"
#include "general/comm.h"
#include "actions/interpreter.h"
#include "actions/multi.h"
#include "general/handler.h"
#include "util/weather.h"

// increase this to support lists of multis > 100
#define MAX_MULTI_OK    100

static char *multi_host[MAX_MULTI_OK];
static char *multi_char[MAX_MULTI_OK];
static int multi_hosts = 0;
static int multi_chars = 0;
static int loaded = 0;

static int load_multi_file(char **dest, char *file)
{
  FILE *f = fopen(file, "r");
  int i;

  if (!f) return 0;
  for (i = 0; i < MAX_MULTI_OK && !feof(f); i++) {
    fgets(buf, sizeof(buf), f);
    dest[i] = strdup(buf);
  }
  fclose(f);
  return i;
}

static void save_multi_file(char **src, char *file, int count)
{
  FILE *f = fopen(file, "w");
  int i;

  if (!f) return;
  for (i = 0; i < count; i++) {
    fprintf(f, "%s\n", src[i]);
  }
  fclose(f);
}

// ugly, but this way I keep this module separate from the load/save code
static void check_multi_loads(void)
{
  if (!loaded) {
    multi_hosts = load_multi_file(multi_host, "misc/multi.hosts");
    multi_chars = load_multi_file(multi_char, "misc/multi.chars");
    loaded = 1;
  }
}

int check_multi_host(char *host)
{
  int i;

  check_multi_loads();
  for (i = 0; i < multi_hosts; i++)
    if (strncmp(host, multi_host[i], strlen(multi_host[i])) == 0) return 1;
  return 0;
}

int check_multi_char(char *chr)
{
  int i;

  check_multi_loads();
  for (i = 0; i < multi_chars; i++)
    if (strcasecmp(chr, multi_char[i]) == 0) return 1;
  return 0;
}

static void list_add(char **list, int *count, char *new)
{
  if (*count == MAX_MULTI_OK) return;
  list[*count] = strdup(new);
  (*count)++;
}

static void list_del(char **list, int *count, char *del)
{
  int i;
  for (i = 0; i < *count; i++) {
    if (strcmp(list[i], del) == 0) {
      memmove(list[i], list[i+1], sizeof(char *) * (*count - i - 1));
      (*count)--;
      return;
    }
  }
}

static void show_multi_usage(CharData *ch)
{
  send_to_char("Usage: multi { add | list | del } { host | char } "
               "[ <name> | <ip> ]\r\n", ch);
}

#define MULTI_HOST 0
#define MULTI_CHAR 1

ACMD(do_multi)
{
  int which;

  one_argument(two_arguments(argument, buf, buf2), buf1);

  if (!*buf || !*buf2) {
    show_multi_usage(ch);
    return;
  }

  if (is_abbrev(buf2, "host"))
    which = MULTI_HOST;
  else if (is_abbrev(buf2, "char"))
    which = MULTI_CHAR;
  else {
    show_multi_usage(ch);
    return;
  }

  check_multi_loads();

  if (is_abbrev(buf, "add")) {
    if (!*buf1) {
      show_multi_usage(ch);
      return;
    }
    list_add(which ? multi_char : multi_host,
             which ? &multi_chars : &multi_hosts,
             buf1);
    save_multi_file(which ? multi_char : multi_host,
                    which ? "misc/multi.chars" : "misc/multi.hosts",
                    which ? multi_chars : multi_hosts);
  } else if (is_abbrev(buf, "del")) {
    if (!*buf1) {
      show_multi_usage(ch);
      return;
    }
    list_del(which ? multi_char : multi_host,
             which ? &multi_chars : &multi_hosts,
             buf1);
    save_multi_file(which ? multi_char : multi_host,
                    which ? "misc/multi.chars" : "misc/multi.hosts",
                    which ? multi_chars : multi_hosts);
  } else if (is_abbrev(buf, "list")) {
    char **list = which ? multi_char : multi_host;
    int count = which ? multi_chars : multi_hosts;
    int i;

    send_to_char(which ? "Multi OK char names:\r\n" : "Multi OK sites:\r\n",
        ch);
    for (i = 0; i < count; i++) {
      send_to_char(list[i], ch);
      send_to_char("\r\n", ch);
    }
  }
}
