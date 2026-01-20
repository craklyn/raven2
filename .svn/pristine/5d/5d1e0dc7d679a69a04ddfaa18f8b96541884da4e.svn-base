/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*  _TwyliteMud_ by Rv.                          Based on CircleMud3.0bpl9 *
*    				                                          *
*  OasisOLC - redit.c 		                                          *
*    				                                          *
*  Copyright 1996 Harvey Gilpin.                                          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*. Original author: Levork .*/

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/comm.h"
#include "util/utils.h"
#include "specials/boards.h"
#include "olc/olc.h"
#include "general/class.h"
#include "actions/act.clan.h"
#include "olc/redit.h"
#include "general/modify.h"
#include "scripts/dg_olc.h"

/*------------------------------------------------------------------------*/
/* function protos */

void rExitFlagMenu(DescriptorData * d);
void rFlagMenu(DescriptorData * d);
void rSectorMenu(DescriptorData * d);
void rSaveInternally(DescriptorData *d);
void rExtraDescMenu(DescriptorData *d);
void rExitMenu(DescriptorData *d);
void rMenu(DescriptorData *d);

/*------------------------------------------------------------------------*/

#define  W_EXIT(room, num) (world[(room)].dir_option[(num)])

/*------------------------------------------------------------------------*\
  Setup for rivers
\*------------------------------------------------------------------------*/

const char *riverspeed[] =
{
      "Not Set",
      "Fast",
      "Quick",
      "Normal",
      "Slow"
};

const char *riverdirs[] = {
    "Not Set",
    "North",
    "East",
    "South",
    "West",
    "Up",
    "Down",
    "\n"
};


/*------------------------------------------------------------------------*\
  Utils and exported functions.
\*------------------------------------------------------------------------*/

void rSetupNew(DescriptorData *d, int number)
{
  int zone = 0;

  while (zone <= top_of_zone_table && number > zone_table[zone].top) zone++;
  if (zone > top_of_zone_table) {
    mlog("rSetupNew: room %d is outside of all known zones!", number);
    zone = 0;
  }
  CREATE(OLC_ROOM(d), RoomData, 1);
  OLC_ROOM(d)->name = str_dup("An unfinished room");
  OLC_ROOM(d)->description = str_dup("You are in an unfinished room.\r\n");
  OLC_ROOM(d)->clan_id = 0;
  OLC_ROOM(d)->zone = zone;
  OLC_ITEM_TYPE(d) = WLD_TRIGGER;
  rMenu(d);
  OLC_VAL(d) = 0;
}

void rSetupExisting(DescriptorData *d, int real_num)
{ RoomData *room;
  struct trig_proto_list *proto, *fproto;
  int counter;
  
  /*. Build a copy of the room .*/
  CREATE (room, RoomData, 1);
  *room = world[real_num];
  /* allocate space for all strings  */
  if (world[real_num].name)
    room->name = str_dup (world[real_num].name);
  if (world[real_num].description)
    room->description = str_dup (world[real_num].description);

  /* exits - alloc only if necessary */
  for (counter = 0; counter < NUM_OF_DIRS; counter++)
  { if (world[real_num].dir_option[counter])
    { CREATE(room->dir_option[counter], struct room_direction_data, 1);
      /* copy numbers over */
      *room->dir_option[counter] = *world[real_num].dir_option[counter];
      /* malloc strings */
      if (world[real_num].dir_option[counter]->general_description)
        room->dir_option[counter]->general_description =
          str_dup(world[real_num].dir_option[counter]->general_description);
      if (world[real_num].dir_option[counter]->keyword)
        room->dir_option[counter]->keyword =
          str_dup(world[real_num].dir_option[counter]->keyword);
    }
  }
  
  /*. Extra descriptions if necessary .*/ 
  if (world[real_num].ex_description) 
  { struct extra_descr_data *this, *temp, *temp2;
    CREATE (temp, struct extra_descr_data, 1);
    room->ex_description = temp;
    for (this = world[real_num].ex_description; this; this = this->next)
    { if (this->keyword)
        temp->keyword = str_dup (this->keyword);
      if (this->description)
        temp->description = str_dup (this->description);
      if (this->next)
      { CREATE (temp2, struct extra_descr_data, 1);
	temp->next = temp2;
	temp = temp2;
      } else
        temp->next = NULL;
    }
  }

  proto = world[real_num].proto_script;
  while (proto) {
    CREATE(fproto, struct trig_proto_list, 1);
    fproto->vnum = proto->vnum;
    if (room->proto_script==NULL)
      room->proto_script = fproto;
    proto = proto->next;
    fproto = fproto->next; /* NULL */
  }
 
  /*. Attatch room copy to players descriptor .*/
  OLC_ROOM(d) = room;
  OLC_VAL(d) = 0;
  OLC_ITEM_TYPE(d) = WLD_TRIGGER;
  dg_olc_script_copy(d);
  rMenu(d);
}

