/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*  _TwyliteMud_ by Rv.                          Based on CircleMud3.0bpl9 *
*    				                                          *
*  OasisOLC - medit.c 		                                          *
*    				                                          *
*  Copyright 1996 Harvey Gilpin.                                          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/comm.h"
#include "general/class.h"
#include "magic/spells.h"
#include "util/utils.h"
#include "general/handler.h"
#include "specials/shop.h"
#include "olc/olc.h"
#include "olc/medit.h"
#include "general/rmath.h"
#include "actions/fight.h"
#include "general/modify.h"
#include "scripts/dg_olc.h"
#include "actions/act.clan.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */


extern int top_shop;					/*. shop.c	.*/
extern struct shop_data *shop_index;			/*. shop.c	.*/

/*-------------------------------------------------------------------*/
/*. Handy  macros .*/


#define GET_NDD(mob) ((mob)->mob_specials.damnodice)
#define GET_SDD(mob) ((mob)->mob_specials.damsizedice)
#define GET_ALIAS(mob) ((mob)->player.name)
#define GET_SDESC(mob) ((mob)->player.short_descr)
#define GET_LDESC(mob) ((mob)->player.long_descr)
#define GET_DDESC(mob) ((mob)->player.description)
#define GET_ATTACK(mob) ((mob)->mob_specials.attack_type)
#define S_KEEPER(shop) ((shop)->keeper)

/*-------------------------------------------------------------------*/
/*. Function prototypes .*/

void mSaveInternally(DescriptorData *d);
void initMob(CharData *mob);
void copyMob(CharData *tmob, CharData *fmob);
void mLoadPosMenu(DescriptorData *d);
void mDefaultPosMenu(DescriptorData *d);
void mActionMenu(DescriptorData *d);
void mAffectMenu(DescriptorData *d);
void mAttackMenu(DescriptorData *d);
void mFreeMob(CharData * mob);

/*-------------------------------------------------------------------*\
  utility functions 
\*-------------------------------------------------------------------*/

/* * * * *
 * Free a mobile structure that has been edited.
 * Take care of existing mobiles and their mob_proto!
 * * * * */
 
void mFreeMob(CharData * mob)
{
  int i;
  
  if (GET_MOB_RNUM(mob) == -1)	/* Non prototyped mobile */
  {
    if (mob->player.name)
      free(mob->player.name);
    if (mob->player.title)
      free(mob->player.title);
    if (mob->player.short_descr)
      free(mob->player.short_descr);
    if (mob->player.long_descr)
      free(mob->player.long_descr);
    if (mob->player.description)
      free(mob->player.description);
  }
  else if ((i = GET_MOB_RNUM(mob)) > -1) /* Prototyped mobile */
  {
    if (mob->player.name && mob->player.name != mob_proto[i].player.name)
      free(mob->player.name);
    if (mob->player.title && mob->player.title != mob_proto[i].player.title)
      free(mob->player.title);
    if (mob->player.short_descr && mob->player.short_descr != mob_proto[i].player.short_descr)
      free(mob->player.short_descr);
    if (mob->player.long_descr && mob->player.long_descr != mob_proto[i].player.long_descr)
      free(mob->player.long_descr);
    if (mob->player.description && mob->player.description != mob_proto[i].player.description)
      free(mob->player.description);
  }

  while (mob->affected)
    affect_remove(mob, mob->affected, FALSE); /* Mort: Added false for fall rooms */

  free(mob);
}

void mSetupNew(DescriptorData *d)
{ CharData *mob;

  /*. Alloc some mob shaped space .*/
  CREATE(mob, CharData, 1);
  initMob(mob);
  
  GET_MOB_RNUM(mob) = -1;
  /*. default strings .*/
  GET_ALIAS(mob) = str_dup("mob unfinished");
  GET_SDESC(mob) = str_dup("the unfinished mob");
  GET_LDESC(mob) = str_dup("An unfinished mob stands here.\r\n");
  GET_DDESC(mob) = str_dup("It looks, err, unfinished.\r\n");

  OLC_MOB(d) = mob;
  OLC_VAL(d) = 0;   /*. Has changed flag .*/
  OLC_ITEM_TYPE(d) = MOB_TRIGGER;
  
  mMenu(d);
}

/*-------------------------------------------------------------------*/

void mSetupExisting(DescriptorData *d, int rmob_num)
{ CharData *mob;

  /*. Alloc some mob shaped space .*/
  CREATE(mob, CharData, 1);
  copyMob(mob, mob_proto + rmob_num);
  OLC_MOB(d) = mob;
  OLC_ITEM_TYPE(d) = MOB_TRIGGER;
  dg_olc_script_copy(d);
  mMenu(d);
}

/*-------------------------------------------------------------------*/
/*. Copy one mob struct to another .*/

void copyMob(CharData *tmob, CharData *fmob)
{
  /*. Free up any used strings .*/
  if (GET_ALIAS(tmob))
    free(GET_ALIAS(tmob));
  if (GET_SDESC(tmob))
    free(GET_SDESC(tmob));
  if (GET_LDESC(tmob))
    free(GET_LDESC(tmob));
  if (GET_DDESC(tmob))
    free(GET_DDESC(tmob));
  
  /*.Copy mob .*/
  *tmob = *fmob;
 
  /*. Realloc strings .*/
  if (GET_ALIAS(fmob))
    GET_ALIAS(tmob) = str_dup(GET_ALIAS(fmob));

  if (GET_SDESC(fmob))
    GET_SDESC(tmob) = str_dup(GET_SDESC(fmob));

  if (GET_LDESC(fmob))
    GET_LDESC(tmob) = str_dup(GET_LDESC(fmob));

  if (GET_DDESC(fmob))
    GET_DDESC(tmob) = str_dup(GET_DDESC(fmob));

}


/*-------------------------------------------------------------------*/
/*. Ideally, this function should be in db.c, but I'll put it here for
    portability.*/

void initMob(CharData *mob)
{
  clear_char(mob);

  GET_HIT(mob) = 1;
  GET_MANA(mob) = 1;
  GET_MAX_MANA(mob) = 100;
  GET_MAX_MOVE(mob) = 100;
  GET_NDD(mob) = 1;
  GET_SDD(mob) = 1;
  GET_WEIGHT(mob) = 200;
  GET_HEIGHT(mob) = 198;

  mob->real_abils.str   = 11;
  mob->real_abils.intel = 11;
  mob->real_abils.wis   = 11;
  mob->real_abils.dex   = 11;
  mob->real_abils.con   = 11;
  mob->real_abils.cha   = 11;
  mob->aff_abils = mob->real_abils;

  SET_BIT_AR(MOB_FLAGS(mob), MOB_ISNPC);
  mob->mob_specials.attack_type2 = mob->mob_specials.attack_type3 = -1;
  mob->player_specials = &dummy_mob;
}

/*-------------------------------------------------------------------*/
/*. Save new/edited mob to memory .*/

