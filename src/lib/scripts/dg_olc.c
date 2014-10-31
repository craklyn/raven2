/*
** dg_olc.c: this source file is used in extending Oasis style OLC for
** dg-scripts onto a CircleMUD that already has dg-scripts (as released
** by Mark Heilpern on 1/1/98) implemented.
**
** Parts of this file by Chris Jacobson of _Aliens vs Predator: The MUD_
*/


#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "util/utils.h"
#include "general/comm.h"
#include "olc/olc.h"
#include "scripts/dg_olc.h"
#include "scripts/dg_event.h"
#include "general/class.h"

/* declare externally defined globals */
extern struct index_data **trig_index;
extern char *trig_types[], *otrig_types[], *wtrig_types[];
extern struct trig_data *trigger_list;
extern int top_of_trigt;
extern struct zone_data *zone_table;


/* prototype externally defined functions */
void trig_data_copy(trig_data *this_data, const trig_data *trg);
void free_varlist(struct trig_var_data *vd);

void trigedit_disp_menu(struct descriptor_data *d);
void trigedit_save(struct descriptor_data *d);
void trigedit_create_index(int znum, char *type);

/* copy an entire script from one holder (mob/obj/room) to another */
void script_copy(void *dst, void *src, int type)
{
  struct script_data *s_src = NULL;
  struct script_data *s_dst = NULL;
  trig_data *t_src, *t_dst;

  /* find the scripts of the source and destination */
  switch (type)
  {
    case MOB_TRIGGER:
      s_src = SCRIPT((struct char_data *)src);
      s_dst = SCRIPT((struct char_data *)dst);
      ((struct char_data *)dst)->proto_script =
        ((struct char_data *)src)->proto_script;
     break;
    case OBJ_TRIGGER:
      s_src = SCRIPT((struct obj_data *)src);
      s_dst = SCRIPT((struct obj_data *)dst);
      ((struct obj_data *)dst)->proto_script =
        ((struct obj_data *)src)->proto_script;
      break;
    case WLD_TRIGGER:
      s_src = SCRIPT((struct room_data *)src);
      s_dst = SCRIPT((struct room_data *)dst);
      ((struct room_data *)dst)->proto_script =
        ((struct room_data *)src)->proto_script;
      break;
    default:
      mlog("TRIGERR: Unknown type code sent to script_copy()!");
      break;
  }

  /* make sure the dst doesnt already have a script       */
  /* if it does, delete it                                */
  if (s_dst) extract_script(s_dst, type);

  /* copy the scrip data */
  s_dst->types = s_src->types;
  t_src = TRIGGERS(s_src);
  while (t_src)
  {
    CREATE(t_dst, trig_data, 1);
    if (!TRIGGERS(s_dst)) TRIGGERS(s_dst) = t_dst;
    trig_data_copy(t_dst, t_src);
    t_dst = t_dst->next;
    t_src = t_src->next;
  }

}

/* called when a mob or object is being saved to disk, so its script can */
/* be saved */
void script_save_to_disk(FILE *fp, void *item, int type)
{
  struct trig_proto_list *t;

  if (type==MOB_TRIGGER)
    t = ((struct char_data *)item)->proto_script;
  else if (type==OBJ_TRIGGER)
    t = ((struct obj_data *)item)->proto_script;
  else if (type==WLD_TRIGGER)
    t = ((struct room_data *)item)->proto_script;
  else {
    mlog("TRIGERR: Invalid type passed to script_save_mobobj_to_disk()");
    return;
  }

  while (t)
  {
    fprintf(fp,"Z %d\n", t->vnum);
    t = t->next;
  }
}


void trigedit_setup_new(struct descriptor_data *d)
{
  struct trig_data *trig;
  
  /*
   * Allocate a scratch trigger structure
   */
  CREATE(trig, struct trig_data, 1);

  trig->nr = -1;

  /*
   * Set up some defaults
   */ 
  trig->name = str_dup("new trigger");
  trig->trigger_type = MTRIG_GREET;

  /* cmdlist will be a large char string until the trigger is saved */
  CREATE(OLC_STORAGE(d), char, MAX_CMD_LENGTH);
  strcpy(OLC_STORAGE(d),
    "say My trigger commandlist is not complete!\r\n");
  trig->narg = 100;

  OLC_TRIG(d) = trig;
  OLC_VAL(d) = 0;  /* Has changed flag. (It hasn't so far, we just made it.) */
    
  trigedit_disp_menu(d);
}