/*------------------------------------------------------------------------*/
      
#define ZCMD (zone_table[zone].cmd[cmd_no])

void
rSaveInternally( DescriptorData *d )
{
  int i, j, room_num, found = 0, zone, cmd_no;
  RoomData *new_world;
  CharData *temp_ch;
  ObjData *temp_obj;

  room_num = real_room(OLC_NUM(d));
  if (room_num > 0) 
  { /*. Room exits: move contents over then free and replace it .*/
    OLC_ROOM(d)->contents = world[room_num].contents;
    OLC_ROOM(d)->people = world[room_num].people;
    rFreeRoom(world + room_num);
    world[room_num] = *OLC_ROOM(d);
    world[room_num].proto_script = OLC_SCRIPT(d);
  } else 
  { /*. Room doesn't exist, hafta add it .*/

    CREATE(new_world, RoomData, top_of_world + 2);

    /* count thru world tables */
    for (i = 0; i <= top_of_world; i++) 
    { if (!found) {
        /*. Is this the place? .*/
        if (world[i].number > OLC_NUM(d)) 
	{ found = 1;

	  new_world[i] = *(OLC_ROOM(d));
	  new_world[i].number = OLC_NUM(d);
	  new_world[i].func = NULL;
	  new_world[i].proto_script = OLC_SCRIPT(d);
          room_num  = i;
	
	  /* copy from world to new_world + 1 */
          new_world[i + 1] = world[i];
          /* people in this room must have their numbers moved */
	  for (temp_ch = world[i].people; temp_ch; temp_ch = temp_ch->next_in_room)
	    if (temp_ch->in_room != -1)
	      temp_ch->in_room = i + 1;

	  /* move objects */
	  for (temp_obj = world[i].contents; temp_obj; temp_obj = temp_obj->next_content)
	    if (temp_obj->in_room != -1)
	      temp_obj->in_room = i + 1;
        } else 
        { /*.   Not yet placed, copy straight over .*/
	  new_world[i] = world[i];
        }
      } else 
      { /*. Already been found  .*/
 
        /* people in this room must have their in_rooms moved */
        for (temp_ch = world[i].people; temp_ch; temp_ch = temp_ch->next_in_room)
	  if (temp_ch->in_room != -1)
	    temp_ch->in_room = i + 1;

        /* move objects */
        for (temp_obj = world[i].contents; temp_obj; temp_obj = temp_obj->next_content)
  	  if (temp_obj->in_room != -1)
	    temp_obj->in_room = i + 1;

        new_world[i + 1] = world[i];
      }
    }
    if (!found)
    { /*. Still not found, insert at top of table .*/
      new_world[i] = *(OLC_ROOM(d));
      new_world[i].number = OLC_NUM(d);
      new_world[i].func = NULL;
      new_world[i].proto_script = OLC_SCRIPT(d);
      room_num  = i;
    }

    /* copy world table over */
    free(world);
    world = new_world;
    top_of_world++;

    /*. Update zone table .*/
    for( zone = 0; zone <= top_of_zone_table; zone++ )
      for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++)
        switch (ZCMD.command)
        {
          case 'M':
          case 'O':
            if (ZCMD.arg3 >= room_num)
              ZCMD.arg3++;
 	    break;
          case 'D':
            if (ZCMD.arg1 >= room_num)
              ZCMD.arg1++;
            break;
          case 'E':
          case 'G':
          case 'P':
          case 'Q':
          case 'R':
          case 'T':
          case '*':
            break;
          default:
            mudlog(NRM, LVL_IMMORT, TRUE, "OLCERR: Unknown zone comand %c for room %d",
                     ZCMD.command, room_num );
        }

    getStartRoom( NULL ); /* Reload the starters */

    /*. Update world exits .*/
    for (i = 0; i < top_of_world + 1; i++)
      for (j = 0; j < NUM_OF_DIRS; j++)
        if (W_EXIT(i, j))
          if (W_EXIT(i, j)->to_room >= room_num)
	    W_EXIT(i, j)->to_room++;

  }
  assign_triggers(&world[room_num], WLD_TRIGGER);
  olc_add_to_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_ROOM);
}