#define ZCMD zone_table[zone].cmd[cmd_no]
/* This function is crashing when it tries to save a newly created mob. */
void
mSaveInternally(DescriptorData *d)
{
  int rmob_num, found = 0, new_mob_num = 0, zone, cmd_no, shop;
  IndexData *new_index;
  CharData  *new_proto;
  CharData  *live_mob;
  DescriptorData *dsc;

  OLC_MOB(d)->proto_script = OLC_SCRIPT(d);

  rmob_num = real_mobile(OLC_NUM(d));

  /*. Mob exists? Just update it .*/
  if (rmob_num != -1)
  {
    copyMob((mob_proto + rmob_num), OLC_MOB(d));
    /*. Update live mobiles .*/
    for(live_mob = character_list; live_mob; live_mob = live_mob->next)
      if(IS_MOB(live_mob) && GET_MOB_RNUM(live_mob) == rmob_num)
      { /*. Only really need update the strings, since these can cause
            protection faults.  The rest can wait till a reset/reboot .*/
        GET_ALIAS(live_mob) = GET_ALIAS(mob_proto + rmob_num);
        GET_SDESC(live_mob) = GET_SDESC(mob_proto + rmob_num);
        GET_LDESC(live_mob) = GET_LDESC(mob_proto + rmob_num);
        GET_DDESC(live_mob) = GET_DDESC(mob_proto + rmob_num);
      }
    } 
    /*. Mob does not exist, hafta add it .*/
    else
    {
      CREATE(new_proto, CharData, top_of_mobt + 2);
      CREATE(new_index, IndexData, top_of_mobt + 2);
      for (rmob_num = 0; rmob_num <= top_of_mobt; rmob_num++)
      {
        if( !found )
        {
          /*. Is this the place?  .*/
          if ((rmob_num > top_of_mobt) ||
              (mob_index[rmob_num].virtual > OLC_NUM(d)))
          { /*. Yep, stick it here .*/
            found = 1;
            GET_MOB_RNUM(OLC_MOB(d)) = rmob_num;
            new_index[rmob_num].virtual = OLC_NUM(d);
            new_index[rmob_num].number = 0;
            new_index[rmob_num].func = NULL;
            new_mob_num = rmob_num;
            copyMob((new_proto + rmob_num), OLC_MOB(d));
            /*
            ** Copy the mob that should be here on top
            */
            new_index[rmob_num + 1] = mob_index[rmob_num];
            new_proto[rmob_num + 1] = mob_proto[rmob_num];
            GET_MOB_RNUM(new_proto + rmob_num + 1) = rmob_num + 1;
          }
          else
          {
            /*. Nope, copy over as normal.*/
            new_index[rmob_num] = mob_index[rmob_num];
            new_proto[rmob_num] = mob_proto[rmob_num];
          }
        }
        else
        {
          /*. We've already found it, copy the rest over .*/
          new_index[rmob_num + 1] = mob_index[rmob_num];
          new_proto[rmob_num + 1] = mob_proto[rmob_num];
          GET_MOB_RNUM(new_proto + rmob_num + 1) = rmob_num + 1;
        }
      }
      if( !found )
      {
        /*
        ** Still not found, must add it to the top of the table
        */
        new_mob_num = rmob_num;
        GET_MOB_RNUM(OLC_MOB(d)) = rmob_num;
        new_index[rmob_num].virtual = OLC_NUM(d);
        new_index[rmob_num].number = 0;
        new_index[rmob_num].func = NULL;
        copyMob((new_proto + rmob_num), OLC_MOB(d));
      }

      /*. Replace tables .*/
      free(mob_proto);
      mob_proto = new_proto;
      free(mob_index);
      mob_index = new_index;
      top_of_mobt++;

      /*. Update live mobile rnums .*/
      for(live_mob = character_list; live_mob; live_mob = live_mob->next)
        if(GET_MOB_RNUM(live_mob) > new_mob_num)
          GET_MOB_RNUM(live_mob)++;
    
      /*. Update zone table .*/
      for (zone = 0; zone <= top_of_zone_table; zone++)
        for (cmd_no = 0; ZCMD.command != 'S'; cmd_no++) 
          if (ZCMD.command == 'M')
            if (ZCMD.arg1 >= new_mob_num)
              ZCMD.arg1++;

      /*. Update shop keepers .*/
      if( shop_index != NULL )
      {
        for(shop = 0; shop <= top_shop; shop++)
          if(SHOP_KEEPER(shop) >= new_mob_num)
            SHOP_KEEPER(shop)++;

        /*. Update keepers in shops being edited .*/
        for(dsc = descriptor_list; dsc; dsc = dsc->next)
          if(dsc->connected == CON_SEDIT)
            if(S_KEEPER(OLC_SHOP(dsc)) >= new_mob_num)
              S_KEEPER(OLC_SHOP(dsc))++;
      }
    }
    olc_add_to_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_MOB);
}


//-------------------------------------------------------------------------
//
// Need to make this write flags in the more legible letter format.
//
void
mSaveToDisk( DescriptorData* d )
{ 
  FILE* mob_file;
  CharData* mob;
  char fname[64];
  int i;

  int zone = zone_table[OLC_ZNUM(d)].number; 
  int top  = zone_table[OLC_ZNUM(d)].top; 

  sprintf( fname, "%s/%i.mob", MOB_PREFIX, zone );

  if( !( mob_file = fopen(fname, "w" )))
  {
    mudlog(BRF, LVL_BUILDER, TRUE, "OLCERR: OLC: Cannot open mob file!");
    return;
  }

  // Seach database for mobs in this zone and save em.
  //
  for( i = zone * 100; i <= top; i++ )
  {
    int rmob_num = real_mobile(i);
    
    if (rmob_num != -1)  {
        mob = (mob_proto + rmob_num);
        /* Mortius: IF name is set to DELETEME don't write to disk */
        if (strcmp(GET_ALIAS(mob), "DELETEME") == 0)
            continue;

     if(fprintf(mob_file, "#%d\n", i) < 0)
      { mudlog(BRF, LVL_BUILDER, TRUE, "OLCERR: OLC: Cannot write mob file!\r\n");
        fclose(mob_file);
        return;
      }

      /*. Clean up strings .*/
      strcpy (buf1, GET_LDESC(mob) ? GET_LDESC(mob) : "undefined");
      strip_string(buf1);
      strcpy(buf2, GET_DDESC(mob) ? GET_DDESC(mob) : "undefined");
      strip_string(buf2);

      fprintf(mob_file, 
	"%c %c %d~\n"                   /* <race> <class> <subrace> */
	"%s~\n"				/* name list */
	"%s~\n"				/* short description */
	"%s~\n"				/* room description */
	"%s~\n"				/* detailed description */
        "%d %d %d %d %d %d %d %d %i\n" /* 4 * action, 4 * affect, align */
	"%d %d %i %dd%d+%d %dd%d+%d\n"  /* <lvl> <thac0> <ac> <hp> <dam> */
	"%d %d\n"                       /* <gold> <xps> */
	"%d %d %d"                      /* <load> <default> <sex> */
	" %d %d %d\n",			/* <att1> <att2> <att3> */
	get_race_char((int)GET_RACE(mob)),
	get_class_char((int)GET_CLASS(mob)),
	mob->mob_specials.sub_race,
	GET_ALIAS(mob) ? GET_ALIAS(mob) : "undefined",
	GET_SDESC(mob) ? GET_SDESC(mob) : "undefined",
	buf1,
	buf2,
        MOB_FLAGS_AR(mob, 0), MOB_FLAGS_AR(mob, 1),
        MOB_FLAGS_AR(mob, 2), MOB_FLAGS_AR(mob, 3),
        AFF_FLAGS_AR(mob, 0), AFF_FLAGS_AR(mob, 1),
        AFF_FLAGS_AR(mob, 2), AFF_FLAGS_AR(mob, 3),
  	GET_ALIGNMENT(mob), 
  	GET_LEVEL(mob),
  	20 - GET_HITROLL(mob), /*. Convert hitroll to thac0 .*/
        GET_AC(mob) / 10,
	GET_HIT(mob),
	GET_MANA(mob),
	GET_MOVE(mob),
	GET_NDD(mob),
	GET_SDD(mob),
	GET_DAMROLL(mob),
	GET_MOB_SKILL_SUC(mob),
	GET_MOB_SPELL_DAM(mob),
	mob->mob_specials.load_pos,
	mob->mob_specials.default_pos,
	GET_SEX(mob),
	mob->mob_specials.attack_type,
	mob->mob_specials.attack_type2,
	mob->mob_specials.attack_type3
      );

      script_save_to_disk(mob_file, mob, MOB_TRIGGER);
    }
  }
  fprintf(mob_file, "$\n");
  fclose(mob_file);
  olc_remove_from_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_MOB);
}