void trigedit_setup_existing(struct descriptor_data *d, int rtrg_num)
{
  struct trig_data *trig;
  struct cmdlist_element *c;

  /*
   * Allocate a scratch trigger structure
   */
  CREATE(trig, struct trig_data, 1);

  trig_data_copy(trig, trig_index[rtrg_num]->proto);

  /* convert cmdlist to a char string */
  c = trig->cmdlist;
  CREATE(OLC_STORAGE(d), char, MAX_CMD_LENGTH);
  strcpy(OLC_STORAGE(d), "");
  
  while (c)
  {
    strcat(OLC_STORAGE(d), c->cmd);
    strcat(OLC_STORAGE(d), "\r\n");
    c = c->next;
  }
  /* now trig->cmdlist is something to pass to the text editor */
  /* it will be converted back to a real cmdlist_element list later */

  OLC_TRIG(d) = trig;
  OLC_VAL(d) = 0;  /* Has changed flag. (It hasn't so far, we just made it.) */
    
  trigedit_disp_menu(d);
}


void trigedit_disp_menu(struct descriptor_data *d)
{
  struct trig_data *trig = OLC_TRIG(d);
  char *attach_type;
  char trgtypes[256];

  get_char_colors(d->character);

  if (trig->attach_type==OBJ_TRIGGER) {
    attach_type = "Objects";
    sprintbit(GET_TRIG_TYPE(trig), otrig_types, trgtypes);
  } else if (trig->attach_type==WLD_TRIGGER) {
    attach_type = "Rooms";
    sprintbit(GET_TRIG_TYPE(trig), wtrig_types, trgtypes);
  } else {
    attach_type = "Mobiles";
    sprintbit(GET_TRIG_TYPE(trig), trig_types, trgtypes);
  }
      
  sprintf(buf,
#if defined(CLEAR_SCREEN)
"[H[J"
#endif

  "Trigger Editor [&02%d&00]\r\n\r\n"
  "&021)&06 Name         : &03%s\r\n"
  "&022)&06 Intended for : &03%s\r\n"
  "&023)&06 Trigger types: &03%s\r\n"
  "&024)&06 Numberic Arg : &03%d\r\n"
  "&025)&06 Arguments    : &03%s\r\n"
  "&026)&06 Commands:\r\n&03%s\r\n"
  "&02Q)&06 Quit\r\n"
  "Enter Choice :",

  OLC_NUM(d), 			/* vnum on the title line */
  GET_TRIG_NAME(trig),		/* name                   */
  attach_type,			/* attach type            */
  trgtypes,			/* greet/drop/etc         */
  trig->narg,			/* numeric arg            */
  trig->arglist?trig->arglist:"",/* strict arg             */
  OLC_STORAGE(d)		/* the command list       */
  );

  send_to_char(buf, d->character);
  OLC_MODE(d) = TRIGEDIT_MAIN_MENU;
}

void trigedit_disp_types(struct descriptor_data *d)
{
  int i, columns = 0;
  char **types;

  switch(OLC_TRIG(d)->attach_type)
  {
    case WLD_TRIGGER:
      types = wtrig_types;
      break;
    case OBJ_TRIGGER:
      types = otrig_types;
      break;
    case MOB_TRIGGER:
    default:
      types = trig_types;
      break;
  }

  get_char_colors(d->character);
#if defined(CLEAR_SCREEN)
  send_to_char("[H[J", d->character);
#endif
  for (i = 0; i < NUM_TRIG_TYPE_FLAGS; i++) {
  sprintf(buf, "%s%2d%s) %-20.20s  %s", grn, i + 1, nrm, types[i],
              !(++columns % 2) ? "\r\n" : "");
    send_to_char(buf, d->character);
  }
  sprintbit(GET_TRIG_TYPE(OLC_TRIG(d)), types, buf1);
  sprintf(buf, "\r\nCurrent types : %s%s%s\r\nEnter type (0 to quit) : ",
                cyn, buf1, nrm);
  send_to_char(buf, d->character);

}

