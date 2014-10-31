
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/comm.h"
#include "general/class.h"
#include "general/handler.h"
#include "actions/interpreter.h"
#include "util/utils.h"
#include "specials/special.h"
#include "general/class.h"
#include "magic/spells.h"
#include "specials/advance.h"
#include "magic/skills.h"
#include "actions/fight.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */
/*
 * Arbaces 19-March-2008
 *
 * I'm again borrowing Digger's code for another special shop.  Thanks Digger/Li.
 */

typedef struct {
  int  (*advanceFunc)(CharData *, CharData *, int, char *);
  char  *name;             /* name the attr is purchased under     */
  int   experience_cost;   /* experience cost to use function */
  int   qp_cost;		   /* quest point cost to use function*/
} advanceEntry;

int advanceChar(CharData *ch, CharData *advancer, int specialization, char *arg) {

   if(GET_ADVANCE_LEVEL(ch) == MAX_ADVANCE_LEVEL) {
      act("$n tells you, 'You have reached the highest level"
         " of being.  I can do nothing more to help you.'",
         FALSE, advancer, NULL, ch, TO_VICT);
      return FALSE;
   }
   GET_ADVANCE_LEVEL(ch) += 1;
   mlog(" (TRANSCEND) %s just transcended to  level %d. ", GET_NAME(ch), GET_ADVANCE_LEVEL(ch) );
   send_to_all("&10%s has transcended to the next stage of advancement! "
        "%s has reached level %d!&00\r\n",
           GET_NAME(ch), capsHSSH(ch), GET_ADVANCE_LEVEL(ch));

   restore(ch, ch);

   return TRUE;
}

int specializeChar(CharData *ch, CharData *advancer, int specialization, char *arg)
{
    int l;
    
    struct specMap {
        char *specName;     /* the name of the specialization entered       */
        int  classRestrict; /* the class which is allowed to specialize     */
        int  specNum;       /* the specialization which this string maps to */
    } specs[] = {
            {"arcanist"       , CLASS_MAGIC_USER     , SPEC_ARCANIST  },
            {"enchanter"      , CLASS_MAGIC_USER     , SPEC_ENCHANTER  },
            {"dark priest"    , CLASS_CLERIC         , SPEC_DARK_PRIEST},
            {"holy priest"    , CLASS_CLERIC         , SPEC_HOLY_PRIEST},
            {"trickster"      , CLASS_THIEF          , SPEC_TRICKSTER},
            {"brigand"        , CLASS_THIEF          , SPEC_BRIGAND},
            {"butcher"        , CLASS_ASSASSIN       , SPEC_BUTCHER},
            {"bounty hunter"  , CLASS_ASSASSIN       , SPEC_BOUNTY_HUNTER},
            {"defender"       , CLASS_WARRIOR        , SPEC_DEFENDER},
            {"dragonslayer"   , CLASS_WARRIOR        , SPEC_DRAGON_SLAYER},
            {"naturalist"     , CLASS_RANGER         , SPEC_NATURALIST},
            {"hunter"         , CLASS_RANGER         , SPEC_HUNTER},
            {"chi warrior"    , CLASS_SHOU_LIN       , SPEC_CHI_WARRIOR},
            {"ancient dragon" , CLASS_SHOU_LIN       , SPEC_ANCIENT_DRAGON},
            {"dragoon"        , CLASS_SOLAMNIC_KNIGHT, SPEC_DRAGOON},
            {"knight templar" , CLASS_SOLAMNIC_KNIGHT, SPEC_KNIGHT_TEMPLAR},
            {"knight errant"  , CLASS_DEATH_KNIGHT   , SPEC_KNIGHT_ERRANT},
            {"defiler"        , CLASS_DEATH_KNIGHT   , SPEC_DEFILER},
            {"prestidigitator", CLASS_SHADOW_DANCER  , SPEC_PRESTIDIGITATOR},
            {"shade"          , CLASS_SHADOW_DANCER  , SPEC_SHADE},
            {"witchdoctor"    , CLASS_NECROMANCER    , SPEC_WITCH_DOCTOR},
            {"revenant"       , CLASS_NECROMANCER    , SPEC_REVENANT},
            {"none"           , CLASS_UNDEFINED      , SPEC_NONE},
            {"\n"             , 0                    , 0 }};
    

    if(GET_ADVANCE_LEVEL(ch) == 0)
    {
        act("$n tells you, 'You must advance before you can specialize.'",
                FALSE, advancer, 0, ch, TO_VICT);
        return FALSE;
    }

    if(GET_SPEC(ch) != 0)
    {
        act("$n tells you, 'You must respecialize before you can specialize again.'",
                FALSE, advancer, 0, ch, TO_VICT);
        return FALSE;
    }

    for (l = 0; *(specs[l].specName) != '\n'; l++)
        if (!strncmp(arg, specs[l].specName, strlen(arg)))
            break;

    if(specs[l].specNum == 0){
            act("$n tells you, 'Please choose a valid specialization'", FALSE, advancer, 0, ch, TO_VICT);
            return(TRUE);
    }

    if(specs[l].classRestrict != GET_CLASS(ch)) {
            act("$n tells you, 'That specialization is not available to your class'", FALSE, advancer, 0, ch, TO_VICT);
            return(TRUE);
    }

    // If we got this far, things are good.  Let's let them choose that specialization.
    GET_SPEC(ch) = specs[l].specNum;
    sendChar(ch, "Your mind clouds with occult knowledge as %s teaches you \r\n"
            "the secrets of the %s specialization.\r\n", GET_NAME(advancer), specs[l].specName);
    mlog(" (TRANSCEND) %s just chose the %s specialization.", GET_NAME(ch), specs[l].specName);
    send_to_all("&10%s chose to specialize in %s!&00\r\n", GET_NAME(ch), specs[l].specName);
    return TRUE;
}