/**************************************************************************
 Menu functions 
 **************************************************************************/

/* Display load position */
void mLoadPosMenu(DescriptorData *d)
{
  int i, columns = 0;

  get_char_colors(d->character);

#if defined(CLEAR_SCREEN)
  send_to_char("^[[H^[[J", d->character);
#endif
  for (i = 0; *position_types[i] != '\n'; i++) {
    sprintf(buf, "%s%2d%s) %-14.14s %s", grn, i, nrm, position_types[i],
	    !(++columns % 3) ? "\r\n" : "");
    send_to_char(buf, d->character);
  }
  send_to_char("\r\nEnter position number : ", d->character);
}

/* Display default position */
void mDefaultPosMenu(DescriptorData *d)
{
  int i, columns = 0;

  get_char_colors(d->character);

#if defined(CLEAR_SCREEN)
  send_to_char("^[[H^[[J", d->character);
#endif
  for (i = 0; *position_types[i] != '\n'; i++) {
    sprintf(buf, "%s%2d%s) %-14.14s %s", grn, i, nrm,
	    position_types[i],
            !(++columns % 3) ? "\r\n" : "");
    send_to_char(buf, d->character);
  }
  send_to_char("\r\nEnter default position number : ", d->character);
}


/*. Display sex (Oooh-err).*/
void mSexMenu(DescriptorData *d)
{
  int i, columns = 0;

  get_char_colors(d->character);

#if defined(CLEAR_SCREEN)
  send_to_char("^[[H^[[J", d->character);
#endif
  for (i = 0; *genders[i] != '\n'; i++) {
    sprintf(buf, "%s%2d%s) %-14.14s %s", grn, i, nrm,
	    genders[i],
            !(++columns % 3) ? "\r\n" : "");
    send_to_char(buf, d->character);
  }
  send_to_char("\r\nEnter sex number : ", d->character);
}


/*. Display attack types menu .*/
/*
void mAttackMenu(DescriptorData *d)
{
  attackTable(d);
}
*/
void mAttackMenu(DescriptorData *d)
{
  int counter, columns = 0;

  OLC_MODE(d) = MEDIT_ATTACK;

  get_char_colors(d->character);

  for (counter = 0; counter < NUM_WEAPON_TYPES; counter++) {
       sprintf(buf, "%s%2d%s) %-15.15s %s", grn, counter, nrm,
            attack_hit_text[counter].singular, !(++columns % 4) ? "\r\n" : "");
       send_to_char(buf, d->character);
  }
  send_to_char("\r\nEnter first type : ", d->character);
}

 

/*. Display mob-flags menu .*/
void mActionMenu(DescriptorData *d)
{
  int i, columns = 0;

  get_char_colors(d->character);
#if defined(CLEAR_SCREEN)
  send_to_char("^[[H^[[J", d->character);
#endif
  for (i = 0; i < NUM_MOB_FLAGS; i++) {
    sprintf(buf, "%s%2d%s) %-14.14s  %s", grn, i + 1, nrm, action_bits[i],
                !(++columns % 4) ? "\r\n" : "");
    send_to_char(buf, d->character);
  }
  sprintbitarray(MOB_FLAGS(d->olc->mob), action_bits, PR_ARRAY_MAX, buf1);
  sprintf(buf, "\r\nCurrent flags : %s%s%s\r\nEnter mob flags (0 to quit) : ",
                  cyn, buf1, nrm);
  send_to_char(buf, d->character);
}


/*. Display aff-flags menu .*/
void mAffectMenu(DescriptorData *d)
{
  int i, columns = 0;

  get_char_colors(d->character);
#if defined(CLEAR_SCREEN)
  send_to_char("^[[H^[[J", d->character);
#endif
  for (i = 0; i < NUM_AFF_FLAGS; i++) {
    sprintf(buf, "%s%2d%s) %-14.14s  %s", grn, i + 1, nrm, affected_bits[i],
                        !(++columns % 4) ? "\r\n" : "");
    send_to_char(buf, d->character);
  }
  sprintbitarray(AFF_FLAGS(d->olc->mob), affected_bits, AF_ARRAY_MAX, buf1);
  sprintf(buf, "\r\nCurrent flags   : %s%s%s\r\nEnter aff flags (0 to quit) : ",
                          cyn, buf1, nrm);
  send_to_char(buf, d->character);
}

  
/* Display class menu */
void mClassMenu(DescriptorData *d)
{ 
  int counter, columns = 0;
  
  get_char_colors(d->character);

#if defined(CLEAR_SCREEN)
  send_to_char("^[[H^[[J", d->character);
#endif
  
  for (counter = 0; counter < NUM_CLASSES; counter++) {
       sprintf(buf, "%s%2d%s) %-15.15s %s", grn, counter, nrm,
            pc_class_types[counter], !(++columns % 4) ? "\r\n" : "");
       send_to_char(buf, d->character);

  }
  send_to_char("\r\nEnter class number :", d->character);
}

void mRaceMenu(DescriptorData *d)
{
  int counter, columns = 0;

  get_char_colors(d->character);

#if defined(CLEAR_SCREEN)
  send_to_char("^[[H^[[J", d->character);
#endif

  for (counter = 0; counter < NUM_RACES; counter++) {
       sprintf(buf, "%s%2d%s) %-15.15s %s", grn, counter, nrm,
            race_types[counter], !(++columns % 4) ? "\r\n" : "");
       send_to_char(buf, d->character);

  }
  if (columns != 0) send_to_char("\r\n", d->character);

  send_to_char("Enter race number :", d->character);
}


