/* ============================================================================ 
reward.c written by Vex of RavenMUD for RavenMUD.
This procedure allows the mob to accept an object from someone, if its the 
right one, it will give them a reward.
============================================================================ */

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "actions/interpreter.h"
#include "general/handler.h"
#include "general/class.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */
#include "util/utils.h"
#include "specials/reward.h"
#include "actions/commact.h"
#include "actions/interpreter.h"

/* ============================================================================ 
The mobs and the objects they want(this is the vnums)
============================================================================ */
#define MOB_LOKI	2818 /* Loki */
#define LOKI_WANT	2834 /* Odin's spear */
#define LOKI_REWARD     2853 /* reward object */

SPECIAL(reward)
{
    int dotmode;
    CharData *the_mob;	/* the_mob that has the procedure. */
    ObjData *obj; 	/* object given to the_mob */
    ObjData *reward_obj; /* object given to ch */
    int reward_vnum;	/* vnum of object to be given to ch */
    int reward_rnum;	/* real number of object to be given to ch */
    char obj_arg[MAX_INPUT_LENGTH], mob_arg[MAX_INPUT_LENGTH];
    char *rem_arg;

    if (!CMD_IS("give")) return FALSE; /* They aren't giving us anything. */

    if (!me) {
	mudlog(NRM, LVL_IMMORT, TRUE, "REWARD: Error -> reward called but me is NULL?");
	return FALSE;
    }

    if (!ch) {
	mudlog(NRM, LVL_IMMORT, TRUE, "REWARD: Error -> reward called but ch is NULL?");
	return FALSE;
    }

    if (IS_SET_AR(PLR_FLAGS(ch), PLR_HUNTED)) return FALSE; /* nice try but no cigar */

    rem_arg = one_argument(argument, obj_arg);
    if (!*obj_arg)
	return FALSE;
    if (is_number(obj_arg))  /* We don't care about money. */
	return FALSE;

    one_argument(rem_arg, mob_arg);
    if ( !(the_mob = give_find_vict(ch, mob_arg)) )
        return FALSE;
    if (!(the_mob == me)) /* there giving something to something else in the same room. */
	return FALSE;

    dotmode = find_all_dots(obj_arg);
    if (dotmode != FIND_INDIV)
	return FALSE;

    if (!(obj = get_obj_in_list_vis(ch, obj_arg, ch->carrying))) {
        return FALSE;
    }

    /* They are giving something to us, but is it what we want? */
    switch (GET_MOB_VNUM(the_mob)) {
    case MOB_LOKI:
	if ( !(GET_OBJ_VNUM(obj) == LOKI_WANT) )
	    return FALSE;

	/* This IS what we wanted! Woo hoo! */
	perform_give(ch, the_mob, obj); /* They give us their object. */
	extract_obj(obj);		/* No double-dipping! */
	do_gen_comm(the_mob, "At last, the beginning of Ragnarok!", 0, SCMD_SHOUT);

	/* Now lets get the object to reward them with. */
	reward_vnum = LOKI_REWARD;
	reward_rnum = real_object(reward_vnum);
	if (reward_rnum > NOTHING) { /* Everything is cool. */
	    reward_obj = read_object(reward_rnum, REAL);
	    obj_to_char(reward_obj, the_mob);
	    sprintf(mob_arg, "I am grateful %s, perhaps you will find this useful.", GET_NAME(ch));
	    do_say(the_mob, mob_arg, 0, 0);
	    sprintf(buf, "%s ", fname(reward_obj->name));
	    strcat(buf, fname(ch->player.name) );
	    do_give(the_mob, buf, 0, 0);
	    mudlog(NRM, LVL_IMMORT, TRUE, "CHECK2: %s", buf);
	}
	else
	    do_say(the_mob, "I'm sorry, but my reward object doesn't exist, please inform a higher god.", 0, 0);
	break;
    default:
	mudlog(NRM, LVL_IMMORT, TRUE, "REWARD: mob[%d] has reward procedure, but isn't set up.", GET_MOB_VNUM(the_mob));
	return FALSE;
    }

    return TRUE;

} /* reward */
