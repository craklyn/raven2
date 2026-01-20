/* ************************************************************************
*   File: modify.c                                      Part of CircleMUD *
*  Usage: Run-time modification of game variables                         *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "util/utils.h"
#include "actions/interpreter.h"
#include "general/handler.h"
#include "general/comm.h"
#include "general/class.h"
#include "magic/spells.h"
#include "specials/mail.h"
#include "specials/boards.h"
#include "olc/olc.h"
#include "scripts/dg_scripts.h" /* for trigedit_string_cleanup */
#include "general/modify.h"
#include "olc/qedit.h"

/* local (file scope) function prototpyes  */
static void parse_edit_action( int command, char *string, DescriptorData *d );
static int improved_editor_execute(struct descriptor_data *d, char *string);
static void playing_string_cleanup(struct descriptor_data *d, int action);
static void exdesc_string_cleanup(struct descriptor_data *d, int action);
static int format_text(char **ptr_string, int mode, struct descriptor_data *d,
        unsigned int maxlen, int low, int high);


/* modification of malloc'ed strings */
/* Put '#if 1' here to erase ~, or roll your own method.  A common idea is
 * smash/show tilde to convert the tilde to another innocuous character to
 * save and then back to display it. Whatever you do, at least keep the
 * function around because other MUD packages use it, like mudFTP. -gg */
void smash_tilde(char *str)
{
  /* Erase any _line ending_ tildes inserted in the editor. The load mechanism
   * can't handle those, yet. - Welcor */
  char *p = str;
  for (; *p; p++)
    if (*p == '~' && (*(p+1)=='\r' || *(p+1)=='\n' || *(p+1)=='\0'))
      *p=' ';
}

void send_editor_help(struct descriptor_data *d)
{
    write_to_output(d, "Instructions: /s to save, /h for more options.\r\n");
}

static int improved_editor_execute(struct descriptor_data *d, char *str)
{
  char actions[MAX_INPUT_LENGTH];

  if (*str != '/')
    return STRINGADD_OK;

  strncpy(actions, str + 2, sizeof(actions) - 1);
  actions[sizeof(actions) - 1] = '\0';
  *str = '\0';

    switch (str[1]) {
    case 'a':
    return STRINGADD_ABORT;
    case 'c':
        if (*(d->str)) {
            free(*d->str);
            *(d->str) = NULL;
            write_to_output(d, "Current buffer cleared.\r\n");
    } else
            write_to_output(d, "Current buffer empty.\r\n");
        break;
    case 'd':
        parse_edit_action(PARSE_DELETE, actions, d);
        break;
    case 'e':
        parse_edit_action(PARSE_EDIT, actions, d);
        break;
    case 'f':
        if (*(d->str))
            parse_edit_action(PARSE_FORMAT, actions, d);
        else
            write_to_output(d, "Current buffer empty.\r\n");
        break;
    case 'i':
        if (*(d->str))
            parse_edit_action(PARSE_INSERT, actions, d);
        else
            write_to_output(d, "Current buffer empty.\r\n");
        break;
    case 'h':
        parse_edit_action(PARSE_HELP, actions, d);
        break;
    case 'l':
        if (*d->str)
            parse_edit_action(PARSE_LIST_NORM, actions, d);
        else
            write_to_output(d, "Current buffer empty.\r\n");
        break;
    case 'n':
        if (*d->str)
            parse_edit_action(PARSE_LIST_NUM, actions, d);
        else
            write_to_output(d, "Current buffer empty.\r\n");
        break;
    case 'r':
        parse_edit_action(PARSE_REPLACE, actions, d);
        break;
    case 's':
    return STRINGADD_SAVE;
    default:
        write_to_output(d, "Invalid option.\r\n");
        break;
    } /* switch */
  return STRINGADD_ACTION;
}

