/**************************************************************************
*   File: ban.c                                         Part of CircleMUD *
*  Usage: banning/unbanning/checking sites and player names               *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
**************************************************************************/

#define __BAN_C__

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "util/utils.h"
#include "general/comm.h"
#include "actions/interpreter.h"
#include "general/handler.h"
#include "general/class.h"
#include "actions/ban.h"

/* global variables locally defined, used externally */
struct ban_list_element *ban_list = NULL;

/* local utility functions */
static void write_ban_list(void);
static void _write_one_node(FILE *fp, struct ban_list_element *node);

char *ban_types[] = {
  "no",
  "shun",
  "rawlog",
  "new",
  "select",
  "all",
  "ERROR"
};

void load_banned(void)
{
  FILE *fl;
  int i, date;
  char site_name[BANNED_SITE_LENGTH + 1], ban_type[100];
  char name[MAX_NAME_LENGTH + 1];
  struct ban_list_element *next_node;

  ban_list = 0;

  if (!(fl = fopen(BAN_FILE, "r"))) {
    if (errno != ENOENT) {
      mlog("SYSERR: Unable to open banfile '%s': %s", BAN_FILE, strerror(errno));
    } else
      mlog("   Ban file '%s' doesn't exist.", BAN_FILE);
    return;
  }
  while (fscanf(fl, " %s %s %d %s ", ban_type, site_name, &date, name) == 4) {
    CREATE(next_node, struct ban_list_element, 1);
    strncpy(next_node->site, site_name, BANNED_SITE_LENGTH);	/* strncpy: OK (n_n->site:BANNED_SITE_LENGTH+1) */
    next_node->site[BANNED_SITE_LENGTH] = '\0';
    strncpy(next_node->name, name, MAX_NAME_LENGTH);	/* strncpy: OK (n_n->name:MAX_NAME_LENGTH+1) */
    next_node->name[MAX_NAME_LENGTH] = '\0';
    next_node->date = date;

    for (i = BAN_NOT; i <= BAN_ALL; i++)
      if (!strcmp(ban_type, ban_types[i]))
	next_node->type = i;

    next_node->next = ban_list;
    ban_list = next_node;
  }

  fclose(fl);
}

int isbanned(char *hostname)
{
  int i;
  struct ban_list_element *banned_node;
  char *nextchar;

  if (!hostname || !*hostname)
    return (0);

  if (strlen(hostname) >= MAX_INPUT_LENGTH) {
	mudlog(NRM, LVL_IMMORT, TRUE, "BAN: Excessively long hostname %s(max length %d), disallowing entry.", hostname, MAX_INPUT_LENGTH);
	return(1);
  }

  i = 0;
  for (nextchar = hostname; *nextchar; nextchar++)
    *nextchar = LOWER(*nextchar);

  for (banned_node = ban_list; banned_node; banned_node = banned_node->next) {
#if 0
    if (strstr(hostname, banned_node->site))	/* if hostname is a substring */
#else
    if (strncmp(hostname, banned_node->site, strlen(banned_node->site)) == 0)
#endif
      i = MAX(i, banned_node->type);
   }

  return (i);
}

static void _write_one_node(FILE *fp, struct ban_list_element *node)
{
  if (node) {
    _write_one_node(fp, node->next);
    fprintf(fp, "%s %s %ld %s\n", ban_types[node->type],
	    node->site, (long) node->date, node->name);
  }
}

static void write_ban_list(void)
{
  FILE *fl;

  if (!(fl = fopen(BAN_FILE, "w"))) {
    perror("SYSERR: Unable to open '" BAN_FILE "' for writing");
    return;
  }
  _write_one_node(fl, ban_list);/* recursively write from end to start */
  fclose(fl);
  return;
}