/*------------------------------------------------------------------------*/

void rSaveToDisk(DescriptorData *d)
{
  int counter, counter2, realcounter;
  FILE *fp;
  RoomData *room;
  struct extra_descr_data *ex_desc;

  sprintf(buf, "%s/%d.wld.new", WLD_PREFIX, zone_table[OLC_ZNUM(d)].number);
  if (!(fp = fopen(buf, "w+")))
  { mudlog(BRF, LVL_BUILDER, TRUE, "OLCERR: OLC: Cannot open room file!");
    return;
  }

  for (counter = zone_table[OLC_ZNUM(d)].number * 100;
       counter <= zone_table[OLC_ZNUM(d)].top;
       counter++) 
  { realcounter = real_room(counter);
    if (realcounter >= 0) 
    { 
      room = (world + realcounter);

  /* Mortius : if room name is DELETEME don't write to disk */
    if (room->name && strcmp(room->name, "DELETEME") == 0)
        continue;

      rCheckRoomFlags(room); /* remove any bad room flags, like BFS mark */

      /*. Remove the '\r\n' sequences from description .*/
      strcpy(buf1, room->description ? room->description : "Empty");
      strip_string(buf1);

      /*. Build a buffer ready to save .*/
     sprintf(buf, "#%d\n%s~\n%s~\n%d %d %d %d %d %d %d %d\n",
	     counter, room->name ? room->name : "undefined", buf1,
	     zone_table[room->zone].number,
	     room->room_flags[0], room->room_flags[1],
	     room->room_flags[2], room->room_flags[3],
	     room->sector_type,
	     room->clan_id, room->clan_recept_sz);

      /*. Save this section .*/
      fputs(buf, fp);

      /*. Handle exits .*/
      for (counter2 = 0; counter2 < NUM_OF_DIRS; counter2++) 
      { if (room->dir_option[counter2]) 
        { int temp_door_flag;

          /*. Again, strip out the crap .*/
          if (room->dir_option[counter2]->general_description)
          { strcpy(buf1, room->dir_option[counter2]->general_description);
            strip_string(buf1);
          } else
          *buf1 = 0;

          /*. Figure out door flag .*/
          if (IS_SET(room->dir_option[counter2]->exit_info, EX_ISDOOR)) 
          { if (IS_SET(room->dir_option[counter2]->exit_info, EX_PICKPROOF))
	      temp_door_flag = 2;
	    else
	      temp_door_flag = 1;
	  } else
	      temp_door_flag = 0;

          /*. Check for keywords .*/
          if(room->dir_option[counter2]->keyword)
            strcpy(buf2, room->dir_option[counter2]->keyword);
          else
            *buf2 = 0;
               
	  /* Some how we wind up creating void exits fix it here. Vex. */
	  if (world[room->dir_option[counter2]->to_room].number == 0) {
	      mudlog(NRM, LVL_IMMORT, TRUE, "(rSaveToDisk): room[%d] D%d has undefined exit, setting to -1.", counter, counter2);
	      world[room->dir_option[counter2]->to_room].number = -1;
	  }
          /*. Ok, now build a buffer to output to file .*/
          sprintf(buf, "D%d\n%s~\n%s~\n%d %d %d\n",
                        counter2, buf1, buf2, temp_door_flag,
			room->dir_option[counter2]->key,
			world[room->dir_option[counter2]->to_room].number
          );
          /*. Save this door .*/
	  fputs(buf, fp);
        }
      }

      if( room->ex_description )
      {
        for( ex_desc = room->ex_description; ex_desc; ex_desc = ex_desc->next )
        {
          // Home straight, just deal with extras descriptions..
          //
          if( ex_desc->description == NULL )
          {
            strcpy( buf1, "EMPTY DESCRIPTION" );
          }
          else
          {
            strcpy( buf1, ex_desc->description );
          }

          strip_string(buf1);
          sprintf(buf, "E\n%s~\n%s~\n", ex_desc->keyword,buf1);
          fputs(buf, fp);
	}
      }
      fprintf(fp, "S\n");
      script_save_to_disk(fp, room, WLD_TRIGGER);
    }
  }
  /* write final line and close */
  fprintf(fp, "$~\n");
  fclose(fp);
  olc_remove_from_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_ROOM);

  sprintf(buf, "%s/%d.wld", WLD_PREFIX, zone_table[OLC_ZNUM(d)].number);
  sprintf(buf1, "%s/%d.wld~", WLD_PREFIX, zone_table[OLC_ZNUM(d)].number);
  sprintf(buf2, "%s/%d.wld.new", WLD_PREFIX, zone_table[OLC_ZNUM(d)].number);

  /* try to back up the existing wld file */
  if (rename(buf, buf1) != 0) {
      mudlog(BRF, LVL_BUILDER, TRUE, "OLC: error backing up old wld file %s: %s",
              buf, strerror(errno));
  }

  /* regardless, press on to replace it with the new one */
  if (rename(buf2, buf) != 0) {
      mudlog(BRF, LVL_BUILDER, TRUE, "OLC: cannot move %s to %s: %s",
              buf2, buf, strerror(errno));
  }
}