/* Display subrace menu */
void mSubraceMenu(DescriptorData *d)
{
  /* TODO: these constants should be elsewhere. */
  const char *subraces[] = {
    "None", "Red Dragon", "Green Dragon", "White Dragon", "Black Dragon",
    "Blue Dragon", "Chromatic Dragon", "Gold Dragon", "Silver Dragon",
    "Bronze Dragon", "Copper Dragon", "Brass Dragon", "Shadow Dragon",
    "Mist Dragon",
  };
  int counter, columns = 0;

  for (counter = 0; counter < NUM_SUBRACES; counter++) {
       sprintf(buf, "%s%2d%s) %-15.15s %s", grn, counter, nrm,
            subraces[counter], !(++columns % 4) ? "\r\n" : "");
       send_to_char(buf, d->character);
  }
  send_to_char("Enter subrace :", d->character);
}


//-------------------------------------------------------------------------
//
void
mMenu( DescriptorData* d )
{
  CharData *mob;

  mob = OLC_MOB(d);
  GET_GOLD(mob) = 99999999; /* Force a GP recalc */
  calculateXpAndGp(mob);

  sendChar(
    d->character,
    "[H[J"
    "-- Mob Number:  [%d]\r\n"
    "&021)&06 Class : &03%s &022)&06 Race: &03%s &023)&06 Subrace: &03%d\r\n"
    "&024)&06 Alias :&00 %s\r\n"
    "&025)&06 S-Desc:&00 %s\r\n"
    "&026)&06 L-Desc:&00-\r\n%s"
    "&027)&06 D-Desc:&00-\r\n%s"
    "&028)&06 Level     :&03 %-3d         &029)&06 Alignment   :&03 %d\r\n"
    "&02A)&06 Hitroll   :&03 %-3d         &02B)&06 Armor Class :&03 %d\r\n"
    "&02C)&06 Hit Pnts  :&03%2dd%3d+%5d &02D)&06 Damage      :&03 %dd%d+%d\r\n"
	"&02N)&06 Skill Suc :&03 %-3d         &02O)&06 Spell Dam   :&03 %d\r\n"
	"&02*)&06 Exp Pnts  :&03 %-9d   &02*)&06 Gold        :&03 %d\r\n",

    OLC_NUM(d),
    pc_class_types[(int)GET_CLASS(mob)],
    race_types[(int)GET_RACE(mob)],
    GET_SUBRACE(mob),
    GET_ALIAS(mob),
    GET_SDESC(mob),
    GET_LDESC(mob),
    GET_DDESC(mob),
    GET_LEVEL(mob),
    GET_ALIGNMENT(mob),
    GET_HITROLL(mob),
    GET_AC(mob), 
    GET_HIT(mob),
    GET_MANA(mob),
    GET_MOVE(mob),
    GET_NDD(mob),
    GET_SDD(mob),
    GET_DAMROLL(mob),
    GET_MOB_SKILL_SUC(mob),
    GET_MOB_SPELL_DAM(mob),
    GET_EXP(mob),
    GET_GOLD(mob)
  );

  sprintbitarray(MOB_FLAGS(mob), action_bits, PR_ARRAY_MAX, buf1);
  sprintbitarray(AFF_FLAGS(mob), affected_bits, AF_ARRAY_MAX, buf2);

  sendChar(
    d->character,
    "&02G)&06 Sex       :&03 %s\r\n"
    "&02H)&06 Position  :&03 %s    &02I)&06 Default     :&03 %s\r\n"
    "&02J)&06 Attacks   :&03 %s %s %s\r\n"
    "&02K)&06 NPC Flags :&03 %s\r\n"
    "&02L)&06 AFF Flags :&03 %s\r\n"
    "&02M)&06 Script    :&03 %s\r\n"
    "&02Q)&06 Quit\r\n"
    "&00Enter choice : ",

    genders[(int)GET_SEX(mob)],
    position_types[(int)d->olc->mob->mob_specials.load_pos],
    position_types[(int)d->olc->mob->mob_specials.default_pos],
    attack_hit_text[mob->mob_specials.attack_type].singular,
    (mob->mob_specials.attack_type2 < 0 ? "None" :
          attack_hit_text[mob->mob_specials.attack_type2].singular),
    (mob->mob_specials.attack_type3 < 0 ? "None" :
          attack_hit_text[mob->mob_specials.attack_type3].singular),
    buf1, 
    buf2,
    mob->proto_script?"Set.":"Not Set."
  );

  OLC_MODE(d) = MEDIT_MAIN_MENU;
}


/**************************************************************************
  The GARGANTAUN event handler
 **************************************************************************/

