
/* ============================================================================ 
*  _TwyliteMud_ by Rv.                          Based on CircleMud3.0bpl9 *
*    				                                          *
*  OasisOLC - zedit.c 		                                          *
*    				                                          *
*  Copyright 1996 Harvey Gilpin.                                          *
============================================================================ */

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/comm.h"
#include "util/utils.h"
#include "olc/olc.h"
#include "general/class.h"
#include "olc/zedit.h"

/*
** As much as I dislike macros there are times when they make sense.
*/
#define ZPTR zone_table[d->olc->zone_num]
#define ZSUB d->olc->zone->cmd[subcmd]
#define DCHR d->character


//-------------------------------------------------------------------------
//
void addCmdToList( ResetCom **list, ResetCom *newcmd, int pos);
void removeCmdFromList(ResetCom **list, int pos);
void deleteCommand(DescriptorData *d, int pos);
int  newCommand(DescriptorData *d, int pos);
int  startChangeCommand(DescriptorData *d, int pos);
void zDispComtype(DescriptorData *d);
void zDispArg1(DescriptorData *d);
void zDispArg2(DescriptorData *d);
void zDispArg3(DescriptorData *d);
void zSaveInternally(DescriptorData *d);
void zCreateIndex(int znum, char *type);
void zMenu(DescriptorData *d);
void zedit_disp_flag_menu(struct descriptor_data *d);

//-------------------------------------------------------------------------
//
// Utility Functions 
//
void
zSetup( DescriptorData* d, int room_num )
{
  int subcmd  = 0;
  int count   = 0;
  int cmdRoom = -1;

  ZoneData* zone = NULL;

  // Alloc some zone shaped space
  //
  CREATE( zone, ZoneData, 1 );

  // Copy in zone header info
  //
  zone->name       = str_dup(ZPTR.name);
  zone->lifespan   = ZPTR.lifespan;
  zone->top        = ZPTR.top;
  zone->reset_mode = ZPTR.reset_mode;
  zone->zone_flags = ZPTR.zone_flags;

  // The remaining fields are used as a 'has been modified' flag
  //
  zone->number     = 0;	/*. Header info has changed .*/
  zone->age        = 0;	/*. Commands have changed   .*/

  // Start the reset command list with a terminator
  //
  CREATE(zone->cmd, ResetCom, 1);
  zone->cmd[0].command = 'S';

  // Add all entries in zone_table that relate to this room
  //
  while( ZPTR.cmd[subcmd].command != 'S' )
  {

    switch( ZPTR.cmd[subcmd].command )
    {
      case 'M':
      case 'O':
        cmdRoom = ZPTR.cmd[subcmd].arg3;
        break;
      case 'D':
      case 'R':
      case 'A':
        cmdRoom = ZPTR.cmd[subcmd].arg1;
        break;
      case 'G':
      case 'E':
      case 'P':
      case 'T':
      case 'Q':
      case 'N':
	break;
      default:
	cmdRoom = -1;
        break;
    }

    if( cmdRoom == room_num)
    {
      addCmdToList( &(zone->cmd), &ZPTR.cmd[subcmd], count );
      count++;
    }
    subcmd++;
  }

  d->olc->zone = zone;

  // Display main menu 
  //
  zMenu(d);
}


// Create a new zone
//
void
zNewZone( CharData* ch, int vzone_num )
{
  ZoneData* new_table;
  FILE* fp;

  int found = 0;
  int i     = 0;
  int room  = 0;

  if( vzone_num > 654 )
  {
    sendChar( ch, "654 is the highest zone allowed.\r\n" );
    return;
  }

  sprintf( buf, "%s/%i.zon", ZON_PREFIX, vzone_num );

  // Check zone does not exist
  //
  room = vzone_num * 100;

  for( i = 0; i <= top_of_zone_table; i++ )
  {
    if(( zone_table[i].number * 100 <= room) && (zone_table[i].top >= room))
    {
      sendChar( ch, "A zone already covers that area.\r\n" );
      return;
    }
  }

  // Create Zone file
  //
  if( !(fp = fopen(buf, "w")))
  {
    mudlog(BRF, LVL_IMPL, TRUE, "OLCERR: OLC: Can't write new zone file");
    return;
  }

  fprintf( fp, 
	  "#%d\n"
	  "New Zone~\n"
	  "%d 30 2 0\n"
	  "S\n"
	  "$\n", 
	  vzone_num,
	 (vzone_num * 100) + 99
  );

  fclose(fp);

  // Create Rooms file
  //
  sprintf( buf, "%s/%d.wld", WLD_PREFIX, vzone_num );

  if( !(fp = fopen(buf, "w"))) 
  {
    mudlog(BRF, LVL_IMPL, TRUE, "OLCERR: OLC: Can't write new world file");
    return;
  }

  fprintf( fp, 
	  "#%d\n"
    	  "The Begining~\n"
	  "Not much here.\n"
	  "~\n"
	  "%d 0 0 0 0 0\n"
	  "S\n"
	  "$\n",
	  vzone_num * 100,
 	  vzone_num
  );

  fclose(fp);

  // Create Mobiles file
  //
  sprintf( buf, "%s/%i.mob", MOB_PREFIX, vzone_num );
  if( !(fp = fopen(buf, "w")))
  {
    mudlog(BRF, LVL_IMPL, TRUE, "OLCERR: OLC: Can't write new mob file");
    return;
  }

  fprintf(fp, "$\n");
  fclose(fp);

  // Create Objects file
  //
  sprintf( buf, "%s/%i.obj", OBJ_PREFIX, vzone_num );

  if( !(fp = fopen(buf, "w"))) 
  {
    mudlog(BRF, LVL_IMPL, TRUE, "OLCERR: OLC: Can't write new obj file");
    return;
  }

  fprintf(fp, "$\n");
  fclose(fp);

  // Create Shops file
  //
  sprintf( buf, "%s/%i.shp", SHP_PREFIX, vzone_num );

  if( !(fp = fopen(buf, "w")))
  {
    mudlog(BRF, LVL_IMPL, TRUE, "OLCERR: OLC: Can't write new shop file");
    return;
  }

  fprintf(fp, "$~\n");
  fclose(fp);

  // Create Quests file
  //
  sprintf( buf, "%s/%i.qst", QST_PREFIX, vzone_num );

  if( !(fp = fopen(buf, "w")))
  {
    mudlog(BRF, LVL_IMPL, TRUE, "OLCERR: OLC: Can't write new quest file");
    return;
  }

  fprintf(fp, "$~\n");
  fclose(fp);

  // Update index files
  //
  zCreateIndex(vzone_num, "zon");
  zCreateIndex(vzone_num, "wld");
  zCreateIndex(vzone_num, "mob");
  zCreateIndex(vzone_num, "obj");
  zCreateIndex(vzone_num, "shp");
  zCreateIndex(vzone_num, "qst");

  // Make a new zone in memory
  //
  CREATE( new_table, struct zone_data, top_of_zone_table );

  new_table[top_of_zone_table + 1].number = 32000;

  for( i = 0; i <= top_of_zone_table + 1; i++ )
  {
    if(!found)
    {
      if( i > top_of_zone_table || zone_table[i].number > vzone_num )
      {
        found = 1;
        new_table[i].name            = str_dup("New Zone");
        new_table[i].number          = vzone_num;
        new_table[i].top             = (vzone_num * 100) + 99;
        new_table[i].lifespan        = 30;
        new_table[i].age             = 0;
        new_table[i].reset_mode      = 2;
	new_table[i].tournament_room = 0;
	new_table[i].kill_count      = 0;
	new_table[i].pkill_room      = 0;
        new_table[i].zone_flags      = 0;

        CREATE(new_table[i].cmd, ResetCom, 1);
        new_table[i].cmd[0].command = 'S';

        if( i <= top_of_zone_table )
        {
          new_table[i+1] = zone_table[i];
        }
      }
      else 
      {
        new_table[i] = zone_table[i];
      }
    }
    else
    {
      new_table[i+1] = zone_table[i];
    }
  }

  free(zone_table); // Digger - crashing here on second call.

  zone_table = new_table;
  top_of_zone_table++;

  mudlog( BRF, LVL_BUILDER, TRUE, "OLC: %s creates new zone #%d", GET_NAME(ch), vzone_num );
  sendChar( ch, "Zone created.\r\n" );
  return;
}


