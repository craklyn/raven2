
// Decided to orgainise player killing/stealing/looting flag stuff
// some what.
// Vex.

// Mortius 110906: Added include for handler.h to resolve the get_char_vis
// outlaw error

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/handler.h"
#include "util/utils.h"
#include "general/class.h"
#include "general/comm.h"
#include "actions/outlaw.h"
#include "actions/interpreter.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */

/* handle pardon lists */
void clear_pardon_list(CharData *ch)
{
    PardonList *pardon;

    while (ch->player_specials->pardons) {
        pardon = ch->player_specials->pardons->next;
        free(ch->player_specials->pardons->name);
        free(ch->player_specials->pardons);
        ch->player_specials->pardons = pardon;
    }
}

void add_to_pardon_list(CharData *ch, char *name)
{
    PardonList *pardon;
    
    CREATE(pardon, PardonList, 1);
    pardon->name = str_dup(name);
    pardon->next = ch->player_specials->pardons;
    ch->player_specials->pardons = pardon;
}

void remove_from_pardon_list(CharData *ch, char *name)
{
    PardonList *pardon = ch->player_specials->pardons, *last = NULL;

    while (pardon) {
        if (strcmp(pardon->name, name) == 0) {
            if (last) last->next = pardon->next;
            else ch->player_specials->pardons = pardon->next;
            free(pardon->name);
            free(pardon);
            pardon = NULL;
        } else {
            last = pardon;
            pardon = pardon->next;
        }
    }
}

int is_in_pardon_list(CharData *ch, char *name)
{
    PardonList *pardon = ch->player_specials->pardons;

    while (pardon) {
        if (strcmp(pardon->name, name) == 0) return 1;
        pardon = pardon->next;
    }

    return 0;
}

void unset_killer_player(CharData *ch)
{
    if (!ch) {
	mudlog(NRM, LVL_IMMORT, TRUE, "OUTLAW ERROR: null ch to unset_killer_player.");
	return;
    }

    if (IS_NPC(ch)) {
	mudlog(NRM, LVL_IMMORT, TRUE, "OUTLAW ERROR: Attempt to remove killer flag from npc.");
	return;
    }
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_KILLER);
    (ch)->player_specials->saved.pkill_countdown = 0;
}

void unset_thief_player(CharData *ch)
{
    if (!ch) {
	mudlog(NRM, LVL_IMMORT, TRUE, "OUTLAW ERROR: null ch to unset_thief_player.");
	return;
    }

    if (IS_NPC(ch)) {
	mudlog(NRM, LVL_IMMORT, TRUE, "OUTLAW ERROR: Attempt to remove thief flag from npc.");
	return;
    }
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_THIEF);
    (ch)->player_specials->saved.pthief_countdown = 0;
}

void unset_hunted_player(CharData *ch)
{
    if (!ch) {
	mudlog(NRM, LVL_IMMORT, TRUE, "OUTLAW ERROR: null ch to unset_hunted_player.");
	return;
    }

    if (IS_NPC(ch)) {
	mudlog(NRM, LVL_IMMORT, TRUE, "OUTLAW ERROR: Attempt to remove hunted flag from npc.");
	return;
    }
    REMOVE_BIT_AR(PLR_FLAGS(ch), PLR_HUNTED);
    (ch)->player_specials->saved.phunt_countdown = 0;
    clear_pardon_list(ch);
}

void
player_attack_victim( CharData * ch,
              	    CharData * vict )
{

  if (!ch) {
	mudlog(NRM, LVL_IMMORT, TRUE, "OUTLAW ERROR: Null ch passed to player_attack_victim");
	return;
  }

  if (!vict) {
	mudlog(NRM, LVL_IMMORT, TRUE, "OUTLAW ERROR: Null vict passed to player_attack_victim");
	return;
  }
  /*
  ** This function assumes that ch is initiating an attack on vict.
  */
  if( IS_NPC(ch) &&
      IS_SET_AR(AFF_FLAGS(ch), AFF_CHARM) &&
      ch->master &&
     !IS_NPC(ch->master))
  {
    if (ch->in_room == ch->master->in_room)
      player_attack_victim(ch->master, vict); /* This picks up use of charms. */
    return;
  }

  /* Here we do some fiddling to treat an attack on a charmed creature
   * as an attack on its master */
  if (IS_NPC(vict) && IS_SET_AR(AFF_FLAGS(vict), AFF_CHARM) && vict->master)
      vict = vict->master;

  if(
     !IS_NPC(vict)                    &&
     !IS_NPC(ch)                      &&
     !PLR_FLAGGED(vict, PLR_HUNTED)   &&
     !PLR_FLAGGED(vict, PLR_THIEF)    &&
     !IN_ARENA(ch)		      &&
     !ZONE_FLAGGED(world[ch->in_room].zone, ZONE_ARENA) &&
     !ZONE_FLAGGED(world[ch->in_room].zone, ZONE_SLEEPTAG) &&
     !IN_QUEST_FIELD(ch)	      &&
     (ch != vict ))
  {
    if (!PLR_FLAGGED(ch, PLR_KILLER)) 
        mudlog( BRF, LVL_IMMORT, TRUE,
        "PC Killer bit set on %s for initiating attack on %s at %s.",
               GET_NAME(ch), GET_NAME(vict), world[vict->in_room].name);
    set_killer_player(ch, KILLER_TIME);
  }

}/* player_attack_victim */


