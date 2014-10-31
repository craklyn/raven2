/* ************************************************************************
*   File: commact.c                                     Part of CircleMUD *
*  Usage: Player-level communication commands                             *
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
#include "general/class.h"
#include "general/comm.h"
#include "actions/interpreter.h"
#include "general/handler.h"
#include "general/color.h"
#include "util/weather.h"
#include "actions/commact.h"
#include "scripts/dg_scripts.h"
#include "magic/spells.h"

// The following function is to test if a player is carrying
// a speech stick, which will qualify them as "able to talk".

int carrying_speech_stick(CharData *ch)
{
	ObjData *obj, *objnext = NULL;

	for( obj = ch->carrying; obj; obj = objnext ) {
		objnext = obj->next_content;
		if ( GET_OBJ_VNUM(obj) == 1445 ) {
			return 1;
		}
	}  // for

	// No quest item, returning 0:
	return 0;
}

ACMD(do_say)
{
  if (!IS_NPC(ch) && IS_SET_AR(PLR_FLAGS(ch), PLR_NOCOMM)) {
      send_to_char("You are unable to talk.\r\n", ch);
      return;
  }

  skip_spaces(&argument);
  
  if (!*argument)
      send_to_char("Yes, but WHAT do you want to say?\r\n", ch);
  else {
      if ( IS_AFFECTED(ch, AFF_SILENCE) ||
              (world[(ch)->in_room].number == 1231 && !carrying_speech_stick(ch)) ){
          send_to_char("Your lips move, but you don't seem to make a sound...\r\n", ch);
          act("$n makes a face at everyone!", FALSE, ch, 0, 0, TO_ROOM);
      }
      else if (!mortal_color(argument, ch))
          return;
      else {
          if (PRF_FLAGGED(ch, PRF_NOREPEAT))
              send_to_char(CONFIG_OK, ch);
          else {
              sprintf(buf, "You say, '%s&00'", argument);
              act(buf, FALSE, ch, 0, argument, TO_CHAR);
          }
          
          if( !PLR_FLAGGED(ch, PLR_SHUNNED)){
              argument = makedrunk(argument, ch);
              sprintf(buf, "$n says, '%s&00'", argument);
              //act(buf, FALSE, ch, 0, 0, TO_ROOM);
              act(buf, FALSE, ch, 0, 0, TO_ROOM|DG_NO_TRIG);
          }
      }
  }

  /* trigger check */
  speech_mtrigger(ch, argument);
  speech_wtrigger(ch, argument);
}


ACMD(do_gsay)
{
  FollowType *f;
  skip_spaces(&argument);

  if (!IS_NPC(ch) && IS_SET_AR(PLR_FLAGS(ch), PLR_NOCOMM)) {
      send_to_char("You are unable to use group say.\r\n", ch);
      return;
  }

  if(!IS_AFFECTED(ch, AFF_GROUP)) {
    sendChar( ch, "But you are not the member of a group!\r\n" );
    return;
  }

  if(IS_AFFECTED(ch, AFF_SILENCE)) {
    sendChar( ch, "Your lips move, but you don't seem to make a sound...\r\n" );
    return;
  }

  if( !*argument ) {
    sendChar( ch, "Yes, but WHAT do you want to group-say?\r\n" );
  }
  else if( !mortal_color(argument, ch) ) return;
  else {
    CharData *k = (ch->master ? ch->master : ch);

    if(PRF_FLAGGED(ch, PRF_NOREPEAT)) {
        sendChar( ch, CONFIG_OK );
    }
    else {
        sprintf( buf, "You tell the group, '%s'", argument );
        act( buf, FALSE, ch, 0, 0, TO_CHAR | TO_SLEEP );
    }

    argument = makedrunk(argument, ch);
    sprintf( buf, "$n tells the group, '%s'&00", argument);

    if( IS_AFFECTED(k, AFF_GROUP) && (k != ch) ) {
      act( buf, FALSE, ch, 0, k, TO_VICT | TO_SLEEP );
    }

    for( f = k->followers; f; f = f->next ) {
      if( IS_AFFECTED(f->follower, AFF_GROUP) && (f->follower != ch) ) {
        act( buf, FALSE, ch, 0, f->follower, TO_VICT | TO_SLEEP );
      }
    }

  }
}