void trigedit_parse(struct descriptor_data *d, char *arg)
{
  int i = 0;

  switch (OLC_MODE(d)) {
    case TRIGEDIT_MAIN_MENU:
     switch (tolower(*arg)) {
       case 'q':
         if (OLC_VAL(d)) { /* Anything been changed? */
           if (!GET_TRIG_TYPE(OLC_TRIG(d))) {
             send_to_char("Invalid Trigger Type! Answer a to abort quit!\r\n",
               d->character);     
           }
           send_to_char("Do you wish to save the changes to the trigger? (y/n): ",
             d->character);     
           OLC_MODE(d) = TRIGEDIT_CONFIRM_SAVESTRING;
         } else
           cleanup_olc(d, CLEANUP_ALL);
           return;
         case '1':
           OLC_MODE(d) = TRIGEDIT_NAME;
           send_to_char("Name: ", d->character);
           break;
         case '2':
           OLC_MODE(d) = TRIGEDIT_INTENDED;
           send_to_char("0: Mobiles, 1: Objects, 2: Rooms: ", d->character);
           break;
         case '3':
           OLC_MODE(d) = TRIGEDIT_TYPES;
           trigedit_disp_types(d);
           break;
         case '4':
           OLC_MODE(d) = TRIGEDIT_NARG;
           send_to_char("Numeric argument: ", d->character);
           break;
         case '5':
           OLC_MODE(d) = TRIGEDIT_ARGUMENT;
           send_to_char("Argument: ", d->character);
           break;
         case '6':
           OLC_MODE(d) = TRIGEDIT_COMMANDS;
           send_to_char(
             "Enter trigger commands: (/s saves /h for help)\r\n\r\n",
             d->character);
           d->backstr = NULL;
           if (OLC_STORAGE(d)) {
             send_to_char(OLC_STORAGE(d), d->character);
             d->backstr = str_dup(OLC_STORAGE(d));
           }
           d->str = &OLC_STORAGE(d);
           d->max_str = MAX_CMD_LENGTH;
           d->mail_to = 0;
           OLC_VAL(d) = 1;

           break;
         default:
           trigedit_disp_menu(d);
           return;
     }
     return;
    
    case TRIGEDIT_CONFIRM_SAVESTRING:
      switch(tolower(*arg)) {
        case 'y':
          trigedit_save(d);
          mudlog( CMP, MAX(LVL_ANGEL, GET_INVIS_LEV(d->character)), TRUE, "OLC: %s edits trigger %d", GET_NAME(d->character),
            OLC_NUM(d));
          /* fall through */
        case 'n':
          cleanup_olc(d, CLEANUP_ALL);
          return;
        case 'a': /* abort quitting */
          break;
        default:
          send_to_char("Invalid choice!\r\n", d->character);
          send_to_char("Do you wish to save the trigger? : ", d->character);
          return;
      }
      break;

    case TRIGEDIT_NAME:
      if (OLC_TRIG(d)->name)
        free(OLC_TRIG(d)->name);
      OLC_TRIG(d)->name = str_dup((arg && *arg) ? arg : "undefined");
      OLC_VAL(d)++;
      break;

    case TRIGEDIT_INTENDED:
      if ((atoi(arg)>=MOB_TRIGGER) || (atoi(arg)<=WLD_TRIGGER))
        OLC_TRIG(d)->attach_type = atoi(arg);
      OLC_VAL(d)++;
      break;

    case TRIGEDIT_NARG:
      OLC_TRIG(d)->narg = atoi(arg);
      OLC_VAL(d)++;
      break;

    case TRIGEDIT_ARGUMENT:
      OLC_TRIG(d)->arglist = (*arg?str_dup(arg):NULL);
      OLC_VAL(d)++;
      break;

    case TRIGEDIT_TYPES:
      if ((i = atoi(arg)) == 0)
        break;
      else if (!((i < 0) || (i > NUM_TRIG_TYPE_FLAGS)))
        TOGGLE_BIT((GET_TRIG_TYPE(OLC_TRIG(d))), 1 << (i - 1));
      OLC_VAL(d)++;
      trigedit_disp_types(d);
      return;

    case TRIGEDIT_COMMANDS:
      break;

  }

  OLC_MODE(d) = TRIGEDIT_MAIN_MENU;
  trigedit_disp_menu(d);
}

