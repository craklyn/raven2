/*
** File (132 column edit): $Id: missile.c,v 1.12 2003/08/20 06:25:37 raven Exp $
**
** $Log: missile.c,v $
** Revision 1.12  2003/08/20 06:25:37  raven
** local changes
**
** Revision 1.11  2003/02/12 04:23:30  raven
** show alts wizcommand, ranged attacks take sanc/shield/ward/etal into account
**
** Revision 1.10  2002/05/08 00:36:49  raven
** fix calm to not work in peace rooms
**
** Revision 1.9  2002/04/13 08:12:04  raven
** demote command, wa/ra revisions
**
** Revision 1.8  2002/01/14 12:56:42  raven
** knight/deathknight changes complete
**
** Revision 1.7  2001/09/28 12:57:46  raven
** byebye bugs
**
** Revision 1.6.2.1  2001/09/28 12:53:51  raven
** fixum bugum
**
** Revision 1.6  2001/08/23 01:06:38  raven
** fixes to envenom, added 'pulse heal' and 'pulse gain' spells and affects, minlevel of 5 to post on board, a few typo fixes
**
** Revision 1.5  2001/08/20 00:18:49  raven
** bugfixes to envenom, escape and retreat
**
** Revision 1.4  2001/08/16 02:40:06  raven
** duration para fix, added escape to parser list
**
** Revision 1.3  2001/08/16 00:19:10  raven
** assassin branch merged
**
** Revision 1.2.2.1  2001/08/15 05:02:17  raven
** Corrections to level usage, stun and movement cost
**
** Revision 1.2  2000/10/10 13:47:04  raven
**
** Transitioned over to the new include structures.
**
** Revision 1.1.1.1  2000/10/10 04:15:17  raven
** RavenMUD 2.0
**
** Revision 1.12  1997/12/07 15:54:27  vex
** Cleanup crew came through.
**
** Revision 1.11  1997/10/19 09:21:07  vex
** Added art of the monkey/wind/flower.
**
** Revision 1.10  1997/10/08 08:50:53  vex
** Fixed crash problem caused from MISSILE_WEAPONS(i.e. shoot skill) using
** action description w/o checking if it was actually set.
**
** Revision 1.9  1997/10/01 13:53:55  digger
** Added a typecast to int for GET_DEX indexing.
**
** Revision 1.8  1997/09/26 05:32:30  vex
** Changed missile_hit to return a value dependent on if the target was
** killed, similiarly for gen_missile. This eliminates a problem with throw
** trying to give an object to somone whose just died.
**
** Revision 1.7  1997/09/19 14:05:21  vex
** Added missile types char* constant.
**
** Revision 1.6  1997/09/18 12:52:36  vex
** world was declared above the inclusion of structs.h, which caused a compiler
** error after I switched everything to typdefs.
**
** Revision 1.5  1997/09/18 10:59:41  vex
** Replaced all obj_data, room_data, mob_special_data, char_data,
** descriptor_data structs with appropriate typedef.
**
** Revision 1.4  1995/07/26 20:45:21  digger
** Added the include string.h.
**
** Revision 1.3  1994/12/19 22:23:06  moon
** Added check so that immortals couldn't be damaged by mortals, but so that
** higher level immorts could damage lower level ones.
**
 * Revision 1.2  1994/12/16  16:58:07  jbp
 * *** empty log message ***
 *
 * Revision 1.1  1994/12/16  14:23:52  jbp
 * Initial revision
 *
*/

/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

/*
** MUD SPECIFIC INCLUDES
*/
#include "general/db.h"
#include "general/structs.h"
#include "general/class.h"
#include "general/color.h"
#include "general/comm.h"
#include "general/handler.h"
#include "actions/interpreter.h"
#include "actions/fight.h"
#include "magic/spells.h"
#include "util/utils.h"
#include "magic/missile.h"
#include "util/weather.h"
#include "magic/skills.h"
#include "actions/fight.h"
#include "magic/sing.h"

/* Constants */
char *missile_types[NUM_MISSILE_TYPES + 1] = {
"Undefined",
"Missile",
"Throw",
"Spell",
"\n" 
};

int grenade_to_vict(CharData *ch, CHAR_DATA *vict, int dir);
ACMD(do_flee);
ObjData *die( CharData *ch, CharData *killer, int pkill);