//Team-say
ACMD(do_tsay)
{
  DescriptorData *questor;

  if (!IS_NPC(ch) && IS_SET_AR(PLR_FLAGS(ch), PLR_NOCOMM)) {
      send_to_char("You are unable to use team say.\r\n", ch);
      return;
  }

  skip_spaces(&argument);

  if((!PRF_FLAGGED(ch, PRF_GOLD_TEAM)) && (!PRF_FLAGGED(ch, PRF_BLACK_TEAM)) && (!PRF_FLAGGED(ch, PRF_ROGUE_TEAM)))
  {
    sendChar( ch, "But you are not the member of a TEAM!" );
    return;
  }

  if( IS_AFFECTED(ch, AFF_SILENCE) )
  {
    sendChar( ch, "Your lips move, but you don't seem to make a sound..." );
    return;
  }

  if( !*argument )
  {
    sendChar( ch, "Yes, but WHAT do you want to tell your team?" );
  }

  else if( !mortal_color(argument, ch) ) return;
  sprintf( buf, "&10%s tells the team, '%s'&00\r\n", GET_NAME(ch), argument);
  if (PRF_FLAGGED(ch, PRF_GOLD_TEAM))
  {  
    for( questor = descriptor_list; questor; questor = questor->next )
  {
      if( !questor->connected &&
         questor->character &&
         PRF_FLAGGED( questor->character, PRF_GOLD_TEAM ))
         send_to_char( buf, questor->character);
    }
  }
  if (PRF_FLAGGED(ch, PRF_BLACK_TEAM))
  {
    for( questor = descriptor_list; questor; questor = questor->next )
    {
      if( !questor->connected &&
         questor->character &&
         PRF_FLAGGED( questor->character, PRF_BLACK_TEAM ))
         send_to_char( buf, questor->character);
    }
  }     
    if (PRF_FLAGGED(ch, PRF_ROGUE_TEAM))
  {
    for( questor = descriptor_list; questor; questor = questor->next )
    {
      if( !questor->connected &&
         questor->character &&
         PRF_FLAGGED( questor->character, PRF_ROGUE_TEAM ))
         send_to_char( buf, questor->character);
    }
  }
}


void perform_tell(CharData *ch, CharData *vict, char *arg)
{
    if( !mortal_color(arg, ch)) return;

    if (PRF_FLAGGED(ch, PRF_NOREPEAT))
        send_to_char(CONFIG_OK, ch);
    else {
        sprintf(buf, "&18You tell $N&18%s, '%s'&00", IS_AFK(vict)? " (AFK)" : "", arg);
        act(buf, FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);

    }

    if( !PLR_FLAGGED(ch, PLR_SHUNNED) ){
        arg = makedrunk(arg, ch);
        sprintf(buf, "&18$n&18 tells you, '%s'&00", arg);
        act(buf, FALSE, ch, 0, vict, TO_VICT | TO_SLEEP);

        GET_LAST_TELL(vict) = GET_IDNUM(ch);
    }

}

/*
 * Yes, do_tell probably could be combined with whisper and ask, but
 * called frequently, and should IMHO be kept as tight as possible.
 */