/*------------------------------------------------------------------------*/

void rFreeRoom(RoomData *room)
{ int i;
  struct extra_descr_data *this, *next;

  if (room->name)
    free(room->name);
  if (room->description)
    free(room->description);

  /*. Free exits .*/
  for (i = 0; i < NUM_OF_DIRS; i++)
  { if (room->dir_option[i])
    { if (room->dir_option[i]->general_description)
        free(room->dir_option[i]->general_description);
      if (room->dir_option[i]->keyword)
        free(room->dir_option[i]->keyword);
    }
    free(room->dir_option[i]);
  }

  /*. Free extra descriptions .*/
  for (this = room->ex_description; this; this = next)
  { next = this->next;
    if (this->keyword)
      free(this->keyword);
    if (this->description)
      free(this->description);
    free(this);
  }
}

/* Removes undesirable room flags. Note that this is invoked in db.c */
/* so be careful which room flags you add to this. */
void
rCheckRoomFlags(RoomData *room)
{
/*128: PROBLEM: Why do we need this?  REMOVED THE COMMENT MARKS
  if (room->room_flags & ROOM_OLC)
	room->room_flags &= ~ROOM_OLC;  not used 
  if (room->room_flags & ROOM_BFS_MARK)
	room->room_flags &= ~ROOM_BFS_MARK;  internal for track 
*/
}

/**************************************************************************
 Menu functions 
 **************************************************************************/

/* For extra descriptions */
void rExtraDescMenu(DescriptorData * d)
{
  struct extra_descr_data *extra_desc = OLC_DESC(d);
  
  sprintf(buf, "[H[J"
  	"&021)&06 Keyword:&00 %s\r\n"
  	"&022)&06 Description:&00\r\n%s\r\n"
        "&023)&06 Goto next description:&00 ",

	extra_desc->keyword ? extra_desc->keyword : "<NONE>",
	extra_desc->description ?  extra_desc->description : "<NONE>"
  );
  
  if (!extra_desc->next)
    strcat(buf, "<NOT SET>\r\n");
  else
    strcat(buf, "Set.\r\n");
  strcat(buf, "Enter choice (0 to quit) : ");
  send_to_char(buf, d->character);
  OLC_MODE(d) = REDIT_EXTRADESC_MENU;
}

/* For exits */
void rExitMenu(DescriptorData * d)
{
  /* if exit doesn't exist, alloc/create it */
  if(!OLC_EXIT(d))
    CREATE(OLC_EXIT(d), struct room_direction_data, 1);

  /* weird door handling! */
  if (IS_SET(OLC_EXIT(d)->exit_info, EX_ISDOOR)) {
    if (IS_SET(OLC_EXIT(d)->exit_info, EX_PICKPROOF))
      strcpy(buf2, "Pickproof");
    else
      strcpy(buf2, "Is a door");
  } else
    strcpy(buf2, "No door");

  sprintf(buf, "[H[J"
	"&021)&06 Exit to     :&00 %d\r\n"
	"&022)&06 Description :&00-\r\n%s\r\n"
  	"&023)&06 Door name   :&00 %s\r\n"
  	"&024)&06 Key         :&00 %d\r\n"
  	"&025)&06 Door flags  :&00 %s\r\n"
  	"&026)&06 Purge exit.&00\r\n"
	"Enter choice, 0 to quit : ",

	world[OLC_EXIT(d)->to_room].number,
        OLC_EXIT(d)->general_description ? OLC_EXIT(d)->general_description : "<NONE>",
	OLC_EXIT(d)->keyword ? OLC_EXIT(d)->keyword : "<NONE>",
	OLC_EXIT(d)->key,
	buf2
  );

  send_to_char(buf, d->character);
  OLC_MODE(d) = REDIT_EXIT_MENU;
}