/*
**
*/
void
zCreateIndex( int   znum,
              char *type )
{
  FILE *newfile, *oldfile;
  char new_name[32], old_name[32], *prefix;
  int num, found = FALSE;

  switch(*type)
  {
    case 'z': prefix = ZON_PREFIX; break;
    case 'w': prefix = WLD_PREFIX; break;
    case 'o': prefix = OBJ_PREFIX; break;
    case 'm': prefix = MOB_PREFIX; break;
    case 's': prefix = SHP_PREFIX; break;
    case 'q': prefix = QST_PREFIX; break;
    default: return; /*. Caller messed up .*/
  }
  sprintf(old_name, "%s/index", prefix);
  sprintf(new_name, "%s/newindex", prefix);

  if(!(oldfile = fopen(old_name, "r")))
  {
    mudlog( BRF, LVL_IMPL, TRUE, "OLCERR: OLC: Failed to open %s", buf);
    return;
  }

  if(!(newfile = fopen(new_name, "w")))
  {
    mudlog( BRF, LVL_IMPL, TRUE, "OLCERR: OLC: Failed to open %s", buf);
    return;
  }
  /*
  ** Index contents must be in order: search through the old file for
  ** the right place, insert the new file, then copy the rest over.
  */
  sprintf(buf1, "%d.%s", znum, type);
  while(get_line(oldfile, buf))
  {
    if (*buf == '$')
    {
      if (!found)
        fprintf(newfile, "%s\n", buf1);
      fprintf(newfile, "$\n");
      break;
    }
    if (!found)
    {
      sscanf(buf, "%d", &num);
      if (num > znum)
      {
        found = TRUE;
        fprintf(newfile, "%s\n", buf1);
      }
    }
    fprintf(newfile, "%s\n", buf);
  }
    
  fclose(newfile);
  fclose(oldfile);
  /*
  ** Out with the old, in with the new
  */
  remove(old_name);
  rename(new_name, old_name);
}


/*
** Save all the information on the players descriptor back into
** the zone table
*/
void
zSaveInternally( DescriptorData *d )
{
  int subcmd = 0, cmd_room = -2, room_num;

  room_num = real_room(OLC_NUM(d));
  /*
  ** Zap all entries in zone_table that relate to this room
  */
  while( ZPTR.cmd[subcmd].command != 'S' )
  {
    switch(ZPTR.cmd[subcmd].command)
    {
      case 'M':
      case 'O':
        cmd_room = ZPTR.cmd[subcmd].arg3;
        break;
      case 'D':
      case 'R':
      case 'A':
        cmd_room = ZPTR.cmd[subcmd].arg1;
        break;
      default:
        break;
    }
    if(cmd_room == room_num)
      removeCmdFromList(&(ZPTR.cmd), subcmd);
    else
      subcmd++;
  }
  /*
  ** Now add all the entries in the players descriptor list
  */
  subcmd = 0;
  while(ZSUB.command != 'S')
  {
    addCmdToList( &(ZPTR.cmd),
                  &ZSUB, subcmd );
    subcmd++;
  }
  /*
  ** Finally, if zone headers have been changed, copy over
  */
  if (d->olc->zone->number)
  {
    free(ZPTR.name);
    ZPTR.name       = str_dup(d->olc->zone->name);
    ZPTR.top        = d->olc->zone->top;
    ZPTR.reset_mode = d->olc->zone->reset_mode;
    ZPTR.lifespan   = d->olc->zone->lifespan;
    ZPTR.zone_flags = d->olc->zone->zone_flags;
  }
  olc_add_to_save_list(ZPTR.number, OLC_SAVE_ZONE);
}