ACMD(do_tell)
{
  CharData *vict;

  if (!IS_NPC(ch) && IS_SET_AR(PLR_FLAGS(ch), PLR_NOCOMM)) {
      send_to_char("You are unable to send tells.\r\n", ch);
      return;
  }

  half_chop(argument, buf, buf2);

  if (!*buf || !*buf2)
    send_to_char("Who do you wish to tell what??\r\n", ch);
  else if (!(vict = get_char_vis(ch, buf, 1)))
    sendChar( ch, CONFIG_NOPERSON );
  else if (ch == vict)
    send_to_char("You try to tell yourself something.\r\n", ch);
  else if (PRF_FLAGGED(ch, PRF_NOTELL))
    send_to_char("You can't tell other people while you have notell on.\r\n", ch);
  else if (ROOM_FLAGGED(ch->in_room, ROOM_SOUNDPROOF) && 
          !PRF_FLAGGED(ch, PRF_NOHASSLE) && !PRF_FLAGGED(vict, PRF_NOHASSLE))
    send_to_char("The walls seem to absorb your words.\r\n", ch);
  else if (!IS_NPC(vict) && !vict->desc)	/* linkless */
    act("$E's linkless at the moment.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else if (IS_AFFECTED(ch, AFF_SILENCE))        /* been silenced */
    send_to_char("Your lips move, but you don't seem to make a sound...\r\n", ch);
  else if (PLR_FLAGGED(vict, PLR_WRITING))
    act("$E's writing a message right now; try again later.",
	FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else if (PRF_FLAGGED(vict, PRF_NOTELL) || 
          (ROOM_FLAGGED(vict->in_room, ROOM_SOUNDPROOF) && 
          !PRF_FLAGGED(ch, PRF_NOHASSLE) && !PRF_FLAGGED(vict, PRF_NOHASSLE)))
    act("$E can't hear you.", FALSE, ch, 0, vict, TO_CHAR | TO_SLEEP);
  else {
    if (ch->desc && SPAM_LEVEL(ch) < 20) SPAM_LEVEL(ch) += 1;
    if (ch->desc && SPAM_LEVEL(ch) > 15) {
        sendChar(ch, "Quiet, you.\r\n");
        add_affect(ch, ch, SPELL_SILENCE, 60, APPLY_NONE, 0, 2 TICKS,
                AFF_SILENCE, FALSE, FALSE, FALSE, FALSE);
        return;
    }
    perform_tell(ch, vict, buf2);
  }
}


ACMD(do_reply)
{
    struct char_data *tch = character_list;

    if(IS_NPC(ch))
        return;

    skip_spaces(&argument);

    if (!IS_NPC(ch) && IS_SET_AR(PLR_FLAGS(ch), PLR_NOCOMM)) {
        send_to_char("You are unable to reply.\r\n", ch);
        return; 
    }

    if ((long)GET_LAST_TELL(ch) == NOBODY)
        send_to_char("You have no-one to reply to!\r\n", ch);
    else if (!*argument)
        send_to_char("What is your reply?\r\n", ch);
    else if (PRF_FLAGGED(ch, PRF_NOTELL))
        send_to_char("You can't tell other people while you have notell on.\r\n", ch);
    else if (IS_AFFECTED(ch, AFF_SILENCE))        /* been silenced */
        send_to_char("Your lips move, but you don't seem to make a sound...\r\n", ch);
    else {
        while(tch && (IS_NPC(tch) || GET_IDNUM(tch) != (long)GET_LAST_TELL(ch)))
            tch = tch->next;

        if(!tch)
            sendChar(ch, "That player is no longer playing.\r\n");
        else if (ROOM_FLAGGED(ch->in_room, ROOM_SOUNDPROOF) && 
                !PRF_FLAGGED(ch, PRF_NOHASSLE) && !PRF_FLAGGED(tch, PRF_NOHASSLE))
            send_to_char("The walls seem to absorb your words.\r\n", ch);
        else if (!IS_NPC(tch) && !tch->desc)	/* linkless */
            act("$E's linkless at the moment.", FALSE, ch, 0, tch, TO_CHAR | TO_SLEEP);
        else if (PLR_FLAGGED(tch, PLR_WRITING))
            act("$E's writing a message right now; try again later.",
                    FALSE, ch, 0, tch, TO_CHAR | TO_SLEEP);
        else if (PRF_FLAGGED(tch, PRF_NOTELL) || 
                (ROOM_FLAGGED(tch->in_room, ROOM_SOUNDPROOF) && 
                !PRF_FLAGGED(ch, PRF_NOHASSLE) && !PRF_FLAGGED(tch, PRF_NOHASSLE)))
            act("$E can't hear you.", FALSE, ch, 0, tch, TO_CHAR | TO_SLEEP);
        else {
            perform_tell(ch, tch, argument);

            // Shut up the spammers
            if (ch->desc && SPAM_LEVEL(ch) < 20) SPAM_LEVEL(ch) += 1;
            if (ch->desc && SPAM_LEVEL(ch) > 15) {
                sendChar(ch, "Quiet, you.\r\n");
                add_affect(ch, ch, SPELL_SILENCE, 60, APPLY_NONE, 0, 2 TICKS,
                AFF_SILENCE, FALSE, FALSE, FALSE, FALSE);
                return;
            }
        }
    }
}
 
ACMD(do_spec_comm)
{
  CharData *vict;
  char *action_sing, *action_plur, *action_others;

  if (!IS_NPC(ch) && IS_SET_AR(PLR_FLAGS(ch), PLR_NOCOMM)) {
      send_to_char("You are unable to do that.\r\n", ch);
      return;
  }

  if (subcmd == SCMD_WHISPER) {
    action_sing = "whisper to";
    action_plur = "whispers to";
    action_others = "$n whispers something to $N.";
  } else {
    action_sing = "ask";
    action_plur = "asks";
    action_others = "$n asks $N a question.";
  }

  half_chop(argument, buf, buf2);

  if (!*buf || !*buf2) {
      sprintf(buf, "Whom do you want to %s.. and what??\r\n", action_sing);
      send_to_char(buf, ch);
  } else if (!(vict = get_char_room_vis(ch, buf)))
      sendChar( ch, CONFIG_NOPERSON );
  else if (vict == ch)
      send_to_char("You can't get your mouth close enough to your ear...\r\n", ch);
  else if (!mortal_color(buf2, ch))
      return;
  else if (IS_AFFECTED(ch, AFF_SILENCE)) {
      act("$n leans over and makes a face at you!\r\n", FALSE, ch, 0, vict, TO_VICT);
      act("$n leans over and makes a face at $N!", FALSE, ch, 0, vict, TO_NOTVICT);
      send_to_char("Your lips move, but you don't seem to make a sound...\r\n", ch);
      return;
  }
  else {
      if (PRF_FLAGGED(ch, PRF_NOREPEAT))
          send_to_char(CONFIG_OK, ch);
      else {
          sprintf(buf, "You %s %s, '%s'\r\n", action_sing, GET_NAME(vict), buf2);
          act(buf, FALSE, ch, 0, 0, TO_CHAR);
      }

      argument = makedrunk(buf2, ch);
      sprintf(buf, "$n %s you, '%s'", action_plur, argument);
      
      act(buf, FALSE, ch, 0, vict, TO_VICT);

  }
}

ACMD(do_write)
{
  ObjData *paper = 0, *pen = 0;
  char *papername, *penname;

  papername = buf1;
  penname = buf2;

  two_arguments(argument, papername, penname);

  if (!ch->desc)
    return;

  if (!*papername) {		/* nothing was delivered */
    send_to_char("Write?  With what?  ON what?  What are you trying to do?!?\r\n", ch);
    return;
  }
  if (*penname) {		/* there were two arguments */
    if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying))) {
      sprintf(buf, "You have no %s.\r\n", papername);
      send_to_char(buf, ch);
      return;
    }
    if (!(pen = get_obj_in_list_vis(ch, penname, ch->carrying))) {
      sprintf(buf, "You have no %s.\r\n", papername);
      send_to_char(buf, ch);
      return;
    }
  } else {			/* there was one arg.. let's see what we can
				 * find */
    if (!(paper = get_obj_in_list_vis(ch, papername, ch->carrying))) {
      sprintf(buf, "There is no %s in your inventory.\r\n", papername);
      send_to_char(buf, ch);
      return;
    }
    if (GET_OBJ_TYPE(paper) == ITEM_PEN) {	/* oops, a pen.. */
      pen = paper;
      paper = 0;
    } else if (GET_OBJ_TYPE(paper) != ITEM_NOTE) {
      send_to_char("That thing has nothing to do with writing.\r\n", ch);
      return;
    }
    /* One object was found.. now for the other one. */
    if (!ch->equipment[WEAR_HOLD]) {
      sprintf(buf, "You can't write with %s %s alone.\r\n", AN(papername),
	      papername);
      send_to_char(buf, ch);
      return;
    }
    if (!CAN_SEE_OBJ(ch, ch->equipment[WEAR_HOLD])) {
      send_to_char("The stuff in your hand is invisible!  Yeech!!\r\n", ch);
      return;
    }
    if (pen)
      paper = ch->equipment[WEAR_HOLD];
    else
      pen = ch->equipment[WEAR_HOLD];
  }


  /* ok.. now let's see what kind of stuff we've found */
  if (GET_OBJ_TYPE(pen) != ITEM_PEN) {
    act("$p is no good for writing with.", FALSE, ch, pen, 0, TO_CHAR);
  } else if (GET_OBJ_TYPE(paper) != ITEM_NOTE) {
    act("You can't write on $p.", FALSE, ch, paper, 0, TO_CHAR);
  }
  else if (paper->action_description)
    send_to_char("There's something written on it already.\r\n", ch);
  else {
      // this crashes, screw that. Vex.
#if 0
      /* we can write - hooray! */
      /* this is the PERFECT code example of how to set up:
       * a) the text editor with a message already loaed
       * b) the abort buffer if the player aborts the message
       */
       ch->desc->backstr = NULL;
       send_to_char("Write your note.  (/s saves /h for help)\r\n", ch);
       /* ok, here we check for a message ALREADY on the paper */
       if (paper->action_description) {
       /* we str_dup the original text to the descriptors->backstr */
           ch->desc->backstr = str_dup(paper->action_description);
           /* send to the player what was on the paper (cause this is already */
           /* loaded into the editor) */
           send_to_char(paper->action_description, ch);
       }
       act("$n begins to jot down a note.", TRUE, ch, 0, 0, TO_ROOM);
       /* assign the descriptor's->str the value of the pointer to the text */
       /* pointer so that we can reallocate as needed (hopefully that made */
       /* sense :>) */
       ch->desc->str = &paper->action_description;
       ch->desc->max_str = MAX_NOTE_LENGTH;
#else
       sendChar(ch, "Sorry, writing notes in this way has been disabled.");
#endif
  }
}