/* ************************************************************************
*  modification of malloc'ed strings                                      *
************************************************************************ */
static void parse_edit_action( int command, char *string, DescriptorData *d )
{
  int indent = 0, rep_all = 0, flags = 0, replaced, i, line_low, line_high, j = 0;
  unsigned int total_len;
  char *s, *t, temp;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  switch (command) {
  case PARSE_HELP:
    write_to_output(d,
                    	"Editor command formats:\r\n"
	"&13/a&00         -  aborts editor\r\n"
	"&13/c&00         -  clears buffer\r\n"
	"&13/d#&00        -  deletes a line #\r\n"
	"&13/e#&00 <text> -  changes the line at # with <text>\r\n"
	"&13/f&00         -  formats text\r\n"
	"&13/fi&00        -  indented formatting of text\r\n"
	"&13/h&00         -  list text editor commands\r\n"
	"&13/i#&00 <text> -  inserts <text> before line #\r\n"
	"&13/l&00         -  lists buffer\r\n"
	"&13/n&00         -  lists buffer with line numbers\r\n"
	"&13/r&00  'a' 'b'-  replace 1st occurance of text <a> in buffer with text <b>\r\n"
	"&13/ra&00 'a' 'b'-  replace all occurances of text <a> within buffer with text <b>\r\n"
    "                    usage: /r[a] 'pattern' 'replacement'\r\n"
    "&13/s&00         -  saves text\r\n");
    break;
  case PARSE_FORMAT:
    if (STATE(d) == CON_TRIGEDIT) {
      write_to_output(d, "Script %sformatted.\r\n", format_script(d) ? "": "not ");
      return;
    }
    while (isalpha(string[j]) && j < 2) {
      if (string[j++] == 'i' && !indent) {
	indent = TRUE;
          flags += FORMAT_INDENT;
        }
    }
    switch (sscanf((indent ? string + 1 : string), " %d - %d ", &line_low, &line_high))
    {
    case -1:
    case 0:
      line_low = 1;
      line_high = 999999;
        break;
    case 1:
      line_high = line_low;
        break;
    case 2:
      if (line_high < line_low) {
        write_to_output(d, "That range is invalid.\\r\\n");
        return;
      }
    break;
    }
    /* in case line_low is negative or zero */
    line_low = MAX(1, line_low);

    format_text(d->str, flags, d, d->max_str, line_low, line_high);
    write_to_output(d, "Text formatted with%s indent.\r\n", (indent ? "" : "out"));
    break;
  case PARSE_REPLACE:
    while (isalpha(string[j]) && j < 2)
      if (string[j++] == 'a' && !indent)
          rep_all = 1;

    if ((s = strtok(string, "'")) == NULL) {
      write_to_output(d, "Invalid format.\r\n");
      return;
    } else if ((s = strtok(NULL, "'")) == NULL) {
      write_to_output(d, "Target string must be enclosed in single quotes.\r\n");
      return;
    } else if ((t = strtok(NULL, "'")) == NULL) {
      write_to_output(d, "No replacement string.\r\n");
      return;
    } else if ((t = strtok(NULL, "'")) == NULL) {
      write_to_output(d, "Replacement string must be enclosed in single quotes.\r\n");
      return;
      /*wb's fix for empty buffer replacement crashing */
    } else if ((!*d->str)) {
      return;
    } else if ((total_len = ((strlen(t) - strlen(s)) + strlen(*d->str))) <= d->max_str) {
      if ((replaced = replace_str(d->str, s, t, rep_all, d->max_str)) > 0) {
	write_to_output(d, "Replaced %d occurance%sof '%s' with '%s'.\r\n", replaced, ((replaced != 1) ? "s " : " "), s, t);
      } else if (replaced == 0) {
	write_to_output(d, "String '%s' not found.\r\n", s);
      } else
	write_to_output(d, "ERROR: Replacement string causes buffer overflow, aborted replace.\r\n");
    } else
      write_to_output(d, "Not enough space left in buffer.\r\n");
    break;
  case PARSE_DELETE:
      switch (sscanf(string, " %d - %d ", &line_low, &line_high)) {
       case 0:
   write_to_output(d, "You must specify a line number or range to delete.\r\n");
   return;
       case 1:
   line_high = line_low;
   break;
       case 2:
   if (line_high < line_low) {
      write_to_output(d, "That range is invalid.\r\n");
      return;
   }
   break;
      }

      i = 1;
      total_len = 1;
      if ((s = *d->str) == NULL) {
   write_to_output(d, "Buffer is empty.\r\n");
   return;
    } else if (line_low > 0) {
      while (s && i < line_low)
     if ((s = strchr(s, '\n')) != NULL) {
        i++;
        s++;
     }
      if (s == NULL || i < line_low) {
      write_to_output(d, "Line(s) out of range; not deleting.\r\n");
      return;
   }
   t = s;
      while (s && i < line_high)
     if ((s = strchr(s, '\n')) != NULL) {
         i++;
         total_len++;
         s++;
     }
      if (s && (s = strchr(s, '\n')) != NULL) {
	while (*(++s))
	  *(t++) = *s;
      } else
	total_len--;
   *t = '\0';
   RECREATE(*d->str, char, strlen(*d->str) + 3);

      write_to_output(d, "%d line%sdeleted.\r\n", total_len, (total_len != 1 ? "s " : " "));
    } else {
      write_to_output(d, "Invalid, line numbers to delete must be higher than 0.\r\n");
   return;
      }
      break;
  case PARSE_LIST_NORM:
    /* Note: Rv's buf, buf1, buf2, and arg variables are defined to 32k so they
     * are ok for what we do here. */
    *buf = '\0';
    if (*string)
      switch (sscanf(string, " %d - %d ", &line_low, &line_high)) {
      case 0:
        line_low = 1;
        line_high = 999999;
        break;
      case 1:
        line_high = line_low;
       break;
    } else {
      line_low = 1;
      line_high = 999999;
    }

    if (line_low < 1) {
      write_to_output(d, "Line numbers must be greater than 0.\r\n");
      return;
    } else if (line_high < line_low) {
      write_to_output(d, "That range is invalid.\r\n");
      return;
    }
      *buf = '\0';
    if (line_high < 999999 || line_low > 1)
   sprintf(buf, "Current buffer range [%d - %d]:\r\n", line_low, line_high);
      i = 1;
      total_len = 0;
      s = *d->str;
      while (s && (i < line_low))
  if ((s = strchr(s, '\n')) != NULL) {
     i++;
     s++;
  }
    if (i < line_low || s == NULL) {
   write_to_output(d, "Line(s) out of range; no buffer listing.\r\n");
   return;
      }
      t = s;
    while (s && i <= line_high)
  if ((s = strchr(s, '\n')) != NULL) {
     i++;
     total_len++;
     s++;
  }
    if (s) {
   temp = *s;
   *s = '\0';
   strcat(buf, t);
   *s = temp;
    } else
      strcat(buf, t);
    sprintf(buf + strlen(buf), "\r\n%d line%sshown.\r\n", total_len, (total_len != 1) ? "s " : " ");
      SEND_RULER(d);
      page_string(d, buf, TRUE);
      break;
    case PARSE_LIST_NUM:
    /* Note: Rv's buf, buf1, buf2, and arg variables are defined to 32k so they
     * are probably ok for what we do here. */
      *buf = '\0';
    if (*string)
  switch (sscanf(string, " %d - %d ", &line_low, &line_high)) {
   case 0:
     line_low = 1;
     line_high = 999999;
     break;
   case 1:
     line_high = line_low;
     break;
    } else {
   line_low = 1;
   line_high = 999999;
      }

      if (line_low < 1) {
   write_to_output(d, "Line numbers must be greater than 0.\r\n");
   return;
      }
      if (line_high < line_low) {
   write_to_output(d, "That range is invalid.\r\n");
   return;
      }
      *buf = '\0';
      i = 1;
      total_len = 0;
      s = *d->str;
    while (s && i < line_low)
  if ((s = strchr(s, '\n')) != NULL) {
     i++;
     s++;
  }
    if (i < line_low || s == NULL) {
   write_to_output(d, "Line(s) out of range; no buffer listing.\r\n");
   return;
      }
      t = s;
    while (s && i <= line_high)
  if ((s = strchr(s, '\n')) != NULL) {
     i++;
     total_len++;
     s++;
     temp = *s;
     *s = '\0';
	sprintf(buf, "%s%4d: ", buf, (i - 1));
     strcat(buf, t);
     *s = temp;
     t = s;
  }
      if (s && t) {
   temp = *s;
   *s = '\0';
   strcat(buf, t);
   *s = temp;
    } else if (t)
      strcat(buf, t);

      page_string(d, buf, TRUE);
      break;

    case PARSE_INSERT:
      half_chop(string, buf, buf2);
      if (*buf == '\0') {
   write_to_output(d, "You must specify a line number before which to insert text.\r\n");
   return;
      }
      line_low = atoi(buf);
      strcat(buf2, "\r\n");

      i = 1;
      *buf = '\0';
      if ((s = *d->str) == NULL) {
   write_to_output(d, "Buffer is empty, nowhere to insert.\r\n");
   return;
      }
      if (line_low > 0) {
           while (s && (i < line_low))
     if ((s = strchr(s, '\n')) != NULL) {
        i++;
        s++;
     }
      if (i < line_low || s == NULL) {
      write_to_output(d, "Line number out of range; insert aborted.\r\n");
      return;
   }
   temp = *s;
   *s = '\0';
      if ((strlen(*d->str) + strlen(buf2) + strlen(s + 1) + 3) > d->max_str) {
      *s = temp;
      write_to_output(d, "Insert text pushes buffer over maximum size, insert aborted.\r\n");
       return;
   }
      if (*d->str && **d->str)
	strcat(buf, *d->str);
   *s = temp;
   strcat(buf, buf2);
      if (s && *s)
	strcat(buf, s);
   RECREATE(*d->str, char, strlen(buf) + 3);

   strcpy(*d->str, buf);
   write_to_output(d, "Line inserted.\r\n");
    } else {
   write_to_output(d, "Line number must be higher than 0.\r\n");
   return;
      }
      break;

    case PARSE_EDIT:
      half_chop(string, buf, buf2);
      if (*buf == '\0') {
   write_to_output(d, "You must specify a line number at which to change text.\r\n");
   return;
      }
      line_low = atoi(buf);
      strcat(buf2, "\r\n");

      i = 1;
      *buf = '\0';
      if ((s = *d->str) == NULL) {
   write_to_output(d, "Buffer is empty, nothing to change.\r\n");
   return;
      }
      if (line_low > 0) {
      /* Loop through the text counting \n characters until we get to the line. */
      while (s && i < line_low)
     if ((s = strchr(s, '\n')) != NULL) {
        i++;
        s++;
     }
      /* Make sure that there was a THAT line in the text. */
      if (s == NULL || i < line_low) {
      write_to_output(d, "Line number out of range; change aborted.\r\n");
      return;
   }
      /* If s is the same as *d->str that means I'm at the beginning of the
       * message text and I don't need to put that into the changed buffer. */
   if (s != *d->str) {
	/* First things first .. we get this part into the buffer. */
      temp = *s;
      *s = '\0';
	/* Put the first 'good' half of the text into storage. */
      strcat(buf, *d->str);
      *s = temp;
   }
      /* Put the new 'good' line into place. */
   strcat(buf, buf2);
   if ((s = strchr(s, '\n')) != NULL) {
        /* This means that we are at the END of the line, we want out of there,
         * but we want s to point to the beginning of the line. AFTER the line
         * we want edited. */
      s++;
	/* Now put the last 'good' half of buffer into storage. */
      strcat(buf, s);
   }
      /* Check for buffer overflow. */
   if (strlen(buf) > d->max_str) {
      write_to_output(d, "Change causes new length to exceed buffer maximum size, aborted.\r\n");
      return;
   }
      /* Change the size of the REAL buffer to fit the new text. */
   RECREATE(*d->str, char, strlen(buf) + 3);
   strcpy(*d->str, buf);
   write_to_output(d, "Line changed.\r\n");
    } else {
   write_to_output(d, "Line number must be higher than 0.\r\n");
   return;
      }
      break;
    default:
      write_to_output(d, "Invalid option.\r\n");
      mudlog(BRF, LVL_IMPL, TRUE, "SYSERR: invalid command passed to parse_edit_action");
      return;
   }
}

