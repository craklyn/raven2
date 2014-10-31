/* ************************************************************************
*   File: fight.c                                       Part of CircleMUD *
*  Usage: Combat system                                                   *
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
#include "general/comm.h"
#include "general/handler.h"
#include "actions/interpreter.h"
#include "general/class.h"
#include "magic/spells.h"
#include "magic/skills.h"
#include "general/color.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */
#include "actions/fight.h"
#include "util/weather.h"
#include "specials/combspec.h"
#include "magic/stun.h"
#include "magic/aggressive.h"
#include "actions/offensive.h"
#include "magic/rescue.h"
#include "specials/special.h"
#include "general/objsave.h"
#include "magic/backstab.h"
#include "actions/outlaw.h"
#include "actions/quest.h"
#include "specials/bloodbowl.h"
#include "specials/flag_game.h"
#include "scripts/dg_scripts.h"
#include "magic/missile.h"
#include "magic/sing.h"
#include "magic/magic.h"
#include "util/utils.h"
#include "actions/act.clan.h"
#include "specials/muckle.h"
#include "general/chores.h"       /* for function chore_check_kill */
#include "specials/mobact.h"
#include "specials/shop.h"
#include "actions/holidays.h"


/*
** c == attack count
** t == attack type
** only return true if you back or circle on the first round
*/
#define IMPROVED_SHADOW_DANCE(ch) (IS_PRESTIDIGITATOR(ch) && GET_ADVANCE_LEVEL(ch) >= FIRST_ADVANCE_SKILL \
         && percentSuccess(66) && skillSuccess(ch, SKILL_DANCE_DEATH)&& skillSuccess(ch, SKILL_DANCE_DEATH))

#define BACKSTAB(h,c,t) (c == 0 || \
      (c == 1 && \
        ((skillSuccess(h, SKILL_DANCE_DEATH) && t == SKILL_BACKSTAB) || (t == SKILL_SHADOW_DANCE && IMPROVED_SHADOW_DANCE(h))))) \
      && (t == SKILL_BACKSTAB || t == SKILL_CIRCLE || t == SKILL_SHADOW_DANCE)

#define IS_BACKSTAB(c) (c == SKILL_BACKSTAB || c == SKILL_CIRCLE \
                     || c == SKILL_SHADOW_DANCE)

#define LEARN_RATE 2

#define BACKMULT(c)   (backstab_multiplier((c)))
#define CIRCMULT(c)   ((backstab_multiplier((c)) - 1))
/*
** Fight Message Storage
*/
MessageList fight_messages[MAX_MESSAGES];

/* Structures */
CharData *combat_list = NULL;	/* head of l-list of fighting chars */
CharData *next_combat_list = NULL;

/* Forward Declarations */
int bonusPetDamage(CharData *ch, CharData *victim, int dam, int attack_type);
static int choose_card(CharData *ch, CharData *victim);
/* local file scope utility functions */
static void free_messages_type(struct msg_type *msg);

#define IS_WEAPON(type) ( ( (((type) >= TYPE_HIT) && ((type) < TYPE_SPECIAL)) || ((type) == SKILL_RIPOSTE)))


/*                                Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  XX  XX  XX  XX  XX  XX  XX  XX  XX */
static int max_dodge_lvls[] =   { 00, 00, 74, 00, 70, 70, 75, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
#define SKILL_DODGE_LEARN         max_dodge_lvls[ (int)GET_CLASS(victim) ]

/*                                Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  XX  XX  XX  XX  XX  XX  XX  XX  XX */
static int max_parry_lvls[] =   { 00, 00, 70, 90, 80, 70, 00, 80, 80, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
#define SKILL_PARRY_LEARN         max_parry_lvls[ (int)GET_CLASS(victim) ]

/*                                Mu  Cl  Th  Wa  Ra  As  Sl  Kn  Dk  Sd  Nm  XX  XX  XX  XX  XX  XX  XX  XX  XX */
static int max_riposte_lvls[] = { 00, 00, 00, 85, 75, 00, 00, 75, 75, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
#define SKILL_RIPOSTE_LEARN       max_riposte_lvls[ (int)GET_CLASS(victim) ]
/* The Fight related routines */

void
appear(CharData * ch)
{
  act("$n slowly fades into existence.", FALSE, ch, 0, 0, TO_ROOM);
  act("You slowly fade into existence.", FALSE, ch, 0, 0, TO_CHAR);

  if( affected_by_spell(ch, SPELL_INVISIBLE) )
    affect_from_char(ch, SPELL_INVISIBLE);

  REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_INVISIBLE);
  if (!affected_by_spell(ch, SPELL_DANCE_SHADOWS))
    REMOVE_BIT_AR(AFF_FLAGS(ch), AFF_HIDE);
}

static void free_messages_type(struct msg_type *msg)
{
  if (msg->attacker_msg)	free(msg->attacker_msg);
  if (msg->victim_msg)		free(msg->victim_msg);
  if (msg->room_msg)		free(msg->room_msg);
}

void free_messages(void)
{
  int i;

  for (i = 0; i < MAX_MESSAGES; i++)
    while (fight_messages[i].msg) {
      struct message_type *former = fight_messages[i].msg;

      free_messages_type(&former->die_msg);
      free_messages_type(&former->miss_msg);
      free_messages_type(&former->hit_msg);
      free_messages_type(&former->god_msg);

      fight_messages[i].msg = fight_messages[i].msg->next;
      free(former);
    }
}

void
load_messages(void)
{
  FILE *fl;
  int i, type;
  struct message_type *messages;
  char chk[128];

  if (!(fl = fopen(MESS_FILE, "r")))
  {
    sprintf(buf2, "Error reading combat message file %s", MESS_FILE);
    perror(buf2);
    exit(1);
  }

  for (i = 0; i < MAX_MESSAGES; i++)
  {
    fight_messages[i].a_type = 0;
    fight_messages[i].number_of_attacks = 0;
    fight_messages[i].msg = 0;
  }

  fgets(chk, 128, fl);
  while (!feof(fl) && (*chk == '\n' || *chk == '*'))
    fgets(chk, 128, fl);

  while( *chk == 'M' )
  {
    fgets(chk, 128, fl);
    sscanf(chk, " %d\n", &type);
    for( i = 0; (i < MAX_MESSAGES) && (fight_messages[i].a_type != type) &&
	 (fight_messages[i].a_type); i++);
    if (i >= MAX_MESSAGES)
    {
      fprintf(stderr, "Too many combat messages.  Increase MAX_MESSAGES and recompile.");
      exit(1);
    }
    CREATE(messages, struct message_type, 1);
    fight_messages[i].number_of_attacks++;
    fight_messages[i].a_type = type;
    messages->next = fight_messages[i].msg;
    fight_messages[i].msg = messages;

    messages->die_msg.attacker_msg = fread_action(fl, i);
    messages->die_msg.victim_msg = fread_action(fl, i);
    messages->die_msg.room_msg = fread_action(fl, i);
    messages->miss_msg.attacker_msg = fread_action(fl, i);
    messages->miss_msg.victim_msg = fread_action(fl, i);
    messages->miss_msg.room_msg = fread_action(fl, i);
    messages->hit_msg.attacker_msg = fread_action(fl, i);
    messages->hit_msg.victim_msg = fread_action(fl, i);
    messages->hit_msg.room_msg = fread_action(fl, i);
    messages->god_msg.attacker_msg = fread_action(fl, i);
    messages->god_msg.victim_msg = fread_action(fl, i);
    messages->god_msg.room_msg = fread_action(fl, i);
    fgets(chk, 128, fl);
    while (!feof(fl) && (*chk == '\n' || *chk == '*'))
      fgets(chk, 128, fl);
  }

  fclose(fl);
}

void update_pos(CharData * victim)
{
  if ((GET_HIT(victim) > 0) && (GET_POS(victim) > POS_STUNNED))
    return;
  else if (GET_HIT(victim) > 0 || (affected_by_spell(victim, SKILL_INIQUITY) && -GET_HIT(victim) < (GET_MAX_HIT(victim)/4)))
      GET_POS(victim) = POS_STANDING;
  else if (GET_HIT(victim) <= -10)
    GET_POS(victim) = POS_DEAD;
  else if (GET_HIT(victim) <= -6)
    GET_POS(victim) = POS_MORTALLYW;
  else if (GET_HIT(victim) <= -3)
    GET_POS(victim) = POS_INCAP;
  else
    GET_POS(victim) = POS_STUNNED;
}

void set_fighting( CharData *ch, CharData *vict )
{
    if( ch == vict ) return;

    if( !FIGHTING(vict) && FIGHTING(ch)) set_fighting(vict, ch);

    if( FIGHTING(ch) ) return;

    ch->next_fighting = combat_list;
    combat_list = ch;

    if (IS_AFFECTED(ch, AFF_SLEEP)) {
        affect_from_char(ch, SPELL_SLEEP);
        affect_from_char(ch, SPELL_DANCE_DREAMS);
    }

    FIGHTING(ch) = vict;
    /* Change position to fighting unless they've just been bashed, tripped, etc. */
    if( !(STUNNED(ch) && (GET_POS(ch) == POS_SITTING)) )
        GET_POS(ch) = POS_FIGHTING;
}

/*
** Remove ch from the list of characters in the fray.
*/
void stop_fighting( CharData * ch )
{
    CharData *tmp;

    if( !FIGHTING(ch) ) return;

    if( ch == next_combat_list )
        next_combat_list = ch->next_fighting;

    if( combat_list == ch )
        combat_list = ch->next_fighting;
    else {
        for( tmp = combat_list; tmp && (tmp->next_fighting != ch );
        tmp = tmp->next_fighting);
        if (!tmp) {
            mlog("SYSERR: Char fighting not found Error (fight.c, stop_fighting)");
            return;
        }
        tmp->next_fighting = ch->next_fighting;
    }

    /* Liam Feb 15, 1995
    ** Added for mob actions
    */
    if( IS_NPC(ch) ){
        ch->mob_specials.max_attack_damage  = 0;
        ch->mob_specials.high_attacker      = NULL;
        ch->mob_specials.max_spell_damage   = 0;
        ch->mob_specials.high_spellcaster   = NULL;
    }

    ch->next_fighting = NULL;
    FIGHTING(ch)      = NULL;
    if (GET_POS(ch) == POS_FIGHTING)
    	GET_POS(ch) = POS_STANDING;
    update_pos(ch);
}


/* completely and utterly end a fight */
void end_fight(CharData *ch)
{   
    CharData *k, *temp;

    if (FIGHTING(ch))
        stop_fighting(ch);

    for (k = combat_list; k; k = temp) {
        temp = k->next_fighting;
        if (FIGHTING(k) == ch)
            stop_fighting(k);
    }
}

/* The make_corpse function creates a new object, dumps new values into it, and
 * returns value to function raw_kill. Additionally, drowning underwater will
 * call this function in others.c
 */
ObjData* make_corpse(CharData *ch)
{
  char corpse_buf[MAX_PROMPT_LENGTH]; //Redefine buf here if to many truncations.
  ObjData *corpse, *o;
  ObjData *money;
  ExtraDescrData *new_descr;
  int i, x, y;

  char *corpseDesc[] ={
    "What remains of %s rest here, fills the air with a stench.",
    "The lifeless body of %s is lying here in a pool of blood.",
    "The dismembered remains of %s are strewn about here.",
    "Glazed, lifeless eyes gaze up at you from the body of %s.",
    "The remains of %s are lying here in a bloody heap.",
    "The mangled remains of %s lie here.",
    "A pile of goop and flesh, formerly known as %s, is lying here.",
    "The corpse of %s is lying here.",
  };
  /* End of declarations. */

  /* Function code begins. Checking for NULL characters  */
  if (ch == NULL) {
    mudlog(BRF, LVL_GOD, TRUE,
            "OBJ: Function make_corpse was sent a NULL CharData.");
    return(0);
  }

  if (IS_NPC(ch) && percentSuccess(ch->decay)) {
    act("As $n falls to the ground, it disintegrates into dust.",
            FALSE, ch, 0, 0, TO_ROOM);
    return (0);
  }

  /* Below is a uniary sizeof operation. The sizeof function is used to return
   * data type sizes. Please be aware this is returning the length of the array
   * and NOT the string size! */
  int maxDesc = sizeof(corpseDesc) / sizeof(corpseDesc[0]) - 1;
  int descNum = number(0, maxDesc);

  /* Corpse memory setup from malloc, now assign values. */
  corpse = create_obj();

  /* Record the decay of the mob */
  corpse->obj_flags.cost = ch->decay; //might be better placed lower in the func.

  /* PC & NPC corpses handled differently. */
  if (!IS_NPC(ch))
    snprintf(corpse_buf, sizeof (corpse_buf), "corpse %s PC_CORPSE", GET_NAME(ch));
  else
    snprintf(corpse_buf, sizeof (corpse_buf), "corpse %s", GET_NAME(ch));
  /* Now copy and dump the name into a new string. */
  corpse->name = strdup(corpse_buf);

  snprintf(corpse_buf, sizeof (corpse_buf), corpseDesc[descNum], GET_NAME(ch));
  corpse->description = strdup(corpse_buf);

  snprintf(corpse_buf, sizeof (corpse_buf), "the corpse of %s", GET_NAME(ch));
  corpse->short_description = strdup(corpse_buf);

  GET_OBJ_TYPE(corpse) = ITEM_CONTAINER;

  for (x = y = 0; x < EF_ARRAY_MAX || y < TW_ARRAY_MAX; x++, y++) {
    if (x < EF_ARRAY_MAX)
      GET_OBJ_EXTRA_AR(corpse, x) = 0;
    if (y < TW_ARRAY_MAX)
      corpse->obj_flags.wear_flags[y] = 0;
  }

  /* Vex - made PC corpses untakeable. */
  if (IS_NPC(ch) && strncmp(ch->player.name, "PC_CORPSE", 9) != 0)
    SET_BIT_AR(GET_OBJ_WEAR(corpse), ITEM_WEAR_TAKE);
  else {
    /* No take flags - but I hate 0's */
    SET_BIT_AR(GET_OBJ_WEAR(corpse), ITEM_WEAR_HOLD);

    /* Setting up another desc structure for corpse save list. */
    if (index(ch->player.name, ' ') != NULL) {
      strcpy(corpse_buf, index(ch->player.name, ' ') + 1);
    } else {
      strcpy(corpse_buf, ch->player.name);
    }
    CREATE(new_descr, struct extra_descr_data, 1);
    new_descr->keyword = strdup(corpse_buf); /* Checked later on.. */
    snprintf(corpse_buf, sizeof (corpse_buf), "Ewwww! What ever killed %s sure made it messy!", GET_NAME(ch));
    new_descr->description = strdup(corpse_buf); /* Just decoration. */
    new_descr->next = NULL;
    corpse->ex_description = new_descr;
  }

  SET_BIT_AR(GET_OBJ_EXTRA(corpse), ITEM_NODONATE);
  GET_OBJ_VAL(corpse, 0) = 0;	/* You can't store stuff in a corpse */
  GET_OBJ_VAL(corpse, 3) = 1;	/* corpse identifier */
  GET_OBJ_WEIGHT(corpse) = GET_WEIGHT(ch) + IS_CARRYING_W(ch);

  if (IS_NPC(ch))   GET_OBJ_RENT(corpse) = GET_MOB_VNUM(ch);
  else   GET_OBJ_RENT(corpse) = 100000;

  if(IS_CLONE(ch))
      GET_OBJ_TIMER(corpse) = number( 1, 3 );
  else if( IS_NPC(ch) ) {
      if( MOB_FLAGGED(ch, MOB_CONJURED) ) GET_OBJ_TIMER(corpse) = number(1, 2);
      else    GET_OBJ_TIMER(corpse) = CONFIG_MAX_NPC_CORPSE_TIME;
  }
  else
      GET_OBJ_TIMER(corpse) = CONFIG_MAX_PC_CORPSE_TIME;

  /* transfer character's inventory to the corpse */
  corpse->contains = ch->carrying;
  for (o = corpse->contains; o != NULL; o = o->next_content) {
      if (IS_CLONE(ch) && IS_SET_AR(o->obj_flags.extra_flags, ITEM_TIMED)) {
          GET_OBJ_TIMER(o) = number(2, 6);
      }
      o->in_obj = corpse;
  }
  object_list_new_owner(corpse, NULL);

  /* transfer character's equipment to the corpse */
  for (i = 0; i < NUM_WEARS; i++)
      if (GET_EQ(ch, i)) {
          ObjData *o;
          remove_otrigger(GET_EQ(ch, i), ch);
          o = unequip_char(ch, i);
          if (IS_CLONE(ch) && IS_SET_AR(o->obj_flags.extra_flags, ITEM_TIMED)) {
              GET_OBJ_TIMER(o) = number(2, 6);
          }
          obj_to_obj(o, corpse);
      }
  
  // Put gold on the corpse, unless it's a pvp holiday or (it's a linkdead player?)
  if (GET_GOLD(ch) > 0) {
      if ((IS_NPC(ch) && pvpFactor() <= 1) || (!IS_NPC(ch) && ch->desc)) {
          money = create_money(GET_GOLD(ch)); /* Get player gold */
          obj_to_obj(money, corpse);
      }

      GET_GOLD(ch) = 0; /* Now set players gold to 0 */
  }
  
  ch->carrying = NULL;
  IS_CARRYING_N(ch) = 0;
  IS_CARRYING_W(ch) = 0;

  /* For now, keep doing this too <<-- Not a very good idea -Xiuh
   * FIXME: Bad idea to have two different utilities writing to the same file
   * here. This should probably be addressed soon. */
  if (!IS_NPC(ch)) obj_corpse_save(corpse, ch);

  obj_to_room(corpse, IN_ROOM(ch));

  /* For player and player ghost corpses, add the corpse to a saved list */
  if (!IS_NPC(ch) || strncmp(ch->player.name, "PC_CORPSE", 9) == 0) {
    GET_OBJ_TIMER(corpse) = CONFIG_MAX_PC_CORPSE_TIME;
    add_saved_corpse(time(NULL), corpse);
    Crash_save_all();
  }

  return corpse;
}


/* When ch kills victim */
void change_alignment(CharData * ch, CharData * victim)
{
#define ALIGNMENT_MAX_GOOD     1000
#define ALIGNMENT_MAX_EVIL     -ALIGNMENT_MAX_GOOD
#define ALIGNMENT_TRUE_NEUTRAL 0
#define ALIGNMENT_RATIO        200

    GET_ALIGNMENT(ch) += (ALIGNMENT_TRUE_NEUTRAL - GET_ALIGNMENT(victim)) / ALIGNMENT_RATIO;
    GET_ALIGNMENT(ch)  = CLAMP_VALUE( ALIGNMENT_MAX_EVIL, GET_ALIGNMENT(ch), ALIGNMENT_MAX_GOOD );    
}

void death_cry(CharData * ch)
{
    int door, was_in;
    char *death_rattle[] = {
        "Your blood freezes as you hear $n's death cry.",
        "$n screams in agony as $s lifeblood drains away.",
        "The lifeless husk of $n drops to the ground."
    };
    int rattle_idx = number( 0, ((sizeof( death_rattle ) / sizeof( death_rattle[0] ))-1));

    act(death_rattle[rattle_idx], FALSE, ch, 0, 0, TO_ROOM);
    was_in = ch->in_room;

    for (door = 0; door < NUM_OF_DIRS; door++) {
        if (CAN_GO(ch, door)) {
            ch->in_room = world[was_in].dir_option[door]->to_room;
            act("Your blood freezes as you hear someone's death cry.", FALSE, ch, 0, 0, TO_ROOM);
            ch->in_room = was_in;
        }
    }
}

int zombies = 0;
static int zombie_vnum = 0;
static int zombie_gifts = 0;

ACMD(do_zombies)
{
    if (!*argument) {
        sprintf(buf, "Current zombie rate: %d%%\r\n", zombies);
        send_to_char(buf, ch);
        return;
    }
    zombies = atoi(argument);
    if (zombies < 0) zombies = 0;
    if (zombies > 100) zombies = 100;
    sprintf(buf, "Zombie rate set to %d%%\r\n", zombies);
    sendChar(ch, buf);
    mudlog(BRF, MAX(LVL_IMMORT, GET_INVIS_LEV(ch)), FALSE, "%s set zombie rate to %d%%", GET_NAME(ch), zombies);
}

ACMD(do_zombiegift)
{
    two_arguments(argument, buf, buf2);

    if (!*buf || !*buf2) {
        sendChar(ch, "Zombie gifts will be one of %d objects", zombie_gifts);
        sendChar(ch, " starting from #%d\r\n", zombie_vnum);
        sendChar(ch, "Change with 'zombiegift <start> <num>'\r\n");
    } else {
        zombie_vnum = atoi(buf);
        zombie_gifts = atoi(buf2);
        sendChar(ch, "Zombie gifts will be one of %d objects", zombie_gifts);
        sendChar(ch, " starting from #%d\r\n", zombie_vnum);
    }
}

static void make_zombie(CharData *ch)
{
    int i = GET_MOB_RNUM(ch), door;
    int was_in = ch->in_room;
    char newbuf[2000];

    act("&08$n's corpse suddenly starts moving!&00", FALSE, ch, 0, 0, TO_ROOM);

    for (door = 0; door < NUM_OF_DIRS; door++) {
        if (CAN_GO(ch, door)) {
            ch->in_room = world[was_in].dir_option[door]->to_room;
            act("An eerie howl chills your blood.", FALSE, ch, 0, 0, TO_ROOM);
            ch->in_room = was_in;
        }
    }

    // replace the mob's long description with a zombieish one
    if (ch->player.long_descr
            && ch->player.long_descr != mob_proto[i].player.long_descr)
        free(ch->player.long_descr);
    sprintf(newbuf, "The zombie of %s prowls here.\r\n", GET_NAME(ch));
    ch->player.long_descr = strdup(newbuf);

    // replace the mob's room description with a zombie one
    if (ch->player.description
            && ch->player.description != mob_proto[i].player.description)
        free(ch->player.description);
    sprintf(newbuf, "%s doesn't look too healthy!\r\n", GET_NAME(ch));
    newbuf[0] = toupper(newbuf[0]);
    ch->player.description = strdup(newbuf);

    // add "zombie" to the mob's keyword
    sprintf(newbuf, "%s zombie", ch->player.name);
    if (ch->player.name && ch->player.name != mob_proto[i].player.name)
        free(ch->player.name);
    ch->player.name = strdup(newbuf);

    // halve its maxhit/maxmana and restore it
    GET_HIT(ch) = GET_MAX_HIT(ch) = GET_MAX_HIT(ch) / 2;
    GET_MANA(ch) = GET_MAX_MANA(ch) = GET_MAX_MANA(ch) / 2;

    // halve its damroll
    SET_DAMROLL(ch, GET_DAMROLL(ch)/2); 
    
    // it gets a -2 point penalty to hitroll
    GET_HITROLL(ch) = MAX(0, GET_HITROLL(ch) - 2);

    // stand it back up
    GET_POS(ch) = POS_STANDING;

    // restore its exp to a third what it was
    // note that its exp will be negative since it died!
    GET_EXP(ch) = -GET_EXP(ch) / 3;

    // make the zombie an undead, so it can't rise again
    SET_RACE(ch, RACE_UNDEAD);

    add_affect(ch, ch, SPELL_CHARM_CORPSE, 1, 0, 0, 8 TICKS, 0, FALSE, FALSE, FALSE, FALSE);
    add_affect(ch, ch, SPELL_ZOMBIE_REWARD, 1, 0, 0, 8 TICKS, 0, FALSE, FALSE, FALSE, FALSE);
    SET_BIT_AR(MOB_FLAGS(ch), MOB_AGGR_EVIL);
    SET_BIT_AR(MOB_FLAGS(ch), MOB_AGGR_GOOD);
    SET_BIT_AR(MOB_FLAGS(ch), MOB_AGGR_NEUTRAL);

    // (try to) give it a zombie gift
    if (zombie_gifts) {
        ObjData *gift;
        
        gift = read_object(zombie_vnum + number(0, zombie_gifts - 1), VIRTUAL);
        if (gift) obj_to_char(gift, ch);
    }
}

#define ZOMBIE_OK(ch)  (IS_NPC(ch) && GET_RACE(ch) != RACE_VAMPIRE && \
                        !IS_UNDEAD(ch) && GET_MOB_RNUM(ch) > -1)

ObjData *raw_kill(CharData * ch, CharData * killer)
{
    int player_shunned = PLR_FLAGGED( ch, PLR_SHUNNED );
    ObjData *corpse = NULL;

    //if (FIGHTING(ch))
        //stop_fighting(ch);
    end_fight(ch);

    if (ch->rider) {
        awkward_dismount(ch->rider);
    }

    if (ch->mount) {
        ch->mount->rider = NULL;
        ch->mount = NULL;
    }

/* affect_remove : TRUE is for saving, it unaffects all spells when saving, this
		   is messing up the fall rooms. */
    while (ch->affected)
        affect_remove(ch, ch->affected, TRUE);

    if( player_shunned )
       SET_BIT_AR(PLR_FLAGS(ch), PLR_SHUNNED);

    /* Auto switch off aggressive after death. */
    if (!IS_NPC(ch))
	ch->player_specials->saved.aggr_pref = AGGR_OFF;

    /* if this is a non-undead NPC and we should be makin' zombies out
     * of them, do so! */
    if (shouldMakeZombie() && ZOMBIE_OK(ch)) {
      make_zombie(ch);
      return;
    }

    if (killer) {
      if (death_mtrigger(ch, killer))
        death_cry(ch);
    } else
      death_cry(ch);

#if 1
    corpse = make_corpse(ch);
#else
    /*
    ** Use this when mud is UNSTABLE
    */
    if (!IS_NPC(ch)) {
	mudlog(NRM, LVL_IMMORT, TRUE, "MORTUARY: %s rent saved, no corpse created.", GET_NAME(ch));
        crashRentSave(ch, 0);
    }
    else {
        corpse = make_corpse(ch);
    }
#endif

    /* this should never be ... */
    if (GET_EXP(ch) < 0) GET_EXP(ch) = 0;

    /* if the killer is a charmie, swing up the chain of blame */
    if (killer && IS_NPC(killer) && IS_AFFECTED(killer, AFF_CHARM))
        killer = killer->master;

    /* did a high level PC just kill a low level PC? */
    if (killer && !IS_NPC(killer) && GET_LEVEL(killer) > GET_LEVEL(ch)) {
        /* if the lower level was a player killer, they deserve
         * whatever they get!  Unless, of course, the killer is an IMM :) */
        if (!PLR_FLAGGED(ch, PLR_KILLER) && IS_MORTAL(killer) && !IS_NPC(ch)) {
            long diff = titles[GET_CLASS(killer)][GET_LEVEL(killer) + 1].exp
                      - titles[GET_CLASS(killer)][GET_LEVEL(killer) + 0].exp;
            /* diff = 100%, we want 0.75% => * 3, / 400, per level: */
            diff = diff * 3 / 400 * (GET_LEVEL(killer) - GET_LEVEL(ch));
            sendChar(killer, "You have lost %dxp for your vicious act!\r\n",
                    diff);
            /* we use _unchecked so they can lose more than 5mil xp */
            gain_exp_unchecked(killer, -diff);
        }
    }

    extract_char(ch);
    return corpse;
}

/* Stuff for diminishing returns exp scaling */
#define INC_PER_KILL    3               /* One kill = how much score? */
#define DEC_PER_TICK    1               /* One tick = how much score? */
#define MAX_SCORE       144             /* Maximum value for score    */
#define LOW_EXP_BASE    50              /* Minimum exp earned         */

/* Record how violent each player has been in each zone */
struct zone_kills { int zone, score; };

struct zone_kill_list {
    long player;                        /* Player ID */
    int count;                          /* # of records */
    struct zone_kills *kills;           /* the records */
    struct zone_kill_list *next;        /* next in list */
} *kill_list = NULL;

/* Diminish this character's exp.  Returns the % the exp should be
 * scaled by. */
int diminish(CharData *ch, CharData *victim)
{
    struct zone_kill_list *temp = kill_list;
    int i, zone, factor;

    /* Need both ch and victim to proceed */
    if (!ch || !victim) return 100;

    /* Make sure we're talking player kills mob, here */
    if (IS_NPC(ch) || !IS_NPC(victim) || GET_MOB_VNUM(victim) == -1)
        return 100;

    zone = GET_MOB_VNUM(victim) / 100;

    /* See if a record exists for this player */
    while (temp && temp->player != GET_IDNUM(ch)) temp = temp->next;

    /* Else, create it */
    if (!temp) {
        CREATE(temp, struct zone_kill_list, 1);

        /* Initialise values */
        temp->kills = NULL;
        temp->count = 0;
        temp->player = GET_IDNUM(ch);

        /* Link it into the list */
        temp->next = kill_list;
        kill_list = temp;
    }

    /* See if the zone has a record already */
    for (i = 0; i < temp->count; i++)
        if (temp->kills[i].zone == zone) break;

    /* Else, create it */
    if (i == temp->count) {
        RECREATE(temp->kills, struct zone_kills, ++temp->count);
        temp->kills[i].zone = zone;
        temp->kills[i].score = 0;
    }

    /* Get the OLD score for factoring */
    factor = temp->kills[i].score;

    /* Add the score for this kill */
    temp->kills[i].score += INC_PER_KILL;
    if (temp->kills[i].score > MAX_SCORE) temp->kills[i].score = MAX_SCORE;

    /* Scale from 100 down to LOW_EXP_BASE */
    factor = MAX_SCORE - factor;
    factor = factor * (100 - LOW_EXP_BASE) / MAX_SCORE;
    return LOW_EXP_BASE + factor;
}

/* Decrement all kill counters */
void update_kill_counts(void)
{
    struct zone_kill_list *temp = kill_list, *next, *prev = NULL;
    int i;

    while (temp) {
        next = temp->next;
        for (i = 0; i < temp->count; i++) {
            temp->kills[i].score -= DEC_PER_TICK;

            /* If a score falls below 1, remove it from the list */
            if (temp->kills[i].score <= 0) {
                if (temp->count == 1) {
                    temp->count = 0;
                    free(temp->kills);
                    temp->kills = NULL;
                } else {
                    memmove(temp->kills + i, temp->kills + i + 1,
                            sizeof(struct zone_kills) * (temp->count - i - 1));
                    RECREATE(temp->kills, struct zone_kills, --temp->count);

                    /* Also have to recheck the current value now! */
                    i--;
                }
            }
        }

        /* If there are no scores left, remove this player's record */
        if (temp->count == 0) {
            if (prev) {
                prev->next = temp->next;
            } else {
                kill_list = temp->next;
            }
            free(temp);
        } else {
            prev = temp;
        }

        temp = next;
    }
}

void show_kills(CharData *ch, char *name)
{
    CharData *vict = get_char_vis(ch, name, 1);
    struct zone_kill_list *temp;
    int i;

    if (!vict || IS_NPC(vict)) {
        sendChar(ch, "There's no player by that name!\r\n");
        return;
    }

    for (temp = kill_list; temp; temp = temp->next) {
        if (temp->player == GET_IDNUM(vict)) {
            sendChar(ch, "Zone Score\r\n");
            for (i = 0; i < temp->count; i++) {
                sendChar(ch, " %3d %d\r\n", temp->kills[i].zone,
                        temp->kills[i].score);
            }
            return;
        }
    }
    sendChar(ch, "No kills registered.\r\n");
}

/*
** Previously, there where bits  of code scattered all over the place each
** doing exps gain/losses. This function  replaced all the little pieces
** so all exp gain/losses for mobs and pcs killing one another should
** be calculated from here. The parameters are:
** ch - the killer(or the highest level member of a party of killers
** victim -  the thing that has been killed
** pkill - true if this death was the result of a player_kill
** death_loss - true if the function is to calculate  the xps loss for the
**              victim. Other wise the function calculates the xps gain for
**		the killer(s). ch is NOT used when death_loss is true, so
**		should be passed in as NULL in that case.
** return value - the xps, +ve if death_loss is false, otherwise -ve
** Vex '97
*/
int
xp_gain(CharData *ch, CharData *victim, int pkill, int death_loss)
{
  int exp;

  if (IS_NPC(victim)) {
    exp = GET_EXP(victim);
    exp = (exp / 10) * diminish(ch, victim) / 10;
  }
  else /* victim is a player, so lets handle this differently. */
  {
    if( GET_LEVEL(victim) < 1 )
    {
      mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: %s has invalid level %d, setting them to level 1.",
               GET_NAME(victim), GET_LEVEL(victim));
      GET_LEVEL(victim) = 1;
    }
    exp = MIN( CONFIG_MAX_EXP_GAIN,
              (titles[(int)GET_CLASS(victim)][(int)GET_LEVEL(victim)].exp -
               titles[(int)GET_CLASS(victim)][(int)GET_LEVEL(victim) - 1].exp) >> 2);

    /* Make sure we don't dump fictitous XP into a mob. */
    exp = MIN( exp, GET_EXP(victim) );

    /* Up to 25% can be transferred to the killer from the pc,
       the pc can lose up to 50%(see below) */
  }


  //if (exp < 1 && !death_loss && !pkill && !IS_NPC(ch) && !IS_AFFECTED(victim, AFF_CHARM)) {
//	  exp = CONFIG_MAX_EXP_GAIN;
 // }

  exp = MAX(exp, 1);


  if( pkill )
  {
    exp = ( death_loss ? exp >> 2 : 1 );
  }

  if( !death_loss && !IS_NPC( victim ) && IS_NPC(ch) && GET_LEVEL(victim) >= 46 )
  {
    /*
    ** Higher levels should be penalized so after 45th only a certain
    ** percentage of the PC's XP will be pumped into the mob. This is
    ** really to avoid 46th+ from using mobs as XP pumps to other chars.
    */
    if( GET_LEVEL(victim) <= MAX_MORTAL )
    {
      /*              Level -> 46   47   48   49   50  */
      float xferPercent[] = { 0.8, 0.7, 0.6, 0.4, 0.2, 0.0 };
      int levelIdx = GET_LEVEL(victim) - 46;
      int adjExp   = (int)(exp * xferPercent[levelIdx]);
      mudlog(NRM, LVL_IMMORT, TRUE, "XP Pumped: From %s to %s total %d adjusted to %d (%d%%:%d)",
                GET_NAME(victim), GET_NAME(ch), exp, adjExp,
                (int)(100.0*xferPercent[levelIdx]), GET_LEVEL(victim) );
      exp = adjExp;
    }
  }

  if (death_loss) 
    exp = -(exp * 2); /* You lose twice as much as they gain */

  return exp;

} /* xp_gain */