ACMD(do_page)
{
  DescriptorData *d;
  CharData *vict;

  half_chop(argument, arg, buf2);

  if (IS_NPC(ch))
    send_to_char("Monsters can't page.. go away.\r\n", ch);
  else if (!*arg)
    send_to_char("Whom do you wish to page?\r\n", ch);
  else {
    sprintf(buf, "\007\007*%s* %s\r\n", GET_NAME(ch), buf2);
    if (!str_cmp(arg, "all")) {
      if (GET_LEVEL(ch) > LVL_CREATOR) {
	for (d = descriptor_list; d; d = d->next)
	  if (!d->connected && d->character)
	    act(buf, FALSE, ch, 0, d->character, TO_VICT);
      } else
	send_to_char("You will never be godly enough to do that!\r\n", ch);
      return;
    }
    if ((vict = get_char_vis(ch, arg, 1)) != NULL) {
      act(buf, FALSE, ch, 0, vict, TO_VICT);
      if (PRF_FLAGGED(ch, PRF_NOREPEAT))
	send_to_char(CONFIG_OK, ch);
      else
	act(buf, FALSE, ch, 0, vict, TO_CHAR);
      return;
    } else
      send_to_char("There is no such person in the game!\r\n", ch);
  }
}


/**********************************************************************
 * generalized communication func, originally by Fred C. Merkel (Torg) *
  *********************************************************************/