// Save all the zone_table for this zone to disk.  Yes, this could
// automatically comment what it saves out, but why bother when you
// have an OLC as cool as this ? :>
//
void
zSaveToDisk( DescriptorData* d )
{
  char  fname[64] = "";
  FILE* zfile     = NULL;
  int   subcmd    = 0;

  // Build a filename based on the zone number, and open it for writing.
  // This should rename the original rather than overwrite it.
  //
  sprintf( fname, "%s/%i.zon", ZON_PREFIX, ZPTR.number);

  if( !(zfile = fopen(fname, "w")) )
  {
    mudlog( BRF, LVL_BUILDER, TRUE,
            "OLCERR: OLC: zSaveToDisk:  Can't write zone %d.", ZPTR.number);
    return;
  }

  // Print zone header to file
  //
  sprintf( buf, 
	  "#%d\n"
  	  "%s~\n"
  	  "%d %d %d %d\n",
	  ZPTR.number,
	  ZPTR.name ? ZPTR.name : "undefined",
 	  ZPTR.top,
    	  ZPTR.lifespan, 
          ZPTR.reset_mode,
          ZPTR.zone_flags
  );
  fprintf(zfile, buf);

  // Loop through each command and store it to the file.
  //
  for( subcmd = 0; ZPTR.cmd[subcmd].command != 'S'; subcmd++)
  {
# ifdef MAKE_COMMENTS
    char comment[80] = "";
# endif
    int  arg1        = -1;
    int  arg2        = -1;
    int  arg3        = -1;

    switch (ZPTR.cmd[subcmd].command)
    {
      case 'M':
        arg1 = mob_index[ZPTR.cmd[subcmd].arg1].virtual;
        arg2 = ZPTR.cmd[subcmd].arg2;
        arg3 = world[ZPTR.cmd[subcmd].arg3].number;
# ifdef MAKE_COMMENTS
        sprintf( comment, "* Mob : %s",
                 mob_proto[ZPTR.cmd[subcmd].arg1].player.short_descr );
# endif
        break;

      case 'O':
        arg1 = obj_index[ZPTR.cmd[subcmd].arg1].virtual;
        arg2 = ZPTR.cmd[subcmd].arg2;
        arg3 = world[ZPTR.cmd[subcmd].arg3].number;
        break;

      case 'G':
        arg1 = obj_index[ZPTR.cmd[subcmd].arg1].virtual;
        arg2 = ZPTR.cmd[subcmd].arg2;
        arg3 = -1;
# ifdef MAKE_COMMENTS
        sprintf( comment, "* Give: %s",
                 obj_proto[ZPTR.cmd[subcmd].arg1].short_description );
# endif
        break;

      case 'E':
        arg1 = obj_index[ZPTR.cmd[subcmd].arg1].virtual;
        arg2 = ZPTR.cmd[subcmd].arg2;
        arg3 = ZPTR.cmd[subcmd].arg3;
# ifdef MAKE_COMMENTS
        sprintf( comment, "* Eqp : %-50s", 
                 obj_proto[ZPTR.cmd[subcmd].arg1].short_description );
# endif
        break;

      case 'P':
        arg1 = obj_index[ZPTR.cmd[subcmd].arg1].virtual;
        arg2 = ZPTR.cmd[subcmd].arg2;
        arg3 = obj_index[ZPTR.cmd[subcmd].arg3].virtual;
# ifdef MAKE_COMMENTS
        sprintf( comment, "* Put : %s",
                 obj_proto[ZPTR.cmd[subcmd].arg1].short_description );
# endif
        break;

      case 'D':
        arg1 = world[ZPTR.cmd[subcmd].arg1].number;
        arg2 = ZPTR.cmd[subcmd].arg2;
        arg3 = ZPTR.cmd[subcmd].arg3;
        break;

      case 'A':
        arg1 = world[ZPTR.cmd[subcmd].arg1].number;
        arg2 = ZPTR.cmd[subcmd].arg2;
        arg3 = ZPTR.cmd[subcmd].arg3;
        break;

      case 'R':
        arg1 = world[ZPTR.cmd[subcmd].arg1].number;
        arg2 = obj_index[ZPTR.cmd[subcmd].arg2].virtual;
        arg3 = -1;
        break;

      // Invalid commands are replaced with '*' - Ignore them
      //
      case '*':
        continue;

      default:
        mudlog( BRF, LVL_BUILDER, TRUE,
                "OLCERR: zedit save %d unknown cmd '%c' - NOT saving",
                 ZPTR.number,
                 ZPTR.cmd[subcmd].command );
        continue;
    }

    sprintf( buf, "%c %d %d %d %d",
                  ZPTR.cmd[subcmd].command,
                  ZPTR.cmd[subcmd].if_flag,
                  arg1, arg2, arg3);

# ifdef MAKE_COMMENTS
    fprintf( zfile, "%-20s %s\n", buf, comment );
# else
    fprintf( zfile, "%-20s\n", buf );
# endif
  }

  fprintf(zfile, "S\n$\n");
  fclose(zfile);
  olc_remove_from_save_list(ZPTR.number, OLC_SAVE_ZONE);
}


// Adds a new reset command into a list.  Takes a pointer to the list
// so that it may play with the memory locations
//
void
addCmdToList( ResetCom** list,
              ResetCom*  newcmd,
              int        pos )
{
// PROBLEM CHILD???? save 350 barfs in the for. Digger
  int count = 0;
  int i     = 0;
  int l     = 0;

  ResetCom *newlist;

  // Count number of commands (not including terminator)
  //
  while( (*list)[count].command != 'S' ) count++;
   
  CREATE( newlist, ResetCom, count + 2 );

  // Tight loop to copy old list and insert new command
  //
  for( i = 0; i <= count; i++ )
  {
    if( i == pos )
    {
      newlist[i] = *newcmd; 
    }
    else
    {
      newlist[i] = (*list)[l++];
    }
  }

  // Add terminator then insert new list
  //
  newlist[count+1].command = 'S';
  free(*list);
  *list = newlist;
}


/*
** Remove a reset command from a list.  Takes a pointer to the list
** so that it may play with the memory locations
*/
void
removeCmdFromList( ResetCom **list,
                   int pos )
{
  int count = 0, i, l;
  ResetCom *newlist;
  /*
  **Count number of commands (not including terminator)
  */
  while((*list)[count].command != 'S') count++;

  CREATE(newlist, ResetCom, count);

  /*
  ** Tight loop to copy old list and skip unwanted command
  */
  l = 0;
  for(i=0;i<count;i++)
    if(i==pos)
      continue;
    else
      newlist[l++] = (*list)[i];

  /*
  ** Add terminator then insert new list
  */
  newlist[count-1].command = 'S';
  free(*list);
  *list = newlist;
}