int respecializeChar(CharData *ch, CharData *advancer, int specialization, char *arg) {

    if(GET_SPEC(ch) == 0) {
        act("$n tells you, 'You do not need to respecialize - you aren't specialized yet.'",
                FALSE, advancer, 0, ch, TO_VICT);
        return FALSE;
    }

    act("$n tells you, 'This won't take but a second.'", FALSE, advancer, 0, ch, TO_VICT);
    act("You instantly forget your specialization knowledge.", FALSE, advancer, 0, ch, TO_VICT);
    GET_SPEC(ch) = 0;

    return TRUE;
}

int masterWeapon(CharData *ch, CharData *advancer, int specialization, char *arg) {
    int mastery = UNDEFINED_MASTERY;

    if(GET_CLASS(ch) != CLASS_WARRIOR)
        return FALSE;
    
    if(GET_MASTERY(ch) != UNDEFINED_MASTERY) {
        act("$n tells you, 'You already have a weapon mastery.'", FALSE, advancer, 0, ch, TO_VICT);
        return TRUE;
    }

    // strcmp returns 0 if they are equal!
    if(strcmp(arg, "edge") == 0) {
        mastery = SKILL_EDGE_MASTERY;
    }
    else if(strcmp(arg, "blunt") == 0) {
        mastery = SKILL_BLUNT_MASTERY;
    }
    else if(strcmp(arg, "exotic") == 0) {
        mastery = SKILL_EXOTIC_MASTERY;
    }
    else if(strcmp(arg, "point") == 0) {
        mastery = SKILL_POINT_MASTERY;
    }
    else mastery = UNDEFINED_MASTERY;

    if(mastery == UNDEFINED_MASTERY) {
        act("$n tells you, 'What type of weapon would you like to master?\r\n"
                "You may choose &10edge&00, &10blunt&00, &10exotic&00, or &10point&00.'", FALSE, advancer, 0, ch, TO_VICT);
        return TRUE;
    }

    GET_MASTERY(ch) = mastery;

    sendChar(ch, "%s gives you a few tips on the use of %s weapons.\r\n", GET_NAME(advancer), arg);
    sendChar(ch, "You are now a master of %s weapons.\r\n", arg);

    sprintf(buf, "%s has mastered weapons of type %s.", GET_NAME(ch), arg);
    mudlog(NRM, LVL_DEITY, TRUE, buf);    

    return TRUE;
}