// Set a KILLER flag on ch for duration ticks.
void set_killer_player( CharData *ch, const int duration )
{
    if (IS_NPC(ch)) {
	mudlog(NRM, LVL_IMMORT, TRUE, "OUTLAW ERROR: attempt to set KILLER flag on a npc.");
	return;	
    }

    if (!PLR_FLAGGED(ch, PLR_KILLER))
        send_to_char("You are now a KILLER!\r\n", ch);

    SET_BIT_AR(PLR_FLAGS(ch), PLR_KILLER);
    if ((ch)->player_specials->saved.pkill_countdown < duration)
        (ch)->player_specials->saved.pkill_countdown = duration;
}

// Set a THIEF flag on ch for duration ticks.
void set_thief_player( CharData *ch, const int duration )
{
    if (IS_NPC(ch)) {
	mudlog(NRM, LVL_IMMORT, TRUE, "OUTLAW ERROR: attempt to set THIEF flag on a npc.");
	return;	
    }

    send_to_char("You are now a known THIEF! \r\n", ch);
    SET_BIT_AR(PLR_FLAGS(ch), PLR_THIEF);
    if ((ch)->player_specials->saved.pthief_countdown < duration)
        (ch)->player_specials->saved.pthief_countdown = duration;
}

// Set a HUNTED flag on ch for duration ticks.
void set_hunted_player( CharData *ch, const int duration )
{
    if (IS_NPC(ch)) {
	mudlog(NRM, LVL_IMMORT, TRUE, "OUTLAW ERROR: attempt to set HUNTED flag on a npc.");
	return;	
    }

    /* if the flag is not set at all, ensure the message appears */
    if (!IS_SET_AR(PLR_FLAGS(ch), PLR_HUNTED))
        (ch)->player_specials->saved.phunt_countdown = duration - 1;

    SET_BIT_AR(PLR_FLAGS(ch), PLR_HUNTED);

    /* only display the HUNTED message if the duration is increased */
    if ((ch)->player_specials->saved.phunt_countdown < duration) {
        (ch)->player_specials->saved.phunt_countdown = duration;
        send_to_char("You are now &08&25HUNTED!&00\r\n", ch);
    }

    // You automatically get a free KILLER flag to go with your HUNTED flag!
    set_killer_player(ch, duration);
}

// ch has stolen from victim, report and set flags
void player_steal_victim( CharData *ch, CharData *victim)
{
    if (IS_NPC(ch) || IS_NPC(victim))
	return;	// we don't care.

    mudlog(NRM, LVL_IMMORT, TRUE, "PLAYER STEAL: %s stole from %s.", GET_NAME(ch), GET_NAME(victim));
    // player stole from a player, tag 'em so other players will know.
    set_thief_player(ch, THIEF_TIME);
    set_hunted_player(ch, HUNTED_TIME);	// Thieves also get a HUNTED flag.
}

// ch has looted the victims corpse, report and set flags.
void player_loot_corpse( CharData *ch, ObjData *corpse )
{
    static CharData *lastch = NULL;
    static ObjData *lastcorpse = NULL;

    if (IS_NPC(ch)) {
	// monster looting a players corpse, thats fine.
        mudlog( BRF, LVL_CREATOR, TRUE, "MOB LOOT: %s looted corpse of %s.", GET_NAME(ch), corpse->ex_description->keyword);
        return;
    }

    REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_CONSENT);

    /* don't get too noisy with this log */
    if (lastch != ch || lastcorpse != corpse) {
        mudlog( BRF, LVL_CREATOR, TRUE, "PLAYER LOOT: %s looted corpse of %s.", GET_NAME(ch), corpse->ex_description->keyword);
    }
    lastch = ch; lastcorpse = corpse;

    /* extract the name of the player whose corpse was looted */
    if (corpse->ex_description) {
        if (!is_in_pardon_list(ch, corpse->ex_description->keyword))
            add_to_pardon_list(ch, corpse->ex_description->keyword);
    }

	// Set corpse to 'has been looted' - craklyn
	SET_BIT_AR(corpse->obj_flags.extra_flags, ITEM_LOOTED);
    
	set_hunted_player(ch, HUNTED_TIME);
}

ACMD(do_pardon)
{
    char arg[100];
    CharData *outlaw;

    // Mortius 110906: Changed order of checks.  If there is no argument why
    // go any further.
    one_argument(argument, arg);

    if (!*arg) {
        sendChar(ch, "Who do you want to forgive?\r\n");
        return;
    }

    if (!(outlaw = get_char_vis(ch, arg, 0))) {
        sendChar(ch, "No-one around by that name.\r\n");
        return;
    }

    /* immortals have a different type of pardon */
    if (GET_LEVEL(ch) > MAX_MORTAL) {
        do_wizutil(ch, argument, cmd, SCMD_PARDON);
        return;
    }

    if (IS_SET_AR(PLR_FLAGS(ch), PLR_HUNTED)) {
        sendChar(ch, "You are not trusted to give pardons right now.\r\n");
        return;
    }

    if (!IS_SET_AR(PLR_FLAGS(outlaw), PLR_HUNTED)) {
        sendChar(ch, "There is nothing to pardon.\r\n");
        return;
    }

    if (IS_NPC(ch) || !is_in_pardon_list(outlaw, GET_NAME(ch))) {
        sendChar(ch, "You have nothing to pardon them for.\r\n");
        return;
    }

    remove_from_pardon_list(outlaw, GET_NAME(ch));

    sprintf(buf, "%s has pardoned you.\r\n", GET_NAME(ch));
    sendChar(outlaw, buf);

    sprintf(buf, "You have pardoned %s.\r\n", GET_NAME(outlaw));
    sendChar(ch, buf);

    /* if that is everyone ... */
    if (!outlaw->player_specials->pardons) {
        sendChar(outlaw, "You are no longer hunted.\r\n");
        unset_hunted_player(outlaw);
    }
}