/*
** Error check user input and then add new (blank) command
*/
int
newCommand( DescriptorData *d,
            int pos )
{
  int subcmd = 0;
  ResetCom *new_com;
  /*
  ** Error check to ensure users hasn't given too large an index
  */
  while(ZSUB.command != 'S')
     subcmd++;
 
  if ((pos > subcmd) || (pos < 0)) return 0;
  /*
  ** Ok, let's add a new (blank) command
  */
  CREATE(new_com, ResetCom, 1);
  new_com->command = 'N'; 
  addCmdToList(&d->olc->zone->cmd, new_com, pos);
  return 1;
}


/*
** Error check user input and then remove command
*/
void
deleteCommand( DescriptorData *d,
               int pos )
{
  int subcmd = 0;
  /*
  ** Error check to ensure users hasn't given too large an index
  */
  while(ZSUB.command != 'S')
    subcmd++;
  
  if ((pos >= subcmd) || (pos < 0)) return;
  /*
  ** Ok, let's zap it
  */
  removeCmdFromList(&d->olc->zone->cmd, pos);
}

void zedit_disp_flag_menu(struct descriptor_data *d)
{
  int counter, columns = 0;

  get_char_colors(d->character);
#if defined(CLEAR_SCREEN)
sprintf(buf, "%c[H%c[J", 27, 27);
send_to_char(buf, d->character);
#endif
  for (counter = 0; counter < NUM_ZONE_FLAGS; counter++) {
    sprintf(buf, "%s%2d%s) %-20.20s %s", grn, counter + 1, nrm,
               zone_bits[counter], !(++columns % 2) ? "\r\n" : "");
    send_to_char(buf, d->character);
  }
  sprintbit(OLC_ZONE(d)->zone_flags, zone_bits, buf1);
  sprintf(buf, "\r\nZone flags: %s%s%s\r\n"
         "Enter zone flags, 0 to quit : ", cyn, buf1, nrm);
  send_to_char(buf, d->character);
  OLC_MODE(d) = ZEDIT_ZONE_FLAGS;
}


/*
** Error check user input and then setup change
*/
int
startChangeCommand( DescriptorData *d, 
                    int pos )
{
  int subcmd = 0;
  /*
  ** Error check to ensure users hasn't given too large an index
  */
  while(ZSUB.command != 'S')
    subcmd++;
  
  if ((pos >= subcmd) || (pos < 0)) return 0;
  /*
  ** Ok, let's get editing
  */
  d->olc->value = pos;
  return 1;
}

/**************************************************************************
 Menu functions 
 **************************************************************************/

/* Vex - functions to print out object and mob lists for one zone. */

/* ============================================================================ 
Find the real number of the first obj defined in znum. Returns -1 if there
are not any, -2 if the zone doesn't exist.
Parameters: znum -> zones vnum
============================================================================ */
int
zFirstObj( int znum )
{
    int ztop, i, firstObj = -1, rzone;

    rzone = real_zone(znum);
    if (rzone < 0 )
	return -2; /* invalid zone */

    ztop = zone_table[rzone].top;

    for (i = (znum * 100); i < ztop; i++)
	if ( real_object(i) >= 0 ) {
	    firstObj = real_object(i);
	    break; /* we found the first obj. */
	}

    return firstObj;
}

/* ============================================================================ 
Find the real number of the first mob defined in znum. Returns -1 if there
are not any, -2 if the zone doesn't exist.
Parameters: znum -> zones vnum
============================================================================ */
int
zFirstMob( int znum )
{
    int ztop, i, firstMob = -1, rzone;

    rzone = real_zone(znum);
    if (rzone < 0 )
	return -2; /* invalid zone */

    ztop = zone_table[rzone].top;

    for (i = (znum * 100); i < ztop ; i++)
	if ( real_mobile(i) >= 0 ) {
	    firstMob = real_mobile(i);
	    break; /* we found the first mob. */
	}

    return firstMob;
}

/* ============================================================================ 
Find the real number of the last obj defined in znum. Returns -1 if there
are no objs defined, -2 if the zone doesn't exist.
============================================================================ */
int
zLastObj( int znum )
{
    int ztop, firstObj, lastObj;

    firstObj = zFirstObj(znum);
    if (firstObj < 0)
	return firstObj; /* same errors as zFirstObj */

    /* zone exists and there is at least one obj in it. */
    ztop = zone_table[real_zone(znum)].top;
    lastObj = firstObj;
    /* step through until next obj is outside this zone. */
    while ( lastObj < top_of_objt && obj_index[lastObj + 1].virtual < ztop )
	lastObj++;

    return lastObj;
}

/* ============================================================================ 
Find the real number of the last mob defined in znum. Returns -1 if there
are no mobs defined, -2 if the zone doesn't exist.
============================================================================ */
int
zLastMob( int znum )
{
    int ztop, firstMob, lastMob;

    firstMob = zFirstMob(znum);
    if (firstMob < 0)
	return firstMob; /* same errors as zFirstMob */

    /* zone exists and there is at least one mob in it. */
    ztop = zone_table[real_zone(znum)].top;
    lastMob = firstMob;
    /* step through until next mob is outside this zone. */
    while ( lastMob < top_of_mobt && mob_index[lastMob + 1].virtual < ztop )
	lastMob++;

    return lastMob;
}

/* ============================================================================  
Print all the mobs in the mob_table from fmob to lmob. fmob and lmob are the
real numbers of the mobs that are to be printed.
============================================================================  */
void
zPrintMobList( DescriptorData *d,
               int fmob,
               int lmob )
{
    int i, cnt;
    char buf[MAX_STRING_LENGTH * 8];

    sprintf(buf, "\r\n"); /* Required to ensure buf in right state. */
    sprintf(buf, "[H[J");
    if (fmob > 0 && lmob >= fmob) {
	for (i = fmob, cnt = 0; i <= lmob; i++) {
	    cnt++;
            sprintf(((buf) + strlen(buf)), "[&06%5d&00] &06%26s&00%s", 
	            mob_index[i].virtual, mob_proto[i].player.short_descr, (cnt % 2 ? "  &08|&00  " : "\r\n"));
	}
	sprintf(((buf) + strlen(buf)), "\r\n");
    } /* if */
    /* Have to make prompt here or it will be screwed once we go over one page. */
    /* since this is a global buffer, could be possible to get around... dubious practice thou. */
    sprintf(((buf) + strlen(buf)), "Enter mob vnum: ");
    page_string(d, buf, 1);
    return;
}