ACMD(do_ban)
{
  char flag[80], site[80], format[50], *nextchar, *timestr;
  int i;
  struct ban_list_element *ban_node;

  if (IS_SET_AR(PLR_FLAGS(ch), PLR_RETIRED)) {
      send_to_char("Relax and enjoy being retired.\r\n", ch);
      return;
  }

  if (IS_NPC(ch)) {
    send_to_char("You Beast!!\r\n", ch);
    return;
  }
  strcpy(buf, "");
  if (!*argument) {
    if (!ban_list) {
      send_to_char("No sites are banned.\r\n", ch);
      return;
    }
    strcpy(format, "%-25.25s  %-8.8s  %-10.10s  %-16.16s\r\n");
    sprintf(buf, format,
	    "Banned Site Name",
	    "Ban Type",
	    "Banned On",
	    "Banned By");
    send_to_char(buf, ch);
    sprintf(buf, format,
	    "---------------------------------",
	    "---------------------------------",
	    "---------------------------------",
	    "---------------------------------");
    send_to_char(buf, ch);

    for (ban_node = ban_list; ban_node; ban_node = ban_node->next) {
      if (ban_node->date) {
	timestr = asctime(localtime(&(ban_node->date)));
	*(timestr + 10) = 0;
	strcpy(site, timestr);
      } else
	strcpy(site, "Unknown");
      sprintf(buf, format, ban_node->site, ban_types[ban_node->type], site,
	      ban_node->name);
      send_to_char(buf, ch);
    }
    return;
  }
  two_arguments(argument, flag, site);
  if (!*site || !*flag) {
    send_to_char("Usage: ban {all | select | new | shun | rawlog} site_name\r\n", ch);
    return;
  }
  if (!(!str_cmp(flag, "select") ||
        !str_cmp(flag, "all")    ||
        !str_cmp(flag, "new")    ||
        !str_cmp(flag, "rawlog") ||
        !str_cmp(flag, "shun"))) {
    send_to_char("Flag must be ALL, SELECT, NEW, RAWLOG, or SHUN.\r\n", ch);
    return;
  }
  for (ban_node = ban_list; ban_node; ban_node = ban_node->next) {
    if (!str_cmp(ban_node->site, site)) {
      send_to_char("That site has already been banned -- unban it to change the ban type.\r\n", ch);
      return;
    }
  }

  CREATE(ban_node, struct ban_list_element, 1);
  strncpy(ban_node->site, site, BANNED_SITE_LENGTH);
  for (nextchar = ban_node->site; *nextchar; nextchar++)
    *nextchar = LOWER(*nextchar);
  ban_node->site[BANNED_SITE_LENGTH] = '\0';
  strncpy(ban_node->name, GET_NAME(ch), MAX_NAME_LENGTH);
  ban_node->name[MAX_NAME_LENGTH] = '\0';
  ban_node->date = time(0);

  for (i = BAN_SHUNNED; i <= BAN_ALL; i++)
    if (!str_cmp(flag, ban_types[i]))
      ban_node->type = i;

  ban_node->next = ban_list;
  ban_list = ban_node;

  mudlog(NRM, MAX(LVL_LRGOD, GET_INVIS_LEV(ch)), TRUE,
     "%s has banned %s for %s players.", GET_NAME(ch), site, ban_types[ban_node->type]);

  sendChar(ch, "Site banned.\r\n");
  write_ban_list();
}

ACMD(do_unban)
{
  char site[80];
  struct ban_list_element *ban_node, *temp;
  int found = 0;

    if (IS_SET_AR(PLR_FLAGS(ch), PLR_RETIRED)) {
        sendChar(ch, "Relax and enjoy being retired.\r\n");
        return;
    }

    if (IS_NPC(ch)) {
        sendChar(ch, "You're a mythological monster, start playing the part!\r\n");
        return;
    }

    one_argument(argument, site);
    if (!*site) {
        sendChar(ch, "A site to unban might help.\r\n");
        return;
    }
    ban_node = ban_list;
  while (ban_node && !found) {
        if (!str_cmp(ban_node->site, site))
            found = 1;
        else
            ban_node = ban_node->next;
    }

  if (!found) {
    sendChar(ch, "That site is not currently banned.\r\n");
    return;
  }
  REMOVE_FROM_LIST(ban_node, ban_list, next);
  sendChar(ch, "Site unbanned.\r\n");
  mudlog(NRM, MAX(LVL_LRGOD, GET_INVIS_LEV(ch)), TRUE, "%s removed the %s-player ban on %s.",
	  GET_NAME(ch), ban_types[ban_node->type], ban_node->site);

  free(ban_node);
  write_ban_list();
}