ACMD(do_gen_comm)
{
  DescriptorData *i;
  char color_on[24];

  /* Array of flags which must _not_ be set in order for comm to be heard */
  static int channels[] = {
    0,
    PRF_DEAF,
    PRF_NORPLAY,
    PRF_NOAUCT,
    PRF_NOGRATZ,
    PRF_NOCLAN,
    PRF_NOGUILD,
    PRF_NOOOC,
    PRF_NOQUERY,
    0
  };

  /*
   * com_msgs: [0] Message if you can't perform the action because of noshout
   * [1] name of the action [2] message if you're not on the channel [3] a
   * color string.
   */
  static char *com_msgs[][4] = {
    {"You are muted!  You cannot holler!!\r\n",      "holler",  "", KYEL},
    {"You are muted!  You cannot shout!!\r\n",       "shout",   "Turn off your noshout flag first!\r\n", KYEL},
    {"You are muted!  You cannot roleplay!!\r\n",    "roleplay","You aren't even on the channel!\r\n", KYEL},
    {"You are muted!  You cannot auction!!\r\n",     "auction", "You aren't even on the channel!\r\n", KMAG},
    {"You are muted!  You cannot congratulate!\r\n", "congrat", "You aren't even on the channel!\r\n", KGRN},
    {"You are muted!  You cannot clansay!\r\n",      "clan",  "You aren't even on the channel!\r\n", "&11"},
    {"You are muted!  You cannot guildsay!\r\n",     "guild",   "You aren't even on the channel!\r\n", "&12"},
    {"You are muted!  You cannot OOC!\r\n",          "OOC",     "You aren't even on the channel!\r\n", "&09"},
    {"You are muted!  You cannot query!\r\n",        "query",   "You aren't even on the channel!\r\n", "&13"}
  };

  if (!IS_NPC(ch) && IS_SET_AR(PLR_FLAGS(ch), PLR_NOCOMM)) {
      send_to_char("You are unable to do that.\r\n", ch);
      return;
  }

  /* to keep pets, etc from being ordered to shout */
  if (IS_SET_AR(AFF_FLAGS(ch), AFF_CHARM)) 
      return;

  if (ch->desc && NOISE_LEVEL(ch) < 15) NOISE_LEVEL(ch) += 1;

  /* watch for spammers; if you use a public channel more than 5 times
   * in 5 seconds, you're considered "noisy".  Your usage allowance
   * grows back at 3 usages per 5 seconds. */
  if (ch->desc && NOISE_LEVEL(ch) > 3) {
      send_to_char("You have been too noisy to use public channels.\r\n", ch);
      return;
  }

  /* Digger added clanspeak */
  if(( subcmd == SCMD_CLAN ) && ( GET_CLAN(ch) <= 0  || GET_CLAN(ch) > 100 )){
      GET_CLAN(ch) = 0;
      send_to_char("You are not in a clan.\r\n", ch);
      return;
  }

  if (PLR_FLAGGED(ch, PLR_NOSHOUT)) {
    send_to_char(com_msgs[subcmd][0], ch);
    return;
  }
  if (ROOM_FLAGGED(ch->in_room, ROOM_SOUNDPROOF)) {
    send_to_char("The walls seem to absorb your words.\r\n", ch);
    return;
  }
  if (IS_AFFECTED(ch, AFF_SILENCE)) {
    send_to_char("Your lips move, but you don't seem to make a sound...\r\n", ch);
    act("$n makes a face at everyone!", FALSE, ch, 0, 0, TO_ROOM);
    return;
  }
  /* level_can_shout defined in config.c */
  if (GET_LEVEL(ch) < CONFIG_LEVEL_CAN_SHOUT) {
    sprintf(buf1, "You must be at least level %d before you can %s.\r\n",
	    CONFIG_LEVEL_CAN_SHOUT, com_msgs[subcmd][1]);
    send_to_char(buf1, ch);
    return;
  }

  /* make sure the char is on the channel */
  if (PRF_FLAGGED(ch, channels[subcmd])) {
    send_to_char(com_msgs[subcmd][2], ch);
    return;
  }
  /* skip leading spaces */
  skip_spaces(&argument);

  /* make sure that there is something there to say! */
  if (!*argument) {
    sprintf(buf1, "Yes, %s, fine, %s we must, but WHAT???\r\n", com_msgs[subcmd][1], com_msgs[subcmd][1]);
    send_to_char(buf1, ch);
    return;
  }
  if (subcmd == SCMD_HOLLER) {
    if (GET_MOVE(ch) < CONFIG_HOLLER_MOVE_COST) {
      send_to_char("You're too exhausted to holler.\r\n", ch);
      return;
    } else
      GET_MOVE(ch) -= CONFIG_HOLLER_MOVE_COST;
  }

  /* If they are using color, see if they are authorised. */
  if (!mortal_color(argument, ch)) return;

  /* set up the color on code */
  strcpy(color_on, com_msgs[subcmd][3]);

  /* first, set up strings to be given to the communicator */
  if (PRF_FLAGGED(ch, PRF_NOREPEAT))
    send_to_char(CONFIG_OK, ch);
  else {
    if (COLOR_LEV(ch) >= C_CMP)
      sprintf(buf1, "%sYou %s, '%s'%s", color_on, com_msgs[subcmd][1], argument, KNRM);
    else
      sprintf(buf1, "You %s, '%s'", com_msgs[subcmd][1], argument);
    act(buf1, FALSE, ch, 0, 0, TO_CHAR | TO_SLEEP);
  }

  if( PLR_FLAGGED(ch, PLR_SHUNNED)) return; /* Bail out here if the char is shunned. */
  
  argument = makedrunk(argument,ch);

  /* Correct the grammar for query and set it up for anon - Kai */
  if (subcmd == SCMD_QUERYSAY)
    sprintf(buf2, "queries, '%s'", argument);
  else if (subcmd == SCMD_OOC) /* didn't like "OOCs" :p */
    sprintf(buf, "$n %s: %s&00", com_msgs[subcmd][1], argument);
  else
    sprintf(buf, "$n %ss, '%s'&00", com_msgs[subcmd][1], argument);

  /* now send all the strings out */
  for (i = descriptor_list; i; i = i->next) {
    if (!i->connected && i != ch->desc && i->character &&
	!PRF_FLAGGED(i->character, channels[subcmd]) &&
	!PLR_FLAGGED(i->character, PLR_WRITING) &&
	(!ROOM_FLAGGED(i->character->in_room, ROOM_SOUNDPROOF) || PRF_FLAGGED(ch, PRF_NOHASSLE)))
    {

      if (subcmd == SCMD_SHOUT &&
	  ((world[ch->in_room].zone != world[i->character->in_room].zone) ||
	   GET_POS(i->character) <= POS_SLEEPING))
	continue;

      /* Digger added clanspeak */
      if( subcmd == SCMD_CLAN)
          if(GET_CLAN(ch) != GET_CLAN(i->character))
              continue;

      /* Kaidon added guild channel - 09/05/97 */
      if( subcmd == SCMD_GUILD && GET_CLASS(ch) != GET_CLASS(i->character)) continue;

      if (COLOR_LEV(i->character) >= C_NRM)
	send_to_char(color_on, i->character);
      /* Make query anonymous to mortals - Kaidon 06/22/97 */
      if (subcmd == SCMD_QUERYSAY) {
        if (GET_LEVEL(i->character) >= 5) /*LVL_IMMORT*/
          sprintf(buf,"$n %s",buf2);
        else
          sprintf(buf,"Someone %s", buf2);
      }
      act(buf, FALSE, ch, 0, i->character, TO_VICT | TO_SLEEP);
      if (COLOR_LEV(i->character) >= C_NRM)
	send_to_char(KNRM, i->character);
    }
  }
}