/*
** print out the letter codes pertaining to the bits set in 'data'
*/
void sprintbits(int data, char *dest)
{
  int i;
  char *p = dest;

  for (i=0; i<32; i++) {
    if (data & (1<<i)) {
      *p = ((i<=25)?('a'+i):('A'+i));
      p++;
    }
  }
  *p = '\0';
}


/* save the zone's triggers to internal memory and to disk */
void trigedit_save(struct descriptor_data *d)
{
  int trig_rnum, i;
  int found = 0;
  char *s;
  trig_data *proto;
  trig_data *trig = OLC_TRIG(d);
  trig_data *live_trig;
  struct cmdlist_element *cmd, *next_cmd;
  struct index_data **new_index;
  struct descriptor_data *dsc;
  FILE *trig_file;
  int zone, top;
  char buf[MAX_CMD_LENGTH];
  char bitBuf[MAX_INPUT_LENGTH];
  char fname[MAX_INPUT_LENGTH];
  char logbuf[MAX_INPUT_LENGTH];


  if ((trig_rnum = real_trigger(OLC_NUM(d))) != -1) {
    proto = trig_index[trig_rnum]->proto;
    for (cmd = proto->cmdlist; cmd; cmd = next_cmd) { 
      next_cmd = cmd->next;
      if (cmd->cmd)
        free(cmd->cmd);
      free(cmd);
    }


    free(proto->arglist);
    free(proto->name);

    /* Recompile the command list from the new script */
    s = OLC_STORAGE(d);

    CREATE(trig->cmdlist, struct cmdlist_element, 1);
    trig->cmdlist->cmd = str_dup(strtok(s, "\n\r"));
    cmd = trig->cmdlist;

    while ((s = strtok(NULL, "\n\r"))) {
      CREATE(cmd->next, struct cmdlist_element, 1);
      cmd = cmd->next;
      cmd->cmd = str_dup(s);
    }

    /* make the prorotype look like what we have */
    trig_data_copy(proto, trig);

    /* go through the mud and replace existing triggers         */
    live_trig = trigger_list;
    while (live_trig)
    {
      if (GET_TRIG_RNUM(live_trig) == trig_rnum) {
        if (live_trig->arglist) {
          free(live_trig->arglist);
          live_trig->arglist = NULL;
        }
        if (live_trig->name) {
          free(live_trig->name);
          live_trig->name = NULL;
        }

        if (proto->arglist)
          live_trig->arglist = str_dup(proto->arglist);
        if (proto->name)
          live_trig->name = str_dup(proto->name);

        live_trig->cmdlist = proto->cmdlist;
        live_trig->curr_state = live_trig->cmdlist;
        live_trig->trigger_type = proto->trigger_type;
        live_trig->attach_type = proto->attach_type;
        live_trig->narg = proto->narg;
        live_trig->data_type = proto->data_type;
        live_trig->depth = 0;
        live_trig->wait_event = NULL;
        if (GET_TRIG_WAIT(live_trig))
          cancel_event(GET_TRIG_WAIT(live_trig));
        free_varlist(live_trig->var_list);
      }

      live_trig = live_trig->next_in_world;
    }
  } else {
    /* this is a new trigger */
    CREATE(new_index, struct index_data *, top_of_trigt + 2);

    /* Recompile the command list from the new script */
    s = OLC_STORAGE(d);
         
    CREATE(trig->cmdlist, struct cmdlist_element, 1);
    trig->cmdlist->cmd = str_dup(strtok(s, "\n\r"));
    cmd = trig->cmdlist;
                                
    while ((s = strtok(NULL, "\n\r"))) {
      CREATE(cmd->next, struct cmdlist_element, 1);
      cmd = cmd->next;
      cmd->cmd = str_dup(s);
    }

    for (i = 0; i < top_of_trigt; i++) {
      if (!found) {
        if (trig_index[i]->virtual > OLC_NUM(d)) {
          found = TRUE;
          trig_rnum = i;
                        
          CREATE(new_index[trig_rnum], struct index_data, 1);
          GET_TRIG_RNUM(OLC_TRIG(d)) = trig_rnum;
          new_index[trig_rnum]->virtual = OLC_NUM(d);
          new_index[trig_rnum]->number = 0; 
          new_index[trig_rnum]->func = NULL;
          CREATE(proto, struct trig_data, 1);
          new_index[trig_rnum]->proto = proto;
          trig_data_copy(proto, trig);

          if (trig->name)
            proto->name = str_dup(trig->name);
          if (trig->arglist)
            proto->arglist = str_dup(trig->arglist);  

          new_index[trig_rnum + 1] = trig_index[trig_rnum];

          proto = trig_index[trig_rnum]->proto;
          proto->nr = trig_rnum + 1;
        } else {
          new_index[i] = trig_index[i];
        }
      } else {
         new_index[i + 1] = trig_index[i];
         proto = trig_index[i]->proto;
         proto->nr = i + 1;
      }
    }

    if (!found) {
      trig_rnum = i;
      CREATE(new_index[trig_rnum], struct index_data, 1);
      GET_TRIG_RNUM(OLC_TRIG(d)) = trig_rnum;  
      new_index[trig_rnum]->virtual = OLC_NUM(d);
      new_index[trig_rnum]->number = 0;
      new_index[trig_rnum]->func = NULL;
                        
      CREATE(proto, struct trig_data, 1);
      new_index[trig_rnum]->proto = proto;
      trig_data_copy(proto, trig);

      if (trig->name)
        proto->name = str_dup(trig->name);
      if (trig->arglist)
        proto->arglist = str_dup(trig->arglist);  
    }
                
    free(trig_index);
                        
    trig_index = new_index;
    top_of_trigt++;         

    /* HERE IT HAS TO GO THROUGH AND FIX ALL SCRIPTS/TRIGS OF HIGHER RNUM */
    for (live_trig = trigger_list; live_trig; live_trig = live_trig->next_in_world)
      if (GET_TRIG_RNUM(live_trig) > trig_rnum)
        GET_TRIG_RNUM(live_trig)++;
        
    /*
     * Update other trigs being edited.
     */
     for (dsc = descriptor_list; dsc; dsc = dsc->next)
       if (dsc->connected == CON_TRIGEDIT)
         if (GET_TRIG_RNUM(OLC_TRIG(dsc)) >= trig_rnum)
           GET_TRIG_RNUM(OLC_TRIG(dsc))++;

  }

  /* now write the trigger out to disk, along with the rest of the  */
  /* triggers for this zone, of course                              */
  /* note: we write this to disk NOW instead of letting the builder */
  /* have control because if we lose this after having assigned a   */
  /* new trigger to an item, we will get SYSERR's upton reboot that */
  /* could make things hard to debug.                               */

  zone = zone_table[OLC_ZNUM(d)].number;
  top = zone_table[OLC_ZNUM(d)].top;

#ifdef CIRCLE_MAC
  sprintf(fname, "%s:%i.new", TRG_PREFIX, zone);
#else
  sprintf(fname, "%s/%i.new", TRG_PREFIX, zone);
#endif

  if (!(trig_file = fopen(fname, "w"))) {
    mudlog(BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE,"SYSERR: OLC: Can't open trig file \"%s\"", fname);
    return;
  }
        
  for (i = zone * 100; i <= top; i++) {
    if ((trig_rnum = real_trigger(i)) != -1) {
      trig = trig_index[trig_rnum]->proto;

      if (fprintf(trig_file, "#%d\n", i) < 0) {
        mudlog(BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(d->character)), TRUE, "SYSERR: OLC: Can't write trig file!");
        fclose(trig_file);
        return;
      }
      sprintbits(GET_TRIG_TYPE(trig), bitBuf);
      fprintf(trig_file,      "%s~\n"
                              "%d %s %d\n"
                              "%s~\n",
           (GET_TRIG_NAME(trig)) ? (GET_TRIG_NAME(trig)) : "unknown trigger",
           trig->attach_type,
           bitBuf, GET_TRIG_NARG(trig),
           GET_TRIG_ARG(trig) ? GET_TRIG_ARG(trig) : "");
                
      /* Build the text for the script */
      strcpy(buf,"");
      for (cmd = trig->cmdlist; cmd; cmd = cmd->next) {
        strcat(buf, cmd->cmd);
        strcat(buf, "\r\n");
      }

      if (!buf[0])
        strcpy(buf, "* Empty script");
                
      fprintf(trig_file, "%s~\n", buf);
      *buf = '\0';
    }
  }
        
  fprintf(trig_file, "$~\n");
  fclose(trig_file);
                