/**************************************************************************
 *  Code to check for invalid names (i.e., profanity, etc.)		  *
 *  Written by Sharon P. Goza						  *
 **************************************************************************/
#ifdef OLD_WAY

typedef char namestring[MAX_NAME_LENGTH];
int num_invalid = 0;
namestring *invalid_list = NULL;

int Valid_Name(char *newname)
{
  int i;

  char tempname[MAX_NAME_LENGTH];

  /* return valid if list doesn't exist */
  if (!invalid_list || num_invalid < 1)
    return 1;

  /* change to lowercase */
  strcpy(tempname, newname);
  for (i = 0; tempname[i]; i++)
    tempname[i] = LOWER(tempname[i]);

  /* Does the desired name contain a string in the invalid list? */
  for (i = 0; i < num_invalid; i++)
    if (strstr(tempname, invalid_list[i]))
      return 0;

  return 1;
}

void Read_Invalid_List(void)
{
  FILE *fp;
  int i = 0;
  char string[80];

  if (!(fp = fopen(XNAME_FILE, "r"))) {
    perror("Unable to open invalid name file");
    return;
  }
  /* count how many records */
  while (fgets(string, 80, fp) != NULL && strlen(string) > 1)
    num_invalid++;

  rewind(fp);

  CREATE(invalid_list, namestring, num_invalid);

  for (i = 0; i < num_invalid; i++) {
    fgets(invalid_list[i], 80, fp);	/* read word */
    invalid_list[i][strlen(invalid_list[i]) - 1] = '\0';	/* cleave off \n */
  }

  fclose(fp);
}
#else

#define MAX_INVALID_NAMES 256

char *invalid_list[MAX_INVALID_NAMES];
int num_invalid = 0;

int Valid_Name(char *newname)
{
  int i;
  struct descriptor_data *dt;
  char tempname[MAX_INPUT_LENGTH];

  /*
   * Make sure someone isn't trying to create this same name.  We want to
   * do a 'str_cmp' so people can't do 'Bob' and 'BoB'.  The creating login
   * will not have a character name yet and other people sitting at the
   * prompt won't have characters yet.
   */
  for (dt = descriptor_list; dt; dt = dt->next)
    if (dt->character && GET_NAME(dt->character) && !str_cmp(GET_NAME(dt->character), newname))
      return (STATE(dt) == CON_PLAYING);

  /* return valid if list doesn't exist */
  if (!invalid_list || num_invalid < 1)
    return 1;

  /* change to lowercase */
  strcpy(tempname, newname);
  for (i = 0; tempname[i]; i++)
    tempname[i] = LOWER(tempname[i]);

  /* Does the desired name contain a string in the invalid list? */
  for (i = 0; i < num_invalid; i++)
    if (strstr(tempname, invalid_list[i]))
      return 0;

  return 1;
}

void free_invalid_list(void)
{
  int invl;

  for (invl = 0; invl < num_invalid; invl++)
    free(invalid_list[invl]);

  num_invalid = 0;
}

void Read_Invalid_List(void)
{
  FILE *fp;
  char temp[256];

  if (!(fp = fopen(XNAME_FILE, "r"))) {
    perror("SYSERR: Unable to open '" XNAME_FILE "' for reading");
    return;
  }

  num_invalid = 0;
  while (get_line(fp, temp) && num_invalid < MAX_INVALID_NAMES)
    invalid_list[num_invalid++] = strdup(temp);

  if (num_invalid >= MAX_INVALID_NAMES) {
    mlog("SYSERR: Too many invalid names; change MAX_INVALID_NAMES in ban.c");
    exit(1);
  }

  fclose(fp);
}
#endif