ObjData *
die( CharData *ch, CharData *killer, int pkill)
{
  ObjData *obj, *objnext;
  int target_room;
  ZKILL_CNT(ch);

  /*
  ** Dismount any riders
  */

  if(pvpHoliday(ch) && !IN_ARENA(ch))
      return;

  /*
  ** Check for quest/chore kills
  */
  check_quest_kill(killer, ch);
  if (IS_NPC(ch)) chore_check_kill(killer, GET_MOB_VNUM(ch));

  /*
  ** Auto switch off aggressive after death.
  */
  if( !IS_NPC(ch) )
  {
    ch->player_specials->saved.aggr_pref = AGGR_OFF;
    REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_CONSENT);
  }

  /*
   * Auto drop ITEM_ARENA objects.
   */
  for( obj = ch->carrying; obj; obj = objnext ) {
    objnext = obj->next_content;
    if ( IS_SET_AR(GET_OBJ_EXTRA(obj), ITEM_ARENA) ) {
      act("$p falls to the ground.", TRUE, ch, obj, NULL, TO_ROOM);
      obj_from_char(obj);
      obj_to_room(obj, ch->in_room);
    }
  }

  if( IN_ARENA(ch) || ZONE_FLAGGED(world[ch->in_room].zone, ZONE_ARENA) ||
      ZONE_FLAGGED(world[ch->in_room].zone, ZONE_SLEEPTAG) || (IN_QUEST_FIELD(ch)))
  {
    // Digger - New tournament/deathmatch code.
    //
    if (IN_ARENA(ch))
        target_room = real_room( ARENA_RM(ch) );
    else
        target_room = real_room(18199);

    death_cry(ch);
    act( "$n is vaporized.", TRUE, ch, 0, 0, TO_ROOM );

    if(IS_NPC(ch))
    {
        // transfer objects to room, if any
        while( ch->carrying )
        {
            obj = ch->carrying;
            obj_from_char(obj);
            obj_to_room(obj, ch->in_room);
        }

        if(!affected_by_spell(ch, SPELL_CHARM_CORPSE))
            extract_char(ch);
        else {
            char_from_room(ch);
            char_to_room(ch, 1);
            affect_from_char(ch, SPELL_CHARM_CORPSE);
        }

        return;
    }
    if (IN_QUEST_FIELD(ch))
    {
      jail_char(ch, FALSE);
      return;
    }
    
    end_fight(ch);

    // The BloodBowl is almost like football.
    //
    if( inBloodBowl( ch ) )
    {
      dieInBloodBowl( ch );
      return;
    }
    else
    {
      char_from_room(ch);
      char_to_room(ch, target_room);
      look_at_room(ch, 0);

      // Generic Arena Stuff - Give them a half-restore, can't full
      // restore or it'd get abused.
      if(the_muckle) {
          restore(ch, ch);
      }
      else {
          GET_HIT (ch) = GET_MAX_HIT (ch)/2;
          GET_MANA(ch) = GET_MAX_MANA(ch)/2;
          GET_MOVE(ch) = GET_MAX_MOVE(ch)/2;
      }
      
      if(!PRF_FLAGGED(ch, PRF_QUEST)) {
          while(ch->affected) // avoid all complications...
              affect_remove(ch, ch->affected, FALSE); /* FALSE added for fall room */
      }
      else {
          affect_from_char(ch, SPELL_BLINDNESS);
          affect_from_char(ch, SPELL_PARALYZE);
          affect_from_char(ch, SPELL_SILENCE);
          affect_from_char(ch, SKILL_DIRTY_TACTICS);
          affect_from_char(ch, SKILL_DUST);
          affect_from_char(ch, SPELL_FEEBLEMIND);
          affect_from_char(ch, SPELL_DEBILITATE);
          affect_from_char(ch, SPELL_SLOW);
      }
      GET_POS(ch) = POS_RESTING;
      act( "$n materializes before you.", TRUE, ch, 0, 0, TO_ROOM );
    }

    return;
  }

  gain_exp(ch, xp_gain(NULL, ch, pkill, TRUE)); 
  return raw_kill(ch, killer);
}


void
log_quest( CharData *winner,
           CharData *loser )
{
  static char *black = "&05(black)&00";
  static char *gold  = "&03(gold)&00";
  static char *rogue = "&08(rogue)&00";
  static char *none  = "&10(no team)&00";
  CharData *mob = winner;

  DescriptorData *questor;
  char qBuf[256] = "";

  char *wColor;
  char *lColor;

  if (IS_NPC(mob) && mob->master) winner = mob->master;

  wColor = ( PRF_FLAGGED( winner, PRF_GOLD_TEAM ) ? gold : black );
  lColor = ( PRF_FLAGGED( loser,  PRF_GOLD_TEAM ) ? gold : black );

  /*
  ** Check to make certain they were on teams.
  */
  if( !PRF_FLAGGED( winner, PRF_GOLD_TEAM ) &&
      !PRF_FLAGGED( winner, PRF_BLACK_TEAM ) ) wColor = none;
  if( !PRF_FLAGGED( loser, PRF_GOLD_TEAM ) &&
      !PRF_FLAGGED( loser, PRF_BLACK_TEAM ) ) lColor = none;
  if( PRF_FLAGGED( winner, PRF_ROGUE_TEAM)) wColor = rogue;
  if( PRF_FLAGGED( loser, PRF_ROGUE_TEAM)) lColor = rogue;

  if (mob != winner) {
    sprintf( qBuf, "&08FLAG GAME:&00 %s %s defeats %s %s with %s at &12%s&00\r\n\r\n",
                  GET_NAME(winner), wColor,
                  GET_NAME(loser), lColor,
                  GET_NAME(mob),
                  world[winner->in_room].name);
  } else {
    sprintf( qBuf, "&08FLAG GAME:&00 %s %s defeats %s %s at &12%s&00\r\n\r\n",
                  GET_NAME(winner), wColor,
                  GET_NAME(loser), lColor,
                  world[winner->in_room].name);
  }

  /*
  ** Provide a little quest commentary.
  */

  for( questor = descriptor_list; questor; questor = questor->next )
  {
    if( !questor->connected &&
         questor->character &&
         PRF_FLAGGED( questor->character, PRF_QUEST ))
      send_to_char( qBuf, questor->character);
  }

  /* update flag score system */
  flag_player_killed(winner, loser);
}

void
perform_group_gain( CharData* ch,
                    int base,
                    CharData* victim,
		    int tot_levels)
{
  unsigned CRIT_POINT = 1250;
  unsigned REMORT_EXP = 2000;

  if(pvpHoliday(ch))
      base = MAX(1, base/10);

  // The following makes it so when you're above 1.25 billion 
  // experience, there is a penalty for not being grouped.
  if(GET_EXP(ch)/1000000 > CRIT_POINT && GET_ADVANCE_LEVEL(ch) >= 5 && GET_ADVANCE_LEVEL(ch) < 10)
	tot_levels = MAX( tot_levels, 
          GET_LEVEL(ch)*(GET_ADVANCE_LEVEL(ch)-4)/2 + 
          25*( GET_EXP(ch)/1000000 - CRIT_POINT)/(REMORT_EXP - CRIT_POINT) );

  if (tot_levels == 0)
      return;

  unsigned int share = ((unsigned)base * (unsigned)GET_LEVEL(ch)) / (tot_levels);

  share = share * expfactor(ch) / 100;

  share = MIN(CONFIG_MAX_EXP_GAIN, MAX(1, share));

  sendChar( ch, "You receive your share of experience -- " );

  if( share > 1 )
    sendChar( ch, "%d points.\r\n", share);
  else
    sendChar( ch, "one measly little point!\r\n" );

  gain_exp( ch, share );
  change_alignment( ch, victim );
}