/* Re-formats message type formatted char *. (for strings edited with d->str)
 * (mostly olc and mail). */
static int format_text(char **ptr_string, int mode, struct descriptor_data *d, unsigned int maxlen, int low, int high)
{
  int line_chars, cap_next = TRUE, cap_next_next = FALSE, color_chars = 0, i, pass_line = 0;
  char *flow, *start = NULL, temp;
  char formatted[MAX_STRING_LENGTH] = "";
  char str[MAX_STRING_LENGTH];

  /* Fix memory overrun. */
  if (d->max_str > MAX_STRING_LENGTH) {
    mlog("SYSERR: format_text: max_str is greater than buffer size.");
    return 0;
  }

  /* XXX: Want to make sure the string doesn't grow either... */
  if ((flow = *ptr_string) == NULL)
    return 0;

  strcpy(str, flow);

  for (i = 0; i < low - 1; i++) {
    start = strtok(str, "\n");
    if (!start) {
      write_to_output(d, "There aren't that many lines!\r\n");
      return 0;
    }
    strcat(formatted, strcat(start, "\n"));
    flow = strstr(flow, "\n");
    strcpy(str, ++flow);
  }

  if (IS_SET(mode, FORMAT_INDENT)) {
    strcat(formatted, "   ");
    line_chars = 3;
  } else {
    line_chars = 0;
  }

  while (*flow && i < high) {
    while (*flow && strchr("\n\r\f\t\v ", *flow)) {
      if (*flow == '\n' && !pass_line)
        if (i++ >= high) {
          pass_line = 1;
          break;
        }
      flow++;
    }

    if (*flow) {
      start = flow;
      while (*flow && !strchr("\n\r\f\t\v .?!", *flow)) {
        if (*flow == '&') {
          if (*(flow + 2) == '&')
            color_chars++;
          else
            color_chars += 3;
          flow++;
        }
        flow++;
      }

      if (cap_next_next) {
        cap_next_next = FALSE;
        cap_next = TRUE;
      }

      /* This is so that if we stopped on a sentence, we move off the sentence
       * delimiter. */
      while (strchr(".!?", *flow)) {
        cap_next_next = TRUE;
        flow++;
      }

      /* Special case: if we're at the end of the last line, and the last
       * character is a delimiter, the flow++ above will have *flow pointing
       * to the \r (or \n) character after the delimiter. Thus *flow will be
       * non-null, and an extra (blank) line might be added erroneously. We
       * fix it by skipping the newline characters in between. - Welcor */
      if (strchr("\n\r", *flow)) {
        *flow = '\0';  /* terminate 'start' string */
        flow++;        /* we know this is safe     */
        if (*flow == '\n' && i++ >= high)
          pass_line = 1;

        while (*flow && strchr("\n\r", *flow) && !pass_line) {
          flow++;      /* skip to next non-delimiter */
          if (*flow == '\n' && i++ >= high)
            pass_line = 1;
        }
        temp = *flow;  /* save this char             */
     } else {
        temp = *flow;
        *flow = '\0';
      }

      if (line_chars + strlen(start) + 1 - color_chars > PAGE_WIDTH) {
        strcat(formatted, "\r\n");
        line_chars = 0;
        color_chars = count_color_chars(start);
      }

      if (!cap_next) {
        if (line_chars > 0) {
          strcat(formatted, " ");
          line_chars++;
        }
      } else {
        cap_next = FALSE;
        CAP(start);
      }

      line_chars += strlen(start);
      strcat(formatted, start);

      *flow = temp;
    }

    if (cap_next_next && *flow) {
      if (line_chars + 3 - color_chars > PAGE_WIDTH) {
        strcat(formatted, "\r\n");
        line_chars = 0;
        color_chars = count_color_chars(start);
      } else if (*flow == '\"' || *flow == '\'') {
        char buf[MAX_STRING_LENGTH];
        sprintf(buf, "%c  ", *flow);
        strcat(formatted, buf);
        flow++;
        line_chars++;
      } else {
        strcat(formatted, "  ");
        line_chars += 2;
      }
    }
  }
  if (*flow)
    strcat(formatted, "\r\n");
  strcat(formatted, flow);
  if (!*flow)
    strcat(formatted, "\r\n");

  if (strlen(formatted) + 1 > maxlen)
    formatted[maxlen - 1] = '\0';
  RECREATE(*ptr_string, char, MIN(maxlen, strlen(formatted) + 1));
  strcpy(*ptr_string, formatted);
  return 1;
}