void
mParse( DescriptorData * d, char *arg )
{ 
  CharData *ch = d->character;
  char chIn    = toupper( *arg );
  int number;

  switch (OLC_MODE(d)) 
  {
/*-------------------------------------------------------------------*/
  case MEDIT_CONFIRM_SAVESTRING:
    /*. Ensure mob has MOB_ISNPC set or things will go pair shaped .*/
    SET_BIT_AR(MOB_FLAGS(OLC_MOB(d)), MOB_ISNPC);
    switch( chIn )
    {
    case 'Y':
      /*. Save the mob in memory and to disk  .*/
      sendChar( ch, "Saving mobile to memory.\r\n" );
      mSaveInternally(d);
      mudlog( CMP, LVL_BUILDER, TRUE, "OLC: %s edits mob %d", GET_NAME(ch), OLC_NUM(d));
      cleanup_olc(d, CLEANUP_ALL);
      return;
    case 'N':
      cleanup_olc(d, CLEANUP_ALL);
      return;
    default:
      sendChar( ch, "Invalid choice!\r\n" );
      sendChar( ch, "Do you wish to save the mobile? : " );
      return;
    }
    break;

/*-------------------------------------------------------------------*/
  case MEDIT_MAIN_MENU:
    d->olc->tableDisp = TRUE;
    switch( chIn )
    {
      case 'Q':
        if (OLC_VAL(d)) /*. Anything been changed? .*/
        {
          sendChar( ch, "Do you wish to save the changes to the mobile? (y/n) : " );
          OLC_MODE(d) = MEDIT_CONFIRM_SAVESTRING;
        }
        else
          cleanup_olc(d, CLEANUP_ALL);
        return;
      case '1': OLC_MODE(d) = MEDIT_CLASS; mClassMenu(d);
	break; 
      case '2': OLC_MODE(d) = MEDIT_RACE; mRaceMenu(d);
	break;
      case '3': OLC_MODE(d) = MEDIT_SUBRACE; mSubraceMenu(d);
	break;
      case '4':
        OLC_MODE(d) = MEDIT_ALIAS;
	sendChar( ch, "Enter alias(s) for this mob: ");
	break;
      case '5':
        OLC_MODE(d) = MEDIT_S_DESC;
	sendChar( ch, "Enter mobs action description: ");
	break;
      case '6':
        OLC_MODE(d) = MEDIT_L_DESC;
	sendChar( ch, "Enter mobs room description: ");
	break;
      case '7':
        OLC_MODE(d) = MEDIT_D_DESC;
	sendChar( ch, "Enter mob description: (/s saves /h for help)\r\n" );
        SEND_RULER(d);
        OLC_VAL(d) = 1;
	d->backstr = NULL;
        if( OLC_MOB(d)->player.description )
        {
          sendChar( ch, "%s", OLC_MOB(d)->player.description );
          d->backstr = str_dup(OLC_MOB(d)->player.description);
        }
        d->str = &OLC_MOB(d)->player.description;
        d->max_str = MAX_MOB_DESC;
        d->mail_to = 0;
        break;
      case '8':
        OLC_MODE(d) = MEDIT_LEVEL;
	sendChar( ch, "Enter level: ");
	break;
      case '9':
        OLC_MODE(d) = MEDIT_ALIGNMENT;
	sendChar( ch, "Enter alignment: ");
	break;
      case 'A':
        OLC_MODE(d) = MEDIT_HITROLL;
	sendChar( ch, "Enter hitroll: ");
	break;
      case 'B':
        OLC_MODE(d) = MEDIT_AC;
	sendChar( ch, "Enter armor class: ");
	break;
      case 'C':
	OLC_MODE(d) = MEDIT_NUM_HP_DICE;
	sendChar( ch, "Enter number hps dice(1,100): ");
	break;
      case 'D':
        OLC_MODE(d) = MEDIT_NDD;
	sendChar( ch, "Enter number damage dice: ");
	break;
      case 'G': OLC_MODE(d) = MEDIT_SEX; mSexMenu(d);
	break;
      case 'H': OLC_MODE(d) = MEDIT_POS; mLoadPosMenu(d);
	break;
      case 'I': OLC_MODE(d) = MEDIT_DEFAULT_POS; mDefaultPosMenu(d);
	break;
      case 'J': OLC_MODE(d) = MEDIT_ATTACK; mAttackMenu(d);
	sendChar( ch, "Enter primary attack: ");
	break;
      case 'K': OLC_MODE(d) = MEDIT_NPC_FLAGS; mActionMenu(d);
	break;
      case 'L': OLC_MODE(d) = MEDIT_AFF_FLAGS; mAffectMenu(d);
	break;
      case 'M':
        OLC_SCRIPT_EDIT_MODE(d) = SCRIPT_MAIN_MENU;
        dg_script_menu(d);
        break;
	  case 'N':
		  OLC_MODE(d) = MEDIT_SKILL_SUC;
		  sendChar( ch, "Enter chance at skill success: ");
		  break;
	  case 'O':
		  OLC_MODE(d) = MEDIT_SPELL_DAM;
		  sendChar( ch, "Enter spell damage multiplier (percent): ");
		  break;
	  default:
        mMenu(d);
	return;
    }
    d->olc->tableDisp = FALSE;
    return;

  case OLC_SCRIPT_EDIT:
    if (dg_script_edit_parse(d, arg)) return;
    break;

/*-------------------------------------------------------------------*/
  case MEDIT_ALIAS:
    if( GET_ALIAS(OLC_MOB(d) ))
      free(GET_ALIAS(OLC_MOB(d)));
    GET_ALIAS(OLC_MOB(d)) = str_dup(arg); 
    break;
/*-------------------------------------------------------------------*/
  case MEDIT_S_DESC:
    if( GET_SDESC(OLC_MOB(d) ))
      free(GET_SDESC(OLC_MOB(d)));
    GET_SDESC(OLC_MOB(d)) = str_dup(arg); 
    break;
/*-------------------------------------------------------------------*/
  case MEDIT_L_DESC:
    if(GET_LDESC(OLC_MOB(d)))
      free(GET_LDESC(OLC_MOB(d)));
    strcpy(buf, arg);
    strcat(buf, "\r\n");
    GET_LDESC(OLC_MOB(d)) = str_dup(buf); 
    break;
/*-------------------------------------------------------------------*/
  case MEDIT_D_DESC:
    /*. We should never get here .*/
    cleanup_olc(d, CLEANUP_ALL);
    mudlog(BRF, LVL_BUILDER, TRUE, "OLCERR: OLC: mParse(): Reached D_DESC case!");
    break;
/*-------------------------------------------------------------------*/
  case MEDIT_NPC_FLAGS:
    if ((number = atoi(arg)) == 0)
      break;
    else if (!((number < 0) || (number > NUM_MOB_FLAGS)))
      TOGGLE_BIT_AR(MOB_FLAGS(OLC_MOB(d)), (number - 1));
    mActionMenu(d);
    return;

/*-------------------------------------------------------------------*/
  case MEDIT_AFF_FLAGS:
    if ((number = atoi(arg)) == 0)
      break;
    else if (!((number < 0) || (number > NUM_AFF_FLAGS)))
      TOGGLE_BIT_AR(AFF_FLAGS(OLC_MOB(d)), (number - 1));
    mAffectMenu(d);
    return;

/*-------------------------------------------------------------------*/
/*. Numerical responses .*/
  case MEDIT_CLASS: //Digger
    number = atoi(arg);
    if( number >= 0 && number < NUM_CLASSES ){
      d->olc->mob->player.class = number;
    }
    break;
  case MEDIT_RACE:
    number = atoi(arg);
    if( number >= 0 ){
      d->olc->mob->player.race = number;
    }
    break;
  case MEDIT_SUBRACE:
    number = atoi(arg);
    if (number >= 0 && number <= NUM_SUBRACES) {
      d->olc->mob->mob_specials.sub_race = number;
    }
    break;
  case MEDIT_SEX:
    number = atoi(arg);
    if( number >= 0 ){
      d->olc->mob->player.sex = number;
    }
    break;
  case MEDIT_HITROLL:
    if (!is_integer(arg))
    {
      sendChar( ch, "%s isn't a valid hitroll.\r\nHitroll: ", arg);
      return;
    }
    GET_HITROLL(OLC_MOB(d)) = MAX(0, MIN(50, atoi(arg)));
    break;
  case MEDIT_NDD:
    if( !is_integer(arg) )
    {
      sendChar( ch, "%s isn't a valid number of dice.\r\nNumber dam dice: ", arg);
      return;
    }
    GET_NDD(OLC_MOB(d)) = MAX(0, MIN(120, atoi(arg)));
    d->olc->mode = MEDIT_SDD;
    sendChar( ch, "Enter damage dice size: ");
    return;
  case MEDIT_SDD:
    if( !is_integer(arg))
    {
      sendChar( ch, "%s isn't a valid dice size.\r\nDam dice size: ", arg);
      return;
    }
    GET_SDD(OLC_MOB(d)) = MAX(0, MIN(120, atoi(arg)));
    d->olc->mode = MEDIT_DAMROLL;
    sendChar( ch, "Damroll: ");
    return;
  case MEDIT_DAMROLL:
    if (!is_integer(arg)) {
	sendChar( ch, "%s isn't a valid damroll.\r\n", arg);
	sendChar( ch, "Damroll: ");
	return;
    }
    SET_DAMROLL(OLC_MOB(d), MAX(0, MIN(100, atoi(arg))));
    break;
  case MEDIT_NUM_HP_DICE:
    if (!is_integer(arg)) {
	sendChar( ch, "%s isn't a valid number of dice.\r\n", arg);
	sendChar( ch, "Number Hps dice(1, 100): ");
	return;
    }
    GET_HIT(OLC_MOB(d)) = MAX(1, MIN(100, atoi(arg)));
    d->olc->mode = MEDIT_SIZE_HP_DICE;
    sendChar( ch, "Hps dice size(1, 30000): ");
    return; /* now get the dice size. */
  case MEDIT_SIZE_HP_DICE:
    if (!is_integer(arg)) {
	sendChar( ch, "%s isn't a valid dice size.\r\n", arg);
	sendChar( ch, "Hps dice size(1, 30000): ");
	return;
    }
    GET_MANA(OLC_MOB(d)) = MAX(1, MIN(30000, atoi(arg)));
    d->olc->mode = MEDIT_ADD_HP;
    sendChar( ch, "Hps add(0, 30000): ");
    return; /* now get the hp add on. */
  case MEDIT_ADD_HP:
    if (!is_integer(arg)) {
	sendChar( ch, "%s isn't a valid hp add.\r\n", arg);
	sendChar( ch, "Hps add(0, 30000): ");
	return;
    }
    GET_MOVE(OLC_MOB(d)) = MAX(0, MIN(30000, atoi(arg)));
    if ( ((GET_MANA(OLC_MOB(d)) * GET_HIT(OLC_MOB(d))) + GET_MOVE(OLC_MOB(d))) > 32000) {
	GET_MANA(OLC_MOB(d)) = GET_HIT(OLC_MOB(d)) = GET_MOVE(OLC_MOB(d)) = 1;
	sendChar( ch, "The mobs hps could exceed the maximum of 32000, try again.\r\n");
	d->olc->mode = MEDIT_NUM_HP_DICE;
	sendChar( ch, "Number hps dice(1, 100): ");
	return; /* They have to re-enter the lot. */
    }
    break; /* done with hps now. */
  case MEDIT_AC:
    if (!is_integer(arg)) {
	sendChar( ch, "%s isn't a valid armor class.\r\n", arg);
	sendChar( ch, "Armor Class: ");
	return;
    }
    GET_AC(OLC_MOB(d)) = MAX(-2000, MIN(100, atoi(arg)));
    break;
  case MEDIT_EXP:
    if (!is_integer(arg)) {
	sendChar( ch, "%s isn't a valid xps value.\r\n", arg);
	sendChar( ch, "Xps: ");
	return;
    }
    GET_EXP(OLC_MOB(d)) = MAX(0, MIN(5000000, atol(arg)));
    break;
  case MEDIT_GOLD:
    if (!is_integer(arg)) {
	sendChar( ch, "%s isn't a valid gold value.\r\n", arg);
	sendChar( ch, "Gold: ");
	return;
    }
    GET_GOLD(OLC_MOB(d)) = MAX(0, MIN(500000, atol(arg)));
    break;
  case MEDIT_POS:
    number = atoi(arg);
    if (number < 0) {
	mLoadPosMenu(d);
	return;
    }
    d->olc->mob->mob_specials.load_pos = number;
    d->olc->mob->char_specials.position = number;
    break;
  case MEDIT_DEFAULT_POS:
    number = atoi(arg);
    if (number < 0) {
	mDefaultPosMenu(d);
	return;
    }
    d->olc->mob->mob_specials.default_pos = number;
    break;
  case MEDIT_ATTACK:
    number = atoi(arg);
    if (number < 0) {
	mAttackMenu(d);
	return;
    }
    d->olc->mob->mob_specials.attack_type = number;
    d->olc->mode = MEDIT_ATTACK2;
    sendChar( ch, "Enter secondary attack(-1 for none): ");
    return;
  case MEDIT_ATTACK2:
    if (is_integer(arg) && (atol(arg) < 0)) {
	d->olc->mob->mob_specials.attack_type2 = d->olc->mob->mob_specials.attack_type3 = -1;
	break; /* return to main menu */
    }
    number = atoi(arg);
    if (number < 0) {
	mAttackMenu(d);
	return;
    }
    d->olc->mob->mob_specials.attack_type2 = number;
    d->olc->mode = MEDIT_ATTACK3;
    sendChar( ch, "Enter tertiary attack(-1 for none): ");
    return;
  case MEDIT_ATTACK3:
    if (is_integer(arg) && (atol(arg) < 0)) {
	d->olc->mob->mob_specials.attack_type3 = -1;
	break; /* return to main menu */
    }
    number = atoi(arg);
    if (number < 0) {
	mAttackMenu(d);
	return;
    }
    d->olc->mob->mob_specials.attack_type3 = number;
    break;
  case MEDIT_LEVEL:
    if (!is_integer(arg)) {
	sendChar( ch, "%s is not a level.\r\n", arg);
	sendChar( ch, "Level: ");
    }
    GET_LEVEL(OLC_MOB(d)) = MAX(1, MIN(LVL_IMMORT, atoi(arg)));
    break;
  case MEDIT_ALIGNMENT:
    if (!is_integer(arg)) {
	sendChar( ch, "%s is not an alignment.\r\n", arg);
	sendChar( ch, "Alignment: ");
    }
    GET_ALIGNMENT(OLC_MOB(d)) = MAX(-1000, MIN(1000, atoi(arg)));
    break;
  case MEDIT_SKILL_SUC:
	  if (!is_integer(arg)) {
		  sendChar( ch, "%s is not a numerical value.\r\n", arg);
		  sendChar( ch, "Enter skill success: ");
	  }
	  GET_MOB_SKILL_SUC(OLC_MOB(d)) = MAX(0, MIN(200, atoi(arg)));
	  break;
  case MEDIT_SPELL_DAM:
	  if (!is_integer(arg)) {
		  sendChar( ch, "%s is not a numerical value.\r\n", arg);
		  sendChar( ch, "Enter spell damage: ");
	  }
	  GET_MOB_SPELL_DAM(OLC_MOB(d)) = MAX(0, MIN(1000, atoi(arg)));
	  break;
/*-------------------------------------------------------------------*/
  default:
    /*. We should never get here .*/
    cleanup_olc(d, CLEANUP_ALL);
    mudlog(BRF, LVL_BUILDER, TRUE, "OLCERR: OLC: mParse(): Reached default case!");
    break;
  }
/*-------------------------------------------------------------------*/
/*. END OF CASE 
    If we get here, we have probably changed something, and now want to
    return to main menu.  Use OLC_VAL as a 'has changed' flag .*/

  OLC_VAL(d) = 1;
  mMenu(d);
}
/*. End of mParse() .*/