#ifdef CIRCLE_MAC
  sprintf(buf, "%s:%d.trg", TRG_PREFIX, zone);
#else
  sprintf(buf, "%s/%d.trg", TRG_PREFIX, zone);
#endif

  remove(buf);        
  rename(fname, buf);

  send_to_char("Saving Index file\r\n", d->character);
  trigedit_create_index(zone, "trg");
}


void trigedit_create_index(int znum, char *type)
{
  FILE *newfile, *oldfile;
  char new_name[32], old_name[32], *prefix;
  int num, found = FALSE;

  prefix = TRG_PREFIX;

  sprintf(old_name, "%s/index", prefix);
  sprintf(new_name, "%s/newindex", prefix);

  if (!(oldfile = fopen(old_name, "r"))) {
    mudlog( BRF, LVL_IMPL, TRUE, "SYSERR: DG_OLC: Failed to open %s", buf);
    return;
  } else if (!(newfile = fopen(new_name, "w"))) {
    mudlog( BRF, LVL_IMPL, TRUE, "SYSERR: DG_OLC: Failed to open %s", buf);
    return;
  }

  /*
   * Index contents must be in order: search through the old file for the
   * right place, insert the new file, then copy the rest over.
   */
  sprintf(buf1, "%d.%s", znum, type);
  while (get_line(oldfile, buf)) {
    if (*buf == '$') {
      fprintf(newfile, "%s\n$\n", (!found ? buf1 : ""));
      break;
    } else if (!found) {
      sscanf(buf, "%d", &num);
      if (num == znum)
        found = TRUE;
      else if (num > znum) {
        found = TRUE;
        fprintf(newfile, "%s\n", buf1);
      }
    }
    fprintf(newfile, "%s\n", buf);
  }

  fclose(newfile);
  fclose(oldfile);

  /*
   * Out with the old, in with the new.
   */
  remove(old_name);
  rename(new_name, old_name);
}