int remasterWeapon(CharData *ch, CharData *advancer, int specialization, char *arg) {

    if(GET_CLASS(ch) != CLASS_WARRIOR) 
        return FALSE;

    if(UNDEFINED_MASTERY == GET_MASTERY(ch)) {
        act("$n tells you, 'You are not yet a master of any weapon class.'", FALSE, advancer, 0, ch, TO_VICT);
	return FALSE;
    }

    GET_MASTERY(ch) = UNDEFINED_MASTERY;
    if(GET_SKILL(ch, SKILL_WEAPON_MASTERY) > MAX_PRACTICE_LEVEL)
        SET_SKILL(ch, SKILL_WEAPON_MASTERY, MIN(MAX_PRACTICE_LEVEL, GET_SKILL(ch, SKILL_WEAPON_MASTERY)));
    act("$n tells you, 'You are now ready to master a new weapon class.'", FALSE, advancer, 0, ch, TO_VICT);
    return TRUE;
}

int relearn(CharData *ch, CharData *advancer, int specialization, char *arg) {
    int i;
    int practices = 0;
    
    for (i = 1; i <= MAX_SKILLS; i++)
      SET_SKILL(ch, i, 0);

    give_initial_skills(ch);

    // Players get at least 2 practices for the first level...
    practices += 5*wis_app[(int)GET_WIS(ch)].bonus;
    practices += wis_app[(int)GET_WIS(ch)].extra;
    
    practices = MAX(10, practices);

    // Now give them all subsequent practices.
    for(i = 1; i < GET_LEVEL(ch); i++) {
        practices += 5*wis_app[(int)GET_WIS(ch)].bonus;
        practices += wis_app[(int)GET_WIS(ch)].extra;
    }

    GET_PRACTICES(ch) = practices/5;
    sendChar(ch, "You have been given %d practice points to spend with your trainer.\r\n", practices/5);
    act("$n tells you, 'You are now ready to relearn your abilities.'", FALSE, advancer, 0, ch, TO_VICT);

    return TRUE;
}

advanceEntry advanceActs[] = {
  { advanceChar,      "advance"       , 1000000000, 100},
  { specializeChar,   "specialize"    ,          0,   0},
  { specializeChar,   "spec"          ,          0,   0},
  { respecializeChar, "respecialize"  ,  250000000,   0},
  { respecializeChar, "respec"        ,  250000000,   0},
  { masterWeapon,     "mastery"       ,          0,   0},
  { remasterWeapon,   "remaster"      ,   50000000,   0},
  { relearn,          "relearn"       ,          0,   0},
  { NULL,             ""              ,          0,   0}
};

SPECIAL( advanceshop ) {
	CharData *advancer = me;
	int spec = 0;

    if( !ch->desc || IS_NPC(ch) ) return FALSE;

    if( !CMD_IS("list" ) &&
        !CMD_IS("buy"  )) return FALSE;
	
    if( CMD_IS( "list" )){
        act( "$n shows you a menu, 'I offer the following services:'", FALSE, advancer, 0, ch, TO_VICT);
        sendChar(ch,
"+---------------+----------------------------------------+------------------+\r\n"
"|   Service     |               Description              |       Rates      |\r\n"
"|               |                                        |   Exp   |   QP   |\r\n"
"| Advance       | Increase your Advanced Level by one    |   1 bil |  100   |\r\n"
"|               |                                        |         |        |\r\n"
"| Specialize    | Choose a path of greater specialty     |     N/A |  N/A   |\r\n"
"|               |                                        |         |        |\r\n"
"| Respecialize  | Rechoose your path                     | 250 mil |  N/A   |\r\n"
"|               |                                        |         |        |\r\n"
"| Relearn       | Relearn the skills of your profession  |     N/A |  N/A   |\r\n"
                );
        if(GET_CLASS(ch) == CLASS_WARRIOR) {
            sendChar(ch,
                    "|               |                                        |         |        |\r\n"
                    "| Mastery       | Master a particular class of weapon    |     N/A |  N/A   |\r\n"
                    "|               |                                        |         |        |\r\n"
                    "| Remaster      | Unlearn your previous mastery          | 50 Mil  |  N/A   |\r\n");
        }
        sendChar(ch, "+---------------------------------------------------------------------------+\r\n");
        sendChar( ch, "\r\n" );
        return( TRUE );
    }

    if( CMD_IS( "buy" )){
        advanceEntry *ptr  = advanceActs;
        char cmd[80];

        argument = one_argument( argument, cmd );
        skip_spaces(&argument);

        while( ptr->advanceFunc != NULL && strcmp(ptr->name, cmd )){
            ptr++;
        }

        if( ptr->advanceFunc == NULL ){
            act("$n tells you, 'Do you keep the funk alive by talking like idiots?'", FALSE, advancer, 0, ch, TO_VICT);
            return(TRUE); 
        }

        if(ptr->qp_cost > 0 && GET_LEVEL(ch) < 50) {
            sendChar(ch, "Come back when you have reached level 50.\r\n");
            return (TRUE);
        }

	// People need to have twice as much exp as it costs.  Otherwise bad things could happen!
        if( (ptr->experience_cost)*2 > GET_EXP(ch) || 
            (1000000000 + ptr->experience_cost > GET_EXP(ch) && ptr->experience_cost != 0) ) {
            act("$n tells you, 'You don't have enough experience.  Train more and return.'", FALSE, advancer, 0, ch, TO_VICT);
            return(TRUE);
        }
	
	if (ptr->qp_cost > GET_QP(ch) ) {
		act("$n tells you, 'You need the blessings of the gods to continue.  Return with more quest points.'", 
			FALSE, advancer, NULL, ch, TO_VICT);
		return( TRUE );
	}

	// Okay, we got this far.  Let's advance them!
	if(ptr->advanceFunc(ch, advancer, spec, argument)) {
		GET_EXP(ch) -= ptr->experience_cost;
		GET_QP(ch) -= ptr->qp_cost;
		save_char( ch, NOWHERE );
		return( TRUE );
	}
    }
    else {
        return( FALSE );
    }

    return FALSE;
}