#define EXP_SCALING_FACTOR 11
#define DAM_FACTOR 30
#define HIT_FACTOR 4
#define LEVEL_FACTOR 20
#define RAND_FACTOR 10
#define GOLD_SCALING_FACTOR 15
#define NEWBIE_XP_LEVEL 1500
/* 
** Vex April 1997
** The factors:
** DAM_FACTOR reduces the relative affect of the mobs damroll the higher the
** DAM_FACTOR is.
** HIT_FACTOR reduces the relative effect of the mobs hitroll the higher the
** HIT_FACTOR is.
** LEVEL_FACTOR reduces the relative effect of the mobs level the higher the
** LEVEL_FACTOR is.
** RAND_FACTOR just determines how random the result is. Don't set it to 0!
** EXP_SCALING_FACTOR is just used to bring the result to a sensible level.
*/
#ifdef USE_OLD_FORMULA
void
calculateXpAndGp(CharData *mob)
{
  int XP_damroll   = GET_DAMROLL(mob);
  int XP_dice_num  = mob->mob_specials.damnodice;
  int XP_dice_sz   = mob->mob_specials.damsizedice;
  int XP_level     = GET_LEVEL(mob);
  int XP_max_hits  = (mob->points.hit * mob->points.mana ) + mob->points.move;
  int XP_sanc_mult = ( AFF_FLAGS(mob) & AFF_SANCTUARY ? 2 : 1 );
  int XP_avgdam    = ((( XP_dice_num * ( XP_dice_sz + 1 )) / 2 ) + XP_damroll);
  int GP_original  = GET_GOLD(mob);
  int XP_THAC      = 30 - mob->points.hitroll; /* roll to hit ac -10 */
  int GP_accept    = (XP_max_hits * XP_sanc_mult * XP_level) +
                        number(1, XP_max_hits);
  int XP_offense   = 10; /* Simulates things like spells etc. */
  int theXP;

  switch (GET_CLASS(mob))
  {
    case CLASS_WARRIOR        : XP_offense += 5; break;
    case CLASS_SOLAMNIC_KNIGHT: XP_offense += 4; break;
    case CLASS_DEATH_KNIGHT   : XP_offense += 10; break;
    case CLASS_THIEF          : XP_offense += 1; break;
    case CLASS_CLERIC         : XP_offense += 8; break;
    case CLASS_MAGIC_USER     : XP_offense += 15; break;
    case CLASS_ASSASSIN       : XP_offense += 2; break;
    case CLASS_SHADOW_DANCER  : XP_offense += 5; break;
    case CLASS_SHOU_LIN       : XP_offense += 8; break;
    case CLASS_RANGER         : XP_offense += 3; break;
    case CLASS_NECROMANCER    : XP_offense += 2; break;
    default : mlog("UNKNOWN CLASS in function calculateExp - medit.c)");
  }

  if (XP_THAC < 1)
    XP_THAC = 1;
  else if (XP_THAC >= 20)
    XP_THAC = 19; 

  GET_LEVEL(mob) = (GET_LEVEL(mob) > LVL_HERO ? LVL_HERO : GET_LEVEL(mob));

  theXP = (((((XP_avgdam + DAM_FACTOR)/10) * (20 - XP_THAC + HIT_FACTOR) + XP_offense)
          * XP_sanc_mult * ((XP_level + LEVEL_FACTOR)/10) * XP_max_hits)/EXP_SCALING_FACTOR);

  GET_EXP(mob) = theXP;
  if (MOB_FLAGGED(mob, MOB_AGGRESSIVE) && !MOB_FLAGGED(mob, MOB_WIMPY))
    GET_EXP(mob) += GET_EXP(mob) >> 3; /* plus 12.5% */
  /*
  ** Do the gold next.
  */
  if (GET_EXP(mob) < NEWBIE_XP_LEVEL) /* Make a bit of extra gold availble */
    GP_accept += GET_EXP(mob);

  GET_GOLD(mob) = ( GP_original < ( GP_accept / GOLD_SCALING_FACTOR) ?
                    GP_original : (GP_accept / GOLD_SCALING_FACTOR));

  if( GET_GOLD(mob) > 100 )
    GET_GOLD(mob) += number( 1, GET_GOLD(mob)/10 );

}
#endif

