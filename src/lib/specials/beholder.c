
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/class.h"
#include "general/comm.h"
#include "actions/interpreter.h"
#include "util/utils.h"
#include "general/handler.h"
#include "specials/special.h"
#include "specials/beholder.h"
#include "magic/spells.h"

ACMD(do_flee);
ACMD(do_stand);

#define EYE_CHARM  0
#define EYE_CLONE 1
#define EYE_SLEEP 2
#define EYE_TELEKINESE 3
#define EYE_PETRIFY 4
#define EYE_DISINTEGRATE 5
#define EYE_FEAR 6
#define EYE_SLOW 7
#define EYE_HARM 8
#define EYE_DEATH 9
#define EYE_ANTI_MAGIC 11
#define MAX_BEHOLDER_VICTIMS 20

extern void die(CharData *, CharData *, int);
extern void death_cry(struct char_data *);

void make_statue(struct char_data * ch)
{
  struct obj_data *statue, *o;
  struct obj_data *money;
  struct extra_descr_data *new_descr;
  int i;

  struct obj_data *create_money(int amount);

  statue = create_obj();

  statue->item_number = NOTHING;
  statue->in_room = NOWHERE;
  statue->name = str_dup("statue");

  sprintf(buf2, "%s is standing here.", GET_NAME(ch));
  statue->description = str_dup(buf2);

  sprintf(buf2, "the statue of %s", GET_NAME(ch));
  statue->short_description = str_dup(buf2);

  GET_OBJ_TYPE(statue) = ITEM_CONTAINER;
  /* Vex - made PC statues untakeable. */
  if (IS_NPC(ch))
    SET_BIT_AR(GET_OBJ_WEAR(statue), ITEM_WEAR_TAKE);
  else
  {
    SET_BIT_AR(GET_OBJ_WEAR(statue), ITEM_WEAR_HOLD); /* No take flags - but I hate 0's */
    sprintf(buf2, "%s", GET_NAME(ch));
    CREATE(new_descr, struct extra_descr_data, 1);
    new_descr->keyword = str_dup(buf2); /* Checked later on.. */
    sprintf(buf2, "%s appears to have been turned to stone!", GET_NAME(ch));
    new_descr->description = str_dup(buf2); /* Just decoration. */
    new_descr->next = NULL;
    statue->ex_description = new_descr;
  }
  SET_BIT_AR(GET_OBJ_EXTRA(statue), ITEM_NODONATE);
  GET_OBJ_VAL(statue, 0) = 0;	/* You can't store stuff in a statue */
  GET_OBJ_VAL(statue, 3) = 1;	/* corpse identifier */
  GET_OBJ_WEIGHT(statue) = GET_WEIGHT(ch) + IS_CARRYING_W(ch);
  GET_OBJ_RENT(statue) = 100000;

    if( IS_CLONE(ch) )
        GET_OBJ_TIMER(statue) = number( 1, 3 );
    else if( IS_NPC(ch) )
        GET_OBJ_TIMER(statue) = CONFIG_MAX_NPC_CORPSE_TIME;
    else
        GET_OBJ_TIMER(statue) = CONFIG_MAX_PC_CORPSE_TIME;

    /* transfer character's inventory to the statue */
    statue->contains = ch->carrying;
    for( o = statue->contains; o != NULL; o = o->next_content ){
        if( IS_CLONE(ch) &&
            IS_SET_AR(o->obj_flags.extra_flags, ITEM_TIMED)){
            GET_OBJ_TIMER(o)=number(2,6);
        }
        o->in_obj = statue;
    }
    object_list_new_owner(statue, NULL);

    /* transfer character's equipment to the statue */
    for( i = 0; i < NUM_WEARS; i++ ){
        if( ch->equipment[i] ){
            ObjData *o = unequip_char(ch, i);
            if( IS_CLONE(ch) &&
                IS_SET_AR(o->obj_flags.extra_flags, ITEM_TIMED)){
                GET_OBJ_TIMER(o)=number(2,6);
            }
            obj_to_obj( o, statue );
        }
    }
  /* transfer gold */
  if (GET_GOLD(ch) > 0) {
    /* following 'if' clause added to fix gold duplication loophole */
    if (IS_NPC(ch) || (!IS_NPC(ch) && ch->desc)) {
      money = create_money(GET_GOLD(ch));
      obj_to_obj(money, statue);
    }
    GET_GOLD(ch) = 0;
  }
  ch->carrying = NULL;
  IS_CARRYING_N(ch) = 0;
  IS_CARRYING_W(ch) = 0;

  obj_to_room(statue, ch->in_room);
}