/* For exit flags */
void rExitFlagMenu(DescriptorData * d)
{
  sprintf(buf,  "&020)&06 No door\r\n"
  		"&021)&06 Closeable door\r\n"
		"&022)&06 Pickproof\r\n"
  		"&00Enter choice : "
  );
  send_to_char(buf, d->character);
}

/* For room flags */
#ifdef OLDWAY
void rFlagMenu(DescriptorData * d) {
    bitTable(d, room_bits, NUM_ROOM_FLAGS, "room", d->olc->room->room_flags);
}
#else
void rFlagMenu(DescriptorData *d)
{
  int counter, columns = 0;

  get_char_colors(d->character);
#if defined(CLEAR_SCREEN)
  send_to_char("^[[H^[[J", d->character);
#endif
  for (counter = 0; counter < NUM_ROOM_FLAGS; counter++) {
    sprintf(buf, "%s%2d%s) %-15.15s %s", grn, counter + 1, nrm,
                room_bits[counter], !(++columns % 4) ? "\r\n" : "");
    send_to_char(buf, d->character);
  }
  sprintbitarray(OLC_ROOM(d)->room_flags, room_bits, RF_ARRAY_MAX, buf1);
  sprintf(buf, "\r\nRoom flags: %s%s%s\r\n"
          "Enter room flags, 0 to quit : ", cyn, buf1, nrm);
  send_to_char(buf, d->character);
  OLC_MODE(d) = REDIT_FLAGS;
}
#endif

/* for sector type */
void rSectorMenu(DescriptorData *d)
{
  int counter, columns = 0;

#if defined(CLEAR_SCREEN)
  send_to_char("^[[H^[[J", d->character);
#endif
  for (counter = 0; counter < NUM_ROOM_SECTORS; counter++) {
    sprintf(buf, "&02%2d&00) %-20.20s %s", counter,
                sector_types[counter], !(++columns % 3) ? "\r\n" : "");
    send_to_char(buf, d->character);
  }
  send_to_char("\r\nEnter sector type : ", d->character);
  OLC_MODE(d) = REDIT_SECTOR;
}

/* It appears this was never implemented. Commenting it out so it won't cause
 * any further confusion. -Xiuh 08.11.09
// for sector type
void rClanMenu(DescriptorData * d) {
    intTable( d, sector_types, 
                 NUM_ROOM_SECTORS,
                "sector type",
                 d->olc->room->sector_type);
}
*/

/* the main menu */
void rMenu(DescriptorData * d)
{ 
  RoomData *room;
  char river_direction[10], river_speed[10];

  room = OLC_ROOM(d);

  /* Mortius: Work out river direction/speed */
//  sprintf(river_direction, "Not-Working", riverdirs[GET_RIVER_DIR(room)]);
//  sprintf(river_speed, "Not-Working", riverspeed[GET_RIVER_SPEED(room)]);
  sprintf(river_direction, "Not-Working");
  sprintf(river_speed, "Not-Working");

  sprintbitarray(OLC_ROOM(d)->room_flags, room_bits, RF_ARRAY_MAX, buf1);
  sprinttype(room->sector_type, sector_types, buf2, sizeof(buf2));
  sprintf(buf,
  	"[H[J"
	"-- Room number : [%d]  	Room zone: [%d]\r\n"
	"&021)&06 Name        :&00 %s\r\n"
	"&022)&06 Description :&00\r\n%s"
  	"&023)&06 Room flags  :&03 %s\r\n"
	"&024)&06 Sector type :&03 %s\r\n"
  	"&025)&06 Exit north  :&03 %d\r\n"
  	"&026)&06 Exit east   :&03 %d\r\n"
  	"&027)&06 Exit south  :&03 %d\r\n"
  	"&028)&06 Exit west   :&03 %d\r\n"
  	"&029)&06 Exit up     :&03 %d\r\n"
  	"&02A)&06 Exit down   :&03 %d\r\n"
  	"&02B)&06 Extra descriptions menu\r\n"
  	"&02C)&06 Set clan id :&03 [&06%d&03] &00%s\r\n"
        "&02D)&06 Room level  :&03 \r\n"
        "&02E)&06 River dir   :&03 [&06%s&03]\r\n"
	"&02F)&06 River Speed :&03 [&06%s&03]\r\n"
	"&02G)&06 Script      :&03 %s\r\n"
  	"&02Q)&06 Quit&00\r\n"
  	"Enter choice : ",

	OLC_NUM(d),
	zone_table[OLC_ZNUM(d)].number,
	room->name,
	room->description,
	buf1,
	buf2,
  	room->dir_option[NORTH] ?
          world[room->dir_option[NORTH]->to_room].number : -1,
	room->dir_option[EAST] ?
          world[room->dir_option[EAST]->to_room].number : -1,
  	room->dir_option[SOUTH] ? 
          world[room->dir_option[SOUTH]->to_room].number : -1,
  	room->dir_option[WEST] ? 
          world[room->dir_option[WEST]->to_room].number : -1,
  	room->dir_option[UP] ? 
          world[room->dir_option[UP]->to_room].number : -1,
  	room->dir_option[DOWN] ? 
          world[room->dir_option[DOWN]->to_room].number : -1,
        room->clan_id, get_clan_name(room->clan_id),
//	room->extra.level,
	river_direction,
	river_speed,
	room->proto_script?"Set.":"Not Set."
  );
  send_to_char(buf, d->character);

  OLC_MODE(d) = REDIT_MAIN_MENU;
}