void group_gain(CharData * ch, CharData * victim, int pkill, int roomNum)
{
    int tot_members;	// Number of members in the room.
    int tot_levels;	// The total levels in the group.
    int base;		// Base xps.
    CharData *high_member, *original_ch;
    FollowType *f, *g;

    // If no ch, bail
    if(!ch)
        return;

    original_ch = ch;

    // if you follow self and you're ungrouped, you must group yourself.
    if(ch->master == NULL && !IS_AFFECTED(ch, AFF_GROUP))
        SET_BIT_AR(AFF_FLAGS(ch), AFF_GROUP);

    while(ch->master && (IS_NPC(ch) || IS_AFFECTED(ch, AFF_GROUP))) {
        group_gain(ch->master, victim, pkill, roomNum); /* k will be the group leader now */
        return;
    }

    high_member = ch;

    if(IS_AFFECTED(ch, AFF_GROUP) && (roomNum == ch->in_room)) {
	tot_levels = GET_LEVEL(ch);
        tot_members = 1;
    }
    else {
	tot_levels = 0;
        tot_members = 0;
    }

    /*
    ** Count the followers in the room.
    */
    for(f = ch->followers; f; f = f->next) {
        if(IS_AFFECTED(f->follower, AFF_GROUP) && roomNum == f->follower->in_room
                || IS_NPC(f->follower) && IS_AFFECTED(f->follower, AFF_CHARM))
	{
            tot_members++;
	    tot_levels += IS_NPC(f->follower)? GET_LEVEL(f->follower)/3:GET_LEVEL(f->follower);
	    if (GET_LEVEL(f->follower) > GET_LEVEL(high_member))
		high_member = f->follower;

            for(g = f->follower->followers; g; g = g->next) {
                if(IS_NPC(g->follower) && IS_AFFECTED(g->follower, AFF_CHARM)) {
                    tot_members++;
                    tot_levels += GET_LEVEL(g->follower)/3;
                }
            }
	}
    }

    /*
    ** Round up to the next highest tot_members.
    */

    if(tot_members >= 1)
        base = MAX(1, xp_gain(high_member, victim, pkill, FALSE));
    else
        base = 0;

    if(pkill) base = 1;

    if( IS_AFFECTED(ch, AFF_GROUP) && roomNum == ch->in_room )
        perform_group_gain(ch, base, victim, tot_levels);

    for (f = ch->followers; f; f = f->next)
        if (IS_AFFECTED(f->follower, AFF_GROUP) && f->follower->in_room == roomNum)
            perform_group_gain(f->follower, base, victim, tot_levels);
}

void ind_gain(CharData *ch, CharData *victim, int pkill)
{
    long exp = xp_gain(ch, victim, pkill, FALSE);

    // Disabled
    //if (IS_AFFECTED(ch, AFF_LEARNING)) exp *= LEARN_RATE;

    if( affected_by_spell(victim, SPELL_CHARM_CORPSE) || IS_AFFECTED(victim, AFF_CHARM) )
        exp = 1;
			
    group_gain(ch, victim, pkill, ch->in_room);
} /* ind_gain */


/*
** This function deals with all the death messages and the killer(s)
** xp gain.
*/
void
resolve_death(CharData *ch, CharData *victim, int pkill)
{
    DescriptorData *pl_desc;

    /* Did someone just get an adrenaline rush? */
    if (ch && skillSuccess(ch, SKILL_ADRENALINE)) {
        add_affect(ch, ch, SKILL_ADRENALINE, GET_LEVEL(ch),
                APPLY_DAMROLL, 1, 2 TICKS, 0, TRUE, TRUE, TRUE, FALSE);
        sendChar(ch, "Your blood pumps with an adrenaline rush.\r\n");
        advanceSkill(ch, SKILL_ADRENALINE, 90,
                "You learn how to make better use of your adrenaline!",
                TRUE, TRUE, TRUE);
    }
    
    group_gain(ch, victim, pkill, ch->in_room);

    if( !IS_NPC(victim) ) { /* Then log this death and make mobs forget them. */
        if( !IN_ARENA(ch) && !IN_QUEST_FIELD(ch) &&
              !ZONE_FLAGGED(world[ch->in_room].zone, ZONE_ARENA) &&
              !ZONE_FLAGGED(world[ch->in_room].zone, ZONE_SLEEPTAG)) {
            mudlog(BRF, LVL_IMMORT, TRUE, "%s killed by %s at %s",
                GET_NAME(victim),
                GET_NAME(ch), world[IN_ROOM(victim)].name);

            if (MOB_FLAGGED(ch, MOB_MEMORY) || SEEKING(ch))
                forget(ch, victim);
        } else if (ZONE_FLAGGED(world[ch->in_room].zone, ZONE_SLEEPTAG)) {
            mudlog(BRF, LVL_IMMORT, TRUE, "%s killed by %s in sleep tag arena",
                GET_NAME(victim), GET_NAME(ch));
        } else if (IN_QUEST_FIELD(ch)) {
            log_quest(ch, victim);
        } else {
            char killmessage[MAX_INPUT_LENGTH];
            CharData *winner = ch, *loser = victim;

            /* charmies rack up a score for their master */
            if (IS_NPC(ch) && ch->master) {
                sprintf(killmessage, "&08%s: %s slain by %s of %s.&00\r\n",
                        (inBloodBowl(ch) ? "BLOODBOWL" : "ARENA" ),
                        GET_NAME(victim), GET_NAME(ch),
                        GET_NAME(ch->master));
                winner = ch->master;
            } else {
                sprintf(killmessage, "&08%s: %s slain by %s.&00\r\n",
                        (inBloodBowl(ch) ? "BLOODBOWL" : "ARENA" ),
                        GET_NAME(victim), GET_NAME(ch));
            }

            /* Calculate arenarank diff if it was PC vs PC */
            if (!IS_NPC(winner) && !IS_NPC(loser) && winner != loser) {
                int diff = GET_ARENA_RANK(winner) - GET_ARENA_RANK(loser);
                int rank;

                /* Approximate a normal curve for expectation of a win */
                rank = 25 * (1 - 0.5 * exp(-(diff/100.0)*(diff/100.0)));

                /* Need to flip this if the winner's score was higher */
                if (diff > 0) rank = 25 - rank;

                /* Adjust both arenaranks */
                GET_ARENA_RANK(winner) += rank;
                GET_ARENA_RANK(loser)  -= rank;
                /* But we pretend we don't track it anymore */
                /*sendChar(winner, "You gain %d arena rank.\r\n", rank);
                 * sendChar(loser, "You lose %d arena rank.\r\n", rank);
                 */

                if(the_muckle == loser) {
                    sprintf(buf, "&08MUCKLE:&00 %s is now the muckle!&00\r\n", GET_NAME(winner));
                    quest_echo(buf);
                    the_muckle = winner;
                }

            }

            for(pl_desc = descriptor_list; pl_desc; pl_desc = pl_desc->next) {
                if( !pl_desc->connected &&
                        pl_desc->character &&
                        !PRF_FLAGGED( pl_desc->character, PRF_NOARENA )) {
                    sendChar( pl_desc->character, killmessage);
                }
            }
        }
    }
}

char *replace_string(char *str, char *weapon_singular, char *weapon_plural)
{
    static char buf[256];
    char *cp;

    cp = buf;

    for( ; *str; str++ ){
        if( *str == '#' ){
            switch( *(++str) ){
                case 'W':
                    for( ; *weapon_plural; *(cp++) = *(weapon_plural++) );
                    break;
                case 'w':
                    for( ; *weapon_singular; *(cp++) = *(weapon_singular++) );
                    break;
                default:
                    *(cp++) = '#';
                    break;
            }
        }
        else
            *(cp++) = *str;

        *cp = 0;
    }
    return (buf);
}


//
//
int
getDamageIdx( const int dam )
{
  int msgnum = 0;

       if( dam ==   0) msgnum = 0;
  else if( dam <=   2) msgnum = 1;
  else if( dam <=   4) msgnum = 2;
  else if( dam <=   6) msgnum = 3;
  else if( dam <=  10) msgnum = 4;
  else if( dam <=  14) msgnum = 5;
  else if( dam <=  19) msgnum = 6;
  else if( dam <=  26) msgnum = 7;
  else if( dam <=  35) msgnum = 8;
  else if( dam <=  47) msgnum = 9;
  else if( dam <=  59) msgnum = 10;
  else if( dam <=  99) msgnum = 11;
  else if( dam <= 150) msgnum = 12;
  else if( dam <= 200) msgnum = 13;
  else if( dam <= 300) msgnum = 14;
  else if( dam <= 400) msgnum = 15;
  else if( dam <= 500) msgnum = 16;
  else                 msgnum = 17;

  return( msgnum );
}

typedef struct
{
  char* singular;
  char* plural;
} DamageWord;

DamageWord damageWords[] =
{
  { "miss",       "es" }, //  0
  { "scratch",    "es" }, //  1
  { "graze",      "s"  }, //  2
  { "hit",        "s"  }, //  3
  { "hard",       ""   }, //  4
  { "very",       ""   }, //  5
  { "extremely",  ""   }, //  6
  { "massacre",   "s"  }, //  7
  { "stagger",    "s"  }, //  8
  { "crush",      "es" }, //  9
  { "obliterate", "s"  }, // 10
  { "enshroud",   "s"  }, // 11
  { "charge",     "s"  }, // 12
  { "mutilate",   "s"  }, // 13
  { "destroy",    "s"  }, // 14
  { "annihilate", "s"  }, // 15
  { "vapourize",  "s"  }, // 16
  { "RUIN",       "S"  }  // 17
};

DamageWord*
getDamageWord( const int dam )
{
  return( &damageWords[ getDamageIdx(dam) ] );
}