ACMD(do_qcomm)
{
  DescriptorData *i;

  if( !PRF_FLAGGED(ch, PRF_QUEST) )
  {
    sendChar( ch, "You aren't even part of the quest!\r\n" );
    return;
  }

  if (!IS_NPC(ch) && IS_SET_AR(PLR_FLAGS(ch), PLR_NOCOMM)) {
      send_to_char("You are unable to use qsay.\r\n", ch);
      return;
  }

  if( IS_AFFECTED(ch, AFF_SILENCE) )
  {
    sendChar( ch, "Your lips move, but you don't seem to make a sound...\r\n" );
    return;
  }

  skip_spaces(&argument);

  if (!*argument)
    sendChar( ch, "Quest-say?  Yes!  Fine!  Quest-say we must, but WHAT??\r\n" );
  else if (!mortal_color(argument, ch))
    return;
  else
  {
    if( PRF_FLAGGED(ch, PRF_NOREPEAT)) sendChar( ch, CONFIG_OK );

    else
    {
      sendChar(ch, "&10You quest-say, '%s'&00\r\n", argument);
    }

    /*
    ** Bail here if they are shunned.
    */
    if( PLR_FLAGGED(ch, PLR_SHUNNED)) return;

    argument = makedrunk(argument, ch);

    sprintf( buf1, "&10$n quest-says, '%s'&00", argument);
    for( i = descriptor_list; i; i = i->next)
    {
      if( !i->connected && i != ch->desc &&
          PRF_FLAGGED(i->character, PRF_QUEST) &&
         !PLR_FLAGGED(i->character, PLR_WRITING))
        act(buf1, 0, ch, 0, i->character, TO_VICT | TO_SLEEP);
    }
  }
}