/**************************************************************************
  The main loop
 **************************************************************************/

void rParse(DescriptorData * d, char *arg)
{
//  extern RoomData *world;
  int number;
  char chIn = toupper( *arg );

  switch( OLC_MODE(d) )
  {
  case REDIT_CONFIRM_SAVESTRING:
    switch (chIn)
    {
    case 'Y':
      rSaveInternally(d);
      mudlog( CMP, LVL_BUILDER, TRUE, "OLC: %s edits room %d", GET_NAME(d->character), OLC_NUM(d));
      /*. Do NOT free strings! just the room structure .*/
      cleanup_olc(d, CLEANUP_STRUCTS);
      send_to_char("Room saved to memory.\r\n", d->character);
      break;
    case 'N':
      /* free everything up, including strings etc */
      cleanup_olc(d, CLEANUP_ALL);
      break;
    default:
      send_to_char("Invalid choice!\r\n", d->character);
      send_to_char("Do you wish to save this room internally? : ", d->character);
      break;
    }
    return;

  case REDIT_MAIN_MENU:
    d->olc->tableDisp = TRUE; /* We want to display the tables. */
    switch( chIn )
    {
    case 'Q':
      if (OLC_VAL(d))
      { /*. Something has been modified .*/
        send_to_char("Do you wish to save this room internally? : ", d->character);
        OLC_MODE(d) = REDIT_CONFIRM_SAVESTRING;
      } else
        cleanup_olc(d, CLEANUP_ALL);
      return;
    case '1':
      send_to_char("Enter room name:-\r\n| ", d->character);
      OLC_MODE(d) = REDIT_NAME;
      break;
    case '2':
      OLC_MODE(d) = REDIT_DESC;
      write_to_output(d, "\x1B[H\x1B[J");
      write_to_output(d, "Enter room description: (/s saves /h for help)\r\n\r\n");
      SEND_RULER(d);
      d->backstr = NULL;
      if (OLC_ROOM(d)->description) {
          write_to_output(d, "%s", OLC_ROOM(d)->description);
	  d->backstr = str_dup(OLC_ROOM(d)->description);
      }
      d->str = &OLC_ROOM(d)->description;
      d->max_str = MAX_ROOM_DESC;
      d->mail_to = 0;
      OLC_VAL(d) = 1;
      break;
    case '3': rFlagMenu(d); OLC_MODE(d) = REDIT_FLAGS; break;
    case '4': rSectorMenu(d);  OLC_MODE(d) = REDIT_SECTOR; break;
    case '5': OLC_VAL(d) = NORTH; rExitMenu(d); break;
    case '6': OLC_VAL(d) = EAST;  rExitMenu(d); break;
    case '7': OLC_VAL(d) = SOUTH; rExitMenu(d); break;
    case '8': OLC_VAL(d) = WEST;  rExitMenu(d); break;
    case '9': OLC_VAL(d) = UP;    rExitMenu(d); break;
    case 'A':
      OLC_VAL(d) = DOWN;
      rExitMenu(d);
      break;
    case 'B':
      /* if extra desc doesn't exist . */
      if (!OLC_ROOM(d)->ex_description) {
	CREATE(OLC_ROOM(d)->ex_description, struct extra_descr_data, 1);
	OLC_ROOM(d)->ex_description->next = NULL;
      }
      OLC_DESC(d) = OLC_ROOM(d)->ex_description;
      rExtraDescMenu(d);
      break;
    case 'C':
      sendChar( d->character, "Enter a clan id:");
      OLC_MODE(d) = REDIT_CLAN;
      break;
    case 'G':
      OLC_SCRIPT_EDIT_MODE(d) = SCRIPT_MAIN_MENU;
      dg_script_menu(d);
      break;
    default:
      send_to_char("Invalid choice!", d->character);
      rMenu(d);
      break;
    }
    d->olc->tableDisp = FALSE; /* Now we only want to prompt, not display table. */
    return;

  case OLC_SCRIPT_EDIT:
    if (dg_script_edit_parse(d, arg)) return;
    break;

  case REDIT_NAME:
    if (OLC_ROOM(d)->name)
      free(OLC_ROOM(d)->name);
    if (strlen(arg) > MAX_ROOM_NAME)
      arg[MAX_ROOM_NAME -1] = 0;
    OLC_ROOM(d)->name = str_dup(arg);
    break;
  case REDIT_DESC:
    /* we will NEVER get here */
    mudlog(BRF, LVL_BUILDER, TRUE, "OLCERR: Reached REDIT_DESC case in parse_redit");
    break;

#ifdef OLDWAY
  case REDIT_FLAGS:
    number = toggleBit(d, arg, room_bits, NUM_ROOM_FLAGS, "room flag", &OLC_ROOM(d)->room_flags);
    if (number < 0) { /* Entry was invalid. */
	rFlagMenu(d);
	return;
    }
    rCheckRoomFlags(d->olc->room);
    break;
#else
  case REDIT_FLAGS:
    number = atoi(arg);
    if ((number < 0) || (number > NUM_ROOM_FLAGS)) {
      send_to_char("That is not a valid choice!\r\n", d->character);
      rFlagMenu(d);
    } else if (number == 0)
        break;
    else {
      /*
       * Toggle the bit.
       */
      TOGGLE_BIT_AR(OLC_ROOM(d)->room_flags, (number - 1));
      rFlagMenu(d);
    }
    return;
#endif
  case REDIT_SECTOR:
    number = atoi(arg);
    if (number < 0 || number >= NUM_ROOM_SECTORS) {
      send_to_char("Invalid choice!", d->character);
      rSectorMenu(d);
      return;
    } else
      OLC_ROOM(d)->sector_type = number;
    break;


  case REDIT_CLAN:
    number = atoi(arg);
    OLC_ROOM(d)->clan_id = number;
    OLC_ROOM(d)->clan_recept_sz = 35;
    break;

  case REDIT_EXIT_MENU:
    switch (*arg) {
    case '0':
      break;
    case '1':
      OLC_MODE(d) = REDIT_EXIT_NUMBER;
      send_to_char("Exit to room number : ", d->character);
      return;
    case '2':
      OLC_MODE(d) = REDIT_EXIT_DESCRIPTION;
       write_to_output(d, "Enter exit description: (/s saves /h for help)\r\n\r\n");
       d->backstr = NULL;
       if (OLC_EXIT(d)->general_description) {
          write_to_output(d, "%s", OLC_EXIT(d)->general_description);
	  d->backstr = str_dup(OLC_EXIT(d)->general_description);
       }
       d->str = &OLC_EXIT(d)->general_description;
      d->max_str = MAX_EXIT_DESC;
      d->mail_to = 0;
      return;
    case '3':
      OLC_MODE(d) = REDIT_EXIT_KEYWORD;
      send_to_char("Enter keywords : ", d->character);
      return;
    case '4':
      OLC_MODE(d) = REDIT_EXIT_KEY;
      send_to_char("Enter key number : ", d->character);
      return;
    case '5':
      rExitFlagMenu(d);
      OLC_MODE(d) = REDIT_EXIT_DOORFLAGS;
      return;
    case '6':
      /* delete exit */
      if (OLC_EXIT(d)->keyword)
	free(OLC_EXIT(d)->keyword);
      if (OLC_EXIT(d)->general_description)
	free(OLC_EXIT(d)->general_description);
      free(OLC_EXIT(d));
      OLC_EXIT(d) = NULL;
      break;
    default:
      send_to_char("Try again : ", d->character);
      return;
    }
    break;

  case REDIT_EXIT_NUMBER:
    number = (atoi(arg));
    if (number != -1)
    { number = real_room(number);
      if (number < 0)
      { send_to_char("That room does not exist, try again : ", d->character);
        return;
      }
    }
    OLC_EXIT(d)->to_room = number;
    rExitMenu(d);
    return;

  case REDIT_EXIT_DESCRIPTION:
    /* we should NEVER get here */
    mudlog(BRF, LVL_BUILDER,TRUE, "OLCERR: Reached REDIT_EXIT_DESC case in parse_redit");
    break;

  case REDIT_EXIT_KEYWORD:
    if (OLC_EXIT(d)->keyword)
      free(OLC_EXIT(d)->keyword);
    OLC_EXIT(d)->keyword = str_dup(arg);
    rExitMenu(d);
    return;

  case REDIT_EXIT_KEY:
    number = atoi(arg);
    OLC_EXIT(d)->key = number;
    rExitMenu(d);
    return;

  case REDIT_EXIT_DOORFLAGS:
    number = atoi(arg);
    if ((number < 0) || (number > 2)) {
      send_to_char("That's not a valid choice!\r\n", d->character);
      rExitFlagMenu(d);
    } else {
      /* doors are a bit idiotic, don't you think? :) */
      if (number == 0)
	OLC_EXIT(d)->exit_info = 0;
      else if (number == 1)
	OLC_EXIT(d)->exit_info = EX_ISDOOR;
      else if (number == 2)
	OLC_EXIT(d)->exit_info = EX_ISDOOR | EX_PICKPROOF;
      /* jump out to menu */
      rExitMenu(d);
    }
    return;

  case REDIT_EXTRADESC_KEY:
    OLC_DESC(d)->keyword = str_dup(arg);
    rExtraDescMenu(d);
    return;

  case REDIT_EXTRADESC_MENU:
    number = atoi(arg);
    switch (number) {
    case 0:
      {
	/* if something got left out, delete the extra desc
	 when backing out to menu */
	if (!OLC_DESC(d)->keyword || !OLC_DESC(d)->description) 
        { struct extra_descr_data **tmp_desc;

	  if (OLC_DESC(d)->keyword)
	    free(OLC_DESC(d)->keyword);
	  if (OLC_DESC(d)->description)
	    free(OLC_DESC(d)->description);

	  /*. Clean up pointers .*/
	  for(tmp_desc = &(OLC_ROOM(d)->ex_description); *tmp_desc;
	      tmp_desc = &((*tmp_desc)->next))
          { if(*tmp_desc == OLC_DESC(d))
	    { *tmp_desc = NULL;
              break;
	    }
	  }
	  free(OLC_DESC(d));
	}
      }
      break;
    case 1:
      OLC_MODE(d) = REDIT_EXTRADESC_KEY;
      send_to_char("Enter keywords, separated by spaces : ", d->character);
      return;
    case 2:
      OLC_MODE(d) = REDIT_EXTRADESC_DESCRIPTION;
      write_to_output(d, "Enter extra description: (/s saves /h for help)\r\n\r\n");
      d->backstr = NULL;
      if (OLC_DESC(d)->description) {
	write_to_output(d, "%s", OLC_DESC(d)->description);
	    d->backstr = str_dup(OLC_DESC(d)->description);
      }
      d->str = &OLC_DESC(d)->description;
      d->max_str = MAX_MESSAGE_LENGTH;
      d->mail_to = 0;
      return;

    case 3:
      if (!OLC_DESC(d)->keyword || !OLC_DESC(d)->description) {
	send_to_char("You can't edit the next extra desc without completing this one.\r\n", d->character);
	rExtraDescMenu(d);
      } else {
	struct extra_descr_data *new_extra;

	if (OLC_DESC(d)->next)
	  OLC_DESC(d) = OLC_DESC(d)->next;
	else {
	  /* make new extra, attach at end */
	  CREATE(new_extra, struct extra_descr_data, 1);
	  OLC_DESC(d)->next = new_extra;
	  OLC_DESC(d) = new_extra;
	}
	rExtraDescMenu(d);
      }
      return;
    }
    break;

  default:
    /* we should never get here */
    mudlog(BRF, LVL_BUILDER, TRUE, "OLCERR: Reached default case in parse_redit");
    break;
  }
  /*. If we get this far, something has been changed .*/
  OLC_VAL(d) = 1;
  rMenu(d);
}

void redit_string_cleanup(struct descriptor_data *d, int terminator)
{
  switch (OLC_MODE(d)) {
  case REDIT_DESC:
    rMenu(d);
    break;
  case REDIT_EXIT_DESCRIPTION:
    rExitMenu(d);
    break;
  case REDIT_EXTRADESC_DESCRIPTION:
    rExtraDescMenu(d);
    break;
  }
}