/* Basic API function to start writing somewhere. 'data' isn't used, but you
 * can use it to pass whatever else you may want through it.  The improved
 * editor patch when updated could use it to pass the old text buffer, for
 * instance. */
void string_write(struct descriptor_data *d, char **writeto, size_t len, long mailto, void *data)
{
  if (d->character && !IS_NPC(d->character))
    SET_BIT_AR(PLR_FLAGS(d->character), PLR_WRITING);

  if (data)
    free(data);

  d->str = writeto;
  d->max_str = len;
  d->mail_to = mailto;
}

/* Add user input to the 'current' string (as defined by d->str) */
void string_add(DescriptorData * d, char *str)
{
  int action;

  /* determine if this is the terminal string, and truncate if so */
  /* changed to accept '/<letter>' style string editing commands(imp_edit) */

  delete_doubledollar(str);
  smash_tilde(str);

  /* Determine if this is the terminal string, and truncate if so. Changed to
   * only accept '@' if it's by itself. - fnord */
  if ((action = (*str == '@' && !str[1])))
    *str = '\0';
  else
    if ((action = improved_editor_execute(d, str)) == STRINGADD_ACTION)
      return;

  if (action != STRINGADD_OK)
    /* Do nothing. */ ;
  else if (!(*d->str)) {
    if (strlen(str) + 3 > d->max_str) { /* \r\n\0 */
      sendChar(d->character, "String too long - Truncated.\r\n");
      strcpy(&str[d->max_str - 3], "\r\n");	/* strcpy: OK (size checked) */
      CREATE(*d->str, char, d->max_str);
      strcpy(*d->str, str);	/* strcpy: OK (size checked) */
    } else {
    CREATE(*d->str, char, strlen(str) + 3);
    strcpy(*d->str, str);	/* strcpy: OK (size checked) */
  }
  } else {
    if (strlen(str) + strlen(*d->str) + 3 > d->max_str) { /* \r\n\0 */
      sendChar(d->character, "String too long.  Last line skipped.\r\n");
      if (action == STRINGADD_OK)
        action = STRINGADD_ACTION;    /* No appending \r\n\0, but still let them save. */
    } else {
      RECREATE(*d->str, char, strlen(*d->str) + strlen(str) + 3); /* \r\n\0 */
      strcat(*d->str, str);	/* strcat: OK (size precalculated) */
    }
      }

  /* Common cleanup code. */
  switch (action) {
    case STRINGADD_ABORT:
      switch (STATE(d)) {
        case CON_TEDIT:
        case CON_REDIT:
        case CON_MEDIT:
        case CON_OEDIT:
        case CON_PLR_DESC:
        case CON_TRIGEDIT:
        case CON_QEDIT:
        case CON_CEDIT:
        case CON_HEDIT:
        free(*d->str);
            *d->str = d->backstr;
        d->backstr = NULL;
        d->str = NULL;
          break;
        default:
          mlog("SYSERR: string_add: Aborting write from unknown origin.");
          break;
    }
      break;
    case STRINGADD_SAVE:
      if (d->str && *d->str && **d->str == '\0') {
        free(*d->str);
        *d->str = strdup("Nothing.\r\n");
    }
      if (d->backstr)
        free(d->backstr);
      d->backstr = NULL;
            break;
    case STRINGADD_ACTION:
            break;
       }

  /* Ok, now final cleanup. */
  if (action == STRINGADD_SAVE || action == STRINGADD_ABORT) {
    int i;
    struct {
      int mode;
      void (*func)(struct descriptor_data *d, int action);
    } cleanup_modes[] = {
      { CON_MEDIT  , medit_string_cleanup },
      { CON_OEDIT  , oedit_string_cleanup },
      { CON_REDIT  , redit_string_cleanup },
      { CON_TEDIT  , tedit_string_cleanup },
      { CON_TRIGEDIT, trigedit_string_cleanup },
      { CON_PLR_DESC , exdesc_string_cleanup },
      { CON_PLAYING, playing_string_cleanup },
      { CON_QEDIT  , qedit_string_cleanup },
      { CON_CEDIT  , cedit_string_cleanup },
      { CON_HEDIT, hedit_string_cleanup },
      { -1, NULL }
    };

    for (i = 0; cleanup_modes[i].func; i++)
      if (STATE(d) == cleanup_modes[i].mode)
        (*cleanup_modes[i].func)(d, action);

    /* Common post cleanup code. */
    d->str = NULL;
    d->mail_to = 0;
    d->max_str = 0;
    if (d->character && !IS_NPC(d->character)) {
      REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_MAILING);
      REMOVE_BIT_AR(PLR_FLAGS(d->character), PLR_WRITING);
    }
      } else if (action != STRINGADD_ACTION && strlen(*d->str) + 3 <= d->max_str) /* 3 = \r\n\0 */
     strcat(*d->str, "\r\n");
}