/* ============================================================================  
Print all the objs in the obj_table from fobj to lobj. fobj and lobj are the
real numbers of the objs that are to be printed.
============================================================================  */
void zPrintObjList(DescriptorData *d, int fobj, int lobj)
{
    int i, cnt;
    char buf[MAX_STRING_LENGTH * 8];

    sprintf(buf, "\r\n"); /* Required to ensure buf in right state. */
    sprintf(buf, "[H[J");
    if (fobj > 0 && lobj >= fobj) {
	for (i = fobj, cnt = 0; i <= lobj; i++) {
	    cnt++;
            sprintf(((buf) + strlen(buf)), "[&06%5d&00] &06%26s&00%s", 
	            obj_index[i].virtual, obj_proto[i].short_description, (cnt % 2 ? "  &08|&00  " : "\r\n"));
	}
	sprintf(((buf) + strlen(buf)), "\r\n");
    } /* if */
    /* Have to make prompt here or it will be screwed once we go over one page. */
    /* since this is a global buffer, could be possible to get around... dubious practice thou. */
    sprintf(((buf) + strlen(buf)), "Enter object vnum: ");
    page_string(d, buf, 1);
    return;
}

/* Digger MARK */

/*
** the main menu
*/
void
zMenu( DescriptorData * d )
{
  static char *zResetMode[] = { "Never", "When Playerless", "Normal", "Normal - God only Zone"};
  int subcmd = 0, room, counter = 0;

  sprintbit((long)OLC_ZONE(d)->zone_flags, zone_bits, buf1);
  room = real_room(OLC_NUM(d));
  /*
  ** Menu header
  */
  sprintf(buf, "[H[J"
    "&02*) &06Zone Id     :&03 %-5d &02Z) &06Zone Name   :&03 %s\r\n"
    "&02*) &06Room Number :&03 %-5d &02L) &06RePop(mins) :&03 %-5d\r\n"
    "&02T) &06Top of Zone :&03 %-5d &02R) &06Reset Mode  :&03 %s\r\n"
    "&02F) &06Zone flags  :&03 %-5s\r\n"
    "&02--------------------------------------------------------------------\r\n"
    "&02*) &06Cmd List    :&02\r\n",

    ZPTR.number,
    d->olc->zone->name ? d->olc->zone->name : "<NONE!>",
    OLC_NUM(d),
    d->olc->zone->lifespan,
    d->olc->zone->top,
    zResetMode[d->olc->zone->reset_mode],
    buf1
  );
  /*
  ** Print the commands for this room into display buffer
  */
  while(ZSUB.command != 'S')
  {
    /*
    ** Translate what the command means
    */
    switch(ZSUB.command)
    {
      case'M':
        sprintf(buf2, "%s&06Load &03%-15s &06[&03%5d&06], Max :&03 %d",
          ZSUB.if_flag ? "&03+" : " ",
          mob_proto[ZSUB.arg1].player.short_descr,
          mob_index[ZSUB.arg1].virtual,
          ZSUB.arg2
        );
        break;
      case'G':
        sprintf(buf2, "%s&06Give it &03%-15s &06[&03%5d&06], Max :&03 %d",
          ZSUB.if_flag ? "&03+" : " ",
          obj_proto[ZSUB.arg1].short_description,
          obj_index[ZSUB.arg1].virtual,
          ZSUB.arg2
        );
        break;
      case'O':
        sprintf(buf2, "%s&06Load &03%-15s &06[&03%5d&06], Max : &03%d",
          ZSUB.if_flag ? "&03+" : " ",
          obj_proto[ZSUB.arg1].short_description,
          obj_index[ZSUB.arg1].virtual,
          ZSUB.arg2
        );
        break;
      case'E':
        sprintf(buf2, "%s&06Equip with &03%-15s &06[&03%5d&06], &03%s&06, Max : &03%d",
          ZSUB.if_flag ? "&03+" : " ",
          obj_proto[ZSUB.arg1].short_description,
          obj_index[ZSUB.arg1].virtual,
          equipment_types[ZSUB.arg3],
          ZSUB.arg2
        );
        break;
      case'P':
        sprintf(buf2, "%s&06Put &03%-15s &06[&03%5d&06] in &03%s &06[&03%d&06], &06Max : &03%d",
          ZSUB.if_flag ? "&03+" : " ",
          obj_proto[ZSUB.arg1].short_description,
          obj_index[ZSUB.arg1].virtual,
          obj_proto[ZSUB.arg3].short_description,
          obj_index[ZSUB.arg3].virtual,
          ZSUB.arg2
        );
        break;
      /*
      ** Random Obj Work - Digger
      */
      case'R':
        sprintf(buf2, "%s&06Remove &03%-15s &06[&03%5d&06] from room",
          ZSUB.if_flag ? "&03+" : " ",
          obj_proto[ZSUB.arg2].short_description,
          obj_index[ZSUB.arg2].virtual
        );
        break;
      case'D':
      {
        sprintf(buf2, "%s&06Set door &03%-15s &06as &03%s",
          ZSUB.if_flag ? "&03+" : " ",
          dirs[ZSUB.arg2],
          ZSUB.arg3 ? ((ZSUB.arg3 == 1) ? "closed" : "locked") : "open"
        );
        break;
      }
      case 'A':
      {
        sprintf(buf2, "%s&06Trap exit &03%-15s &06at level &03%d",
            ZSUB.if_flag ? "&03+" : " ",
            dirs[ZSUB.arg2], ZSUB.arg3);
        break;
      }
      default:
        strcpy(buf2, "<Unknown Command>");
        break;
    }
    /*
    ** Build the display buffer for this command
    */
    sprintf(buf1, "&06%d - %s\r\n", counter++, buf2 );
    strcat(buf, buf1);
    subcmd++;
  }

  /*
  ** Finish off menu
  */
  sprintf(buf1,
    "&06%d&06 - <END OF LIST>&00\r\n"
    "&02--------------------------------------------------------------------\r\n"
    "&02N)&06ew  &02E)&06dit  &02D)&06elete  &02Q)&06uit\r\n"
    "&06Enter your choice : &00", counter
  );

  strcat(buf, buf1);
  sendChar( DCHR, buf );

  d->olc->mode = ZEDIT_MAIN_MENU;
}