void dg_olc_script_free(struct descriptor_data *d)
{
  struct trig_proto_list *editscript, *prevscript;

  editscript = OLC_SCRIPT(d);
  while (editscript) {
    prevscript = editscript;
    editscript = editscript->next;
    free(prevscript);
  }
}


void dg_olc_script_copy(struct descriptor_data *d)
{
  struct trig_proto_list *origscript, *editscript;

  if (OLC_ITEM_TYPE(d)==MOB_TRIGGER)
    origscript = OLC_MOB(d)->proto_script;
  else if (OLC_ITEM_TYPE(d)==OBJ_TRIGGER)
    origscript = OLC_OBJ(d)->proto_script;
  else origscript = OLC_ROOM(d)->proto_script;

  if (origscript) {
    CREATE(editscript, struct trig_proto_list, 1);
    OLC_SCRIPT(d) = editscript;

    while (origscript) {
      editscript->vnum = origscript->vnum;
      origscript = origscript->next;
      if (origscript)
        CREATE(editscript->next, struct trig_proto_list, 1);
      editscript = editscript->next;
    }
  } else
      OLC_SCRIPT(d) = NULL;
}

void dg_script_menu(struct descriptor_data *d)
{
  struct trig_proto_list *editscript;
  int i = 0;

  /* make sure our input parser gets used */
  OLC_MODE(d) = OLC_SCRIPT_EDIT;
  OLC_SCRIPT_EDIT_MODE(d) = SCRIPT_MAIN_MENU;


#if defined(CLEAR_SCREEN) /* done wierd to compile with the vararg send() */
#define FMT    "[H[J     Script Editor\r\n\r\n     Trigger List:\r\n"
#else
#define FMT    "     Script Editor\r\n\r\n     Trigger List:\r\n"
#endif
  send_to_char(FMT,d->character);
#undef FMT

  editscript = OLC_SCRIPT(d);
  while (editscript) {
    sprintf(buf,"     %2d) [&06%d&00] &06%s&00", ++i,
      editscript->vnum,
      trig_index[real_trigger(editscript->vnum)]->proto->name);
    send_to_char(buf, d->character);
    if (trig_index[real_trigger(editscript->vnum)]->proto->attach_type !=
        OLC_ITEM_TYPE(d))
      sprintf(buf,"   &02** Mis-matched Trigger Type **&00\r\n");
    else
      sprintf(buf,"\r\n");
    send_to_char(buf, d->character);

    editscript = editscript->next;
  }
  if (i==0) send_to_char("     <none>\r\n", d->character);

  sprintf(buf, "\r\n"
    " &02N)&06 New trigger for this script\r\n"
    " &02D)&06 Delete a trigger in this script\r\n"
    " &02X)&06 Exit Script Editor\r\n\r\n"
    "     &00Enter choice :");
  send_to_char(buf,d->character);
}