static void playing_string_cleanup(struct descriptor_data *d, int action) {
    if (PLR_FLAGGED(d->character, PLR_MAILING)) {
        if (action == STRINGADD_SAVE && *d->str) {
            store_mail(d->mail_to, GET_IDNUM(d->character), *d->str);
            write_to_output(d, "Message sent!\r\n");
        } else
            write_to_output(d, "Mail aborted.\r\n");
        d->mail_to = 0;
        free(*d->str);
        free(d->str);
    }

    /* We have no way of knowing which slot the post was sent to so we can only
     * give the message.   */
    if (d->mail_to >= BOARD_MAGIC) {
        board_save_board(d->mail_to - BOARD_MAGIC);
        if (action == STRINGADD_ABORT)
            write_to_output(d, "Post not aborted, use REMOVE <post #>.\r\n");
    }
}

static void exdesc_string_cleanup(struct descriptor_data *d, int action) {
    if (action == STRINGADD_ABORT)
        write_to_output(d, "Description aborted.\r\n");

    write_to_output(d, "%s", CONFIG_MENU);
    STATE(d) = CON_MENU;
}

/* **********************************************************************
*  Modification of character skills                                     *
********************************************************************** */

ACMD(do_skillset)
{
  CharData *vict;
  char name[100], buf2[100], buf[100], help[MAX_STRING_LENGTH];
  int skill, value, i, qend;

  argument = one_argument(argument, name);

  if (!*name) {			/* no arguments. print an informative text */
    send_to_char("Syntax: skillset <name> '<skill>' <value>\r\n", ch);
    strcpy(help, "Skill being one of the following:\n\r");
    for (i = 0; *spells[i] != '\n'; i++) {
      if (*spells[i] == '!')
	continue;
      sprintf(help + strlen(help), "%18s", spells[i]);
      if (i % 4 == 3) {
	strcat(help, "\r\n");
	send_to_char(help, ch);
	*help = '\0';
      }
    }
    if (*help)
      send_to_char(help, ch);
    send_to_char("\n\r", ch);
    return;
  }
  if (!(vict = get_char_vis(ch, name, 0))) {
    sendChar( ch, CONFIG_NOPERSON );
    return;
  }
  skip_spaces(&argument);

  /* If there is no chars in argument */
  if (!*argument) {
    send_to_char("Skill name expected.\n\r", ch);
    return;
  }
  if (*argument != '\'') {
    send_to_char("Skill must be enclosed in: ''\n\r", ch);
    return;
  }
  /* Locate the last quote && lowercase the magic words (if any) */

  for (qend = 1; *(argument + qend) && (*(argument + qend) != '\''); qend++)
    *(argument + qend) = LOWER(*(argument + qend));

  if (*(argument + qend) != '\'') {
    send_to_char("Skill must be enclosed in: ''\n\r", ch);
    return;
  }
  strcpy(help, (argument + 1));
  help[qend - 1] = '\0';
  if ((skill = find_skill_num(help)) <= 0) {
    send_to_char("Unrecognized skill.\n\r", ch);
    return;
  }
  argument += qend + 1;		/* skip to next parameter */
  argument = one_argument(argument, buf);

  if (!*buf) {
    send_to_char("Learned value expected.\n\r", ch);
    return;
  }
  value = atoi(buf);
  if (value < 0) {
    send_to_char("Minimum value for learned is 0.\n\r", ch);
    return;
  }
  if (value > 100) {
    send_to_char("Max value for learned is 100.\n\r", ch);
    return;
  }
  if (IS_NPC(vict)) {
    send_to_char("You can't set NPC skills.\n\r", ch);
    return;
  }
  mudlog(BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), TRUE, "%s changed %s's %s to %d.", GET_NAME(ch), GET_NAME(vict),
	  spells[skill], value);

  SET_SKILL(vict, skill, value);

  sprintf(buf2, "You change %s's %s to %d.\n\r", GET_NAME(vict),
	  spells[skill], value);
  send_to_char(buf2, ch);
}