/* ============================================================================ 
    Find the average damage /round this mob will do as a result of its raw
    fighting ability to a victim with the "nominal_ac".
============================================================================ */
int avgDamPerRound(CharData *mob, int nominal_ac)
{
    int num_attacks, avg_dam, hr_factor;

    if (!IS_NPC(mob)) {
	mudlog(NRM, LVL_IMMORT, TRUE, "OLCERR: Player passed to avgDamPerRound!");
	return 0;
    }

    num_attacks = calculate_attack_count(mob);
    avg_dam = mob->mob_specials.damnodice * (mob->mob_specials.damsizedice + 1)/2 + GET_DAMROLL(mob);
    hr_factor = MAX(5, MIN( 145, 100 - ((thacoCalc(mob) - nominal_ac) * 5)  ));
    return ( num_attacks * avg_dam * hr_factor );
}

/* ============================================================================ 
Approximate the offensive spell ability of this mob.
============================================================================ */
//#define SPELL_DAM_REDUCTION 4
#define SPELL_DAM_REDUCTION 2
int spellAttackStrength(CharData *mob)
{
    int level, power = 0;
    level = GET_LEVEL(mob);

    switch (GET_CLASS(mob)) {
    case CLASS_MAGIC_USER:
	if (level >= 50)
		power = 261 + (level * 6);	/* db */
	else if (level >= 45)
		power = 171 + (level * 5);	/* fb */
	else if (level >= 35)
		power = 191 + (level * 3); /* lb */
	else if (level >= 30)
		power = 141 + (level * 2); /* shriek */
	else if (level >= 20)
		power = 121 + level; /* color spray */
	else if (level >= 10)
		power = 42 + level; /* burning hands */
	else
		power = 20 + level; /* magic missile */
	break;
    case CLASS_CLERIC:
	if (level >= 43)
		power = 300; /* holy/unholy word */
	if (level >= 25 )
		power = 212; /* harm */
	else
	        power = 20; /* cause light */
	break;
    case CLASS_SOLAMNIC_KNIGHT:
	if (level >= 45)
		power = 300; /* holy word */
	break;
    case CLASS_SHADOW_DANCER:
	if (level >= 12)
		power = 50 + (3 * (level + 1)/2) + (3 * level); /* shadow blades */
	break;
    case CLASS_DEATH_KNIGHT:
	if (level >= 45)
		power = 191 + (3 * level); /* death touch */
	else if (level >= 35)
		power = 121 + (2 * level);	/* black breath */
	else if (level >= 25)
		power = 60; /* cause critic */
	else if (level >= 10)
		power = 20 + level; 	/* black dart */
	else if (level >= 2)
		power = 20;	/* cause light */
	break;
    default:
    break;
    }
    power = power/SPELL_DAM_REDUCTION;
    if (!MOB_FLAGGED(mob, MOB_NOBASH))
        power = power/SPELL_DAM_REDUCTION;
    power = (int)(power * MIN(200, GET_MOB_SPELL_DAM(mob))) / 100;
    return (power * MIN(95, GET_MOB_SKILL_SUC(mob))); /* chance of cast succeeding, max 95% */
}