int dg_script_edit_parse(struct descriptor_data *d, char *arg)
{
  struct trig_proto_list *trig, *currtrig;
  int count, pos, vnum;

  switch(OLC_SCRIPT_EDIT_MODE(d)) {
    case SCRIPT_MAIN_MENU:
      switch(tolower(*arg)) {
        case 'x':
          if (OLC_ITEM_TYPE(d)==MOB_TRIGGER) {
            trig = OLC_MOB(d)->proto_script;
            OLC_MOB(d)->proto_script = OLC_SCRIPT(d);
          } else if (OLC_ITEM_TYPE(d)==OBJ_TRIGGER) {
            trig = OLC_OBJ(d)->proto_script;
            OLC_OBJ(d)->proto_script = OLC_SCRIPT(d);
          } else {
            trig = OLC_ROOM(d)->proto_script;
            OLC_ROOM(d)->proto_script = OLC_SCRIPT(d);
          }

          while (trig) {
            currtrig = trig->next;
            free(trig);
            trig = currtrig;
          }
          return 0;
        case 'n':
          send_to_char("\r\nPlease enter position, vnum   (ex: 1, 200):",
            d->character);
          OLC_SCRIPT_EDIT_MODE(d) = SCRIPT_NEW_TRIGGER;
          break;
        case 'd':
          send_to_char("     Which entry should be deleted?  0 to abort :",
            d->character);
          OLC_SCRIPT_EDIT_MODE(d) = SCRIPT_DEL_TRIGGER;
          break;
        default:
          dg_script_menu(d);
          break;
      }
      return 1;

    case SCRIPT_NEW_TRIGGER:
      vnum = -1;
      count = sscanf(arg,"%d, %d",&pos,&vnum);
      if (count==1) {
        vnum = pos;
        pos = 999;
      }

      if (pos<=0) break; /* this aborts a new trigger entry */

      if (vnum==0) break; /* this aborts a new trigger entry */

      if (real_trigger(vnum)<0) {
        send_to_char("Invalid Trigger VNUM!\r\n"
            "Please enter position, vnum   (ex: 1, 200):",
            d->character);
        return 1;
      }

      /* add the new info in position */
      currtrig = OLC_SCRIPT(d);
      CREATE(trig, struct trig_proto_list, 1);
      trig->vnum = vnum;

      if (pos==1 || !currtrig) {
        trig->next = OLC_SCRIPT(d);
        OLC_SCRIPT(d) = trig;
      } else {
        while (currtrig->next && --pos) {
          currtrig = currtrig->next;
        }
        trig->next = currtrig->next;
        currtrig->next = trig;
      }
      OLC_VAL(d)++;
      break;

    case SCRIPT_DEL_TRIGGER:
      pos = atoi(arg);
      if (pos<=0) break;

      if (pos==1 && OLC_SCRIPT(d)) {
        OLC_VAL(d)++;
        currtrig = OLC_SCRIPT(d);
        OLC_SCRIPT(d) = currtrig->next;
        free(currtrig);
        break;
      }

      pos--;
      currtrig = OLC_SCRIPT(d);
      while (--pos && currtrig) currtrig = currtrig->next;
      /* now curtrig points one before the target */
      if (currtrig && currtrig->next) {
        OLC_VAL(d)++;
        trig = currtrig->next;
        currtrig->next = trig->next;
        free(trig);
      }
      break;
  }

  dg_script_menu(d);
  return 1;      
}