void make_dust(struct char_data * ch)
{
  struct obj_data *dust, *o;
  struct obj_data *money;
  struct extra_descr_data *new_descr;
  int i;

  struct obj_data *create_money(int amount);

  dust = create_obj();

  dust->item_number = NOTHING;
  dust->in_room = NOWHERE;
  dust->name = str_dup("dust");

  sprintf(buf2, "A pile of dust is lying here.");
  dust->description = str_dup(buf2);

  sprintf(buf2, "the dust of %s", GET_NAME(ch));
  dust->short_description = str_dup(buf2);

  GET_OBJ_TYPE(dust) = ITEM_CONTAINER;
  /* Vex - made PC dusts untakeable. */
  if (IS_NPC(ch))
    SET_BIT_AR(GET_OBJ_WEAR(dust), ITEM_WEAR_TAKE);
  else
  {
    SET_BIT_AR(GET_OBJ_WEAR(dust), ITEM_WEAR_HOLD); /* No take flags - but I hate 0's */
    sprintf(buf2, "%s", GET_NAME(ch));
    CREATE(new_descr, struct extra_descr_data, 1);
    new_descr->keyword = str_dup(buf2); /* Checked later on.. */
    sprintf(buf2, "It would appear that %s was disintergrated...", GET_NAME(ch));
    new_descr->description = str_dup(buf2); /* Just decoration. */
    new_descr->next = NULL;
    dust->ex_description = new_descr;
  }
  SET_BIT_AR(GET_OBJ_EXTRA(dust), ITEM_NODONATE);
  GET_OBJ_VAL(dust, 0) = 0;	/* You can't store stuff in a dust */
  GET_OBJ_VAL(dust, 3) = 1;	/* dust identifier */
  GET_OBJ_WEIGHT(dust) = GET_WEIGHT(ch) + IS_CARRYING_W(ch);
  GET_OBJ_RENT(dust) = 100000;

    if( IS_CLONE(ch) )
        GET_OBJ_TIMER(dust) = number( 1, 3 );
    else if( IS_NPC(ch) )
        GET_OBJ_TIMER(dust) = CONFIG_MAX_NPC_CORPSE_TIME;
    else
        GET_OBJ_TIMER(dust) = CONFIG_MAX_PC_CORPSE_TIME;

    /* transfer character's inventory to the dust */
    dust->contains = ch->carrying;
    for( o = dust->contains; o != NULL; o = o->next_content ){
        if( IS_CLONE(ch) &&
            IS_SET_AR(o->obj_flags.extra_flags, ITEM_TIMED)){
            GET_OBJ_TIMER(o)=number(2,6);
        }
        o->in_obj = dust;
    }
    object_list_new_owner(dust, NULL);

    /* transfer character's equipment to the dust */
    for( i = 0; i < NUM_WEARS; i++ ){
        if( ch->equipment[i] ){
            ObjData *o = unequip_char(ch, i);
            if( IS_CLONE(ch) &&
                IS_SET_AR(o->obj_flags.extra_flags, ITEM_TIMED)){
                GET_OBJ_TIMER(o)=number(2,6);
            }
            obj_to_obj( o, dust );
        }
    }
  /* transfer gold */
  if (GET_GOLD(ch) > 0) {
    /* following 'if' clause added to fix gold duplication loophole */
    if (IS_NPC(ch) || (!IS_NPC(ch) && ch->desc)) {
      money = create_money(GET_GOLD(ch));
      obj_to_obj(money, dust);
    }
    GET_GOLD(ch) = 0;
  }
  ch->carrying = NULL;
  IS_CARRYING_N(ch) = 0;
  IS_CARRYING_W(ch) = 0;

  obj_to_room(dust, ch->in_room);
}


/* Pick an eye to use */
int select_eye(sh_int  eyes[10])
{
	int j;
    	int safety = 0; /* Just in case ... */ 

	j = number(0, 9);
	while(!eyes[j]) {
		j = number(0, 9);
		safety++;
		if (safety > 1000)  /* So I'm paranoid */
		{
		     mudlog(NRM, LVL_IMMORT, TRUE, "BEHOLDER: WARNING! Passed 1000 iterations looking for a unused eye!");
		     return FALSE;
		}
	}
	eyes[j] = FALSE;
	return j;
}