CHAR_DATA *find_vict_dir( CHAR_DATA *ch, char *vict_name, int range, int dir )
{
    int i = 0;
    int curr_room = 0;
    CHAR_DATA *vict = 0;

	if (!*vict_name || !ch)
		return (0);
	curr_room = ch->in_room;

	for( ;i < (range+1); i++ ) {
		if(( vict = get_targ_room_vis( ch, curr_room, vict_name )))
			break;
			
		if( ROOM_EXIT(curr_room, dir) )
			curr_room = world[curr_room].dir_option[dir]->to_room;
	}
	return (vict);
}


CHAR_DATA *get_targ_room_vis( CHAR_DATA *ch, int room, char *name )
{
	CharData *i;
	int j, number;
	char tmpname[MAX_INPUT_LENGTH];
	char *tmp;
	
	if (!*name)
		return (0);

	strcpy(tmpname, name);
	tmp = tmpname;
	if (!(number = get_number(&tmp)))
		return (0);
	
	for (i = world[room].people, j = 0; i && (j <= number); i = i->next_in_room)
	{
        if (isname(tmp, i->player.name))
			if (CAN_SEE(ch, i))
				if (++j == number)
					return(i);
	}
	return NULL; 
}


void gen_grenade(CHAR_DATA *ch, CHAR_DATA *vict, int dir, int missile_type) {
    int room_no;
    int range;
    
    if (missile_type != MISSILE_TYPE_GRENADE) {
        mudlog(NRM, LVL_IMMORT, TRUE, "missile.c: Function gen_grenade called incorrectly");
        return;
    }

    room_no = ch->in_room;
    
    range = grenade_to_vict(ch, vict, dir);

    return;
}


int grenade_to_vict( CHAR_DATA *ch, CHAR_DATA *vict, int dir )
{
    static char *throw_directions[] = { "north", "east", "south", "west", "up", "down", "\n"};
	static char *rev_directions[] = {"the south", "the west", "the north", "the east", "below", "above"};
	char buf[MAX_INPUT_LENGTH];
	int range = 0;
	int room_no;

	room_no = ch->in_room;

	while ( room_no != vict->in_room)
	{
		if (world[room_no].people)
		{
			sprintf(buf, "A flashbang grenade flies off %swards.", throw_directions[dir]);
			act(buf, FALSE, world[room_no].people, 0, 0, TO_ROOM);
			act(buf, FALSE, world[room_no].people, 0, 0, TO_CHAR);
		}
		room_no = world[room_no].dir_option[dir]->to_room;
		if (world[room_no].people)
		{
			sprintf(buf, "A flashbang grenade flies in from %s.", rev_directions[dir]);
			act(buf, FALSE, world[room_no].people, 0, 0, TO_ROOM);
			act(buf, FALSE, world[room_no].people, 0, 0, TO_CHAR);
		}
		range++;
	}
	return (range);
}



/* This function returns TRUE if vict is slain, FALSE otherwise. Vex. */
bool
gen_missile( CHAR_DATA *ch, CHAR_DATA *vict, OBJ_DATA *miss_obj, int spell_no, int dir, int missile_type )
{
    int room_no;
    int range;
    bool fatality = FALSE;
        
    room_no = ch->in_room;

    switch (missile_type)
    {
        case MISSILE_TYPE_MISSILE:
            range = missile_to_vict(ch, vict, miss_obj->action_description, dir);
            fatality = missile_hit(ch, vict, miss_obj, range, missile_type);
            break;
        case MISSILE_TYPE_THROW:
            range = missile_to_vict(ch, vict, miss_obj->short_description, dir);
            fatality = missile_hit(ch, vict, miss_obj, range, missile_type);
            break;
        case MISSILE_TYPE_GRENADE:
            //range = grenade_to_vict(ch, vict, dir);
            //fatality = missile_hit(ch, vict, miss_obj, range, missile_type);
        case MISSILE_TYPE_SPELL:
            /* some function to deal with missile-oriented spells, i guess */
            break;
        default:
            break;
    }

    return fatality;
}