void trigedit_string_cleanup(struct descriptor_data *d, int terminator)
{
  switch (OLC_MODE(d)) {
    case TRIGEDIT_COMMANDS:
      trigedit_disp_menu(d);
      break;
  }
}

int format_script(struct descriptor_data *d)
{
  char nsc[MAX_CMD_LENGTH], *t, line[READ_SIZE];
  char *sc;
  size_t len = 0, nlen = 0, llen = 0;
  int indent = 0, indent_next = FALSE, found_case = FALSE, i, line_num = 0;

  if (!d->str || !*d->str)
    return FALSE;

  sc = strdup(*d->str); /* we work on a copy, because of strtok() */
  t = strtok(sc, "\n\r");
  *nsc = '\0';

  while (t) {
    line_num++;
    skip_spaces(&t);
    if (!strn_cmp(t, "if ", 3) ||
        !strn_cmp(t, "switch ", 7)) {
      indent_next = TRUE;
    } else if (!strn_cmp(t, "while ", 6)) {
      found_case = TRUE;  /* so you can 'break' a loop without complains */
      indent_next = TRUE;
    } else if (!strn_cmp(t, "end", 3) ||
               !strn_cmp(t, "done", 4)) {
      if (!indent) {
        write_to_output(d, "Unmatched 'end' or 'done' (line %d)!\r\n", line_num);
        free(sc);
        return FALSE;
      }
      indent--;
      indent_next = FALSE;
    } else if (!strn_cmp(t, "else", 4)) {
      if (!indent) {
        write_to_output(d, "Unmatched 'else' (line %d)!\r\n", line_num);
        free(sc);
        return FALSE;
      }
      indent--;
      indent_next = TRUE;
    } else if (!strn_cmp(t, "case", 4) ||
               !strn_cmp(t, "default", 7)) {
      if (!indent) {
        write_to_output(d, "Case/default outside switch (line %d)!\r\n", line_num);
        free(sc);
        return FALSE;
      }
      if (!found_case) /* so we don't indent multiple case statements without a break */
        indent_next = TRUE;
      found_case = TRUE;
    } else if (!strn_cmp(t, "break", 5)) {
      if (!found_case || !indent ) {
        write_to_output(d, "Break not in case (line %d)!\r\n", line_num);
        free(sc);
        return FALSE;
      }
      found_case = FALSE;
      indent--;
    }

    *line = '\0';
    for (nlen = 0, i = 0;i<indent;i++) {
      strncat(line, "  ", sizeof(line)-1);
      nlen += 2;
    }
    llen = snprintf(line + nlen, sizeof(line) - nlen, "%s\r\n", t);
    if (llen < 0 || llen + nlen + len > d->max_str - 1 ) {
      write_to_output(d, "String too long, formatting aborted\r\n");
      free(sc);
      return FALSE;
    }
    len = len + nlen + llen;
    strcat(nsc, line);  /* strcat OK, size checked above */

    if (indent_next) {
      indent++;
      indent_next = FALSE;
    }
    t = strtok(NULL, "\n\r");
  }

  if (indent)
    write_to_output(d, "Unmatched if, while or switch ignored.\r\n");

  free(*d->str);
  *d->str = strdup(nsc);
  free(sc);

  return TRUE;
}