// message for doing damage with a weapon
//
void
dam_message( int dam, CharData* ch, CharData* victim, int w_type )
{
  char *buf;
  int msgnum = getDamageIdx( dam );

  static struct dam_weapon_type {
    char *to_room;
    char *to_char;
    char *to_victim;
  } dam_weapons[] = {

  { "$n misses $N with $s #w.",				/* 0: 0.  */
    "You miss $N with your #w.",
    "$n misses you with $s #w." },

  { "$n scratches $N with $s #w.",			/* 1: 1..2  */
    "You scratch $N as you #w $M.",
    "$n scratches you as $e #W you." },

  { "$n barely #W $N.",					/* 2: 3..4  */
    "You barely #w $N.",
    "$n barely #W you." },

  { "$n #W $N.",					/* 3: 5..6  */
    "You #w $N.",
    "$n #W you." },

  { "$n #W $N hard.",					/* 4: 7..10  */
    "You #w $N hard.",
    "$n #W you hard." },

  { "$n #W $N very hard.",				/* 5: 11..14   */
    "you #w $N very hard.",
    "$n #W you very hard." },

  { "$n #W $N extremely hard.",				/* 6: 15..19   */
    "You #w $N extremely hard.",
    "$n #W you extremely hard." },

  { "$n massacres $N to small fragments with $s #w.",	/* 7: 20..26   */
    "You massacre $N to small fragments with your #w.",
    "$n massacres you to small fragments with $s #w." },

  { "$n staggers $N with $s #w.",			/* 8: 27..35   */
    "You stagger $N with your fearsome #w.",
    "$n staggers you with $s fearsome #w." },

  { "$n #W $N resulting in a bone crushing sound.", 	/* 9: 36..47   */
    "You #w $N, resulting in a bone crushing sound.",
    "$n #W you, resulting in a bone crushing sound." },

  { "$n obliterates $N with $s deadly #w.", 		/* 10: 48..59  */
    "You obliterate $N with your deadly #w.",
    "$n obliterates you with $s deadly #w." },

  { "$n #W $N, enshrouding $M in a mist of $S own blood.",/* 11: 60..99  */
    "You enshroud $N in a mist of $S own blood with your #w.",
    "$n enshrounds you in a mist of your own blood with $s #w." },

  { "$n charges $N, ripping completely through $M.", 	/* 12: > 100   */
    "You charge $N, ripping completely through $M.",
    "$n charges you, ripping completely through your body." },

  { "$n mutilates $N with inhuman power.", 	                /* 13 */
    "You mutilate $N with inhuman power.",
    "$n mutilates you with an inhuman power." },

  { "$n &14* destroys *&00 $N with $s deadly #w.", 		/* 14 */
    "You &14* destroy *&00 $N with your deadly #w.",
    "$n &14* destroys *&00 you with $s deadly #w." },

  { "$n &14** annihilates **&00 $N with $s deadly #w.",	/* 15 */
    "You &14** annihilate **&00 $N with your deadly #w.",
    "$n &14** annihilates **&00 you with $s deadly #w." },

  { "$n &11*** vapourizes ***&00 $N with $s deadly #w.",	/* 16 */
    "You &11*** vapourize ***&00 $N with your deadly #w.",
    "$n &11*** vapourizes ***&00 you with $s deadly #w." },

  { "$n &01<&08^&10> &09RUINS &10<&08^&00&01>&00 $N with $s mighty #w.",	/* 17 */
    "You &01<&08^&10> &09RUIN &10<&08^&00&01>&00 $N with your mighty #w.",	/* 17 */
    "$n &01<&08^&10> &09RUINS &10<&08^&00&01>&00 you with $s mighty #w."}	/* 17 */

  };

  if( w_type == TYPE_SPECIAL ) return;

  // SHADOWBOX damage type done a little differently. Imhotep.
  if( w_type == SKILL_SHADOWBOX) {
    char namebuf[200];
    char *oldname;
    int oldgender;

    oldgender = ch->player.sex;
    ch->player.sex = SEX_NEUTRAL;

    if (IS_NPC(ch)) {
      oldname = ch->player.short_descr;
      sprintf(namebuf, "%s's shadow", GET_NAME(ch));
      ch->player.short_descr = namebuf;
    } else {
      oldname = ch->player.name;
      sprintf(namebuf, "%s's shadow", GET_NAME(ch));
      ch->player.name = namebuf;
    }

    /* damage message to onlookers */
    buf = replace_string(dam_weapons[msgnum].to_room,
        "hit", "hits");
    act(buf, FALSE, ch, NULL, victim, TO_NOTVICT);

    /* damage message to damagee */
    send_to_char(CCRED(victim, C_CMP), victim);
    buf = replace_string(dam_weapons[msgnum].to_victim,
	  "hit", "hits");
    act(buf, FALSE, ch, NULL, victim, TO_VICT | TO_SLEEP);
    send_to_char(CCNRM(victim, C_CMP), victim);

    /* damage message to damager */
    sprintf(namebuf, "your shadow");
    buf = replace_string(dam_weapons[msgnum].to_room,
        "hit", "hits");
    act(buf, FALSE, ch, NULL, victim, TO_CHAR);

    ch->player.sex = oldgender;
    if (IS_NPC(ch)) ch->player.short_descr = oldname;
    else ch->player.name = oldname;
    return;	/* finished with assistant handling */
  }

  // ASSISTANT damage type done a little differently. Imhotep.
  if( w_type == TYPE_ASSISTANT) {
    char namebuf[200];
    char *oldname;
    int oldgender;

    oldgender = ch->player.sex;
    ch->player.sex = SEX_NEUTRAL;

    if (IS_NPC(ch)) {
      oldname = ch->player.short_descr;
      sprintf(namebuf, "%s's assistant", GET_NAME(ch));
      ch->player.short_descr = namebuf;
    } else {
      oldname = ch->player.name;
      sprintf(namebuf, "%s's assistant", GET_NAME(ch));
      ch->player.name = namebuf;
    }

    /* damage message to onlookers */
    buf = replace_string(dam_weapons[msgnum].to_room,
        "touch", "touches");
    act(buf, FALSE, ch, NULL, victim, TO_NOTVICT);

    /* damage message to damagee */
    send_to_char(CCRED(victim, C_CMP), victim);
    buf = replace_string(dam_weapons[msgnum].to_victim,
	  "touch", "touches");
    act(buf, FALSE, ch, NULL, victim, TO_VICT | TO_SLEEP);
    send_to_char(CCNRM(victim, C_CMP), victim);

    /* damage message to damager */
    sprintf(namebuf, "your assistant");
    buf = replace_string(dam_weapons[msgnum].to_room,
        "touch", "touches");
    act(buf, FALSE, ch, NULL, victim, TO_CHAR);

    ch->player.sex = oldgender;
    if (IS_NPC(ch)) ch->player.short_descr = oldname;
    else ch->player.name = oldname;
    return;	/* finished with assistant handling */
  }

  // RIPOSTE damage type done a little differently. Vex.
  if( w_type == SKILL_RIPOSTE) {
    /* damage message to onlookers */
    buf = replace_string(dam_weapons[msgnum].to_room,
          "riposte", "riposte");
    act(buf, FALSE, ch, NULL, victim, TO_NOTVICT);

    /* damage message to damager */
    send_to_char(CCYEL(ch, C_CMP), ch);
    buf = replace_string(dam_weapons[msgnum].to_char,
	  "riposte", "riposte");
    act(buf, FALSE, ch, NULL, victim, TO_CHAR);
    send_to_char(CCNRM(ch, C_CMP), ch);

    /* damage message to damagee */
    send_to_char(CCRED(victim, C_CMP), victim);
    buf = replace_string(dam_weapons[msgnum].to_victim,
	  "riposte", "riposte");
    act(buf, FALSE, ch, NULL, victim, TO_VICT | TO_SLEEP);
    send_to_char(CCNRM(victim, C_CMP), victim);
    return;	/* finished with riposte handling */
  }

  w_type -= TYPE_HIT;		/* Change to base of table with text */

  /*
  ** There are buggy weapons out there some where...
  */
  if( w_type < 0 || w_type >= NUM_WEAPON_TYPES )
  {
    if( ch->equipment[WEAR_WIELD] )
      mudlog(NRM, LVL_IMMORT, TRUE, "Object[%d] wielded by %s has invalid w_type %d.",
                GET_OBJ_VNUM(ch->equipment[WEAR_WIELD]),
                GET_NAME(ch), w_type);
    else if( IS_NPC(ch) )
      mudlog(NRM, LVL_IMMORT, TRUE, "Mob[%d] used invalid natural attack form %d.", GET_MOB_VNUM(ch), w_type);
    else
      mudlog(NRM, LVL_IMMORT, TRUE, "Player[%s] used invalid natural attack form %d.", GET_NAME(ch), w_type);

    w_type = 0; /* use hit */
  }

  /* Anti-SPAM should be config : Digger */
  if( msgnum == 0 ) return;

  /* damage message to onlookers */
  buf = replace_string(dam_weapons[msgnum].to_room,
          attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
  act(buf, FALSE, ch, NULL, victim, TO_NOTVICT);

  /* damage message to damager */
  send_to_char(CCYEL(ch, C_CMP), ch);
  buf = replace_string(dam_weapons[msgnum].to_char,
	  attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
  act(buf, FALSE, ch, NULL, victim, TO_CHAR);
  send_to_char(CCNRM(ch, C_CMP), ch);

  /* damage message to damagee */
  send_to_char(CCRED(victim, C_CMP), victim);
  buf = replace_string(dam_weapons[msgnum].to_victim,
	  attack_hit_text[w_type].singular, attack_hit_text[w_type].plural);
  act(buf, FALSE, ch, NULL, victim, TO_VICT | TO_SLEEP);
  send_to_char(CCNRM(victim, C_CMP), victim);

}


/*
 * message for doing damage with a spell or skill
 *  C3.0: Also used for weapon damage on miss and death blows
 */
int skill_message(int dam, CharData * ch, CharData * vict, int attacktype)
{
  int i, j, nr;
  struct message_type *msg;

  struct obj_data *weap = WIELDING(ch) ? ch->equipment[WEAR_WIELD] : NULL;

    if( fight_messages[0].a_type != 5 ){
        raw_input_logging = 0;
        mudlog(NRM, LVL_IMMORT, TRUE, "-*** FIGHT MESSAGES ARE SCREWED ***-" );
        load_messages();
    }

  for (i = 0; i < MAX_MESSAGES; i++) {
    if (fight_messages[i].a_type == attacktype) {
      nr = dice(1, fight_messages[i].number_of_attacks);
      for (j = 1, msg = fight_messages[i].msg; (j < nr) && msg; j++)
	msg = msg->next;

      if (!IS_NPC(vict) && (GET_LEVEL(vict) >= LVL_IMMORT)) {
	act(msg->god_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
	act(msg->god_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT);
	act(msg->god_msg.room_msg, FALSE, ch, weap, vict, TO_ACTSPAM);
      } else if (dam != 0) {
	if (GET_POS(vict) == POS_DEAD) {
	  send_to_char(CCYEL(ch, C_CMP), ch);
	  act(msg->die_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
	  send_to_char(CCNRM(ch, C_CMP), ch);

	  send_to_char(CCRED(vict, C_CMP), vict);
	  act(msg->die_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
	  send_to_char(CCNRM(vict, C_CMP), vict);

	  act(msg->die_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
	} else {
	  send_to_char(CCYEL(ch, C_CMP), ch);
	  act(msg->hit_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
	  send_to_char(CCNRM(ch, C_CMP), ch);

	  send_to_char(CCRED(vict, C_CMP), vict);
	  act(msg->hit_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
	  send_to_char(CCNRM(vict, C_CMP), vict);

	  act(msg->hit_msg.room_msg, FALSE, ch, weap, vict, TO_NOTVICT);
	}
      } else if (ch != vict) {	/* Dam == 0 */
	send_to_char(CCYEL(ch, C_CMP), ch);
	act(msg->miss_msg.attacker_msg, FALSE, ch, weap, vict, TO_CHAR);
	send_to_char(CCNRM(ch, C_CMP), ch);

	send_to_char(CCRED(vict, C_CMP), vict);
	act(msg->miss_msg.victim_msg, FALSE, ch, weap, vict, TO_VICT | TO_SLEEP);
	send_to_char(CCNRM(vict, C_CMP), vict);

	act(msg->miss_msg.room_msg, FALSE, ch, weap, vict, TO_ACTSPAM);
      }
      return 1;
    }
  }
  return 0;
}

/* This function returns the increased damage done by spells due to race, class, gear, etc.  3
** points cooresponds to 1% bonus damage.
*/
int spell_damage_gear(CharData *ch) {
	int i, j;
        CharData *tmp_ch;
        int foundPet = FALSE;

	// Find gear that improves or hurts spell damage.
	int increase = 0;
	/* check for any gear that increases spell damage */
	for( j = 0; j < NUM_WEARS; j++ )	{
		if( ch->equipment[j]) {
			for ( i = 0; i < MAX_OBJ_AFFECT; i++) {
				if (ch->equipment[j]->affected[i].location == APPLY_SPELL_DAMAGE)
					increase += ch->equipment[j]->affected[i].modifier;
			}
		}
	}	

	// Ogre Magi have a natural ability to cast damaging spells for 30%
        // bonus damage.  Drow have 5% bonus damage.  Air elemental get a
        // 20% bonus.
        if(IS_DROW(ch))
            increase += 15;
        else if(IS_ELEMENTAL(ch) && GET_SUBRACE(ch) == AIR_ELEMENTAL)
            increase += 45;
        else if(GET_RACE(ch) == RACE_OGRE)
            increase += 75;
        else if(GET_RACE(ch) == RACE_SOGRE)
            increase += 105;

	// Arcanists have 1% bonus damage from spells.  Dark priests have 2%.
	if(IS_ARCANIST(ch))
            increase += GET_ADVANCE_LEVEL(ch) * 3;
        if(IS_DARK_PRIEST(ch) || IS_REVENANT(ch) || IS_DEFILER(ch))
            increase += GET_ADVANCE_LEVEL(ch) * 6;

        /* A revenant who controls no pet gets 30% bonus spell damage */
        if(IS_REVENANT(ch)) {
            for(tmp_ch = character_list; tmp_ch; tmp_ch = tmp_ch->next)
                if(tmp_ch->master && tmp_ch->master == ch && IS_NPC(tmp_ch) && IS_AFFECTED(tmp_ch, AFF_CHARM))
                {
                    foundPet = TRUE;
                    break;
                }

            if(!foundPet)
                increase += 90;
        }

        if(ch->affected){
            struct affected_type *aff;

            for( aff = ch->affected; aff; aff = aff->next ){
                if(aff->location == APPLY_SPELL_DAMAGE)
                    increase += aff->modifier;
            }
        }

       // if(affected_by_spell(ch, SPELL_AWAKEN))
       //     increase += 45;
       // if(affected_by_spell(ch, SKILL_SHADOWFORM))
       //     increase += 45;
       // if(affected_by_spell(ch, SKILL_ADUMBRATION))
       //     increase += spell_level(ch, SKILL_ADUMBRATION);

	return increase;
}


/* This function handles the any affects from the victims ac or spells on damage */
int damage_affects(CharData *ch, CharData *victim,
			  int dam, int type)
{
    int ac_dam_reduction;
    CharData *tch, *next_tch;

    if (dam <= 0) return 0; /* either missed, or perhaps screwy damage */

    //  Remove a slew of spells which shouldn't be on players during combat.
    affect_from_char(victim, SPELL_REGENERATE);
    affect_from_char(ch, SPELL_REGENERATE);
    affect_from_char(victim, SKILL_BLACKJACK);
    affect_from_char(ch, SKILL_PET_MEND);
    affect_from_char(victim, SKILL_PET_MEND);

    // First figure things that affect damage by exact values:

    /* If their ac is below -100, reduce weapon attack damage by 1pt for every 10 pts of ac */
    if( GET_ADVANCE_LEVEL(victim) >= SECOND_ADVANCE_SKILL &&
       (IS_DRAGOON(victim) || IS_KNIGHT_ERRANT(victim)) )
        ac_dam_reduction = number(5*(realAC(victim)/10 + 10), 0);
    else
        ac_dam_reduction = number(3*(realAC(victim)/10 + 10), 0);

    if ( (type >= TYPE_HIT) &&
	 (type < TYPE_SPECIAL) &&
	 (ac_dam_reduction < 0) )
	dam = MAX(1, (dam + ac_dam_reduction));

    // add in flaming weapon damage
    if (dam > 0 && IS_AFFECTED(ch, AFF_FLAME_BLADE)) {
        // make sure the damage is a weapon type, not a skill or spell
        if (type >= TYPE_HIT && type < TYPE_SPECIAL) {
            dam += dice(1, aff_level(ch, AFF_FLAME_BLADE) / 10);
            //type = TYPE_BURN;  This is applied later.
        }
    }

    if(dam > 0 && affected_by_spell(ch, SKILL_METAMORPHOSIS))
        while(percentSuccess(affected_by_spell(ch, SPELL_HASTE)?33:50))
            dam += spell_level(ch, SKILL_METAMORPHOSIS);

    // Many advancement trees improve the damage of your pet
    if(ch->master)
        dam += bonusPetDamage(ch, victim, dam, type);

    // Effects only applied to spells:
    if ( dam > 0 && type <= MAX_SPELLS && type > 0) {
        dam += (int)((dam * (spell_damage_gear(ch) / 3))/100);
        
        if(IS_ARCANIST(ch) && percentSuccess(GET_ADVANCE_LEVEL(ch))) {
            act("&06$n's fingers crackle with magic as a mighty spell is released!&00", FALSE, ch, 0, victim, TO_NOTVICT);
            sendChar(ch, "&06Your fingers crackle with energy as you release a critical %s!&00\r\n", spells[type]);
            sendChar(victim, "&06%s's fingers crackle with energy with the release of a critical %s!&00\r\n", GET_NAME(ch), spells[type]);
            
            dam += dam/2;
        }
        
        // Incorporate builder-set spell damage. Craklyn
        if(IS_NPC(ch)) {
            dam += (int)((dam * (GET_MOB_SPELL_DAM(ch) - 100))/100);
            if (dam < 1)
                dam = 1;
        }

    }
    else { // Effects only applied to non-spells:
        if(GET_RACE(ch) == RACE_SDWARF && (GET_COND(ch, DRUNK) > 8))
            if(percentSuccess( (GET_COND(ch, DRUNK) - 8) / 2)) {
                act("&06$n summons the might of Ladonnis Hammerbeard!&00", FALSE, ch, 0, victim, TO_NOTVICT);
                sendChar(ch, "&06Your muscles bulge as you summon the might of Ladonnis Hammerbeard!&00\r\n");
                sendChar(victim, "&06%s's muscles bulge as %s summons the might of Ladonnis Hammerbeard!&00\r\n", GET_NAME(ch), HSSH(ch));
                
                dam += dam / 2;
            }
    }


    if (AFF_FLAGGED(victim, AFF_SHIELD) && (type <= MAX_SPELLS) && (type > 0)) {
	dam = MAX(1, ((dam << 1)/3));
	sprintf(buf, "A force field around $N crackles as $n's %s strikes it!", spells[type]);
        act(buf, FALSE, ch, 0, victim, TO_NOTVICT);
	sendChar(ch, "A force field around %s crackles as your %s strikes it!\r\n", GET_NAME(victim), spells[type]);
	sendChar(victim, "The force field around you crackles as %s's %s strikes it!\r\n", GET_NAME(ch), spells[type]);
    }

    if(IS_ELEMENTAL(victim) && GET_SUBRACE(victim) == WATER_ELEMENTAL && (type <= MAX_SPELLS) && (type > 0)) {
        dam = MAX(1, dam*85/100);
    }

    if(affected_by_spell(victim, SKILL_PHASESHIFT) && (type <= MAX_SPELLS) && (type > 0)
            && percentSuccess(80)) {
        sendChar(victim, "&06The magic distorts as it enters your field.&00\r\n");
        dam >>= 1;
    }

    if (IS_ELEMENTAL(victim) && GET_SUBRACE(victim) == EARTH_ELEMENTAL &&
         type >= TYPE_HIT && type < TYPE_SPECIAL && dam>0)
        dam = MAX(1, dam * 95 / 100);

    if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
        dam >>= 1; /* 1/2 damage when sanctuary */
    }
    else if ( (IS_AFFECTED(victim, AFF_PROTECT_GOOD) && IS_GOOD(ch) ) ||
              (IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch) ) )
    {
       int attacker_dam; 

       dam          = MAX(1, ((dam << 1)/3));   /* 2/3 damage to victim */
       attacker_dam = MAX(1,(dam >> 2));        /* 1/6 damage to attacker */

       if (type != TYPE_ASSISTANT && type != SKILL_SHADOWBOX) {
         send_to_char ("A shooting pain runs up your arm!\r\n", ch );
         GET_HIT(ch) -= attacker_dam;

         /* 
          * Can't actually kill someone this way... doesn't work. 
          * This is an issue when more than one person is attacking.
          */
         GET_HIT(ch) = MAX(1, GET_HIT(ch));
         update_pos(ch);
       }
    }
    else if ( IS_AFFECTED(victim, AFF_WARD) )
    {
      dam = ((3 * dam)/4);   /* Damage reduced 25% */
    }
    /* Reduces damage after the normal protective spells. */
    //if (affected_by_spell(victim, SPELL_STONESKIN) && (type > MAX_SPELLS))
	//dam = MAX(1, dam - 3);
    if (affected_by_spell(victim, SPELL_BARKSKIN) && (type > MAX_SPELLS))
	dam = MAX(1, dam - 2);

    /* Wraithform will reduce physical damage you do */
    if (IS_AFFECTED(ch, AFF_WRAITHFORM)) {
        if (type > MAX_SPELLS) {
            dam -= (dam * (60 - aff_level(ch, AFF_WRAITHFORM))) / 100;
        }
    }
    
    if (SINGING(ch) == SKILL_BERSERK && (ch->state == BERSERK2 || ch->state == BERSERK3) )
    {
        dam *= 2;
        if ( !CAN_SEE(ch, victim) && percentSuccess(75))
        {
            send_to_char("You fumble about blindly and lose track of your enemy.\r\n", ch);
            act("$n attacks blindly and completely misses the strike.", TRUE, ch, 0, 0, TO_ROOM);
            return 0;
        }
        // Sorry for the magic number, but this keeps
        // things from just getting silly
	if(IS_NPC(ch))
            dam = MIN(501, dam);
    }

    /* debilitate increases all damage taken */
    if (affected_by_spell(victim, SPELL_DEBILITATE)) {
        dam += (dam * spell_level(victim, SPELL_DEBILITATE)) / 200;
    }
    
    if(affected_by_spell(victim, SPELL_HARM))
        dam += (dam * spell_level(victim, SPELL_HARM))/100;

    if (affected_by_spell(victim, SKILL_STEADFASTNESS)) {
        dam = 100 * dam / number(105, 112);
        if(IS_DEFENDER(victim) && GET_ADVANCE_LEVEL(victim) >= SECOND_ADVANCE_SKILL && percentSuccess(8))
            dam /= 2;
    }

    if (affected_by_spell(victim, SKILL_REDOUBT)) {
	dam /= number(1, 4);
    }

    if(affected_by_spell(ch, SPELL_HIPPOCRATIC_OATH))
        dam /= 2;

    if (affected_by_spell(victim, SKILL_NM_SACRIFICE)) {
        dam = dam - SACRIFICE(victim);
        
        send_to_char( "Your &07dark aura&00 protects you from harm.\n\r", victim );
        sprintf(buf, "%s's &07dark aura&00 reacts to protect %e from harm.",
                GET_NAME(victim), HSSH(victim) );
        act(buf, FALSE, victim, 0, 0, TO_ROOM);
        
        if(dam < 0)
            SACRIFICE(victim) = -dam;
        else
            affect_from_char(victim, SKILL_NM_SACRIFICE);
    }
    
    // Random crits against the lochaber wielder 12% of the time
    if (isUsingLochaber(victim) && percentSuccess(12))
	dam += (int)(2*dam);

    // Damage reduced in necropolis for non-necros
    if(IN_NECROPOLIS(ch) && dam > 0) {
       if( !IS_NPC(ch) && GET_CLASS(ch) != CLASS_NECROMANCER )
          dam /= 5;
       else if (IS_NPC(ch) && ch->master && !IS_NPC(ch->master) &&
                GET_CLASS(ch->master) != CLASS_NECROMANCER)
          dam /= 8;

       dam = MAX(1, dam);
    }

    if(IS_CHI_WARRIOR(ch) && GET_ADVANCE_LEVEL(ch) >= 5) {
        for( tch = world[ch->in_room].people; tch; tch = next_tch ){
            next_tch = tch->next_in_room;

            if(FIGHTING(tch) && chGroupedWithTarget(FIGHTING(tch), ch) && tch != FIGHTING(ch)) {
                GET_HIT(tch) = MAX(1, GET_HIT(tch) - dam/10);
                sendChar(tch, "You are burned by %s's chiwaves.\r\n", GET_NAME(ch));
                sendChar(ch, "You burn %s with your chiwaves.\r\n", GET_NAME(tch));
            }
        }
    }

    dam = MAX(1, dam);
    return dam;
}

int bonusPetDamage(CharData *ch, CharData *victim, int dam, int attack_type) {
	// This function returns how much damage is increased over previous amount.	
	int increase = 0;

	if(affected_by_spell(ch, SPELL_CALL_OF_THE_WILD) && ch->master) {
		if(IS_HUNTER(ch->master)) {
                    increase += dam * GET_ADVANCE_LEVEL(ch->master)/ 15;
                    if(GET_ADVANCE_LEVEL(ch->master)>= SECOND_ADVANCE_SKILL && percentSuccess(10)) {
			act("&01$n lets loose a ferocious attack on $N!&00", FALSE, ch, 0, victim, TO_NOTVICT);
			sendChar(ch, "&01You let loose a ferocious attack against %s!&00\r\n", GET_NAME(victim));
			sendChar(victim, "&01%s attacks you ferociously!&00\r\n", GET_NAME(ch));
			increase += dam * number(2, 3);
                    }
                }
                else if(IS_NATURALIST(ch->master) && GET_ADVANCE_LEVEL(ch->master) >= FIRST_ADVANCE_SKILL &&
                        !affected_by_spell(victim, SKILL_COMMANDING_SHOUT) &&
                        percentSuccess(4) ) {
                    add_affect( ch, victim, SKILL_COMMANDING_SHOUT, GET_LEVEL(victim), 0,
                            0, 5, 0, FALSE, FALSE, FALSE, FALSE);
                    act("&01$n lets out a feral howl!&00", FALSE, ch, 0, victim, TO_NOTVICT);
                    sendChar(ch, "&01You loose a feral howl!&00\r\n");
                    sendChar(victim, "&01%s looses a feral howl, leaving you confounded!&00\r\n", GET_NAME(ch));
		}
	}
	else if (affected_by_spell(ch, SPELL_GATE_MAJOR_DEMON) && ch->master && IS_ENCHANTER(ch->master))
		increase += GET_ADVANCE_LEVEL(ch->master)*dam / 100;

	return increase;
}


/*
** This function handles all limitations on damage.
*/
int
damage_limit( CharData *ch,
              CharData *victim, 
              int dam, int attacktype)
{
  int chance;
  // Assassinate skill check.
  if((BACKSTAB(ch, 0, attacktype)) &&
       skillSuccess(ch, SKILL_ASSASSINATION) &&
      (percentSuccess( MAX( 5, GET_LEVEL(ch) - GET_LEVEL(victim) + 
	  (attacktype == SKILL_BACKSTAB ? 40 : 10) + (IS_BOUNTY_HUNTER(ch) ? GET_ADVANCE_LEVEL(ch) : 0)))
          || !AWAKE(victim) || IS_AFFECTED(victim, AFF_PARALYZE)))
  {
      dam = MAX(dam, 0); /* No cap at all! */
      sendChar( ch, "You strike deep into %s's vitals!\r\n", GET_NAME(victim));
      if(!IS_NPC(ch))
          advanceSkill( ch, SKILL_ASSASSINATION, 90,
                  "You feel like a master in the art of death.\r\n", FALSE, FALSE, FALSE );

      if (attacktype != SKILL_BACKSTAB)
          chance = 10;
      else
          chance = 33;
      
      if (skillSuccess(ch, SKILL_CRITICAL_HIT) && percentSuccess(chance)) {
          sendChar(ch, "Your weapon finds a critical nerve connection!\r\n");
          advanceSkill(ch, SKILL_CRITICAL_HIT, 90,
          "Your abilities with your weapon grow!\r\n",
          FALSE, FALSE, FALSE);
          dam *= 5;
      }
  }
  else if(BACKSTAB(ch, 0, attacktype) && IS_BUTCHER(ch) && skillSuccess(ch, SKILL_ASSASSINATION))
  {
      if(dam > MAX_DAMAGE)
          dam = MAX(number(MAX_DAMAGE, number(MAX_DAMAGE, dam)), 0);
  }
  else if(attacktype == SKILL_SHOOT)
  {
      dam = MAX(dam, 0); /* No cap at all! */
  }
  else if( attacktype > MAX_SPELLS && attacktype < TYPE_SPECIAL &&
         ( GET_LEVEL(ch) < LVL_IMMORT )) /* spells not capped! */
  {
    dam = MAX(MIN(dam, MAX_DAMAGE), 0);
  }

  // 1/2 damage with shunned attackers and double on shunned victims.
  //
  dam = ( PLR_FLAGGED( ch,     PLR_SHUNNED ) ? dam>>1 : dam );
  dam = ( PLR_FLAGGED( victim, PLR_SHUNNED ) ? dam<<1 : dam );

  // Imms are impervious unless in arena.
  //
  if( !IS_NPC(victim) && GET_LEVEL(victim) >= LVL_IMMORT && !IN_ARENA(victim))
      dam = 0;
  
  return dam;
}

/* ============================================================================ 
Vex. Split up damage function into smaller functions to make it a bit
easier to understand and modify. '98
============================================================================ */
// Is it ok for ch to damage victim?
// Return 1 if it is, 0 otherwise.
// called from damage
int
ok_damage_victim( CharData *ch, CharData *victim)
{
  if(!victim) 
      return 0;

//  if(GET_POS(victim) <= POS_DEAD)
//      return 0;
  
  if(ch->in_room != victim->in_room) 
      return 0;

  if(!ok_damage_shopkeeper(ch, victim)) 
      return 0;

  return 1;
}

// ch is fighting victim, make any neccessary combat checks.
// return 1 if combat can continue, 0 otherwise.
// called from damage.
int
setup_combat( CharData *ch, CharData *victim)
{
    if (victim == ch) { // function used incorrectly.
	mudlog(NRM, LVL_IMMORT, TRUE, "SETUP_COMBAT: fatal error -> attempting to set ch fighting himself!");
	exit(1);
    }

    if( GET_POS(ch) > POS_STUNNED )
    {
      if( !(FIGHTING(ch)) )
          set_fighting(ch, victim);

      // Will mob being hit switch from charmed mob to its master?
      if( IS_NPC(ch) && IS_NPC(victim) && victim->master &&
         !number(0, 100) && IS_AFFECTED(victim, AFF_CHARM) &&
         (victim->master->in_room == ch->in_room))
      {
        if( FIGHTING(ch) )
          stop_fighting(ch);
        hit(ch, victim->master, TYPE_UNDEFINED);
        return 0; // circumstances have changed, bail out.
      }

      // If a charmed mob is fighting w/o master present, it will
      // be inclined to run away.
      if( FIGHTING(victim) && IS_AFFECTED(victim, AFF_CHARM) &&
          (victim->master->in_room != victim->in_room) && number(0, 1))
      {
          /* A master just lost control of their charms */
        do_flee(victim, "", 0, 0);
      }
    }
    if( !FIGHTING(victim) )
      player_attack_victim(ch, victim);
    if( GET_POS(victim) > POS_STUNNED && !FIGHTING(victim))
    {
      set_fighting(victim, ch);
      // Remember who was responsible for attacking us.
      if (MOB_FLAGGED(victim, MOB_MEMORY) && (!IS_NPC(ch) || PC_CONTROLLED(ch)) && (GET_LEVEL(ch) < LVL_IMMORT))
      {
        if (PC_CONTROLLED(ch))
          remember(victim, ch->master);
        else
          remember(victim, ch);
      }
    }
    return 1; // Nothing unusual happened, continue processing.
}

// We are about to have ch start hurting victim, this function
// deals with the immediate consequences prior to any damage
// literally occuring.
void
damage_begin( CharData *ch, CharData *victim)
{
  if( victim->master == ch )
  {
    stop_follower(victim);
  }

  if(( IS_AFFECTED(ch, AFF_INVISIBLE) || (IS_AFFECTED(ch, AFF_HIDE)
          && !affected_by_spell(ch, SPELL_DANCE_SHADOWS))))
  {
    appear(ch);
  }

  // attacker loses weapon if its too heavy.
  //
  if( !IS_NPC(ch) && ch->equipment[WEAR_WIELD] && 
      (GET_OBJ_WEIGHT(ch->equipment[WEAR_WIELD]) >
       str_app[STRENGTH_APPLY_INDEX(ch)].wield_w))
  {
    obj_to_char(unequip_char(ch, WEAR_WIELD), ch);
    sendChar( ch, "You remove your weapon, finding it too heavy to wield.\n\r" );
  }
}


/* 
 * Liam Feb 15, 1995
 *
 * The idea is that mobs should react to the people that are causing them
 * the most damage during a fight.  We keep track of both "standard" 
 * weapon attacks, and of "spell" type attacks.  Spell casting mobs will
 * target whoever is doing them the most damage, and "warrior" type mobs
 * will target BASH and other stunning attacks against the spellcasters
 * in the group.  This is going to make players REALLY unhappy, I imagine.
 * But it is certainly more realistic.
*/
void
remember_what_hurts( CharData *ch, CharData *victim, int dam, int attacktype)
{
    if( IS_NPC( victim ) && !IS_WEAPON( attacktype )) {
        /* We're being damaged by a SPELL */
        if( attacktype < MAX_SPELLS ){
            if( dam > victim->mob_specials.max_spell_damage ) {
                victim->mob_specials.max_spell_damage = dam;
                victim->mob_specials.high_spellcaster = ch;
            }
            else {
              /* It's a skill -- don't do anything yet -- might wanna
               * check for bash here -- spellcasters might want to pick
               * on the Wa that is preventing them from casting...
               */
              ;
            }
        }
      /* 
       * We update the "straight attacks in hit(), so we can keep track
       * of "total" damage (ie. due to multiple attacks
       */
    }
}

void performAutoknight(CharData *ch, CharData *vict, const int dam) {
	// Remember, victim is the guy getting hit.  
	int chance = 0;
	
	if(GET_ADVANCE_LEVEL(vict) < FIRST_ADVANCE_SKILL || !(IS_KNIGHT_ERRANT(vict) || IS_DRAGOON(vict)))
		return;

	if(GET_HIT(vict) != 0)
		chance = 500*dam/GET_HIT(vict);

	// We evaluate it twice to reduce the standard deviation without reducing some overall chance.
	if(chance > number(1, 700) && chance > number(1, 700)) {
		if(IS_KNIGHT_ERRANT(vict)) {
			act("&07The lichcurse envelops you, protecting you temporarily from death!&00", FALSE, ch, 0, vict, TO_VICT);
			act("&07$n is enveloped in an evil shroud of life-sustaining energy!&00", FALSE, vict, 0, ch, TO_ROOM);
                    add_affect( vict, vict, SKILL_INIQUITY, GET_LEVEL(vict), 0, 0, 6, 0,
                                    FALSE, FALSE, FALSE, FALSE);
		}
		else if(IS_DRAGOON(vict)) {
			act("&14White wings sprout from $N's back.  You are now fighting a divine host!&00", FALSE, ch, 0, vict, TO_CHAR);
			act("&14You feel emcompassed in protective forces.  Everything is going to be alright!&00", FALSE, ch, 0, vict, TO_VICT);
			act("&14$N is enveloped in a white glow!&00", FALSE, ch, 0, vict, TO_NOTVICT);
                        int hp = MIN(number(dam/3, dam), 120);

			add_affect( vict, vict, SKILL_GUARDIAN_ANGEL, GET_LEVEL(vict),
                                APPLY_HIT, hp, 6, 0, FALSE, FALSE, FALSE, FALSE);
                        GET_HIT(ch) += number(0, hp);
		}
	}

}


// Show messages to characters in the mud to let them know what just
// occurred. Also update character positions as appropriate.
/*
 * skill_message sends a message from the messages file in lib/misc.
 * dam_message just sends a generic "You hit $n extremely hard.".
 * skill_message is preferable to dam_message because it is more
 * descriptive.
 * 
 * If we are _not_ attacking with a weapon (i.e. a spell), always use
 * skill_message. If we are attacking with a weapon: If this is a miss or a
 * death blow, send a skill_message if one exists; if not, default to a
 * dam_message. Otherwise, always send a dam_message.
*/
void
damage_display( CharData *ch, CharData *victim, int dam, int attacktype)
{
  char damb[20];
  sprintf(damb, "(%d) ", dam);

  // The following will tell players how much damage they're doing.
  if ( dam > 0 && affected_by_spell(ch, SKILL_SHOWDAM) || 
	  (GET_LEVEL(ch) > MAX_MORTAL || IS_PVP(ch, victim) || IN_ARENA(ch) || ZONE_FLAGGED(world[ch->in_room].zone, ZONE_ARENA))  &&
	  (PLR_IS_EXPLORER(ch) || GET_LEVEL(ch) > MAX_MORTAL) &&
	  PRF_FLAGGED(ch, PRF_SHOWDAM))
	  send_to_char(damb, ch);
  if ( dam > 0 && affected_by_spell(ch, SKILL_SHOWDAM) || 
	  (GET_LEVEL(victim) > MAX_MORTAL || IS_PVP(victim, ch) ||IN_ARENA(victim) || ZONE_FLAGGED(world[victim->in_room].zone, ZONE_ARENA)) &&
	  (PLR_IS_EXPLORER(victim) || GET_LEVEL(victim) > MAX_MORTAL) &&
	  PRF_FLAGGED(victim, PRF_SHOWDAM) )
	  send_to_char(damb, victim);

  if( IS_PVP(ch, victim) )
      dam *= pvpFactor();

    update_pos(victim);
    if (attacktype == SKILL_CIRCLE || attacktype == SKILL_SHADOW_DANCE)
	attacktype = SKILL_BACKSTAB; /* so we use right messages */

    if (!IS_WEAPON(attacktype) && attacktype != TYPE_ASSISTANT
        && attacktype != SKILL_SHADOWBOX)
        skill_message(dam, ch, victim, attacktype);
    else {
        if (GET_POS(victim) == POS_DEAD || dam == 0) {
            if (!skill_message(dam, ch, victim, attacktype))
                dam_message(dam, ch, victim, attacktype);
        }
        else /* Check for attacktype >= TYPE_SPECIAL ? */
            dam_message(dam, ch, victim, attacktype);
    }

  /* Use send_to_char -- act() doesn't send message if you are DEAD. */
  switch (GET_POS(victim)) {
  case POS_MORTALLYW:
    act("$n is mortally wounded, and will die soon, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("You are mortally wounded, and will die soon, if not aided.\r\n", victim);
    break;
  case POS_INCAP:
    act("$n is incapacitated and will slowly die, if not aided.", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("You are incapacitated and will slowly die, if not aided.\r\n", victim);
    break;
  case POS_STUNNED:
    act("$n is stunned, but will probably regain consciousness again.", TRUE, victim, 0, 0, TO_ROOM);
    send_to_char("You're stunned, but will probably regain consciousness again.\r\n", victim);
    break;
  case POS_DEAD:
      if(pvpHoliday(victim) && !IN_ARENA(victim) && GET_HIT(victim) <= -10) {
          GET_HIT(victim) = -6;
          GET_POS(victim) = POS_MORTALLYW;
          stop_fighting(ch);
          stop_fighting(victim);
      }
      else {
          act("$n is dead!  R.I.P.", FALSE, victim, 0, 0, TO_ROOM);
          send_to_char("You are dead!  Sorry...\r\n", victim);
      }
    break;

  default:			/* >= POSITION SLEEPING */
    if (dam > (GET_MAX_HIT(victim) >> 2)) {
        act("That really did HURT!", FALSE, victim, 0, 0, TO_CHAR);

    }
    performAutoknight(ch, victim, dam);
    if (GET_HIT(victim) < (GET_MAX_HIT(victim) >> 2)) {
      sprintf(buf2, "%sYou wish that your wounds would stop BLEEDING so much!%s\r\n",
	      CCRED(victim, C_SPR), CCNRM(victim, C_SPR));
      send_to_char(buf2, victim);
      if (MOB_FLAGGED(victim, MOB_WIMPY))
	do_flee(victim, "", 0, 0);
    }
    if (!IS_NPC(victim) && GET_WIMP_LEV(victim) && victim != ch &&
	GET_HIT(victim) < GET_WIMP_LEV(victim) && !IS_AFFECTED(ch, AFF_BERSERK)
        && GET_POS(victim) >= POS_SLEEPING) {
      send_to_char("You wimp out, and attempt to flee!\r\n", victim);
      do_flee(victim, "", 0, 0);
    }
    break;
  } /* switch */
}


static void perform_autoloot(CharData *ch, ObjData *corpse)
{
  int lootobj = 0, lootgold = 0;
  ObjData *obj, *next;
  char gold_buf[30];
  long coins;

  if(!ch) {
      mudlog(BRF, LVL_GOD, TRUE, "(SYSERR): perform_autoloot was called, but there is no player!");
      return;
  }

  // mobs don't autoloot
  if (IS_NPC(ch)) return;

  if(!corpse) {
      mudlog(BRF, LVL_GOD, TRUE, "(SYSERR): %s tried to autoloot, but there is no corpse!", GET_NAME(ch));
      return;
  }

  // if we can't see the corpse, we're not going to paw through it
  if (!CAN_SEE_OBJ(ch, corpse)) return;

  // if it's a player corpse, we're not going to auto loot it
  if (!CAN_WEAR(corpse, ITEM_WEAR_TAKE)) return;

  // Can't loot if you're sleeping (or worse)
  if (GET_POS(ch) <= POS_SLEEPING) return;

  lootgold = PRF_FLAGGED(ch, PRF_AUTOGOLD);
  lootobj = PRF_FLAGGED(ch, PRF_AUTOLOOT);

  // if you've got a group leader ...
  if (IS_AFFECTED(ch, AFF_GROUP) && ch->master
    && IS_AFFECTED(ch->master, AFF_GROUP)
    && ch->in_room == ch->master->in_room) {
      perform_autoloot(ch->master, corpse);     // they get to loot first
      lootobj = 0;                              // and you can't loot objects
  }

  // rifle through the corpse, grabbing things
  coins = GET_GOLD(ch);
  for (obj = corpse->contains; obj; obj = next) {
    next = obj->next_content;
    if (lootobj || (lootgold && GET_OBJ_TYPE(obj) == ITEM_MONEY))
      perform_get_from_container(ch, obj, corpse, FIND_OBJ_ROOM);
    //else if (!lootobj && ch->master && PRF_FLAGGED(ch->master, PRF_AUTOLOOT))
      //perform_get_from_container(ch->master, obj, corpse, FIND_OBJ_ROOM);
  }
  coins = GET_GOLD(ch) - coins;

  sprintf(gold_buf, "%ld", coins);
      
  if (IS_AFFECTED(ch, AFF_GROUP) && PRF_FLAGGED(ch, PRF_AUTOSPLIT))
    if (coins >= 2) do_split(ch, gold_buf, 0, 0);
}

void perform_thecards(CharData *ch, CharData *victim)
{
	ObjData *final_scroll;
	struct extra_descr_data *new_descr;
	int spellnum = 0;

	spellnum = choose_card(ch, victim);

	if ((spellnum < 1) || (spellnum > MAX_SPELLS)) {
            send_to_char("A terrible error has occured with The Cards.  Please report to an immortal.\r\n", ch);
            return;
        }

	sprintf(buf, "You create a card of death.\r\n");
	send_to_char(buf, ch);
	act("$n picks up a small card.", FALSE, ch, 0, 0, TO_ROOM);

	final_scroll = create_obj();

	final_scroll->item_number = NOTHING;
	final_scroll->in_room = NOWHERE;
	sprintf(buf2, "thecard%s scythe card", GET_NAME(ch));
	final_scroll->name = str_dup(buf2);

	sprintf(buf2, "A small card lies here.");
	final_scroll->description = str_dup(buf2);

	sprintf(buf2, "a card");
	final_scroll->short_description = str_dup(buf2);

	/* extra description coolness! */
	CREATE(new_descr, struct extra_descr_data, 1);
	new_descr->keyword = str_dup(final_scroll->name);
	sprintf(buf2, "It is small and rectangular.  On the back is an image of two crossed scythes.");
	new_descr->description = str_dup(buf2);
	new_descr->next = NULL;
	final_scroll->ex_description = new_descr;
 
	GET_OBJ_TYPE(final_scroll) = ITEM_SCROLL;
	SET_BIT_AR(GET_OBJ_WEAR(final_scroll), ITEM_WEAR_TAKE);
	SET_BIT_AR(GET_OBJ_EXTRA(final_scroll), ITEM_NORENT);
        SET_BIT_AR(GET_OBJ_EXTRA(final_scroll), ITEM_TIMED);
	SET_BIT_AR(GET_OBJ_EXTRA(final_scroll), ITEM_NOSELL);
        SET_BIT_AR(GET_OBJ_EXTRA(final_scroll), ITEM_ARTIFACT);
	GET_OBJ_VAL(final_scroll, 0) = GET_LEVEL(ch);
	GET_OBJ_VAL(final_scroll, 1) = spellnum;
	GET_OBJ_VAL(final_scroll, 2) = 0;
	GET_OBJ_VAL(final_scroll, 3) = 0;
	GET_OBJ_COST(final_scroll) = GET_LEVEL(ch) * 100;
	GET_OBJ_WEIGHT(final_scroll) = 1;
	GET_OBJ_RENT(final_scroll) = 0;
	GET_OBJ_TIMER(final_scroll)  = number(2,5);
	
	obj_to_char(final_scroll, ch);
}

void ch_kill_victim( CharData *ch, CharData *victim)
{
    int pkill = FALSE;

    if( (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM)) && !IS_NPC(victim) )
	pkill = TRUE;

    // If it's an arena holiday and outside the arena, do not kill the player.
    if((pvpHoliday(ch) || pvpHoliday(victim)) && !IN_ARENA(victim))
        return;

    if( IS_DEFILER(ch) &&  GET_ADVANCE_LEVEL(ch) >= 3 && GET_MAX_HIT(victim) > number(1, number(1, 7500)))
        perform_thecards(ch, victim);

    if(IS_NPC(victim) && affected_by_spell(victim, SPELL_ZOMBIE_REWARD)) {
        int reward = number(1, 5);
        if(reward == 1) {
            sendChar(ch, "As you drain the last vestage of life out of the zombie, you feel empowered!\r\n");
            add_affect(ch, ch, SPELL_ZOMBIE_REWARD, GET_LEVEL(ch), APPLY_DAMROLL, number(1, GET_LEVEL(ch)/3), number(0, 4 TICKS), FALSE, FALSE, FALSE, FALSE, FALSE);
        }
        else if(reward == 2) {
            sendChar(ch, "As you drain the last vestage of life out of the zombie, you feel embiggened!\r\n");
            add_affect(ch, ch, SPELL_ZOMBIE_REWARD, GET_LEVEL(ch), APPLY_HIT, number(1, GET_LEVEL(ch)*3), number(0, 4 TICKS), FALSE, FALSE, FALSE, FALSE, FALSE);
        }
        else if(reward == 3) {
            sendChar(ch, "As you drain the last vestage of life out of the zombie, you feel overcome with arcane insight!\r\n");
            add_affect(ch, ch, SPELL_ZOMBIE_REWARD, GET_LEVEL(ch), APPLY_MANA, number(1, GET_LEVEL(ch)*3), number(0, 4 TICKS), FALSE, FALSE, FALSE, FALSE, FALSE);
        }
        else if(reward == 4) {
            sendChar(ch, "As you drain the last vestage of life out of the zombie, you feel overcome with arcane power!\r\n");
            add_affect(ch, ch, SPELL_ZOMBIE_REWARD, GET_LEVEL(ch), APPLY_SPELL_DAMAGE, number(1, GET_LEVEL(ch)), number(0, 4 TICKS), FALSE, FALSE, FALSE, FALSE, FALSE);
        }
        
    }

    resolve_death( ch, victim, pkill );

    if( IN_QUEST_FIELD(victim) &&
      ( PRF_FLAGGED(victim, PRF_GOLD_TEAM) ||
        PRF_FLAGGED(victim, PRF_BLACK_TEAM)))
    {
      gain_exp( victim, xp_gain(NULL, victim, pkill, TRUE) ); 
      jail_char(victim, FALSE);
    }
    else
    {
      ObjData *corpse = NULL;

      /* Log immortal mob kills... *mutter* */
      if (IS_NPC(victim) && !IS_NPC(ch) && (GET_LEVEL(ch) >= LVL_IMMORT) && (GET_LEVEL(ch) < LVL_GOD))
      {
          mudlog(BRF, LVL_GOD, TRUE, "(GODKILL): %s killed %s", GET_NAME(ch), GET_NAME(victim));
      }

      if(victim->in_room != NOWHERE)
          corpse = die(victim, ch, pkill);

      /* Mortius: Handle all of the auto looting/splitting */
      /* Imhotep: Shuffle this elsewhere to do more checks */
      /* Imhotep: If you killed yourself, leave your corpse alone */
      if(corpse && victim != ch &&
              !IN_ARENA(ch) &&
              !ZONE_FLAGGED(world[ch->in_room].zone, ZONE_ARENA) &&
              !ZONE_FLAGGED(world[ch->in_room].zone, ZONE_SLEEPTAG) &&
              !IN_QUEST_FIELD(ch))
          perform_autoloot(ch, corpse);
	  
    }
}

static int choose_card(CharData *ch, CharData *victim)
{

	if(GET_MAX_HIT(victim) > 30000) {
		if(percentSuccess(25))
			return SPELL_DOOM_BOLT;
		else if(percentSuccess(15))
			return SPELL_PARALYZE;
		else if(percentSuccess(15))
			return SPELL_TELEPORT;
		else if(percentSuccess(10))
			return SPELL_SACRIFICE;
		else if(percentSuccess(10))
			return SPELL_WRATH_OF_THE_ANCIENTS;
	}

	if(GET_MAX_HIT(victim)> 15000) {
		if(percentSuccess(25))
			return SPELL_DISEASE;
		else if (percentSuccess(10))
			return SPELL_NOXIOUS_SKIN;
		else if (percentSuccess(10))
			return SPELL_TREMOR;
	}
	if(percentSuccess(15)) 
		return SPELL_DEATH_TOUCH;
	else if(percentSuccess(15))
		return SPELL_BLACK_BREATH;
	else if(percentSuccess(15))
		return SPELL_PESTILENCE;
	else if(percentSuccess(15))
		return SPELL_UNHOLY_WORD;
	else if(percentSuccess(15))
		return SPELL_BLINDNESS;
	else if(percentSuccess(15))
		return SPELL_BLACK_DART;
	else if(percentSuccess(15))
		return SPELL_CAUSE_CRITIC;
	else if(percentSuccess(15))
		return SPELL_CAUSE_WOUND;
	else return SPELL_DEATH_TOUCH;	

}


void
protect_unlinked_victim(CharData *victim)
{
  if( !IS_NPC(victim) &&
      !(victim->desc) &&
      !IS_SET_AR(PLR_FLAGS(victim), PLR_HUNTED))
  {
    do_flee(victim, "", 0, 0);
    if (!FIGHTING(victim))
    {
      act("$n is rescued by divine forces.", FALSE, victim, 0, 0, TO_ROOM);
      GET_WAS_IN(victim) = victim->in_room;
      char_from_room(victim);
      char_to_room(victim, 0);
    }
  }
}

void
weaponAffect( CharData *ch,
              CharData *victim,
              int dam, int damtype)
{
#define NO_APPLY APPLY_NONE
#define NO_BIT   0
#define NO_MOD   0
    ObjData *weapon;
    int chance = 100;
    int aff_val;
    int lev;

    if( !WIELDING(ch) ) return;
    if( dam == 0 ) return;

    weapon = WIELDING(ch);

    /* lower chance based on attack type */
    if (!IS_BACKSTAB(damtype)) {
        chance = 75;
        if (damtype != SKILL_HAMSTRING && damtype != SKILL_CUT_THROAT) {
            chance = 40;
            /* if it was any other damage type, bail now */
            if (damtype != (weapon->obj_flags.value[3] + TYPE_HIT)) return;
        }
    }

    for (aff_val = 0; aff_val < MAX_OBJ_AFFECT; aff_val++) {
        /* if the weapon is envenomed in this slot */
        if (weapon->affected[aff_val].location == APPLY_POISON) {
            lev = weapon->affected[aff_val].modifier;

            /* remove the envenom charge */
            weapon->affected[aff_val].location = APPLY_NONE;
            weapon->affected[aff_val].modifier = 0;

            /* test the chance level */
            if (number(1,100) > chance) continue;

            /* You cannot envenom a weapon for your alts */
            if(!has_owner(weapon))
                continue;

            /* Poison first */
            if (lev >= 35 && !magic_savingthrow(ch, victim, SAVING_PARA)) {
                if (!affected_by_spell(victim, SPELL_POISON)) {
                    add_affect( NULL, victim, SPELL_POISON, 0, APPLY_STR, -2,
                            4 TICKS, AFF_POISON, FALSE, FALSE, FALSE, FALSE);
                    act("You suddenly feel ill!", FALSE, victim, 0, 0,
                            TO_CHAR);
                    act("$n turns pale and looks deathly sick!", FALSE,
                            victim, 0, 0, TO_ROOM);
                }
            }

            /* Blindness next */
            if (lev >= 43 && !magic_savingthrow(ch, victim, SAVING_PARA) &&
              !(IS_NPC(victim) && IS_SET_AR(MOB_FLAGS(victim), MOB_NOBLIND))) {
                if (!affected_by_spell(victim, SPELL_BLINDNESS)) {
                    add_affect( ch, victim, SPELL_BLINDNESS, 0, NO_APPLY,
                            NO_MOD, 2 TICKS, AFF_BLIND, FALSE, FALSE, FALSE, FALSE);
                    act("Your vision fades to black.", FALSE, victim, 0, 0,
                            TO_CHAR);
                    act("$n seems to be blinded!", FALSE, victim, 0, 0,
                            TO_ROOM);
                 }
            }

            /* Paralyze finally */
            if (lev >= 48 && GET_HIT(victim) <= lev * 100 &&
                    !magic_savingthrow(ch, victim, SAVING_PARA)) {
                if (!affected_by_spell(victim, SPELL_PARALYZE)) {
                    add_affect( ch, victim, SPELL_PARALYZE, 0, APPLY_AC, 20,
                            1 TICKS, AFF_PARALYZE, FALSE, FALSE, FALSE, FALSE);
                    act("Your limbs stiffen, immobilizing you.", FALSE,
                            victim, 0, 0, TO_CHAR);
                    act("$n's limbs stiffen, immobilizing $m.", FALSE,
                            victim, 0, 0, TO_ROOM);
                }
            }
        }
    }
}



void
damage( CharData *ch, CharData *victim, int dam, int attacktype )
{
  int mana_shield = 0;
  int damFactor = pvpFactor();

  if( victim == NULL )
  {
    mudlog(BRF, LVL_IMMORT, TRUE, "(FIGHT.C) damage: victim is NULL.");
    return;
  }

  if( ch == NULL )
  {
    mudlog(BRF, LVL_IMMORT, TRUE, "(FIGHT.C) damage: ch is NULL.");
    return;
  }

  /*
  if( GET_POS(victim) == POS_DEAD )
  {
    mudlog(NRM, LVL_IMMORT, TRUE, "(FIGHT.c) damage: victim %s already dead.",
                    GET_NAME(victim));
    return;
  }
  */

  if( !ok_damage_victim(ch, victim) ) return;

  if( victim != ch )
  {
    if( !setup_combat(ch, victim) ) return;
  }

  if(affected_by_spell(ch, SPELL_PARANOIA) && percentSuccess(8))
      victim = ch;

  if( !CONFIG_PK_ALLOWED && !IS_NPC(victim))
  {
    if ((!IS_NPC(ch) || (IS_SET_AR(AFF_FLAGS(ch), AFF_CHARM) && ch->master &&
            !IS_NPC(ch->master))) &&
            !IN_ARENA(ch) &&
            !ZONE_FLAGGED(world[ch->in_room].zone, ZONE_ARENA) && 
            !ZONE_FLAGGED(world[ch->in_room].zone, ZONE_SLEEPTAG) &&
            !IN_QUEST_FIELD(ch)) {
      stop_fighting(ch);
      if (FIGHTING(victim) == ch)
        stop_fighting(victim);
      return;
    }
  }

  damage_begin(ch, victim);

  dam = damage_affects(ch, victim, dam, attacktype);
  
  if (dam > 1) {
      if( IS_PVP(ch, victim) && damFactor != 0)
          // Just to be sure that we're not dividing by zero.
          dam /= damFactor;
	  dam = MAX(1, dam);
  }
  
  dam = damage_limit(ch, victim, dam, attacktype);

  // Invoke special combat procedures for the attacker and/or their objects.
  //
  attackerSpecial(ch, victim, &dam, &attacktype);

  // if they set damage to under 0, this blow should not fall!
  if (dam < 0) return;

  // Victim has been hit, see if we should poison them.
  weaponAffect(ch, victim, dam, attacktype);

  if (dam > 0 && IS_AFFECTED(ch, AFF_FLAME_BLADE)) {
		// make sure the damage is a weapon type, not a skill or spell
		if (attacktype >= TYPE_HIT && attacktype < TYPE_SPECIAL) {
			attacktype = TYPE_BURN;
		}
  }
  // Now special combat procedures for the victim and/or their objects.
  //
  victimSpecial(ch, victim, &dam, &attacktype);

  remember_what_hurts(ch, victim, dam, attacktype);
  
  // Mana shield will reduce damage taken and change it into mana loss.
  if (SINGING(victim) == SPELL_SHIELD)
  {
      // mana_shield = how much damage is let through the shield
      // dam - mana_shield is the reduction in damage
      mana_shield = 60 - number(1, GET_LEVEL(victim));

      if (dam > mana_shield && number(0, 4))
          if( GET_MANA(victim) > ((dam - mana_shield)/2) )
          {
              GET_MANA(victim) -= ((dam - mana_shield)/2);
              dam = mana_shield;
              send_to_char( "Your &14mana shield&00 flares and reduced the damage you take.\n\r", victim );
              sprintf(buf, "%s's &14mana shield&00 reacts to reduce the damage %s takes.",
                      GET_NAME(victim), HSSH(victim) );
              act(buf, FALSE, victim, 0, 0, TO_ROOM);
          }
  }

  GET_HIT(victim) -= dam;

  CharData *tch, *next_tch;
  // While affected by shadowform, a cleric heals group for 10% of damage dealt.
  if (affected_by_spell(ch, SKILL_SHADOWFORM) && (attacktype <= MAX_SPELLS) && (attacktype > 0)) {
      for( tch = world[ch->in_room].people; tch; tch = next_tch ){
          next_tch = tch->next_in_room;

          if(chGroupedWithTarget(ch, tch))
              GET_HIT(tch) = MIN(GET_HIT(tch) + dam*15/100, GET_MAX_HIT(tch));
      }
  }

  if (GET_RACE(victim) == RACE_SMINOTAUR && GET_MAX_HIT(victim) != 0 &&
          20*dam/(GET_MAX_HIT(victim)) > 0) {
      add_affect(victim, victim, SKILL_RAGE, GET_LEVEL(victim), APPLY_DAMROLL,
              20*dam/(GET_MAX_HIT(victim)), 20, 0, FALSE, FALSE, FALSE, FALSE);
      add_affect(victim, victim, SKILL_RAGE, GET_LEVEL(victim), APPLY_SPELL_DAMAGE,
              40*dam/(GET_MAX_HIT(victim)), 20, 0, FALSE, FALSE, FALSE, FALSE);
  }
  
  if (IS_AFFECTED(victim, AFF_ASSISTANT) && GET_ASSIST_HP(victim) > 0) {
    GET_ASSIST_HP(victim) -= dam;
    if (GET_ASSIST_HP(victim) <= 0) {
      act("Your assistant wails horribly and vanishes!",
          TRUE, victim, 0, 0, TO_CHAR);
      act("$n's assistant wails horribly and vanishes!",
          TRUE, victim, 0, 0, TO_ROOM);
    }
  }

  damage_display(ch, victim, dam, attacktype);

  // did a poison blister just burst?
  if (!(victim->tickstate & STATE_NOXIOUS)
          && affected_by_spell(victim, SPELL_NOXIOUS_SKIN)
          && attacktype > MAX_SPELLS && dam > 0) {
      int chance = MAX(10, MIN(dam / 2, 90));

      if (percentSuccess(chance)) {
          int sp_lev = spell_level(victim, SPELL_NOXIOUS_SKIN);

          victim->tickstate |= STATE_NOXIOUS;
          
          act("A noxious blister on $n's skin bursts open!", TRUE, victim, 0, 0,
          TO_ROOM);
          act("A noxious blister on your skin bursts open!", TRUE, victim, 0, 0,
          TO_CHAR);
          
          chance = MAX(30, MIN(80 + (GET_LEVEL(victim) - GET_LEVEL(ch) * 5), 95));
          if ( GET_LEVEL(ch) > MAX_MORTAL )
              chance = 0;
          if ( IS_NPC(victim) && IS_SET_AR( MOB_FLAGS( victim ), MOB_NONOX) )
              chance = 0;

          /* Attempt a blindness */
          if (percentSuccess(chance)) {
              add_affect(victim, ch, SPELL_BLINDNESS, (sp_lev - 10),
                      NO_APPLY, 0, 1 TICKS, AFF_BLIND, FALSE, FALSE, FALSE, FALSE);
              act("Your vision fades to nothing!", TRUE, ch, 0, victim, TO_CHAR);
              act("$n looks like they're having trouble seeing clearly!", TRUE,
              ch, 0, 0, TO_ROOM);
          }

          /* Attempt a debilitate, for spell level > 35 */
          if (sp_lev > 35 && percentSuccess(chance)) {
              add_affect(victim, ch, SPELL_DEBILITATE, (sp_lev - 10),
                      APPLY_STR, -3, 2 TICKS, NO_MOD, FALSE, FALSE, FALSE, FALSE);
              act("You feel horribly weakened.", TRUE, ch, 0, victim, TO_CHAR);
              act("$n's skin takes on a gray hue.", TRUE, ch, 0, 0, TO_ROOM);
          }
          
          /* Attempt a faint, for spell level > 45 */
          if (sp_lev > 45 && percentSuccess(chance)) {
              affect_from_char(ch, SPELL_MALEDICT);
              add_affect(victim, ch, SPELL_SLEEP, sp_lev-10,
                      NO_APPLY, 0, 2 TICKS, AFF_SLEEP, FALSE, FALSE, FALSE, FALSE);
              act("You are overcome by $N's toxicity.", TRUE, ch, 0, victim, TO_CHAR);
              act("$n keels over in a dead faint.", TRUE, ch, 0, 0, TO_ROOM);
              GET_POS(ch) = POS_SLEEPING;
              end_fight(ch);
          }
      }
  }

  // see if this means an undead just used their vampiric hit
  if (IS_UNDEAD(ch)) {
      if(dam > 0 && GET_HIT(ch) < GET_MAX_HIT(ch)*12/10) {
          GET_HIT(ch) += dam/10 + (dam%10 > number(0, 9) ? 1:0);
      }
  }

  // see if they can be fortify healed this tick
  if (IS_AFFECTED(victim, AFF_FORTIFY)) {
    if (!(victim->tickstate & STATE_FORTIFIED)
            && GET_HIT(victim) < (GET_MAX_HIT(victim) / 3)) {
      victim->tickstate |= STATE_FORTIFIED;
      act("$n's wounds and weariness lessen instantly!", TRUE, victim,
          0, victim->fortifier, TO_ROOM);
      act("$N's protection lessens your wounds.", TRUE, victim, 0,
          victim->fortifier, TO_CHAR);
      if (GET_LEVEL(victim->fortifier) >= 50) {
        GET_HIT(victim) += 200;
      } else {
        GET_HIT(victim) += 100;
      }
      if(IS_HOLY_PRIEST(victim->fortifier) && GET_ADVANCE_LEVEL(victim->fortifier) >= 5) {
          // Since fortifier has improved fort, the victim gets cleansed and stuns removed
          spell_cleanse(GET_LEVEL(victim), victim, victim, 0, 0, SAVING_SPELL);
          WAIT_STATE(ch, 0);
      }

    }
  }
  
  protect_unlinked_victim(victim);

  // If the victim became unconcious, drop them out of the fight.
  //
  if( !AWAKE(victim) )
  {
    if( FIGHTING(victim))
      stop_fighting(victim);
  }

  if( GET_POS(victim) == POS_DEAD )
  {
      // If protected by quicken, you rise from the dead
      if(affected_by_spell(victim, SPELL_QUICKEN)) {
          GET_HIT(victim) = GET_MAX_HIT(victim);
          GET_POS(victim) = POS_FIGHTING;
          set_fighting(ch, victim);
          while (victim->affected)
              affect_remove(victim, victim->affected, TRUE);

          // Having skills removed might have hurt the character.  So let's reset them again.
          restore(ch, ch);

          COOLDOWN(victim, SLOT_QUICKEN) = 30;
          act("You rise from the grave to make $M pay!",
                  FALSE, victim, 0, ch, TO_CHAR);
          act("$n rises from the grave to make you pay for what you've done!",
                  FALSE, victim, 0, ch, TO_VICT);
          act("$n rises from the grave to get revenge on $N!",
                  FALSE, victim, 0, ch, TO_NOTVICT);
      }
      else ch_kill_victim(ch, victim);
  }
}


// BEGIN Digger Reconstruction
//

void
damageSummary( CharData *ch, CharData *victim, MeleeDamageData *damSum )
{
  if( !ok_damage_victim(ch, victim) ) return;

  if( victim != ch )
  {
    if( !setup_combat(ch, victim) ) return;
  }

  damage_begin(ch, victim);

  while( damSum->valid )
  {
    int dam  = damSum->damage;
    int type = damSum->type;

    dam = damage_affects(ch, victim, dam, type);
    dam = damage_limit(ch, victim, dam, type);

    /* Invoke any special combat procedures for the attacker and/or their objects. */
    attackerSpecial(ch, victim, &dam, &type);
    /* Now any special combat procedures for the victim and/or their objects. */
    victimSpecial(ch, victim, &dam, &type);

    remember_what_hurts(ch, victim, dam, type);
    

	GET_HIT(victim) -= dam;


    damage_display(ch, victim, dam, type);

    protect_unlinked_victim(victim);

    // If the victim became unconcious, drop them out of the fight.
    if (!AWAKE(victim))
      if (FIGHTING(victim))
        stop_fighting(victim);

    if( GET_POS(victim) == POS_DEAD )
	  ch_kill_victim(ch, victim);

    damSum += 1;
  }
}

int dirty_bitchslap(CharData *ch, CharData *victim)
{
    act("You bitchslap $N's face twice, stunning $M into silence!",
            FALSE, ch, 0, victim, TO_CHAR);
    act("$n bitchslaps you twice, stunning you into silence!",
            FALSE, ch, 0, victim, TO_VICT);
    act("$n slaps $N in the face twice, $N looks stunned!",
            FALSE, ch, 0, victim, TO_ACTSPAM);
    add_affect(ch, victim, SKILL_DIRTY_TACTICS, 0, APPLY_NONE, 0, 1 TICKS,
            AFF_SILENCE, FALSE, FALSE, FALSE, FALSE);
    return 0;
}

int dirty_gouge(CharData *ch, CharData *victim)
{
    if (WIELDING(ch) && CAN_SEE(ch, victim)) {
        act("You drop your weapon and jam both thumbs into $N's eyes!",
                FALSE, ch, 0, victim, TO_CHAR);
        act("$n drops $s weapon and jams both thumbs into your eyes!",
                FALSE, ch, 0, victim, TO_VICT);
        act("$n drops $s weapon and jams both thumbs into $N's eyes!",
                FALSE, ch, 0, victim, TO_ACTSPAM);
        obj_to_char(unequip_char(ch, WEAR_WIELD), ch);
    } else {
        act("You jam both thumbs into $N's eyes!",
                FALSE, ch, 0, victim, TO_CHAR);
        act("$n jams both thumbs into your eyes!",
                FALSE, ch, 0, victim, TO_VICT);
        act("$n jams both thumbs into $N's eyes!",
                FALSE, ch, 0, victim, TO_ACTSPAM);
    }
    add_affect(ch, victim, SKILL_DIRTY_TACTICS, 0, APPLY_NONE, 0, 1 TICKS,
            AFF_BLIND, FALSE, FALSE, FALSE, FALSE);
}

int dirty_stomp(CharData *ch, CharData *victim)
{
    act("You smash your heel onto $N's feet, resulting in a crunch!",
            FALSE, ch, 0, victim, TO_CHAR);
    act("$n stomps your feet so hard you hear the crunching of bones!",
            FALSE, ch, 0, victim, TO_VICT);
    act("$n stomps $N's feet so hard you hear the crunching of bones!",
            FALSE, ch, 0, victim, TO_ACTSPAM);
    add_affect(ch, victim, SKILL_DIRTY_TACTICS, 0, APPLY_NONE, 0, 1 TICKS,
            AFF_HAMSTRUNG, FALSE, FALSE, FALSE, FALSE);
    return 0;
}

int dirty_sucker(CharData *ch, CharData *victim)
{
    if (GET_POS(victim) >= POS_FIGHTING) {
        act("You snap your fist into $N's face, causing $M to fall over!",
                FALSE, ch, 0, victim, TO_CHAR);
        act("$n snaps $s fist into your face, causing you to fall over!",
                FALSE, ch, 0, victim, TO_VICT);
        act("$n snaps $s fist into $N's face, causing $M to fall over!",
                FALSE, ch, 0, victim, TO_ACTSPAM);
        if (IS_NPC(victim) && victim->rider)
            awkward_dismount(victim->rider);
        GET_POS(victim) = POS_SITTING;
    } else {
        act("You snap your fist into $N's face!",
                FALSE, ch, 0, victim, TO_CHAR);
        act("$n snaps $s fist into your face!",
                FALSE, ch, 0, victim, TO_VICT);
        act("$n snaps $s fist into $N's face!",
                FALSE, ch, 0, victim, TO_ACTSPAM);
    }
}

int dirty_necksnap(CharData *ch, CharData *victim)
{
    act("You smash $N's face backwards!",
            FALSE, ch, 0, victim, TO_CHAR);
    act("$n smashes your face back!  Everything stops for a moment.",
            FALSE, ch, 0, victim, TO_VICT);
    act("$n smashes $N's in the face.  $N becomes momentarily unresponsive.",
            FALSE, ch, 0, victim, TO_ACTSPAM);
    add_affect(ch, victim, SKILL_DIRTY_TACTICS, 0, APPLY_NONE, 0, 2,
            AFF_PARALYZE, FALSE, FALSE, FALSE, FALSE);
    return 0;
}

int
autoWarrior( CharData *ch,
             CharData *victim )
{
    if( !IS_WARRIOR(ch) ) return 0;

    if (!IS_NPC(victim) && victim->mount) return 0;

    if( skillSuccess( ch, SKILL_DIRTY_TACTICS) && number(1,100) < 5 ){
        typedef int (*dirty_affect)(CharData *ch, CharData *victim);
        typedef struct {
            int   minDamage;
            int   maxDamage;
            char *dirtSelf;
            char *dirtVict;
            char *dirtRoom;
            dirty_affect affect;
        } DirtyFighting;

        DirtyFighting theMoves[] = {
            { 1,  4, "You bitchslap $N across the face!",
                     "$n bitchslaps you across the face, the brute!",
                     "$n slaps $N across the face!",
                     dirty_bitchslap},
            { 2,  8, "You gouge $N in the eyes!",
                     "$n gouges you in the eyes!",
                     "$n gouges $N in the eyes!",
                     dirty_gouge},
            { 3,  6, "You bite $N!",
                     "$n just bit you on the hand!",
                     "$n bites $N!",
                     NULL},
            { 4,  8, "You smash your heel down on $N's toes!",
                     "$n stomps down HARD on your toes!",
                     "$n stomps on $N's toes!",
                     dirty_stomp},
            { 6, 12, "You manage to sucker punch $N, the fool!",
                     "$n sucker punches you in the mouth!",
                     "$n sucker punched $N in the mouth!",
                     dirty_sucker},
            { 4, 10, "You elbow $N in the ribs!",
                     "$n hits you with an elbow to the ribs!",
                     "$n jabs $N in the ribs with an elbow!",
                     NULL}
        };
#define MAX_TACTIC (sizeof(theMoves)/sizeof(theMoves[0]) - 1)
        int theIdx = number( 0, MAX_TACTIC );
        DirtyFighting *theDirt = &theMoves[theIdx];
        int thedam;

        thedam = number(theDirt->minDamage, theDirt->maxDamage);
        if (theDirt->affect && percentSuccess(15)) {
            theDirt->affect(ch, victim);
        } else {
            act(theDirt->dirtSelf, FALSE, ch, 0, victim, TO_CHAR);
            act(theDirt->dirtVict, FALSE, ch, 0, victim, TO_VICT);
            act(theDirt->dirtRoom, FALSE, ch, 0, victim, TO_ACTSPAM);
        }

		if(percentSuccess(25) && IS_DRAGONSLAYER(ch) && GET_ADVANCE_LEVEL(ch) >= THIRD_ADVANCE_SKILL) {
			dirty_necksnap(ch, victim);
			thedam += number(2, 40);
		}

        return( thedam );

    }/* Dirty Fighting */

    return(0);
}

void
artCrane(CharData *ch)
{
    int i, num_kicks, stun;
    int nobash = FALSE;
    CharData *victim;

    if (!FIGHTING(ch)) return;
    if (GET_POS(ch) < POS_FIGHTING) return; /* can't kick sitting! */

    /* We've got someone to kick. */
    victim = FIGHTING(ch);
    /* Save old stun so we can restore it later. */
    stun = ch->player.stunned;

    if(!IS_NPC(ch) && GET_ASPECT(ch) != ASPECT_CRANE)
        return;

    if( !(skillSuccess(ch, SKILL_ART_CRANE) && GET_ASPECT(ch) == ASPECT_CRANE && number(1,100) < 9) )
	return;

    if(IS_SET_AR( MOB_FLAGS(victim), MOB_NOBASH )) {
        nobash = TRUE;
        REMOVE_BIT_AR(MOB_FLAGS(victim), MOB_NOBASH);
    }

    act("You release a flurry of kicks against $N!", FALSE, ch, 0, victim, TO_CHAR);
    act("$n releases a flurry of kicks against you!", FALSE, ch, 0, victim, TO_VICT);
    act("$n releases a flurry of kicks against $N!", FALSE, ch, 0, victim, TO_ACTSPAM);
    num_kicks = number(2, 5);
    if (GET_LEVEL(ch) >= MAX_MORTAL) num_kicks++;
    for( i= 1; i <= num_kicks; i++)
	if (FIGHTING(ch)) do_kick(ch, "", 0, 0);
    if (FIGHTING(ch)) do_sweep(ch, "", 0, 0);

    // Remember to turn back on NOBASH!
    if(nobash)
        SET_BIT_AR(MOB_FLAGS(victim), MOB_NOBASH);

    /* Art of the Crane has no effect on ch's stun state. */
    ch->player.stunned = stun;
}/* Art of the Crane */

int
autoShouLin( CharData *ch,
             CharData *victim )
{
    if( !IS_SHOU_LIN(ch) || WIELDING(ch) ) return 0;

    if( skillSuccess( ch, SKILL_ART_DRAGON) && number(1,100) < 5 ){
        act("You execute the Art of the Dragon!", FALSE, ch, 0, victim, TO_CHAR);
        act("$n executes the Art of the Dragon on YOU!",  FALSE, ch, 0, victim, TO_VICT);
        act("$n executes the Art of the Dragon on $N!",  FALSE, ch, 0, victim, TO_ACTSPAM);
        return( number( GET_LEVEL(ch)>>1, GET_LEVEL(ch)) + 10 );
    }/* Art of the Dragon */

    else if( skillSuccess(ch, SKILL_ART_TIGER) && number(1,100) < 5 ){
        act("You snarl as you reach out and claw $N!", FALSE, ch, 0, victim, TO_CHAR);
        act("$n snarls as $e reaches out and claws you!", FALSE, ch, 0, victim, TO_VICT);
        act("$n snarls as $e reaches out and claws $N!", FALSE, ch, 0, victim, TO_ACTSPAM);
        return( number( GET_LEVEL(ch)>>1, GET_LEVEL(ch)) );
    }/* Art of the Tiger */

    else if(GET_ASPECT(ch) == ASPECT_SNAKE && skillSuccess( ch, SKILL_ART_SNAKE ) && number(1,100) < 15) {
        act("You swiftly reach forth and touch $N's temple!", FALSE, ch, 0, victim, TO_CHAR);
        act("$n swiftly reaches forth and touches your temple!", FALSE, ch, 0, victim, TO_VICT);
        act("$n swiftly reaches forth and touches $N's temple!", FALSE, ch, 0, victim, TO_ACTSPAM);
        if( !mag_savingthrow(victim, SAVING_PARA)) {
            add_affect(ch, victim, SPELL_POISON, 0, APPLY_STR, -1, 2 TICKS, AFF_POISON, FALSE, FALSE, FALSE, FALSE);
            // 33% chance of short term paralysis
            if(!number(0, 2))
                add_affect(ch, victim, SKILL_ART_SNAKE, 0, 0, 0, 3 , AFF_PARALYZE, FALSE, FALSE, FALSE, FALSE);
            act("You suddenly feel ill!", FALSE, victim, 0, 0, TO_CHAR);
            act("$n turns pale and looks deathly sick!", FALSE, victim, 0, 0, TO_ROOM);
        }
        return 0;
    }/* Art of the Snake */

    return 0;
}

/* ============================================================================ 
This function returns the score ch requires out of 20 to hit an armor
class of zero.
============================================================================ */
int
thacoCalc(CharData *ch)
{
    int theThaco;

    if( !IS_NPC(ch) )
        theThaco = thaco[(int) GET_CLASS(ch)][(int) GET_LEVEL(ch)];
    else
        theThaco = 20; /* THAC0 for monsters is set in the HitRoll */
    theThaco -= GET_HITROLL(ch);

    /* Modify for abilities. */
    if (!IS_NPC(ch)) /* Don't mod for monsters to keep hr predictable */
        theThaco -= str_app[STRENGTH_APPLY_INDEX(ch)].tohit;

    /* Shou-lin gets bonus if there using bare hands. */
    if( IS_SHOU_LIN(ch) && !WIELDING(ch) )
        theThaco -= shouLinMods[GET_LEVEL(ch)/5].to_hit;

    /* Draconian gets penalty if wings are covered. */
    if (IS_DRACONIAN(ch) && ch->equipment[WEAR_ABOUT])
	theThaco += 1;

    return theThaco;
}

/* ============================================================================ 
This function calculates whether ch would successfuly hit victim. The victim
may still dodge an otherwise succssful attack.
If ch does hit, it returns 1, otherwise it returns 0.
============================================================================ */
int
chHitVictim(CharData *ch, CharData *victim, int skill)
{
    int victim_ac, diceroll;

    /* backstab hitting is determined elsewhere */
    if (BACKSTAB(ch, 0, skill)) return 1;

    if ( !AWAKE(victim) ) return 1; /* automatic hit. */

    /* you *always* hit someone waiting in ambush */
    if (victim->ambushing) {
      free(victim->ambushing); victim->ambushing = NULL; return 1;
    }

    diceroll = number(1, 20);
    if (diceroll == 20) return 1; /* Natural 20 always hits. */
    if (diceroll == 1) return 0;  /* Natural 1 always misses. */

    /* For the purposes of not being hit, AC goes no lower than -10 */
    victim_ac = realAC(victim)/10;
    victim_ac = MAX(-10, victim_ac);

    /* You have an advantage on an enemy that can't see you. */
    if (!CAN_SEE(ch, victim))
	IS_SHOU_LIN(ch) ? (diceroll -= 2) : (diceroll -= 4);
    else if (!CAN_SEE(victim, ch))
	IS_SHOU_LIN(victim) ? (diceroll += 2) : (diceroll += 4);

    /* Some skills help, assuming you can actually see the target! */
    if( CAN_SEE(ch, victim) &&
	skillSuccess(ch, SKILL_FIND_WEAKNESS) && 
	percentSuccess(7) )
    {
        act("You think you spot a weakness in $N's defenses.", FALSE, ch, 0, victim, TO_CHAR);
        diceroll += number(4,8);
    }

    // If your enemy is wielding a 2H weapon, they have reduced defenses
    if(isUsingLochaber(victim))
	diceroll += 4;

    if( diceroll > (thacoCalc(ch) - victim_ac) )
	return 1; /* hit */

    return 0; /* missed */
}

int does_target_parry(CharData *victim) {
    if (affected_by_spell(victim, SKILL_SHOOT))
        return FALSE;

    if (percentSuccess(10))
        return TRUE;

    if(isUsingLochaber(victim) && percentSuccess(40))
        return TRUE;

    if(affected_by_spell(victim, SKILL_DETERRENCE) && percentSuccess(50))
        return TRUE;

    if(IS_BRIGAND(victim) && percentSuccess(3*GET_ADVANCE_LEVEL(victim)/2))
        return TRUE;

    return FALSE;
}

int does_target_dodge(CharData *victim) {
    /* Your combat stance affects your chance to dodge */
    if (victim->stance == STANCE_NEUTRAL && percentSuccess(10))
        return TRUE;
    else if (victim->stance == STANCE_DEFENSIVE && percentSuccess(20))
        return TRUE;
    // While in offensive stance, you have no chance of dodging.

    // Ancient dragons in monkey aspect dodge more often
    if(IS_ANCIENT_DRAGON(victim) && GET_ADVANCE_LEVEL(victim) >= 2 &&
            GET_ASPECT(victim) == ASPECT_MONKEY &&
            skillSuccess(victim, SKILL_ART_MONKEY) && percentSuccess(15))
        return TRUE;

    if(IS_BRIGAND(victim) && percentSuccess(3*GET_ADVANCE_LEVEL(victim)/2))
        return TRUE;

    if(affected_by_spell(victim, SKILL_DETERRENCE) && percentSuccess(50))
        return TRUE;

    if(affected_by_spell(victim, SKILL_SHADOW_STEP))
        return TRUE;

    return FALSE;
}

int does_target_shadowDodge(CharData *victim) {
    if (!(IS_DARK(victim->in_room) || IS_AFFECTED(victim, AFF_SHADOW_SPHERE)))
        return FALSE;

    if (percentSuccess(7))
        return TRUE;

    if (IS_SHADE(victim) && percentSuccess(3*GET_ADVANCE_LEVEL(victim)/2))
        return TRUE;

    return FALSE;
}

int
contactIsMade( CharData *ch, CharData *victim)
{
    int riposte_dam = 0;	// How much damage does counter-stroke inflict?
    int dodged = 0;
    int parried = 0;
    int shadowDodged = 0;

    /*
    ** Let sleeping dogs lie ? Shyeah, not on THIS mud.
    */
    if( !AWAKE(victim) ) return 1;

    set_fighting(ch, victim);

    if (IS_AFFECTED(victim, AFF_PARALYZE)) /* Can't dodge when paralyzed */
	return(1);

    /* You can't dodge their attack if you don't see it coming. */
    if (!CAN_SEE(victim, ch) && !IS_SHOU_LIN(victim) )
	return(1);

    dodged = does_target_dodge(victim);
    parried = does_target_parry(victim);
    shadowDodged = does_target_shadowDodge(victim);

    if (skillSuccess( victim, SKILL_DODGE ) && dodged) {
        act("$N dodges your attack!", FALSE, ch, 0, victim, TO_CHAR);
        act("You dodge $n's attack!", FALSE, ch, 0, victim, TO_VICT);
        act("$N dodges $n's attack!", FALSE, ch, 0, victim, TO_ACTSPAM);
        if( !IS_NPC(victim) && percentSuccess(20) )
            advanceSkill( victim, SKILL_DODGE, SKILL_DODGE_LEARN, NULL, FALSE, FALSE, FALSE );
        return(0);
    }
    else if( IS_AFFECTED(victim, AFF_BLINK) && (number(1, 100) < 10 ) ){
        act("$N blinks out of your range!", FALSE, ch, 0, victim, TO_CHAR);
        act("You blink out of $n's way!", FALSE, ch, 0, victim, TO_VICT);
        act("$N blinks out of $n's way!", FALSE, ch, 0, victim, TO_ACTSPAM);
        return(0);
    }
    else if(WIELDING(victim) && skillSuccess(victim, SKILL_PARRY) && parried){
        act("$N parries your attack!", FALSE, ch, 0, victim, TO_CHAR);
        act("You parry $n's attack!", FALSE, ch, 0, victim, TO_VICT);
        act("$N parries $n's attack!", FALSE, ch, 0, victim, TO_ACTSPAM);
        if( !IS_NPC(victim) )
            advanceSkill( victim, SKILL_PARRY, SKILL_PARRY_LEARN, NULL, FALSE, FALSE, FALSE );

        /* Riposte skill, perhaps the warrior can now counter-strike! */
        if (skillSuccess(victim, SKILL_RIPOSTE)) {
            // Ok, so lets see if they connect...
            if (!chHitVictim(victim, ch, SKILL_RIPOSTE)) {
                damage(victim, ch, 0, SKILL_RIPOSTE);
            }
            else { // They hit! Addup the damage and hurt the target.
                if( !IS_NPC(victim) )
                    advanceSkill( victim, SKILL_RIPOSTE, SKILL_RIPOSTE_LEARN, NULL, FALSE, FALSE, FALSE );
                riposte_dam = calculateDamage(victim, ch, TYPE_UNDEFINED, 1);
                damage(victim, ch, riposte_dam, SKILL_RIPOSTE);
            }
        }
        return(0);
    }
    else if (skillSuccess(victim, SKILL_SHADOW_DANCE) && shadowDodged){
	act("You lose sight of $N in the shadows!", FALSE, ch, 0, victim, TO_CHAR);
	act("$n loses sight of you in the shadows!", FALSE, ch, 0, victim, TO_VICT);
	act("The shadows around $N obscure $n's attack!", FALSE, ch, 0, victim, TO_ACTSPAM);
	return(0);
    }
    else if(IS_DRAGONSLAYER(victim) && percentSuccess(5) && GET_ADVANCE_LEVEL(victim) >= SECOND_ADVANCE_SKILL) {
        act("$n spots an opening to attack.", FALSE, ch, 0, victim, TO_CHAR);
        act("You think you see an opening to make an attack.", FALSE, ch, 0, victim, TO_VICT);
	act("$N spots an opening to attack $n!", FALSE, ch, 0, victim, TO_ACTSPAM);

        riposte_dam = calculateDamage(victim, ch, TYPE_UNDEFINED, 1);
        if (!chHitVictim(victim, ch, SKILL_RIPOSTE)) {
            damage(victim, ch, 0, SKILL_RIPOSTE);
        }
        else { // They hit! Addup the damage and hurt the target.
            riposte_dam = calculateDamage(victim, ch, TYPE_UNDEFINED, 1);
            damage(victim, ch, riposte_dam, SKILL_RIPOSTE);
        }
        if(!ch)
            return 0;
    }
    return(1);
}

int calculateSimpleDamage(CharData *ch)
{
  int dam = GET_DAMROLL(ch) + str_app[STRENGTH_APPLY_INDEX(ch)].todam;
  /*
  ** Calculate weapon or open had damage
  */
  if(WIELDING(ch)) {
      if (!IS_NPC(ch))
          dam += dice( GET_OBJ_VAL( WIELDING(ch), 1 ),
                  GET_OBJ_VAL( WIELDING(ch), 2 ));
      else
          dam += MAX(dice(GET_OBJ_VAL(WIELDING(ch), 1), GET_OBJ_VAL(WIELDING(ch), 2)),
                  dice(ch->mob_specials.damnodice, ch->mob_specials.damsizedice));
  } else if (IS_NPC(ch)) {
      if (IS_CLONE(ch)) {
          dam += dice( 4, 6 );
      } else {
          dam += dice( ch->mob_specials.damnodice, ch->mob_specials.damsizedice );
      }
  } else if( IS_SHOU_LIN(ch) || (GET_LEVEL(ch) >= LVL_IMMORT)){
      //dam += number(0, (shouLinMods[GET_LEVEL(ch)/5].number_of_dice*shouLinMods[GET_LEVEL(ch)/5].size_of_dice));
      dam += dice(shouLinMods[GET_LEVEL(ch)/5].number_of_dice,
              shouLinMods[GET_LEVEL(ch)/5].size_of_dice);
      dam += shouLinMods[GET_LEVEL(ch)/5].to_damage;

      if (IS_DRACONIAN(ch))
          dam += number(2, 14); /* Draconian shous also get bonus damage. */
  } else if (IS_DRACONIAN(ch))
      dam += number(4, 16); /* Draconians do better damage since they have claws. */
  else
      dam += number(0, 2); /* Max. 2 dam with bare hands */

  return dam;
}

/* This function used to be static.. I could'nt see any reason for it to be static thou..*/
int
calculateDamage( CharData *ch,
                 CharData *victim,
                 int attackType,
                 int attackNumber )
{
  int dam = calculateSimpleDamage(ch);
  int psucc;

  /* berserk means a higher enhanced damage rate, and so does being a
   * knight and fighting the opposite align */
  psucc = IS_AFFECTED(ch, AFF_BERSERK) ? 12 : 7;
  if (GET_CLASS(ch) == CLASS_SOLAMNIC_KNIGHT && IS_EVIL(victim)) psucc += 5;
  if (GET_CLASS(ch) == CLASS_DEATH_KNIGHT && IS_GOOD(victim)) psucc += 5;

  if( skillSuccess( ch, SKILL_ENHANCED_DAMAGE ) && percentSuccess(psucc))
  {
    sendChar(ch, "You strike powerfully.\r\n");
    dam += number(1,( GET_LEVEL(ch) >> 1 ));
  }

  if(IS_DRAGONSLAYER(ch) || IS_BUTCHER(ch))
      dam += GET_ADVANCE_LEVEL(ch);

  if (WIELDING(ch)) {
      int mastery = 0;
      switch (GET_OBJ_VAL(ch->equipment[WEAR_WIELD], 3) + TYPE_HIT) {
          case TYPE_SLASH:
          case TYPE_TEAR:
                mastery = SKILL_EDGE_MASTERY;
              break;
          case TYPE_PIERCE:
          case TYPE_IMPALE:
              mastery = SKILL_POINT_MASTERY;
              break;
          case TYPE_CRUSH:
          case TYPE_BLUDGEON:
          case TYPE_POUND:
              mastery = SKILL_BLUNT_MASTERY;
              break;
          case TYPE_BURN:
          case TYPE_DRAIN:
          case TYPE_BLAST:
              mastery = SKILL_EXOTIC_MASTERY;
              break;
      }
      if (mastery == GET_MASTERY(ch) && skillSuccess(ch, SKILL_WEAPON_MASTERY)) {
          /* being a master of your weapon gives you quite the boost */
          dam += number(2,10);

          /* at true meaning, you're even better with it */
          if (GET_SKILL(ch, SKILL_WEAPON_MASTERY) >= 90)
              dam += number(3,5);

          /* this skill is very slow to advance */
          if (percentSuccess(2))
              advanceSkill( ch, SKILL_WEAPON_MASTERY, 90,
                      "You gain a deeper understanding of your weapon.",
                      FALSE, FALSE, FALSE );
      }
  }

  /*
  ** If the victim isn't fighting then give the char a bonus
  ** based on the difference in postures.
  */
  if( GET_POS(victim) < POS_FIGHTING )
    dam *= 1 + (POS_FIGHTING - GET_POS(victim)) / 4;

  if( BACKSTAB(ch, 0, attackType)){
    int damMult = (attackType == SKILL_CIRCLE ? CIRCMULT(ch):BACKMULT(ch));
    dam *= damMult;
  }

  if(!IS_BACKSTAB(attackType)) {
    switch( GET_CLASS(ch) ){
      case CLASS_SHOU_LIN: dam += autoShouLin(ch, victim); break;
      case CLASS_WARRIOR:  dam += autoWarrior(ch, victim); break;
      default: break;
    }
  }

  /* stances affect your damage */
  if (ch->stance == STANCE_OFFENSIVE)
      dam += 5;
  else if (ch->stance == STANCE_DEFENSIVE)
      dam -= 5;

  /* powerstrike = mana -> damage */
  
  if (GET_MANA(ch) >= ch->powerstrike) {
	  if ( IS_NPC(ch) && ch->master && !IS_NPC(victim) )
		 GET_MANA(ch) -= ch->powerstrike;
	  else if ( !(IS_NPC(ch) || IS_NPC(victim)) )
		  GET_MANA(ch) -= ch->powerstrike/4;
	  else GET_MANA(ch) -= ch->powerstrike;
      
	  dam += ch->powerstrike;
  } else {
      ch->powerstrike = 0;
  }

  return( MAX(1, dam) );
}

/* This function sees how many types of attacks a mob has. */
/* This function assumes: ONLY a mob will be passed in. */
int variety_of_attacks(CharData *mob)
{
	int  variety = 1;
	if (mob->mob_specials.attack_type2 >= 0)
	{
		variety++;
		if (mob->mob_specials.attack_type3 >= 0)
			variety++;
	}
	return variety;
}

/* 
** This function chooses an attack for the mob to use from the ones 
** the mob  can  deliver. It assumes:
** That the variety of attacks passed in IS correct. Serious problem if
** its not. 
*/
int  natural_attack(CharData *ch, int variety)
{
	int nat_type, num;
	switch (variety) {
	case 1:
		nat_type = ch->mob_specials.attack_type + TYPE_HIT;
		break;
	case 2:
		if (number(1, 3) < 3)
			nat_type = ch->mob_specials.attack_type + TYPE_HIT;
		else
			nat_type = ch->mob_specials.attack_type2 + TYPE_HIT;
		break;
	case 3:
		num = number(1, 7);
		if (num < 5)
			nat_type = ch->mob_specials.attack_type + TYPE_HIT;
		else if (num == 7)
			nat_type = ch->mob_specials.attack_type3 + TYPE_HIT;
		else
			nat_type = ch->mob_specials.attack_type2 + TYPE_HIT;
		break;
	default:
		mudlog(NRM, LVL_IMMORT, TRUE, "ERROR!!! : Variety expected in range 1 - 3 in function natural_attack(fight.c)");
		break;
	}
	return nat_type;
}


int realAC(CharData *ch)
{
    int theAC;

    theAC = GET_AC(ch); /* from equipment and spells */

    /* Modify for abilities. */
    if( AWAKE(ch) )
	theAC += dex_app[GET_DEX(ch)].defensive;

    /* Modify for class. */
    if( IS_SHOU_LIN(ch) ) 
        theAC += shouLinMods[(GET_LEVEL(ch)/5)].base_ac;
    if(IS_DRAGOON(ch) || IS_KNIGHT_ERRANT(ch))
        theAC -= 5* GET_ADVANCE_LEVEL(ch);

    /* Modify for race. */
    if (!IS_NPC(ch)) switch (GET_RACE(ch)) {
	case RACE_TROLL:
        case RACE_STROLL:
	    theAC -= 10;
	    break;
	case RACE_DRACONIAN:
        case RACE_SDRACONIAN:
	    if (!ch->equipment[WEAR_ABOUT])
	        theAC -= 20;
	    else
		theAC -= 10;
	    break;
        case RACE_ELEMENTAL:
            if (GET_SUBRACE(ch) == EARTH_ELEMENTAL)
                theAC -= 30;
            break;
	default:
		break;
    }

    /* modify for stances */
    /* stances affect your damage */
    if (ch->stance == STANCE_OFFENSIVE)
        theAC += 30;
    else if (ch->stance == STANCE_DEFENSIVE)
        theAC -= 30;

    return theAC;
}


void
hit( CharData *ch,
     CharData *victim,
     int type )
{
    CharData *tch, *next_tch;

#define OLD_COMBAT
  ObjData *wielded = WIELDING(ch) ? ch->equipment[WEAR_WIELD] : NULL;
  int w_type;
  int variety;
  int blockedAttack = -1;
  int blockedAttackTwo = -1;
  int numAttacks    = 0;
  int attackCnt     = 0;
  int totalDam      = 0;
  int i;
  int briarDamage   = 0;
  MeleeDamageData damSum[MAX_NUM_ATTACKS+1];

  // Is ch trying to fight a non-combatant?
  //
  if( chIsNonCombatant(victim) )
  {
    sendChar( ch, "You can't fight %s, they are a non-combatant.\r\n",
                   GET_NAME(victim));
    stop_fighting(ch);
    return;
  }

  // If you are affected by commanding shout, just sit this one out.
  if(affected_by_spell(ch, SKILL_COMMANDING_SHOUT)) {
      affect_from_char(ch, SKILL_COMMANDING_SHOUT);
      return;
  }

  /* check if the character has a fight trigger */
  fight_mtrigger(ch);

  if( ch->in_room != victim->in_room )
  {
    // This log becomes very annoying with recent scripts, and serves no real useful purpose. --Maestro
    //mudlog(NRM, LVL_IMMORT, TRUE, "SYSERR: Different rooms [%s:%d] and [%s:%d]",
    //          GET_NAME(ch), ch->in_room, GET_NAME(victim), victim->in_room );
    stop_fighting(ch);
    return;
  }

  if( wielded && GET_OBJ_TYPE(wielded) == ITEM_WEAPON )
    w_type = GET_OBJ_VAL(wielded, 3) + TYPE_HIT;
  else
    w_type = (IS_DRACONIAN(ch) ? TYPE_CLAW : TYPE_HIT);
    
  numAttacks = calculate_attack_count(ch);

  if(IS_BUTCHER(ch) && GET_ADVANCE_LEVEL(ch) >= 3) {
      if(spell_level(ch, SKILL_DIRE_BLOWS) < 120 && percentSuccess(75))
          add_affect(ch, ch, SKILL_DIRE_BLOWS, spell_level(ch, SKILL_DIRE_BLOWS) + 1,
                  APPLY_DAMROLL, 1, 5, 0, TRUE, TRUE, TRUE, FALSE);
      else
          add_affect(ch, ch, SKILL_DIRE_BLOWS, spell_level(ch, SKILL_DIRE_BLOWS),
                  APPLY_DAMROLL, 0, 5, 0, TRUE, TRUE, TRUE, FALSE);
  }
      

  /* if we're who the victim is fighting, they might get to block an attack */
  // Now, give mobs extra blocks against players - Craklyn
  if (IS_AFFECTED(victim, AFF_SHIELDBLOCK) ) {
	  if( IS_NPC(victim) || FIGHTING(victim) == ch) {
		  blockedAttack = number(0, numAttacks - 1);
		  if(!IS_NPC(victim) || percentSuccess(15) )
			  REMOVE_BIT_AR( AFF_FLAGS(victim), AFF_SHIELDBLOCK );
	  }
          if(FIGHTING(victim) == ch && 
                  (IS_DRAGOON(ch) || IS_KNIGHT_ERRANT(ch)) &&
                  GET_ADVANCE_LEVEL(ch) >= THIRD_ADVANCE_SKILL && percentSuccess(20))
              blockedAttackTwo = number(0, numAttacks - 1);
  }
  
  if( IS_NPC(ch) ) variety = variety_of_attacks(ch);

  // Prepare the damSum recording
  for( i = 0; i < (sizeof(damSum)/sizeof(damSum[0])); i++ )
  {
    damSum[i].damage = 0;
    damSum[i].type   = TYPE_UNDEFINED;
    damSum[i].valid  = FALSE;
  }

  // Digger Working
  while( attackCnt < numAttacks )
  {
    int dam = 0;

    // Ok, lets work out NPC attack types here.
    if( IS_NPC(ch) && !wielded )
      w_type = natural_attack(ch, variety);

    if (attackCnt == blockedAttack || attackCnt == blockedAttackTwo) {
        act("$N blocks your attack with $S shield!",
                FALSE, ch, 0, victim, TO_CHAR);
        act("You block $n's attack with your shield!",
                FALSE, ch, 0, victim, TO_VICT);
        act("$N blocks $n's attack with $S shield!",
                FALSE, ch, 0, victim, TO_ACTSPAM);
    }
    else if( contactIsMade(ch, victim) ) {
        if( !chHitVictim(ch, victim, type) )      // Attack Missed
        {
            damSum[attackCnt].damage = 0;
            damSum[attackCnt].type = w_type;
#ifdef OLD_COMBAT
            damage( ch, victim, 0, w_type );
#endif
        }
        else
        {
            // Contact was made so let's calculate the damage.
            if( BACKSTAB(ch, attackCnt, type) )
            {
                dam += calculateDamage( ch, victim, type, attackCnt );
                damSum[attackCnt].damage = dam;
                damSum[attackCnt].type   = SKILL_BACKSTAB;
#ifdef OLD_COMBAT
                damage( ch, victim, dam, type );
#endif
            }
            else
            {
                dam += calculateDamage( ch, victim, w_type, attackCnt );
                damSum[attackCnt].damage = dam;
                damSum[attackCnt].type   = w_type;
#ifdef OLD_COMBAT 
                damage( ch, victim, dam, w_type );
#endif
            }
        }

        // check if the victim has a hitprcnt trigger
        hitprcnt_mtrigger(victim);
    }
    totalDam += dam;

    // Information in this position has been set properly.
    damSum[attackCnt].valid = TRUE;
    if( FIGHTING(ch) )
      attackCnt += 1;
    else
      attackCnt  = numAttacks;

    if(!IS_NPC(ch) || (ch->master && !IS_NPC(ch->master))) {
        add_affect(ch, ch, TMP_COMBAT_BUFF, GET_LEVEL(ch), APPLY_HITROLL, -attackCnt*GET_LEVEL(ch)/10, 2, 0, TRUE, TRUE, TRUE, TRUE);
    }
  }/* while */

  affect_from_char(ch, TMP_COMBAT_BUFF);

  // check for the assistant attack type
  if (FIGHTING(ch) && IS_AFFECTED(ch, AFF_ASSISTANT) && GET_ASSIST_HP(ch) > 0 && !IS_AFFECTED(ch, AFF_HASTE)) {
    int dam = 0;
    damSum[attackCnt].type = TYPE_ASSISTANT;
    dam += dice(2, GET_LEVEL(ch)/2) + GET_LEVEL(ch)/2;
    if (number(1, 25) == 1) dam *= 2;
    damSum[attackCnt].damage = dam;
#ifdef OLD_COMBAT
    damage(ch, victim, dam, TYPE_ASSISTANT);
#endif
    hitprcnt_mtrigger(victim);
  } else
  // check for shadowboxing.  Can't have assistant AND shadowbox!
  if (FIGHTING(ch) && skillSuccess(ch, SKILL_SHADOWBOX) && number(1,10) == 1) {
    int dam = GET_DAMROLL(ch) + str_app[STRENGTH_APPLY_INDEX(ch)].todam;
    dam += number(0, 2);
    damSum[attackCnt].type = SKILL_SHADOWBOX;
    damSum[attackCnt].damage = dam;
#ifdef OLD_COMBAT
    damage(ch, victim, dam, SKILL_SHADOWBOX);
#endif
    hitprcnt_mtrigger(victim);
  }

  if(ch && victim && affected_by_spell(victim, SPELL_BARKSKIN)) {      
      act("As you strike $N, needle-thin thorns protruding from $M cut into you.", FALSE, ch, 0, victim, TO_CHAR);
      act("After brushing into your thorny skin, $n jerks away in pain.", FALSE, ch, 0, victim, TO_VICT);

      briarDamage = (totalDam/2 + GET_DAMROLL(victim))*GET_LEVEL(victim)/200;
      damage(victim, ch, briarDamage, SPELL_BARKSKIN);
  }
	
  // Ok, we have collected all the damage data for this round, now
  // we display it to each person in the room according to their
  // spam configuration. Tricky bit is to ensure victim isn't
  // hurt twice! Easiest way to do this is to just cheat. First,
  // we check to see if the damage would actually kill the
  // victim. If not, we deduct the damage then just print the
  // messages however we like. If it DOES kill them, then we pass
  // it off to the normal damage function. This means the killing
  // round of attacks will always be "spammy", it also means this
  // is the only round of attacks you might be able to flee in the
  // middle of.

  // Firstly, to the people who like it spamy.

  // Then to the people who don't.

  if(IS_SHOU_LIN(ch) || GET_LEVEL(ch) >= LVL_GOD)
    artCrane(ch);

  if(FIGHTING(ch) && (totalDam > 0) && !IS_NPC(ch))  {
    (void)diag_char_to_char( victim, ch );
  }
  else if(IS_NPC(victim) && FIGHTING(victim))  {
    if(totalDam > victim->mob_specials.max_attack_damage) {
      /* This dude is has hurt us the most! Lets get 'em! */
      victim->mob_specials.max_attack_damage = totalDam;
      victim->mob_specials.high_attacker = ch;
    }
  }

  // feign death then attack == 3 round lag.
  if(HAS_FEIGNED(ch)) {
      WAIT_STATE(ch, (PULSE_VIOLENCE * 3) + 1);
	HAS_FEIGNED(ch) = 0;
  }

  if(affected_by_spell(ch, SPELL_MALEDICT)) {
      for( tch = world[ch->in_room].people; tch; tch = next_tch ){
          next_tch = tch->next_in_room;

          if(affected_by_spell(tch, SPELL_MALEDICT2) && TARGET(tch) == ch) {
              mag_damage(spell_level(tch, SPELL_MALEDICT2), tch, ch, SPELL_MALEDICT, SAVING_SPELL);
              add_affect(tch, ch, SPELL_MALEDICT, spell_level(ch, SPELL_MALEDICT), 0, 0, 5, 0, TRUE, TRUE, TRUE, FALSE);
              add_affect(tch, tch, SPELL_MALEDICT2, spell_level(tch, SPELL_MALEDICT2), 0, 0, 5, 0, TRUE, TRUE, TRUE, FALSE);
          }
      }
  }

}


static CharData *find_guarded(CharData *ch)
{
    CharData *master;
    FollowType *group;

    if (!ch->guarding) return NULL;
    if (ch->guarding != &guard_group) return ch->guarding;
    if (!IS_AFFECTED(ch, AFF_GROUP)) return NULL;

    /* find the master of the group */
    master = ch; if (master->master) master = master->master;

    /* if the master needs saving, do so */
    if (FIGHTING(master) && ch->in_room == master->in_room && ch != master)
        return master;

    group = master->followers;

    /* find the first candidate charge in the group */
    while (group) {
        CharData *ward = group->follower;
        if (FIGHTING(ward) && ch->in_room == ward->in_room && ch != ward)
            return ward;
        group = group->next;
    }

    return  NULL;
}

void
checkGuarded()
{
/* Keeping in mind this will be checked every combat round. */
#define GUARD_FAIL_PERCENT 40
#define GUARD_SKILL(ch) (IS_NPC(ch) ? 0 : GET_SKILL(ch, SKILL_GUARD))

  DescriptorData *d;
    
  for( d = descriptor_list; d; d = d->next )
  {
    if( !d->connected && d->character )
    {
      CharData *ch = d->character;
      CharData *charge = find_guarded(ch);
      if( charge != NULL && !FIGHTING(ch) )
      {
        if( FIGHTING(charge) && ch->in_room == charge->in_room )
        {
          CharData *opponent = FIGHTING(charge);
          if( number(1,100) <= GUARD_FAIL_PERCENT )
            return; /* bad luck. */
          if( FIGHTING(opponent) != charge )
            act( "$N may soon need a hand!",  FALSE, ch, 0, charge, TO_CHAR );
          else if( GET_POS(ch) < POS_FIGHTING )
            act( "You're in no position to lend a hand to $M.", FALSE, ch, 0, charge, TO_CHAR );
          else if( !CAN_SEE( ch, opponent ) )
            act( "You can't see who is fighting $M!", FALSE, ch, 0, charge, TO_CHAR );
          else
          {
            if( skillSuccess(ch, SKILL_GUARD) )
            {
              char_rescue_char(ch, charge);
              advanceSkill( ch, SKILL_GUARD, 90,
                       "You've become notably more proficient.\r\n",
                        FALSE, FALSE, FALSE );
              return;
            }
            else if( GET_POS(ch) == POS_STANDING )
            {
              sendChar(ch, "You fail to rescue your charge.\r\n");
              /*do_assist(ch, GET_NAME(charge), 0, 0);*/
            }
          }
        }
      }
    }
  }
}

// The mobile_combat_moves function is used so mobs can now act 
// every instant instead of only during combat rounds. -Craklyn
void
mobile_combat_moves(void)
{
	CharData *ch, *nextchar;

        for (ch = character_list; ch; ch = nextchar) {
            nextchar = ch->next;

            if(IS_NPC(ch) && STUNNED(ch)) {
                /*
                char buf[500];
                sprintf(buf, "$n is stunned for %d.", (ch)->player.stunned);
                act(buf, TRUE, ch, 0, ch, TO_ROOM);
                */
                if( ch->player.stunned < 1 || ch->player.stunned > 99 )
                    ch->player.stunned = 1;
                else
                    ch->player.stunned -= 1;
            }
        }

	for( ch = combat_list; ch; ch = next_combat_list )
	{
            next_combat_list = ch->next_fighting;
            
            if (ch->desc != NULL)
                continue;  /* Switched mob only do what the imm wants */
            
            if( FIGHTING(ch) == NULL || ch->in_room != FIGHTING(ch)->in_room)
                stop_fighting(ch);
            else {
                if( IS_NPC(ch) && ch->desc == NULL && !MOB_FLAGGED( ch, MOB_SPEC ))
                    smart_NPC_combat(ch);
            }
        }
        
}

/* control the fights going on.  Called every 2 seconds from comm.c. */
void perform_violence(void)
{
    DescriptorData *d;
    CharData *ch;

    checkGuarded();

    for( ch = combat_list; ch; ch = next_combat_list ){
        next_combat_list = ch->next_fighting;
        
        /*
         * 		if (SINGING(ch) && SONG_FLAGS(ch, SST_FIGHTING))
         * 		stop_singing(ch);
         */
        if( FIGHTING(ch) == NULL || ch->in_room != FIGHTING(ch)->in_room)
            stop_fighting(ch);
        else
        {
            if( !IS_AFFECTED( ch, AFF_PARALYZE ))
            {
                hit(ch, FIGHTING(ch), TYPE_UNDEFINED);
                if (MOB_FLAGGED(ch, MOB_SPEC) && mob_index[GET_MOB_RNUM(ch)].func != NULL)
                    (mob_index[GET_MOB_RNUM(ch)].func) (ch, ch, 0, "");
            }

            /*
             * if( IS_NPC(ch) && ch->desc == NULL && !MOB_FLAGGED( ch, MOB_SPEC )){
             * smart_NPC_combat(ch);
             * }
             */
        }
    }

	/* purge all SHIELDBLOCK bits every round so nothing hangs around */
	for( d = descriptor_list; d; d = d->next )
        {
            if( !d->connected && d->character )
            {
                CharData *ch = d->character;
                REMOVE_BIT_AR( AFF_FLAGS(ch), AFF_SHIELDBLOCK );
            }
        }
}

// The damage amount that PvP is divided by
int pvpFactor(void)
{
    time_t temp = time(NULL);
    struct tm *now = localtime(&temp);

    // Temporarily set it to always be 1 until pvp mode is ready.
    // muckle_pvpFactor enabled.
    return MAX(muckle_pvpFactor, (now->tm_mday == 1 ? 1 : 1));
}


ACMD(do_finish)
{
    char buf[MAX_STRING_LENGTH];
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    char buf3[MAX_STRING_LENGTH];
    CharData *victim;
    OBJ_DATA *wield;
    
    one_argument(argument, arg);

    IF_CH_CANT_SEE_VICTIM( "Finish off who?\r\n" );
    IF_CH_CANT_BE_VICTIM ( "You don't want to do that.\r\n" );
    IF_ROOM_IS_PEACEFUL  ( "A sense of well being overwhelms you.\r\n" );

    if(!CONFIG_PK_ALLOWED && !IS_NPC(victim))
    { 
        sendChar(ch, "Sorry, peace has been declared.\n\r");
        return;
    }

    if(IN_ARENA(ch))
    { 
        sendChar(ch, "There is no capping in the Arena.\n\r");
        return;
    }
    
    if(FIGHTING(ch) && !IS_NPC(ch))
    {
        sendChar(ch, "You're too busy.\n\r");
        return;
    }
    
    if ( argument[0] == '\0' )
    { 
        send_to_char( "Finish off who?\n\r", ch );
        return;
    }
    
    if ( IS_NPC(victim) )
    { 
        send_to_char( "Just kill them!\n\r", ch );
        return;
    }

    if((IS_NPC(victim) || GET_LEVEL(ch) < 10 || GET_LEVEL(victim) < 10) && !IS_NPC(ch))
    { 
        sendChar(ch, "For your protection, newbies are prevented from pkilling and being pked.\n\r");
        return;
    }
    if (GET_LEVEL(ch) >= LVL_IMMORT)
    {
        send_to_char( "You reconsider your actions.\n\r", ch );
        return;
    }
    if ( GET_POS(victim) >= POS_SLEEPING ) {
        send_to_char( "You can only finish off mortally wounded players.\n\r", ch );
        return;
    }
    
    switch(number(1,4))
    {
        case 1:
            sprintf( buf, "%s just got their head ripped off by %s.", GET_NAME(victim), GET_NAME(ch) );
            sprintf( buf1, "You drive your hand into $N's neck and rip off $S head!" );
            sprintf( buf2, "$n drives their hand into your neck and rips off your head!" );
            sprintf( buf3, "$n drives $m hand into $N's neck and rips off $S head!" );
            break;
        case 2:
            sprintf( buf, "%s has been decapitated by %s.", GET_NAME(victim), GET_NAME(ch) );
            sprintf( buf1, "You growl 'There can be only One!' and decapitate $N!" );
            if (WIELDING(ch))
                sprintf( buf2, "$n growls 'There can be only One!' and brings $s $p down on your neck!");
            else
                sprintf( buf2, "$n growls 'There can be only One!' and brings $s meaty fist down on your neck!");

            sprintf( buf3, "$n growls 'There can be only One!' and decapitates $N!" );
            break;
        case 3:
            sprintf( buf, "%s just got their eyeballs and brain plucked out by %s.", GET_NAME(victim), GET_NAME(ch) );
            sprintf( buf1, "You ram your weapon into $N's face and tear it out, removing eyes and brain." );
            sprintf( buf2, "$n rams $s weapon into your face and lobotomizes you." );
            sprintf( buf3, "$n rams $s weapon into $N's face and tears it out, removing eyes and brain." );
            break;
        case 4:
            sprintf( buf, "%s's head has been smashed in by %s.", GET_NAME(victim), GET_NAME(ch) );
            sprintf( buf1, "You bring your weapon down on $N's head, and brains spray from $S ears!" );
            sprintf( buf2, "$n brings $s weapon down on your head, forcing your brain to vacate the premises!" );
            sprintf( buf3, "$n brings $s weapon down on $N's head, and brains spray from $S ears!" );
            break;
        default:
            sprintf( buf, "%s has been finished off by %s.", GET_NAME(victim), GET_NAME(ch) );
            sprintf( buf1, "You finish $N off!" );
            sprintf( buf2, "$n finishes you off!" );
            sprintf( buf3, "$n finishes $N off!" );
            break;
    }
    
    /* use npc short description instead of name keyword */
    if ( IS_NPC(ch) )
        sprintf( buf, "%s just got their head ripped off by %s.", GET_NAME(victim), ch->player.short_descr);

    act( buf1, FALSE, ch, ch->equipment[WEAR_WIELD], victim, TO_CHAR );
    act( buf2, FALSE, ch, ch->equipment[WEAR_WIELD], victim, TO_VICT );
    act( buf3, FALSE, ch, ch->equipment[WEAR_WIELD], victim, TO_NOTVICT );
    send_to_all(buf);
    
    sprintf( buf, "%s capped at %d by %s with %d remaining.",
    GET_NAME(victim),
    IN_ROOM(ch),
    (IS_NPC(ch) ? ch->player.short_descr : GET_NAME(ch)),
    GET_HIT(ch));

    mudlog(BRF, LVL_GOD, TRUE, buf);

    return;
}


/* ============================================================================ 
Constants.
============================================================================ */

/* Hard coded class bonuses *********************************************/
const struct class_ModT shouLinMods[13] = {
    {  4,  1, 3, -10, 0, 1 },
    {  9,  2, 3, -10, 1, 2 },
    { 14,  3, 3, -15, 2, 3 },
    { 19,  4, 3, -15, 3, 4 },
    { 24,  5, 4, -20, 4, 5 },
    { 29,  6, 4, -25, 5, 6 },
    { 34,  7, 4, -30, 6, 7 },
    { 39,  8, 5, -35, 7, 7 },
    { 44,  9, 5, -40, 8, 8 },
    { 49, 10, 5, -45, 9, 8 },
    { 54, 10, 6, -50, 9, 9 },
    { 59, 10, 7, -55, 9, 9 },
    { 64, 10, 8, -60, 9, 9 }
};/* shouLinMods */