ACMD(do_aspect) {
    char theArg[MAX_INPUT_LENGTH];

    typedef struct {
        char  *name;             /* name of the aspect */
        int   skillNum;         /* skill number player is using */
        int   aspectNum;        /* the number of the aspect */
    } aspectEntry;

    aspectEntry aspectActs[] = {
        { "wind", SKILL_ART_WIND, ASPECT_WIND},
        { "snake", SKILL_ART_SNAKE, ASPECT_SNAKE},
        { "monkey", SKILL_ART_MONKEY, ASPECT_MONKEY},
        { "crane", SKILL_ART_CRANE, ASPECT_CRANE},
        { "flower", SKILL_ART_FLOWER, ASPECT_FLOWER},
        { "none"  , 0, ASPECT_NONE},
        { NULL, 0, 0}
    };

    aspectEntry *ptr  = aspectActs;

    if(!IS_SHOU_LIN(ch) || GET_LEVEL(ch) < 20)
        sendChar(ch, "You have no ability in this area.\r\n");

    if (!*argument) {
	sendChar(ch, "Possible options are:\r\n");
        sendChar(ch, "NONE - You will not take on a specialized form.\r\n");
        sendChar(ch, "WIND - You will be ready to defend against projectiles.\r\n");
        if(GET_LEVEL(ch) >= 30)
            sendChar(ch, "SNAKE - You can poison opponents.\r\n");
        if(GET_LEVEL(ch) >= 35)
            sendChar(ch, "MONKEY - You will focus on melee combat and skills.\r\n");
        if(GET_LEVEL(ch) >= 40)
            sendChar(ch, "CRANE - You will try to stun your opponent.\r\n");
        if(GET_LEVEL(ch) >= 45)
            sendChar(ch, "FLOWER - You will protect yourself from magic attacks.\r\n");
        sendChar(ch, "Your current setting is: &08%s&00.\r\n", aspects[GET_ASPECT(ch)]);
	return;
    }

    one_argument(argument, theArg);

    while(ptr->name != NULL && strcmp(ptr->name, theArg)){
        ptr++;
    }

    if( ptr->name == NULL ){
        act("Which aspect would you like to take?", FALSE, ch, 0, ch, TO_CHAR);
        return;
    }

    if(skillSuccess(ch, ptr->skillNum)) {
        GET_ASPECT(ch) = ptr->aspectNum;
        if(GET_ASPECT(ch) != 0) {
            sendChar(ch, "You take on the aspect of the %s.\r\n", aspects[GET_ASPECT(ch)]);
            sprintf(buf, "$n takes on an aspect of the %s!", aspects[GET_ASPECT(ch)]);
            act(buf, TRUE, ch, NULL, NULL, TO_ROOM);
        }
    }
    else {
        sendChar(ch, "You fail to take on an aspect.\r\n");
        GET_ASPECT(ch) = ASPECT_NONE;
    }

    return;
}