/* Attack victim with eye. */
void eye_ray_victim(struct char_data *beholder, struct char_data *victim, int eye)
{

	switch(eye) {
		case EYE_CHARM:
			mudlog(NRM, LVL_IMMORT, TRUE, "BEHOLDER: %s charm ray %s", GET_NAME(beholder), GET_NAME(victim));
			break;
		case EYE_CLONE:
			mudlog(NRM, LVL_IMMORT, TRUE, "BEHOLDER: %s clone ray %s", GET_NAME(beholder), GET_NAME(victim));
			break;
		case EYE_SLEEP:
			mudlog(NRM, LVL_IMMORT, TRUE, "BEHOLDER: %s sleep ray %s", GET_NAME(beholder), GET_NAME(victim));
			break;
		case EYE_TELEKINESE:
			mudlog(NRM, LVL_IMMORT, TRUE, "BEHOLDER: %s telekinese ray %s", GET_NAME(beholder), GET_NAME(victim));
			break;
		case EYE_PETRIFY:
			mudlog(NRM, LVL_IMMORT, TRUE, "BEHOLDER: %s petrify ray %s", GET_NAME(beholder), GET_NAME(victim));
			sprintf(buf, "A pale grey ray from %s strikes %s!", GET_NAME(beholder), GET_NAME(victim));
			act(buf, FALSE, victim, FALSE, FALSE, TO_ROOM);
			if (!mag_savingthrow(victim, SAVING_PETRI))
			{
				mudlog(NRM, LVL_IMMORT, TRUE, "%s petrified by %s.",  GET_NAME(victim), GET_NAME(beholder));
				sendChar(victim, "You have been petrified!\r\n");
				death_cry(victim);
				make_statue(victim);
				extract_char(victim);
			}
			break;
		case EYE_DISINTEGRATE:
			mudlog(NRM, LVL_IMMORT, TRUE, "BEHOLDER: %s disintegrate ray %s", GET_NAME(beholder), GET_NAME(victim));
			sprintf(buf, "A deathly green ray from %s strikes %s!", GET_NAME(beholder), GET_NAME(victim));
			act(buf, FALSE, victim, FALSE, FALSE, TO_ROOM);
			if (!mag_savingthrow(victim, SAVING_ROD))
			{
				mudlog(NRM, LVL_IMMORT, TRUE, "%s disintegrated by %s.",  GET_NAME(victim), GET_NAME(beholder));
				sendChar(victim, "You have been disintegrated!\r\n");
				death_cry(victim);
				make_dust(victim);
				extract_char(victim);
			}
			break;
		case EYE_FEAR:
			mudlog(NRM, LVL_IMMORT, TRUE, "BEHOLDER: %s fear ray %s", GET_NAME(beholder), GET_NAME(victim));
			sprintf(buf, "A yellow ray from %s strikes %s!", GET_NAME(beholder), GET_NAME(victim));
			act(buf, FALSE, victim, FALSE, FALSE, TO_ROOM);
			if (!mag_savingthrow(victim, SAVING_BREATH))
			{
				sendChar(victim, "You feel afraid!\r\n");
				do_flee(victim, "", 0, 0);
			}
			break;
		case EYE_SLOW:
			mudlog(NRM, LVL_IMMORT, TRUE, "BEHOLDER: %s slow ray %s", GET_NAME(beholder), GET_NAME(victim));
			break;
		case EYE_HARM:
			mudlog(NRM, LVL_IMMORT, TRUE, "BEHOLDER: %s harm ray %s", GET_NAME(beholder), GET_NAME(victim));
			break;
		case EYE_DEATH:
			mudlog(NRM, LVL_IMMORT, TRUE, "BEHOLDER: %s death ray %s", GET_NAME(beholder), GET_NAME(victim));
			sprintf(buf, "%s is struck by a ebony ray from %s!", GET_NAME(victim), GET_NAME(beholder));
			act(buf, FALSE, victim, FALSE, FALSE, TO_ROOM);
			if (!mag_savingthrow(victim, SAVING_PARA))
			{
				mudlog(NRM, LVL_IMMORT, TRUE, "%s slain by %s's death ray.", GET_NAME(victim), GET_NAME(beholder));
				die(victim, NULL, FALSE);
			}
			break;
		default:
			mudlog(NRM, LVL_IMMORT, TRUE, "BEHOLDER: ERROR, unknown eye ray.");
			break;
	} /* switch */
	return;
}