int missile_to_vict( CHAR_DATA *ch, CHAR_DATA *vict, char *obj_name, int dir )
{
    static char *throw_directions[] = { "north", "east", "south", "west", "up", "down", "\n"};
    static char *rev_directions[] = {"the south", "the west", "the north", "the east", "below", "above"};
    char buf[MAX_INPUT_LENGTH];
    int range = 0;
    int room_no;

    room_no = ch->in_room;

	while ( room_no != vict->in_room)
	{
		if (world[room_no].people)
		{
			sprintf(buf, "%s flies off %swards.", obj_name, throw_directions[dir]);
			act(buf, FALSE, world[room_no].people, 0, 0, TO_ROOM);
			act(buf, FALSE, world[room_no].people, 0, 0, TO_CHAR);
		}
		room_no = world[room_no].dir_option[dir]->to_room;
		if (world[room_no].people)
		{
			sprintf(buf, "%s flies in from %s.", obj_name, rev_directions[dir]);
			act(buf, FALSE, world[room_no].people, 0, 0, TO_ROOM);
			act(buf, FALSE, world[room_no].people, 0, 0, TO_CHAR);
		}
		range++;
	}
	return (range);
}

/* ============================================================================ 
artWind
Shou-lin art of the wind skill. This skill allows a shou-lin to evade missile
attacks.
ch -> attacker
vict -> shou-lin
miss_obj -> the missile
type -> missile type
============================================================================ */
int
artWind( CharData *ch, CharData *vict, ObjData *miss_obj, int type)
{
    char obj_name[MAX_INPUT_LENGTH];

    if (!AWAKE(vict)) return 0;

    if (!CAN_SEE_OBJ(vict, miss_obj)) return 0;

    /* Can only dodge normal missiles */
    if (!(type == MISSILE_TYPE_MISSILE || type == MISSILE_TYPE_THROW)) return 0;

    if (!skillSuccess(vict, SKILL_ART_WIND) || GET_ASPECT(vict) != ASPECT_WIND )
            return 0;

    strcpy(obj_name, miss_obj->short_description);
    /* vict has stopped miss_obj from hitting him/her */

    /* Btw, be careful if you want the shou-lin to catch the missile, cause
    ** the throw skill does the same thing... */
    sprintf(buf, "$N deftly deflects %s.", obj_name);
    act(buf, FALSE, vict, 0, vict, TO_ROOM);
    if (ch->in_room != vict->in_room)
	act(buf, FALSE, ch, 0, vict, TO_CHAR);
    sprintf(buf, "You deftly knock %s aside.", obj_name);
    act(buf, FALSE, vict, 0, vict, TO_CHAR);

    return 1;
}

void CheckEnvenomed (CHAR_DATA *ch, CHAR_DATA *vict, OBJ_DATA *miss_obj)
{
#define NO_APPLY APPLY_NONE
#define NO_BIT   0
#define NO_MOD   0
  int aff_val;
  int lev;
  for(aff_val = 0; aff_val < MAX_OBJ_AFFECT; aff_val++ ) {
    if( miss_obj->affected[aff_val].location == APPLY_POISON){
      lev =  miss_obj->affected[aff_val].modifier;
      miss_obj->affected[aff_val].location = APPLY_NONE;
      miss_obj->affected[aff_val].modifier = 0;
      if( lev >= 35 && !mag_savingthrow(vict, SAVING_PARA) && number(1,3) == 1){
        if (affected_by_spell(vict, SPELL_POISON)) send_to_char("Nothing seems to happen.\r\n", vict);
        else {
          add_affect( NULL, vict, SPELL_POISON, 0, APPLY_STR, -2, 4 TICKS, AFF_POISON,
              FALSE, FALSE, FALSE, FALSE);
          act("You suddenly feel ill!", FALSE, vict, 0, 0, TO_CHAR);
          act("$n turns pale and looks deathly sick!", FALSE, vict, 0, 0, TO_ROOM);
        }
      }
      if(lev >= 43 && !mag_savingthrow(vict, SAVING_PARA) && number(1,5) == 1 &&
              !(IS_NPC(vict) && IS_SET_AR(MOB_FLAGS(vict), MOB_NOBLIND))) {
          if (affected_by_spell(vict, SPELL_BLINDNESS))
              send_to_char("Nothing seems to happen.\r\n", vict);
          else {
              add_affect( ch, vict, SPELL_BLINDNESS, 0, NO_APPLY, NO_MOD, 2 TICKS,
                      AFF_BLIND, FALSE, FALSE, FALSE, FALSE);
              act("Your vision fades to black.", FALSE, vict, 0, 0, TO_CHAR);
              act("$n seems to be blinded!", FALSE, vict, 0, 0, TO_ROOM);
          }
      }
      if(lev >= 48 && !mag_savingthrow(vict, SAVING_PARA) && number(1,6) == 1 &&
              !(GET_HIT(vict) > lev * 100 || GET_LEVEL(vict) > lev)) {
        if (affected_by_spell(vict, SPELL_PARALYZE))
          send_to_char("Nothing seems to happen.\r\n", vict);
        else {
          add_affect( ch, vict, SPELL_PARALYZE, 0,APPLY_AC, 20, 1 TICKS, AFF_PARALYZE,
              FALSE, FALSE, FALSE, FALSE);
          act("Your limbs stiffen, immobilizing you.", FALSE, vict, 0, 0, TO_CHAR);
          act("$n's limbs stiffen, immobilizing them.", FALSE, vict, 0, 0, TO_ROOM);
        }
      }
    }
  }
}