/* db stuff *********************************************** */

/* The call that gets the paging ball rolling... */
void page_string(struct descriptor_data *d, char *str, int keep_internal)
{
  if (!d)
    return;

    if (!str || !*str)
    return;

  if (keep_internal) {
    CREATE(d->showstr_head, char, strlen(str) + 1);
    strcpy(d->showstr_head, str);
    d->showstr_point = d->showstr_head;
  } else
    d->showstr_point = str;

  show_string(d, "");
}

/* The call that displays the next page. */
void show_string(DescriptorData * d, char *input)
{
  char buffer[MAX_STRING_LENGTH], buf[MAX_INPUT_LENGTH];
  register char *scan, *chk;
  int lines = 0, toggle = 1;

  one_argument(input, buf);

  if (*buf) {
    if (d->showstr_head) {
      free(d->showstr_head);
      d->showstr_head = 0;
    }
    d->showstr_point = 0;
    return;
  }
  /* show a chunk */
  for (scan = buffer;; scan++, d->showstr_point++)
    if ((((*scan = *d->showstr_point) == '\n') || (*scan == '\r')) &&
	((toggle = -toggle) < 0))
      lines++;
    else if (!*scan || (lines >= 22)) {
      *scan = '\0';
      write_to_output(d,"%s", buffer);

      /* see if this is the end (or near the end) of the string */
      for (chk = d->showstr_point; isspace(*chk); chk++);
      if (!*chk) {
	if (d->showstr_head) {
	  free(d->showstr_head);
	  d->showstr_head = 0;
	}
	d->showstr_point = 0;
      }
      return;
    }
}