/*-------------------------------------------------------------------*/
/*. Print the command type menu and setup response catch. */

void zDispComtype(DescriptorData *d)
{ 
  sprintf(buf, "[H[J"
    "&02M)&06 Load Mobile to room             &02O)&06 Load Object to room\r\n"
    "&02E)&06 Equip mobile with object        &02G)&06 Give an object to a mobile\r\n"
    "&02P)&06 Put object in another object    &02D)&06 Open/Close/Lock a Door\r\n"
    "&02R)&06 Remove an object from the room  &02A)&06 Place a trap on an exit\r\n"
    "&03What sort of command will this be? : "
  );
  sendChar( DCHR, buf );
  d->olc->mode=ZEDIT_COMMAND_TYPE;
}


/*-------------------------------------------------------------------*/
/*. Print the appropriate message for the command type for arg1 and set
    up the input catch clause .*/

void zDispArg1(DescriptorData *d)
{
  switch(d->olc->zone->cmd[d->olc->value].command)
  { case 'M':
      sendChar( DCHR, "Input mob's vnum (0 for list): " );
      d->olc->mode = ZEDIT_ARG1;
      break;
    case 'O':
    case 'E':
    case 'P':
    case 'G':
      sendChar( DCHR, "Input object vnum (0 for list): " );
      d->olc->mode = ZEDIT_ARG1;
      break;
    case 'D':
    case 'R':
    case 'A':
      /*. Arg1 for these is the room number, skip to arg2 .*/
      d->olc->zone->cmd[d->olc->value].arg1 = real_room(OLC_NUM(d));
      zDispArg2(d);
      break;
    default:
      /*. We should never get here .*/
      cleanup_olc(d, CLEANUP_ALL);
      mudlog(BRF, LVL_BUILDER, TRUE, "OLCERR: OLC: zDispArg1(): Help!");
      return;
  }
}

    

/*-------------------------------------------------------------------*/
/*. Print the appropriate message for the command type for arg2 and set
    up the input catch clause .*/

void zDispArg2(DescriptorData *d)
{ int i = 0;
  switch(d->olc->zone->cmd[d->olc->value].command)
  { case 'M':
    case 'O':
    case 'E':
    case 'P':
    case 'G':
      sendChar( DCHR, "Input the maximum number that can exist on the mud : " );
      break;
    case 'A':
      while(*dirs[i] != '\n')
      {
        sendChar( DCHR, "%d) Exit %s.\r\n", i, dirs[i] );
        i++;
      }
      sendChar( DCHR, "Enter exit number for trap : " );
      break;
    case 'D':
      while(*dirs[i] != '\n')
      {
#if 0
        sprintf(buf, "%d) Exit %s.\r\n", i, dirs[i]);
        sendChar( DCHR, buf );
#else
        sendChar( DCHR, "%d) Exit %s.\r\n", i, dirs[i] );
#endif
        i++;
      }
      sendChar( DCHR, "Enter exit number for door : " );
      break;
    case 'R':
      sendChar( DCHR, "Input object's vnum : " );
      break;
    default:
      /*. We should never get here .*/
      cleanup_olc(d, CLEANUP_ALL);
      mudlog(BRF, LVL_BUILDER, TRUE, "OLCERR: OLC: zDispArg2(): Help!");
      return;
  }
  d->olc->mode = ZEDIT_ARG2;
}


/*-------------------------------------------------------------------*/
/*. Print the appropriate message for the command type for arg3 and set
    up the input catch clause .*/

void zDispArg3(DescriptorData *d)
{ int i = 0;
  switch(d->olc->zone->cmd[d->olc->value].command)
  { 
    case 'E':
      /*
      ** Better menu for equipment slot is needed
      */
      while(*equipment_types[i] !=  '\n')
      {
        sprintf(buf, "%2d) %26.26s %2d) %26.26s\r\n", i, 
          equipment_types[i], i+1, (*equipment_types[i+1] != '\n') ?
          equipment_types[i+1] : "");
        sendChar( DCHR, buf );
        if( *equipment_types[i+1] != '\n' )
          i+=2;
        else
          break;
      }
      sendChar( DCHR, "Enter EQ Slot: " );
      break;
    case 'P':
      sendChar( DCHR, "Input the vnum of the container : " );
      break;
    case 'A':
      sendChar( DCHR, "Level of trap : " );
      break;
    case 'D':
      sendChar( DCHR, "0)  Door open\r\n"
                      "1)  Door closed\r\n"
                      "2)  Door locked\r\n" 
                      "Enter state of the door : " );
      break;
    case 'M':
    case 'O':
    case 'R':
    case 'G':
    default:
      /*. We should never get here .*/
      cleanup_olc(d, CLEANUP_ALL);
      mudlog(BRF, LVL_BUILDER, TRUE, "OLCERR: OLC: zDispArg3(): Help!");
      return;
  }
  d->olc->mode = ZEDIT_ARG3;
}

    
    
/**************************************************************************
  The GARGANTAUN event handler
 **************************************************************************/