/* Mortius: Adding back in the prayer command */
ACMD (do_prayer)
{
  DescriptorData *i;

  if (ch->player_specials->saved.prayer_time >= 1)
    {
      sendChar (ch, "Your prayers cannot be heard at this time.\r\n");
      return;
    }

  skip_spaces (&argument);

  sprintf(buf, "&03You pray to the heavens above, '%s'.&00\r\n", argument);
  sprintf (buf2, "Prayer: &03%s prays, '%s'&00", GET_NAME (ch), argument);

  mlog (buf2);
  sendChar (ch, buf);
  ch->player_specials->saved.prayer_time = 120; /* 120 ticks */

  for (i = descriptor_list; i; i = i->next)
    {
      if (!i->connected && i != ch->desc &&
          GET_LEVEL (i->character) >= LVL_IMMORT &&
          !PLR_FLAGGED (i->character, PLR_WRITING))
        act (buf2, 0, ch, 0, i->character, TO_VICT | TO_SLEEP);
    }
}

/*
 * Drunk struct
 */
struct drunk_struct
{
        int     min_drunk_level;
        int     number_of_rep;
        char    *replacement[11];
};


/* How to make a string look drunk... by Apex (robink@htsa.hva.nl) */
/* Modified and enhanced for envy(2) by the Maniac from Mythran    */
/* Ported to Stock Circle 3.0 by Haddixx (haddixx@megamed.com)     */