#define NOMINAL_AC -7
#define NOMINAL_THAC0 10 /* 50-50 */
#define CONST_SCALE 60
#define TOTAL_SCALE 4
#define AC_SCALE 10
/* Attempt at a new xps formula. */
void
calculateXpAndGp(CharData *mob)
{
    int rawXp;			/* the raw xps total */
    int theXp;			/* the final xps */
    int GP_original;		/* the original gold total */
    int GP_accept;		/* the acceptable total */
    int raw_damage;		/* average dam mob does/round */
    int avg_hps;		/* average hps of this mob */
    int ac_factor;		/* mobs armor class */
    int raw_toughness;		/* account for amount of damage it takes to kill this mob */
    int mob_behaviour;		/* behaviour of this mob */
#if 0 /* lets not get too fancy just yet */
    int skill_defense;		/* defensive skill factor */
    int special;		/* unusual special abilities */
#endif

    GP_original = GET_GOLD(mob);
    raw_damage = avgDamPerRound(mob, NOMINAL_AC);
    raw_damage += spellAttackStrength(mob);

    /* How much punishment can this beast take? */
    avg_hps = (mob->points.hit * (mob->points.mana + 1)/2) + mob->points.move;

    ac_factor = GET_AC(mob);
    if (ac_factor > 100)
		ac_factor = 100;
	else if (ac_factor < -100)
		ac_factor = -100;
	ac_factor = (AC_SCALE * 100) - ac_factor;	// lower ac, higher ac factor.
	if (GET_AC(mob) < - 100) 
	{	// take account of damage reduction.
		ac_factor += (100 - GET_AC(mob))/10;	// note: ac is NEGATIVE.
	}


    raw_toughness = (avg_hps * ac_factor)/AC_SCALE;
    if (IS_AFFECTED(mob, AFF_SHIELD))
		raw_toughness += raw_toughness/10;
	if (IS_AFFECTED(mob, AFF_SANCTUARY))
		raw_toughness += raw_toughness;
	else if (IS_AFFECTED(mob, AFF_WARD))
		raw_toughness += raw_toughness/3;
	else if (IS_AFFECTED(mob, AFF_PROTECT_GOOD) || IS_AFFECTED(mob, AFF_PROTECT_EVIL))
		raw_toughness += raw_toughness/6;
	
    /* Adjust by percentage according to behaviour patterns. */
    mob_behaviour = 100; /* normal */
	if (MOB_FLAGGED(mob, MOB_AWARE))
		mob_behaviour += 5;
	if (MOB_FLAGGED(mob, MOB_AGGRESSIVE))
		mob_behaviour += 40;
	else
	{
		if (MOB_FLAGGED(mob, MOB_AGGR_EVIL))
			mob_behaviour += 10;
		if (MOB_FLAGGED(mob, MOB_AGGR_GOOD))
			mob_behaviour += 10;
		if (MOB_FLAGGED(mob, MOB_AGGR_NEUTRAL))
			mob_behaviour  += 10;
		if (MOB_FLAGGED(mob, MOB_HELPER))
			mob_behaviour += 10;
		else if (MOB_FLAGGED(mob, MOB_MEMORY))
			mob_behaviour += 10;
	}
	if (MOB_FLAGGED(mob, MOB_WIMPY))
		mob_behaviour += 10;
	// Give mobs bonuses and penalties for their ability to make life hard for players.
	if (!IS_AFFECTED(mob, AFF_INFRAVISION))
		mob_behaviour -= 5;
	if (!IS_AFFECTED(mob, AFF_SENSE_LIFE))
		mob_behaviour -= 5;
	if (MOB_FLAGGED(mob, MOB_NOBLIND))
		mob_behaviour += 5;

    rawXp = (raw_damage * mob_behaviour)/100 * MAX(1, rsqrt(raw_toughness))/100 + ((raw_damage + raw_toughness)/CONST_SCALE);
    theXp = rawXp * TOTAL_SCALE;

    GET_EXP(mob) = theXp;
    /* Same as before for gold.*/
    GP_accept   = raw_toughness/100 * GET_LEVEL(mob);
    if (GET_EXP(mob) < NEWBIE_XP_LEVEL) /* Make a bit of extra gold availble */
        GP_accept += GET_EXP(mob);
    GET_GOLD(mob) = ( GP_original < ( GP_accept / GOLD_SCALING_FACTOR) ?
                      GP_original : (GP_accept / GOLD_SCALING_FACTOR));
    if (mob_index[mob->nr].func == shop_keeper)
	GET_GOLD(mob) = MIN(GP_original, 500000);

    if( GET_GOLD(mob) > 100 )
        GET_GOLD(mob) += number( 1, GET_GOLD(mob)/10 );

    // Animals, plants, elementals give +10% bonus exp, but no gold
    if(GET_RACE(mob) == RACE_ANIMAL ||
            GET_RACE(mob) == RACE_PLANT ||
            IS_ELEMENTAL(mob)) {
        GET_GOLD(mob) = 0;
        GET_EXP(mob) += GET_EXP(mob) / 10;
    }

    // Dragons give a significant bonus to gold, at the cost of exp.
    if(IS_DRAGON(mob)) {
        GET_GOLD(mob) += GET_GOLD(mob) / 2;
        GET_EXP(mob) -= GET_EXP(mob)/3;        
    }
}



void medit_string_cleanup(struct descriptor_data *d, int terminator)
{
  switch (OLC_MODE(d)) {

  case MEDIT_D_DESC:
  default:
     mMenu(d);
     break;
  }
}

/* I'm removing the two functions completely because they're not use anywhwere in
 * the source. -Xiuh
 

// redo all mob proto-type spell dam and skill suc
ACMD(do_resetSucAndDam)
{
    int i;
    for (i = 0; i < top_of_mobt; i++)
	{
        calculateSucAndDam(ch, mob_proto+i, i);
	}
}

void
calculateSucAndDam( CharData *ch, CharData *mob, int rmob_num )
{
	DescriptorData *d;
	int level = GET_LEVEL(mob);

	// Set the level of mob skill success and spell damage to an arbitrary, but hopefully fair, level.

	if(level < 20)
	{
		GET_MOB_SPELL_DAM(mob) = 40;
		GET_MOB_SKILL_SUC(mob) = 35;
	}
	else if(level < 30)
	{
		GET_MOB_SPELL_DAM(mob) = 40;
		GET_MOB_SKILL_SUC(mob) = 40;
	}
	else if(level < 40)
	{
		GET_MOB_SPELL_DAM(mob) = 60;
		GET_MOB_SKILL_SUC(mob) = 50;
	}
	else if(level < 46)
	{
		GET_MOB_SPELL_DAM(mob) = 75;
		GET_MOB_SKILL_SUC(mob) = 65;
	}
	else if(level < 50)
	{
		GET_MOB_SPELL_DAM(mob) = 80;
		GET_MOB_SKILL_SUC(mob) = 70;
	}
	else if(level < LVL_IMMORT)
	{
		GET_MOB_SPELL_DAM(mob) = 90;
		GET_MOB_SKILL_SUC(mob) = 80;
	}
	else if(level >= LVL_IMMORT)
	{
		GET_MOB_SPELL_DAM(mob) = 100;
		GET_MOB_SKILL_SUC(mob) = 90;
	}

}

 */