/* Changed this to return false if victim lived, true if they died. */
bool missile_hit( CHAR_DATA *ch, CHAR_DATA *vict, OBJ_DATA *miss_obj, int range, int type )
{
    int calc_thaco;
    int range_val;
    int vict_ac;
    int dam;
    bool fatality = FALSE;
    char obj_name[240];
    byte diceroll;
    char  buf[MAX_INPUT_LENGTH];
    char buf2[MAX_INPUT_LENGTH];
    CHAR_DATA *tmp_ch;
    OBJ_DATA *wpn = 0;
    char *p;
	
    if ( type == MISSILE_TYPE_MISSILE &&
        miss_obj->obj_flags.type_flag == ITEM_MISSILE &&
        miss_obj->action_description)
        strcpy(obj_name, miss_obj->action_description);
    else
        strcpy(obj_name, miss_obj->short_description);

    // Now test for, and remove that newline character
    if ((p = strchr(obj_name, '\n')) != NULL || (p = strchr(obj_name, '\r')) != NULL)
        *p = '\0';

    if (!IS_NPC(ch))
        calc_thaco = thaco[(int) GET_CLASS(ch)][(int) GET_LEVEL(ch)];
    else
        calc_thaco = 20;

    if (miss_obj->obj_flags.type_flag == ITEM_MISSILE || miss_obj->obj_flags.type_flag == ITEM_WEAPON)
        calc_thaco -= dex_app[(int)GET_DEX(ch)].miss_att;
    else
        calc_thaco -= str_app[STRENGTH_APPLY_INDEX(ch)].tohit;
    if (miss_obj->obj_flags.type_flag == ITEM_MISSILE)
        range_val = miss_obj->obj_flags.value[1];
    else
        range_val = DEFAULT_LONG_RANGE;
    
    if (range > range_val / 4) {
        if (range <= range_val /2)
            calc_thaco += 2;
        else
            calc_thaco += 5;
    }

    calc_thaco -= GET_HITROLL(ch);
    diceroll = number(1,20);
    vict_ac = GET_AC(vict) / 10;
	
    if (AWAKE(vict)) vict_ac += dex_app[(int)GET_DEX(ch)].defensive;

    vict_ac = MAX(-10, vict_ac);
	
    if ((((diceroll < 20) && AWAKE(vict)) && ((diceroll == 1) || ((calc_thaco - diceroll) > vict_ac)))) {
        switch (number(1,2)) {
            case 1:
                sprintf(buf, "You miss $N with %s.", obj_name);
                act(buf, FALSE, ch, 0, vict, TO_CHAR);
                sprintf(buf, "You narrowly avoid %s.", obj_name);
                act(buf, FALSE, ch, 0, vict, TO_VICT);
                sprintf(buf, "$N narrowly avoids %s.", obj_name);
                act(buf, FALSE, vict, 0, vict, TO_ROOM);
                break;
            case 2:
                sprintf(buf, "$N barely avoids %s.", obj_name);
                act(buf, FALSE, ch, 0, vict, TO_CHAR);
                sprintf(buf,"You dodge nimbly out of the way of %s.", obj_name);
                act(buf, FALSE, ch, 0, vict, TO_VICT);
                sprintf(buf,"$N dodges nimbly out of the way of %s.", obj_name);
                act(buf, FALSE, vict, 0, vict, TO_ROOM);
                break;
        }
        return FALSE; /* victim still alive. */
    } else {
        switch (miss_obj->obj_flags.type_flag) {
            case ITEM_MISSILE:
                dam = dice(miss_obj->obj_flags.value[1],
                           miss_obj->obj_flags.value[2]);
                break;
            case ITEM_WEAPON:
                dam = dice(miss_obj->obj_flags.value[1],
                           miss_obj->obj_flags.value[2]);
                dam += str_app[STRENGTH_APPLY_INDEX(ch)].todam;
                break;
            case ITEM_FOOD:
                dam = 1; /* ** Digger hack */
                break;
            default:
                dam = MAX(10, ((miss_obj->obj_flags.weight > 0 ?
                                miss_obj->obj_flags.weight : 1) / 5));
                dam += str_app[STRENGTH_APPLY_INDEX(ch)].todam;
                break;
        }
        if (GET_POS(vict) < POS_FIGHTING)
            dam *= 1 + (POS_FIGHTING - GET_POS(vict)) / 3;

        dam = MAX(1, dam);

        if (artWind(ch, vict, miss_obj, type))
        {
            tmp_ch = ch;
            ch = vict;
            vict = tmp_ch;
        }

        if (skillSuccess(ch, SKILL_BULLSEYE) && percentSuccess(50)) {
            sendChar(ch, "You shoot straight and true.\r\n");
            dam *= 2;
            if (percentSuccess(75))
                advanceSkill(ch, SKILL_BULLSEYE, 90,
                    "You are the reincarnation of William Tell!\r\n",
                    FALSE, FALSE, FALSE);
        }

        if (FIGHTING(ch) == vict && type == MISSILE_TYPE_MISSILE)
        {
            if( (wpn = ch->equipment[WEAR_HOLD]) ) {
                // Bonus damage for significant dex bonuses
                dam += number(0, (GET_DEX(ch)-13)*(GET_DEX(ch)-16));

                if(GET_RACE(ch) == RACE_SELF)
                    dam += number(8, 14);

                int i;
                int diminishingReturns = 0;
                // Stun 2 gives -10%.  Stun 3 gives -30%.  4->60%.  5->100%.
                for(i=1; i <= wpn->obj_flags.value[2]; i++)
                    diminishingReturns += (i-1);

                dam = dam*(100*wpn->obj_flags.value[2] - 10*diminishingReturns)/100;
            }

            if(IS_HUNTER(ch) && percentSuccess(40)) {
                sendChar(ch, "The world around you stops for an instant.  You draw a bead on your target.\r\n");
                dam *= 3;
            }
        }

        dam = damage_affects(ch, vict, dam, SKILL_SHOOT);
        dam = damage_limit(ch, vict, dam, SKILL_SHOOT);

        if (GET_LEVEL(vict) < LVL_IMMORT) {
            if(ch->in_room == vict->in_room)
                damage(ch, vict, dam, SKILL_SHOOT);
            else
                GET_HIT(vict) -= dam;
            CheckEnvenomed(ch, vict, miss_obj);
        }
        update_pos(vict);
        if (GET_POS(vict) == POS_DEAD) {
            switch (number(1,3)) {
                case 1:
                    sprintf(buf, "$N is struck in the head by %s and dies.",
                            obj_name);
                    act(buf, FALSE, vict, 0, vict, TO_ROOM);
                    if (ch->in_room != vict->in_room)
                        act(buf, FALSE, ch, 0, vict, TO_CHAR);
                    sprintf(buf, "You are struck solidly in the head by %s "
                                 "and die...", obj_name);
                    act(buf, FALSE, vict, 0, vict, TO_CHAR);
                    break;
                case 2:
                    sprintf(buf, "$N takes %s in the neck, and collapes, "
                                 "clutching at $S throat.", obj_name);
                    act(buf, FALSE, vict, 0, vict, TO_ROOM);
                    if (ch->in_room != vict->in_room)
                        act(buf, FALSE, ch, 0, vict, TO_CHAR);
                    sprintf(buf, "You take %s in the throat, and collapse to "
                                 "the ground.", obj_name);
                    act(buf, FALSE, vict, 0, vict, TO_CHAR);
                    break;
                case 3:
                    sprintf(buf, "$N is hit by %s in the chest and crumbles "
                                 "to the ground.", obj_name);
                    act(buf, FALSE, vict, 0, vict, TO_ROOM);
                    if (ch->in_room != vict->in_room)
                        act(buf, FALSE, ch, 0, vict, TO_CHAR); 
                    sprintf(buf, "You take %s in the chest, and fall to the "
                                 "ground, dead.", obj_name);
                    act(buf, FALSE, vict, 0, vict, TO_CHAR);
                    break;
            }
        } else {
            switch (number(1,3)) {
                case 1:
                    sprintf(buf, "$N is struck solidly by %s.", obj_name);
                    act(buf, FALSE, vict, 0, vict, TO_ROOM);
                    if (ch->in_room != vict->in_room)
                        act(buf, FALSE, ch, 0, vict, TO_CHAR); 
                    sprintf(buf, "You are struck solidly by %s.", obj_name);
                    act(buf, FALSE, vict, 0, vict, TO_CHAR);
                    break;
                case 2:
                    sprintf(buf, "$N takes %s in the shoulder.", obj_name);
                    act(buf, FALSE, vict, 0, vict, TO_ROOM);
                    if (ch->in_room != vict->in_room)
                        act(buf, FALSE, ch, 0, vict, TO_CHAR); 
                    sprintf(buf, "You take %s in the shoulder.", obj_name);
                    act(buf, FALSE, vict, 0, vict, TO_CHAR);
                    break;
                case 3:
                    sprintf(buf, "$N takes %s in the leg.", obj_name);
                    act(buf, FALSE, vict, 0, vict, TO_ROOM);
                    if (ch->in_room != vict->in_room)
                        act(buf, FALSE, ch, 0, vict, TO_CHAR); 
                    sprintf(buf, "You take %s in the leg.", obj_name);
                    act(buf, FALSE, vict, 0, vict, TO_CHAR);
                    break;
            }
        }

        switch (GET_POS(vict)) {
            case POS_MORTALLYW:
                act("$n is mortally wounded, and will die soon, if not aided.",
                        TRUE, vict, 0, 0, TO_ROOM);
                send_to_char("You are mortally wounded, and will die soon, "
                             "if not aided.\r\n", vict);
                break;
            case POS_INCAP:
                act("$n is incapacitated and will slowly die, if not aided.",
                        TRUE, vict, 0, 0, TO_ROOM);
                send_to_char("You are incapacitated and will slowly die, if not "
                             "aided.\r\n", vict);
                break;
            case POS_STUNNED:
                act("$n is stunned, but will probably regain consciousness "
                    "again.", TRUE, vict, 0, 0, TO_ROOM);
                send_to_char("You're stunned, but will probably regain "
                             "consciousness again.\r\n", vict);
                break;
            case POS_DEAD:
                act("$n is dead!  R.I.P.", FALSE, vict, 0, 0, TO_ROOM);
                send_to_char("You are dead!  Sorry...\r\n", vict);
                break;

            default:          /* >= POSITION SLEEPING */
                if (dam > (GET_MAX_HIT(vict) >> 2))
                act("That really did HURT!", FALSE, vict, 0, 0, TO_CHAR);

                if (GET_HIT(vict) < (GET_MAX_HIT(vict) >> 2)) {
                    sprintf(buf2, "%sYou wish that your wounds would stop "
                                  "BLEEDING so much!%s\r\n",
                    CCRED(vict, C_SPR), CCNRM(vict, C_SPR));
                    send_to_char(buf2, vict);
                    if (MOB_FLAGGED(vict, MOB_WIMPY))
                    do_flee(vict, "", 0, 0);
                }
                if (!IS_NPC(vict) && GET_WIMP_LEV(vict) && vict != ch &&
                    GET_HIT(vict) < GET_WIMP_LEV(vict)) {
                    send_to_char("You wimp out, and attempt to flee!\r\n",
                            vict);
                    do_flee(vict, "", 0, 0);
                }
                break;
        }

        if (!IS_NPC(vict) && !(vict->desc)) {
            do_flee(vict, "", 0, 0);
            if (!FIGHTING(vict)) {
                act("$n is rescued by divine forces.", FALSE, vict, 0, 0,
                        TO_ROOM);
                GET_WAS_IN(vict) = vict->in_room;
                char_from_room(vict);
                char_to_room(vict, 0);
            }
        }
        if (!AWAKE(vict)) if (FIGHTING(vict)) stop_fighting(vict);

        /* stick xp gain in here if we want it for remote attacks */

        if (GET_POS(vict) == POS_DEAD) {
            if (!IS_NPC(vict)) {
                mudlog(BRF, LVL_IMMORT, TRUE, "%s killed by %s at %s", GET_NAME(vict),
                        GET_NAME(ch), world[IN_ROOM(vict)].name);
                /* insert seek check here */
                if (MOB_FLAGGED(ch, MOB_MEMORY)) forget(ch, vict);
            }
            die(vict, NULL, 0);
            fatality = TRUE;
        }
    }
    return fatality;
}