char * makedrunk (char *string, struct char_data * ch)
{

/* This structure defines all changes for a character */
  struct drunk_struct drunk[] =
  {
    {3, 10,
      {"a", "a", "a", "A", "aa", "ah", "Ah", "ao", "aw", "oa", "ahhhh"}},
    {8, 5,
     {"b", "b", "b", "B", "B", "vb"}},
    {3, 5,
     {"c", "c", "C", "cj", "sj", "zj"}},
    {5, 2,
     {"d", "d", "D"}},
    {3, 3,
     {"e", "e", "eh", "E"}},
    {4, 5,
     {"f", "f", "ff", "fff", "fFf", "F"}},
    {8, 2,
     {"g", "g", "G"}},
    {9, 6,
     {"h", "h", "hh", "hhh", "Hhh", "HhH", "H"}},
    {7, 6,
     {"i", "i", "Iii", "ii", "iI", "Ii", "I"}},
    {9, 5,
     {"j", "j", "jj", "Jj", "jJ", "J"}},
    {7, 2,
     {"k", "k", "K"}},
    {3, 2,
     {"l", "l", "L"}},
    {5, 8,
     {"m", "m", "mm", "mmm", "mmmm", "mmmmm", "MmM", "mM", "M"}},
    {6, 6,
     {"n", "n", "nn", "Nn", "nnn", "nNn", "N"}},
    {3, 6,
     {"o", "o", "ooo", "ao", "aOoo", "Ooo", "ooOo"}},
    {3, 2,
     {"p", "p", "P"}},
    {5, 5,
     {"q", "q", "Q", "ku", "ququ", "kukeleku"}},
    {4, 2,
     {"r", "r", "R"}},
    {2, 5,
     {"s", "ss", "zzZzssZ", "ZSssS", "sSzzsss", "sSss"}},
    {5, 2,
     {"t", "t", "T"}},
    {3, 6,
     {"u", "u", "uh", "Uh", "Uhuhhuh", "uhU", "uhhu"}},
    {4, 2,
     {"v", "v", "V"}},
    {4, 2,
     {"w", "w", "W"}},
    {5, 6,
     {"x", "x", "X", "ks", "iks", "kz", "xz"}},
    {3, 2,
     {"y", "y", "Y"}},
    {2, 9,
     {"z", "z", "ZzzZz", "Zzz", "Zsszzsz", "szz", "sZZz", "ZSz", "zZ", "Z"}}
  };

  char buf[1024];      /* this should be enough (?) */
  char temp;
  int pos = 0;
  int randomnum;
  char debug[256];

  if(GET_COND(ch, DRUNK) > 0)  /* character is drunk */
  {
     do
     {
       if(!percentSuccess(GET_COND(ch, DRUNK))) {
           buf[pos++] = *string;
           continue;
       }

       temp = toupper(*string);
       if( (temp >= 'A') && (temp <= 'Z') )
       {
         if(GET_COND(ch, DRUNK) > drunk[(temp - 'A')].min_drunk_level)
         {
           randomnum = number(0, (drunk[(temp - 'A')].number_of_rep));
           strcpy(&buf[pos], drunk[(temp - 'A')].replacement[randomnum]);
           pos += strlen(drunk[(temp - 'A')].replacement[randomnum]);
         }
         else
           buf[pos++] = *string;
       }
       else
       {
         if ((temp >= '0') && (temp <= '9'))
         {
           temp = '0' + number(0, 9);
           buf[pos++] = temp;
         }
         else
           buf[pos++] = *string;
       }
     }while (*string++);

     buf[pos] = '\0';          /* Mark end of the string... */
     strcpy(string, buf);
     return(string);
  }
  return (string); /* character is not drunk, just return the string */
}