ACMD(do_advance_toggle) {
    CharData *tch, *next_tch, *mob;
    ObjData *obj;
    FollowType *f;
    int stun = 0;
    int found = FALSE;
    char arg[100];

    one_argument( argument, arg );

    switch(subcmd) {
        case SCMD_MEND_PET:
            if(!IS_ENCHANTER(ch)) {
                send_to_char("You are not able to mend creatures.\r\n", ch);
                return;
            }
            
            for(tch = world[ch->in_room].people; tch; tch = next_tch ){
                next_tch = tch->next_in_room;

                if(tch && tch->master && tch->master == ch &&
                        (affected_by_spell(tch, SPELL_GATE) ||
                        affected_by_spell(tch, SPELL_CONJURE_ELEMENTAL) ||
                        affected_by_spell(tch, SPELL_MONSTER_SUMMON)))
                {
                    if(affected_by_spell(tch, SKILL_PET_MEND))
                        affect_from_char(tch, SKILL_PET_MEND);
                    add_affect(ch, tch, SKILL_PET_MEND, GET_LEVEL(ch), 0, 0, -1, 0, FALSE, FALSE, FALSE, FALSE);
                    act("$n magically mends $N's wounds.", FALSE, ch, NULL, tch, TO_ROOM);
                    found = TRUE;
                }
            }

            if(!found) {
                sendChar(ch, "You cannot find a target to mend.\r\n");
                return;
            }

            act("You magically mend your pet's wounds.", FALSE, ch, NULL, NULL, TO_CHAR);
            GET_MANA(ch) = MAX(GET_MANA(ch) - 100, 0);
            break;

        case SCMD_POTENCY:
            if(!IS_ARCANIST(ch)) {
                send_to_char("You are not able to focus magic.\r\n", ch);
                return;
            }

            GET_MANA(ch) = MAX(0, GET_MANA(ch)/2);
            stun = 1;
            
            if(affected_by_spell(ch, SKILL_POTENCY)) {
                send_to_char("You are no longer in your potent state.\r\n", ch);
                affect_from_char(ch, SKILL_POTENCY);
            }
            else {
                act("You enter a state of increased potency.", FALSE, ch, NULL, NULL, TO_CHAR);
                act("Lightning crackles between $n's fingertips!", TRUE, ch, NULL, NULL, TO_ROOM);
                add_affect( ch, ch, SKILL_POTENCY, GET_LEVEL(ch),
                        0, 0, -1, 0, FALSE, FALSE, FALSE, FALSE);
            }
            break;
        case SCMD_DETERRENCE:
            if( (IS_BOUNTY_HUNTER(ch) || IS_BRIGAND(ch)) && GET_ADVANCE_LEVEL(ch) >= 3) {
                if(COOLDOWN(ch, SLOT_DETERRENCE))
                    send_to_char("You are not ready to preform another amazing feat.\r\n", ch);
                else {
                    stun = 1;
                    act("You take evasive action!", FALSE, ch, NULL, NULL, TO_CHAR);
                    act("$n takes evasive action!", TRUE, ch, NULL, NULL, TO_ROOM);
                    add_affect(ch, ch, SKILL_DETERRENCE, GET_LEVEL(ch),
                            0, 0, 10, 0, FALSE, FALSE, FALSE, FALSE);
                    COOLDOWN(ch, SLOT_DETERRENCE) = 5;
                }
            }
            else
                send_to_char("You are doing the best you can!\r\n", ch);
            break;
        case SCMD_SHADOWFORM:
            if(IS_DARK_PRIEST(ch)) {
                if(affected_by_spell(ch, SKILL_SHADOWFORM))
                    affect_from_char(ch, SKILL_SHADOWFORM);
                else {
                    stun = 1;
                    act("You step away from the healing arts.", FALSE, ch, NULL, NULL, TO_CHAR);
                    act("$n is surrounded by a menacing cloud.", TRUE, ch, NULL, NULL, TO_ROOM);
                    add_affect(ch, ch, SKILL_SHADOWFORM, GET_LEVEL(ch),
                            APPLY_SPELL_DAMAGE, 45, -1, 0, FALSE, FALSE, FALSE, FALSE);
                    COOLDOWN(ch, SLOT_SHADOW_FORM) = 5;
                    GET_MANA(ch) = MAX(0, GET_MANA(ch) - 150);
                }
            }
            else {
                send_to_char("You are doing the best you can!\r\n", ch);
                return;
            }
            break;
        case SCMD_PYRE:
            if(IS_CHI_WARRIOR(ch))
            {
                stun = 1;
                if (ch->affected) {
                    while (ch->affected) /* affect_remove FALSE is for fall rooms */
                        affect_remove(ch, ch->affected, FALSE);
                    send_to_char("There is a brief flash of light!\r\n"
                                 "You feel slightly different.\r\n", ch);
                    GET_HIT(ch) = MAX(1, (GET_HIT(ch)*4)/5);
                    GET_MANA(ch) = MAX(0, (GET_MANA(ch)*4)/5);
                    GET_MOVE(ch) = MAX(0, (GET_MOVE(ch)*4)/5);
                }
                else
                    sendChar(ch, "You are affected by no spells.\r\n");
            }
            else
                sendChar(ch, "You do not know how.\r\n");
            break;
        case SCMD_COM_SHOUT:
            if(COOLDOWN(ch, SLOT_COMMANDING_SHOUT)) {
                send_to_char("You are not ready to perform another amazing feat.", ch);
                return;
            }

            stun = 2;
            if(IS_DEFENDER(ch) && percentSuccess(80) && GET_ADVANCE_LEVEL(ch) >= FIRST_ADVANCE_SKILL) {
                COOLDOWN(ch, SLOT_COMMANDING_SHOUT) = 5;
                act("You let out a blood curdling yell.", FALSE, ch, NULL, NULL, TO_CHAR);
                act("&10$n lets out a blood curdling yell!&00", TRUE, ch, NULL, NULL, TO_ROOM);
                for( tch = world[ch->in_room].people; tch; tch = next_tch ){
                    next_tch = tch->next_in_room;

                    add_affect(ch, tch, SKILL_COMMANDING_SHOUT, GET_LEVEL(ch),
                            0, 0, 1, 0, FALSE, FALSE, FALSE, FALSE);
                }
            }
            else {
                do_say(ch, "Hey, can't we all just get along?", 0, 0);
            }
            break;
        case SCMD_SACRIFICE:
            stun = number(1,2);

            if(IS_WITCH_DOCTOR(ch) && GET_ADVANCE_LEVEL(ch) >= 2) {
                for( tch = world[ch->in_room].people; tch; tch = next_tch ){
                    next_tch = tch->next_in_room;

                    if(tch && tch->master && tch->master == ch && affected_by_spell(tch, SPELL_CHARM_CORPSE)) {
                        found = TRUE;
                        break;
                    }
                }

                if(!found) {
                    send_to_char("You can not find a nearby charmed corpse to sacrifice.\r\n", ch);
                    return;
                }

                // transfer objects to room, if any
                while( tch->carrying )
                {
                    obj = tch->carrying;
                    obj_from_char(obj);
                    obj_to_room(obj, tch->in_room);
                }

                char_from_room(tch);
                char_to_room(tch, 1);
                
                act("You sacrifice your charmed corpse for the greater good.", FALSE, ch, NULL, NULL, TO_CHAR);
                act("$n is surrounded by a dark aura.", TRUE, ch, NULL, NULL, TO_ROOM);
                affect_from_char(ch, SKILL_NM_SACRIFICE);
                add_affect(ch, ch, SKILL_NM_SACRIFICE, GET_LEVEL(ch),
                        0, 0, 1 TICKS, 0, FALSE, FALSE, FALSE, FALSE);
                SACRIFICE(ch) = GET_HIT(tch)/number(2, 10);
                GET_MANA(ch) = MAX(0, GET_MANA(ch) - 150);

                if(affected_by_spell(tch, SPELL_CHARM_CORPSE))
                    affect_from_char(tch, SPELL_CHARM_CORPSE);
            }
            else {
                send_to_char("You do not know how.\r\n", ch);
                return;
            }
            break;

        case SCMD_METAMORPH:
            if(!IS_WITCH_DOCTOR(ch)) {
                send_to_char("You do not know how to metamorphosize.\r\n", ch);
                return;
            }

            if(COOLDOWN(ch, SLOT_METAMORPHOSIS)) {
                send_to_char("You are not ready to metamorph again.\r\n", ch);
                return;
            }

            if(!*argument) {
                send_to_char("What corpse do you want to metamorphpsize?\r\n", ch);
                return;
            }
            else if(!(obj = get_obj_in_list_vis(ch, arg, ch->carrying))) {
                sendChar( ch, "You don't seem to have %s %s.\r\n", AN(arg), arg);
                return;
            }

            if (GET_OBJ_TYPE(obj) != ITEM_CONTAINER || GET_OBJ_VAL(obj, 3) != 1) {
                send_to_char("You don't know how to use that.\r\n", ch);
                return;
            }

            if (obj->contains) {
                send_to_char("The corpse must be empty first.\r\n", ch);
                return;
            }

            // May not control pets while METAMORPHOSIS is active
            for (f = ch->followers; f; f = f->next) {
                if (IS_AFFECTED(f->follower, AFF_CHARM) && (f->follower->master == ch))
                {
                    sendChar(ch, "You lose control of your creature!\r\n");
                    affect_from_char(f->follower, SPELL_CHARM_CORPSE);
                }
            }

            mob = read_mobile(obj->obj_flags.cost_per_day, VIRTUAL);
            if (!mob) {
                send_to_char("The life has completely left this creature.\r\n", ch);
                return;
            }

            char_to_room(mob, ch->in_room);

            GET_HIT(ch) -= GET_MAX_HIT(ch)/5;
            GET_MANA(ch) = MAX(0, GET_MANA(ch) - GET_MANA(ch)/number(3, 7));
            act("You awaken from anxious dreams and discover you've transformed "
                    "into a monstrous, verminous bug.", FALSE, ch, NULL, NULL, TO_CHAR);
            act("$n transforms into a monstrous, verminous bug.", TRUE, ch, NULL, NULL, TO_ROOM);
            add_affect(ch, ch, SKILL_METAMORPHOSIS, MIN(calculateSimpleDamage(mob), 100),
                        0, 0, 25, 0, FALSE, FALSE, FALSE, FALSE);
            COOLDOWN(ch, SLOT_METAMORPHOSIS) = 3;
            stun = 1;

            extract_char(mob);
            extract_obj(obj);
            break;

        case SCMD_PHASESHIFT:
            if(!(IS_SHADE(ch) && GET_ADVANCE_LEVEL(ch) >= SECOND_ADVANCE_SKILL)) {
                sendChar(ch, "You don't know how.\r\n");
                return;
            }
            if(COOLDOWN(ch, SLOT_NODESHIFT)) {
                sendChar(ch, "You are not ready to phaseshift again.\r\n");
                return;
            }

            stun = 1;
            COOLDOWN(ch, SLOT_NODESHIFT) = 5;
            act("You shift your spectrum.", FALSE, ch, NULL, NULL, TO_CHAR);
            act("The image of $n distorts from unseen forces.", TRUE, ch, NULL, NULL, TO_ROOM);
            add_affect(ch, ch, SKILL_PHASESHIFT, GET_LEVEL(ch), 0, 0, 18, 0, FALSE, FALSE, FALSE, FALSE);
            break;
 
        case SCMD_HIPP:
            if(!IS_KNIGHT_TEMPLAR(ch)) {
                sendChar(ch, "You may not take this oath.\r\n");
                return;
            }

            stun = 1;

            if(affected_by_spell(ch, SPELL_HIPPOCRATIC_OATH)) {
                affect_from_char(ch, SPELL_HIPPOCRATIC_OATH);
                sendChar(ch, "You sacrifice your convictions and suffer for it.\r\n");
                GET_ALIGNMENT(ch) = MAX(-1000, GET_ALIGNMENT(ch) - 100);
                add_affect(ch, ch, SPELL_BANISH, GET_LEVEL(ch), APPLY_SKILL_SUCCESS, -40, 60, 0, FALSE, FALSE, FALSE, FALSE);
                break;
            }

            act("You bolster your conviction of divine reverence.", FALSE, ch, NULL, NULL, TO_CHAR);
            act("$n glows in a holy aura.", TRUE, ch, NULL, NULL, TO_ROOM);
            add_affect(ch, ch, SPELL_HIPPOCRATIC_OATH, GET_LEVEL(ch), 0, 0, -1, 0, FALSE, FALSE, FALSE, FALSE);

            break;
            
        default:
            send_to_char("Error!  Report this!\r\n", ch);
            break;
    }

    if(stun)
        WAIT_STATE(ch, SET_STUN(stun));
}