void
zParse( DescriptorData *d, char *arg )
{
  int pos, i = 0, number;

  char chIn = toupper(*arg);
  switch( d->olc->mode )
  {
/*-------------------------------------------------------------------*/
  case ZEDIT_CONFIRM_SAVESTRING:
    switch( chIn )
    {
    case 'Y':
      /*
      ** Save the zone in memory
      */
      sendChar( DCHR, "Saving zone info in memory.\r\n" );
      zSaveInternally(d);
      mudlog( CMP, LVL_BUILDER, TRUE, "OLC: %s edits zone info for room %d", GET_NAME(DCHR), OLC_NUM(d));
      cleanup_olc(d, CLEANUP_ALL);
      break;
    case 'N':
      cleanup_olc(d, CLEANUP_ALL);
      break;
    default:
      sendChar( DCHR, "Invalid choice!\r\n" );
      sendChar( DCHR, "Do you wish to save the zone info? " );
      break;
    }
    break; /* end of ZEDIT_CONFIRM_SAVESTRING */

/*-------------------------------------------------------------------*/
  case ZEDIT_MAIN_MENU:
    switch( chIn )
    {
    case 'Q':
      if( d->olc->zone->age || d->olc->zone->number )
      {
        sendChar( DCHR, "Do you wish to save the changes to the zone info? (y/n) : " );
        d->olc->mode = ZEDIT_CONFIRM_SAVESTRING;
      }
      else
      {
        sendChar( DCHR, "No changes made.\r\n" );
        cleanup_olc(d, CLEANUP_ALL);
      }
      break;
    case 'N':
      /*. New entry .*/
      sendChar( DCHR, "What number in the list should the new command be? " );
      d->olc->mode = ZEDIT_NEW_ENTRY;
      break;
    case 'E':
      /*. Change an entry .*/
      sendChar( DCHR, "Which command do you wish to change? " );
      d->olc->mode = ZEDIT_CHANGE_ENTRY;
      break;
    case 'D':
      /*. Delete an entry .*/
      send_to_char("Which command do you wish to delete? : ", DCHR);
      d->olc->mode = ZEDIT_DELETE_ENTRY;
      break;
    case 'Z':
      /*. Edit zone name .*/
      send_to_char("Enter new zone name : ", DCHR);
      d->olc->mode = ZEDIT_ZONE_NAME;
      break;
    case 'T':
      /*. Edit zone top .*/
      if(GET_LEVEL(DCHR) < LVL_IMPL)
        zMenu(d);
      else
      { send_to_char("Enter new top of zone : ", DCHR);
        d->olc->mode = ZEDIT_ZONE_TOP;
      }
      break;
    case 'L':
      /*. Edit zone lifespan .*/
      send_to_char("Enter new zone lifespan : ", DCHR);
      d->olc->mode = ZEDIT_ZONE_LIFE;
      break;
    case 'R':
      /*. Edit zone reset mode .*/
      send_to_char("\r\n"
                   "0) Never reset\r\n"
                   "1) Reset only when no players in zone\r\n"
                   "2) Normal reset\r\n"
		   "3) Reset as 2, but only allow gods to enter.\r\n"
                   "Enter new zone reset type : ", DCHR);
      d->olc->mode = ZEDIT_ZONE_RESET;
      break;
    case 'F':
      zedit_disp_flag_menu(d);
      break;
    default:
      zMenu(d);
      break;
  }
  break; /*. End ZEDIT_MAIN_MENU .*/

/*-------------------------------------------------------------------*/
  case ZEDIT_NEW_ENTRY:
    /*. Get the line number and insert the new line .*/
    pos = atoi(arg);
    if (isdigit(*arg) && newCommand(d, pos))
    {  if (startChangeCommand(d, pos))
       { zDispComtype(d);
         d->olc->zone->age = 1;
       }
    } else
      zMenu(d);
    break;

/*-------------------------------------------------------------------*/
  case ZEDIT_DELETE_ENTRY:
    /*. Get the line number and delete the line .*/
    pos = atoi(arg);
    if(isdigit(*arg))
    { deleteCommand(d, pos);
      d->olc->zone->age = 1;
    }
    zMenu(d);
    break;

/*-------------------------------------------------------------------*/
  case ZEDIT_CHANGE_ENTRY:
    /*. Parse the input for which line to edit, and goto next quiz .*/
    pos = atoi(arg);
    if(isdigit(*arg) && startChangeCommand(d, pos))
    { zDispComtype(d);
      d->olc->zone->age = 1;
    } else
      zMenu(d);
    break;

/*-------------------------------------------------------------------*/
  case ZEDIT_COMMAND_TYPE:
    /*. Parse the input for which type of command this is, 
        and goto next quiz .*/
    d->olc->zone->cmd[d->olc->value].command = toupper(*arg);
    if (!d->olc->zone->cmd[d->olc->value].command || (strchr("MOPEDGRA", d->olc->zone->cmd[d->olc->value].command) == NULL))
    { send_to_char("Invalid choice, try again : ", DCHR);
    } else
    { if (d->olc->value)
      { /*. If there was a previous command .*/
        send_to_char("Is this command dependent on the success of the previous one? (y/n)\r\n", DCHR);
        d->olc->mode = ZEDIT_IF_FLAG;
      } else
      { /*. 'if-flag' not appropriate .*/
        d->olc->zone->cmd[d->olc->value].if_flag = 0;
        zDispArg1(d);
      }
    }
    break;

/*-------------------------------------------------------------------*/
  case ZEDIT_IF_FLAG:
    /*. Parse the input for the if flag, and goto next quiz .*/
    switch( chIn )
    {
      case 'Y':
        d->olc->zone->cmd[d->olc->value].if_flag = 1;
        break;
      case 'N':
        d->olc->zone->cmd[d->olc->value].if_flag = 0;
        break;
      default:
        send_to_char("Try again : ", DCHR);
        return;
    }
    zDispArg1(d);
    break;


/*-------------------------------------------------------------------*/
  case ZEDIT_ARG1:
    /*. Parse the input for arg1, and goto next quiz .*/
    if( !isdigit( chIn ))
    {
      sendChar( DCHR, "Must be a numeric value, try again : " );
      return;
    }
    switch( d->olc->zone->cmd[d->olc->value].command )
    {
    case 'M':
      pos = real_mobile(atoi(arg));
      if( pos > 0 )
      {
        d->olc->zone->cmd[d->olc->value].arg1 = pos;
        zDispArg2(d);
      }
      else
      {
        zPrintMobList(d, zFirstMob(ZPTR.number), zLastMob(ZPTR.number));
      }
      break;
    case 'O':
    case 'P':
    case 'E':
    case 'G':
      pos = real_object(atoi(arg));
      if( pos > 0 )
      {
        d->olc->zone->cmd[d->olc->value].arg1 = pos;
        zDispArg2(d);
      }
      else
        zPrintObjList(d, zFirstObj(ZPTR.number), zLastObj(ZPTR.number));
      break;
    case 'A':
    case 'D':
    case 'R':
    default:
      /*. We should never get here .*/
      cleanup_olc(d, CLEANUP_ALL);
      mudlog(BRF, LVL_BUILDER, TRUE, "OLCERR: OLC: zParse(): case ARG1: Ack!");
      break;
    }
    break;


/*-------------------------------------------------------------------*/
  case ZEDIT_ARG2:
    /*. Parse the input for arg2, and goto next quiz .*/
    if( !isdigit( chIn ))
    {
      sendChar( DCHR, "Must be a numeric value, try again : " );
      return;
    }
    switch(d->olc->zone->cmd[d->olc->value].command)
    {
    case 'M':
    case 'O':
      d->olc->zone->cmd[d->olc->value].arg2 = atoi(arg);
      d->olc->zone->cmd[d->olc->value].arg3 = real_room(OLC_NUM(d));
      zMenu(d);
      break;
    case 'G':
      d->olc->zone->cmd[d->olc->value].arg2 = atoi(arg);
      zMenu(d);
      break; 
    case 'P':
    case 'E':
      d->olc->zone->cmd[d->olc->value].arg2 = atoi(arg);
      zDispArg3(d);
      break;
    case 'A':
    case 'D':
      pos = atoi(arg);
      /*. Count dirs .*/
      while(*dirs[i] != '\n')
        i++;
      if ((pos < 0) || (pos > i))
        sendChar( DCHR, "Try again : " );
      else
      {
        d->olc->zone->cmd[d->olc->value].arg2 = pos;
        zDispArg3(d);
      }
      break;
    case 'R':
      pos = real_object(atoi(arg));
      if (pos >= 0)
      {
        d->olc->zone->cmd[d->olc->value].arg2 = pos;
        zMenu(d);
      }
      else
        sendChar( DCHR, "That object does not exist, try again : " );
      break;
    default:
      /*. We should never get here .*/
      cleanup_olc(d, CLEANUP_ALL);
      mudlog(BRF, LVL_BUILDER, TRUE, "OLCERR: OLC: zParse(): case ARG2: Ack!");
      break;
    }
    break;

/*-------------------------------------------------------------------*/
  case ZEDIT_ARG3:
    /*. Parse the input for arg3, and go back to main menu.*/
    if( !isdigit( chIn ))
    {
      sendChar( DCHR, "Must be a numeric value, try again : " );
      return;
    }
    switch(d->olc->zone->cmd[d->olc->value].command)
    {
    case 'E':
      pos = atoi(arg);
      /*. Count number of wear positions (could use NUM_WEARS,
          this is more reliable) .*/
      while(*equipment_types[i] != '\n')
        i++;
      if ((pos < 0) || (pos > i))
        sendChar( DCHR, "Try again : " );
      else
      {
        d->olc->zone->cmd[d->olc->value].arg3 = pos;
        zMenu(d);
      }
      break;
    case 'P':
      pos = real_object(atoi(arg));
      if (pos >= 0)
      {
        d->olc->zone->cmd[d->olc->value].arg3 = pos;
        zMenu(d);
      } 
      else
        sendChar( DCHR, "That object does not exist, try again : " );
      break;
    case 'D':
      pos = atoi(arg);
      if ((pos < 0) || (pos > 2))
        sendChar( DCHR, "Try again : " );
      else
      {
        d->olc->zone->cmd[d->olc->value].arg3 = pos;
        zMenu(d);
      }
      break;
    case 'A':
      pos = atoi(arg);
      if ((pos < 1) || (pos > LVL_IMMORT))
        sendChar( DCHR, "Try again : " );
      else
      {
        d->olc->zone->cmd[d->olc->value].arg3 = pos;
        zMenu(d);
      }
      break;
    case 'M':
    case 'O':
    case 'G':
    case 'R':
    default:
      /*. We should never get here .*/
      cleanup_olc(d, CLEANUP_ALL);
      mudlog(BRF, LVL_BUILDER, TRUE, "OLCERR: OLC: zParse(): case ARG3: Ack!");
      break;
    }
  break;
  
/*-------------------------------------------------------------------*/
  case ZEDIT_ZONE_NAME:
    /*. Add new name and return to main menu .*/
    free(d->olc->zone->name);
    d->olc->zone->name = str_dup(arg);
    d->olc->zone->number = 1;
    zMenu(d);
    break;

/*-------------------------------------------------------------------*/
  case ZEDIT_ZONE_RESET:
    /*. Parse and add new reset_mode and return to main menu .*/
    pos = atoi(arg);
    if (!isdigit(*arg) || (pos <  0) || (pos > 3))
      sendChar( DCHR, "Try again (0-2) : " );
    else
    {
      d->olc->zone->reset_mode = pos;
      d->olc->zone->number = 1;
      zMenu(d);
    }
    break; 

/* --------------------------------------------------------------- */
case ZEDIT_ZONE_FLAGS:

  number = atoi(arg);
  if ((number < 0) || (number > NUM_ZONE_FLAGS)) {
    send_to_char("That is not a valid choice!\r\n", d->character);
    zMenu(d);
    }
  else
    if (number == 0) {
    zMenu(d); 
     break; 
     }
  else {
    TOGGLE_BIT(OLC_ZONE(d)->zone_flags, 1 << (number - 1));
    OLC_ZONE(d)->number = 1;
    zMenu(d);
  }
  return;
  break;

/*-------------------------------------------------------------------*/
  case ZEDIT_ZONE_LIFE:
    /*. Parse and add new lifespan and return to main menu .*/
    pos = atoi(arg);
    if (!isdigit(*arg) || (pos <  0) || (pos > 240))
      sendChar( DCHR, "Try again (0-240) : " );
    else
    {
      d->olc->zone->lifespan = pos;
      d->olc->zone->number = 1;
      zMenu(d);
    }
    break; 

/*-------------------------------------------------------------------*/
  case ZEDIT_ZONE_TOP:
    /*. Parse and add new top room in zone and return to main menu .*/
    if (d->olc->zone_num == top_of_zone_table)
      d->olc->zone->top = MAX(d->olc->zone_num * 100, MIN(32000, atoi(arg)));
    else
      d->olc->zone->top = MAX(d->olc->zone_num * 100,
        MIN(zone_table[d->olc->zone_num+1].number * 100, atoi(arg)));
    zMenu(d);
    break; 

/*-------------------------------------------------------------------*/
  default:
    /*. We should never get here .*/
    cleanup_olc(d, CLEANUP_ALL);
    mudlog(BRF, LVL_BUILDER, TRUE, "OLCERR: OLC: zParse(): Reached default case!");
    break;
  }
}
/*. End of parse_zedit() .*/