SPECIAL(beholder)
{
    
    struct char_data *beholder, *tmp_ch, *victim;
    struct char_data *attacker_array[MAX_BEHOLDER_VICTIMS]; /* If there are more than 20, the extras are just lucky :p */
    sh_int eyes[10];
    int  i, eye, no_attackers, no_eyes, arc_size, front, left, right, rear, valid_arcs;

    for (i=0; i<MAX_BEHOLDER_VICTIMS; i++)  attacker_array[i] = NULL;
    for (i=0; i<10; i++) eyes[i] = TRUE;
    no_attackers = -1; /*  nobody so far */
    beholder = ch;

    if( !IS_NPC( beholder ))                   return FALSE;
    if( STUNNED( beholder ))                   return FALSE;
    if( IS_AFFECTED( beholder, AFF_PARALYZE )) return FALSE;

    if( FIGHTING(beholder) && (GET_POS(beholder) == POS_SITTING || GET_POS(beholder) == POS_RESTING)) {
        do_stand( beholder, "", 0, 0 );
        return FALSE;
    }


   if(GET_POS(ch) != POS_FIGHTING && !FIGHTING(beholder)) return FALSE;

/*
** Need to search room beholder is in and make up a list of all the
** victims, consider list to be circular, wrapping around from 1st
** person to last.
*/

    for (tmp_ch = world[beholder->in_room].people; tmp_ch; tmp_ch = tmp_ch->next_in_room) {
        if( FIGHTING(tmp_ch) == beholder ){
		no_attackers++;
		attacker_array[no_attackers] = tmp_ch;
        }
	if (no_attackers == (MAX_BEHOLDER_VICTIMS - 1))
	{
		mudlog(NRM, LVL_IMMORT, TRUE, "BEHOLDER: Counted %d attackers against %s, continuing.", no_attackers + 1,  GET_NAME(beholder));
		continue;
	}
    }
/*
** At this point the number of people fighting the  beholder is equal to
** no_attackers + 1
*/

    if (no_attackers < 0)
    {
	mudlog(NRM, LVL_IMMORT, TRUE, "BEHOLDER: invoked, but no one fighting %s in the room. Confused, so bailing out.", GET_NAME(beholder));
	return FALSE;
    }

/*
** Work out if any eyes have been destroyed. Work out if any destroyed eyes
** have healed.
*/



   else if( cmd == 0 ){

/*
** Pick a target for anti-magic eye, this is beholders facing.
** Divide list by 4 to determine the beholders arcs, must be at least
** 3 people in each arc
*/
	front = number(0, no_attackers); /* Just pick  at random for now */

	/* Move the target round to the front, makes it easier to spread things around */
	tmp_ch = attacker_array[0];
	attacker_array[0] = attacker_array[front];
	attacker_array[front] = tmp_ch;
	front = 0;

	victim = attacker_array[front];
	mudlog(NRM, LVL_IMMORT, TRUE, "BEHOLDER: Anti-magic eye targetted against %s", GET_NAME(victim));

	valid_arcs = MIN(4 ,MAX(1, (no_attackers + 1)/2));
	mudlog(NRM, LVL_IMMORT, TRUE, "BEHOLDER: valid_arcs = %d", valid_arcs);
	arc_size = (no_attackers + 1)/valid_arcs;  
	left = front + arc_size + ((no_attackers + 1) % valid_arcs) - 1;

/*
** For central arc, pick 1d4 eyes and randomly attack players in that
** arc.
*/
	no_eyes = number(1, 4);
	for (i = 1; i <= no_eyes; i++)
	{
	    victim = attacker_array[number(front, left)];
	    eye = select_eye(eyes);
	    eye_ray_victim(beholder, victim, eye);
	} /* for */
/*
** For all other arcs, assign 1d2 eyes and attack players in that arc
*/
	if (valid_arcs >= 2)
	{
		right = left + arc_size;
		no_eyes = number(1, 2);
		for (i = 1; i <= no_eyes; i++)
		{
	    		victim = attacker_array[number(left, right)];
	   		eye = select_eye(eyes);
	    		eye_ray_victim(beholder, victim, eye);
		} /* for */
	}
	if (valid_arcs >= 3)
	{
		rear = right + arc_size;
		no_eyes = number(1, 2);
		for (i = 1; i <= no_eyes; i++)
		{
	    		victim = attacker_array[number(right, rear)];
	   		eye = select_eye(eyes);
	    		eye_ray_victim(beholder, victim, eye);
		} /* for */
	}
	if (valid_arcs == 4)
	{
		no_eyes = number(1, 2);
		for (i = 1; i <= no_eyes; i++)
		{
	    		victim = attacker_array[number(rear, no_attackers)];
	   		eye = select_eye(eyes);
	    		eye_ray_victim(beholder, victim, eye);
		} /* for */
	}

   }/* else */

   return FALSE;

}/* beholder */
