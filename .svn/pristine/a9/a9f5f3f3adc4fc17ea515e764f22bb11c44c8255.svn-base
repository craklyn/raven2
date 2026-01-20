/*
**  FACILITY:  RavenMUD
**
**  LEGAL MUMBO JUMBO:
**
**      This is based on code developed for DIKU and Circle MUDs.
**
**  MODULE DESCRIPTION:
**
**  AUTHORS:
**
**      Arbaces from RavenMUD
**
**  NOTES:
**
**      Use 132 column editing in here.
*/

/*
** STANDARD U*IX INCLUDES
*/
#include "general/conf.h"
#include "general/sysdep.h"

#define THIS_SKILL                SKILL_DELUSION

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You better understand your shadow."
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               TRUE
#define INT_AFFECTS               FALSE
#define WIS_AFFECTS               FALSE

#define STUN_MIN                  1
#define STUN_MAX                  3

/*
** MUD SPECIFIC INCLUDES
*/
#include "general/db.h"
#include "general/structs.h"
#include "general/class.h"
#include "general/comm.h"
#include "general/handler.h"
#include "actions/interpreter.h"
#include "magic/skills.h"
#include "magic/spells.h"
#include "magic/delusion.h"
#include "util/utils.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */
#include "actions/fight.h"

ACMD(do_delusion)
{
    #define MANA_COST	40
    
    CharData* delusion = NULL;
    CharData *tch, *next_tch, *victim;
    int j;
    int leaves_combat = TRUE;
    
    char delusionDescr[80] = "";
    char delusionName[80]  = "";
    
    IF_UNLEARNED_SKILL   ( "You delude yourself into believing you can delude others.\r\n" );
    IF_ROOM_IS_PEACEFUL  ( "You pull a rose boquet from your sleeve.\r\n" );
    
    if ( GET_MANA(ch) < MANA_COST ) {
        send_to_char("You don't have enough energy to conjure a shadowy double.", ch);
        return;
    }
    
    if(!(victim = FIGHTING(ch))) {
        send_to_char("You must be in combat to delude an opponent.\n\r", ch);
        return;
    }
    
    if(skillSuccess(ch, THIS_SKILL)) {
        advanceSkill( ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS );
        
        // Create the delusion and add it to the list.
        delusion = create_char();

        delusion->nr              = 0;
        delusion->in_room         = ch->in_room;
        delusion->was_in_room     = NOWHERE;
        delusion->player_specials = &dummy_mob;

        strcpy(delusionName, "PC_delusion shadow");
        strcat(delusionName, ch->player.name);
        delusion->player.name     = strdup(delusionName);
        
        strcpy(delusionDescr, ch->player.name);
        strcat(delusionDescr, "'s shadow");
        delusion->player.short_descr = strdup(delusionDescr);
        delusion->player.long_descr  = NULL;
        delusion->player.title       = NULL;

        SET_BIT_AR(MOB_FLAGS(delusion), MOB_ISNPC);
        SET_BIT_AR(MOB_FLAGS(delusion), MOB_NOCHARM);
        SET_BIT_AR(AFF_FLAGS(delusion), AFF_FLY);
        GET_ALIGNMENT(delusion)  =  GET_ALIGNMENT(ch);
        GET_LEVEL(delusion)      = GET_LEVEL(ch);
        GET_EXP(delusion)        = 1;
        GET_MAX_HIT(delusion)    = GET_MAX_HIT(ch);
        GET_MAX_MANA(delusion)   = GET_MAX_MANA(ch);
        GET_MAX_MOVE(delusion)   =  GET_MAX_MOVE(ch);
        GET_HIT(delusion)        = GET_MAX_HIT(ch);
        GET_MANA(delusion)       = GET_MAX_MANA(ch);
        GET_MOVE(delusion)       = GET_MAX_MOVE(ch);
        
        delusion->points.hitroll           = 0;
        delusion->points.armor             = 0;
        delusion->mob_specials.damnodice   = 1;
        delusion->mob_specials.damsizedice = GET_LEVEL(ch);
        SET_DAMROLL(delusion, 0);
        delusion->mob_specials.attack_type = 15;
        delusion->char_specials.position   = POS_STANDING;
        delusion->mob_specials.default_pos = POS_STANDING;
        
        GET_SEX(delusion)    = GET_SEX(ch);
        GET_CLASS(delusion)  = GET_CLASS(ch);
        GET_WEIGHT(delusion) = GET_WEIGHT(ch);
        GET_HEIGHT(delusion) = GET_HEIGHT(ch);
        SET_RACE(delusion, GET_RACE(ch)); 

        for (j = 0;j < 3; j++) GET_COND(delusion, j) = -1;
        for (j = 0;j < 5; j++) GET_SAVE(delusion, j) = 0;
        roll_real_abils(delusion);
        
        GET_GOLD(delusion) = 0;

        if(GET_LEVEL(delusion) >= LVL_IMMORT)
            GET_LEVEL(delusion) = LVL_IMMORT;

        delusion->mobskill_suc = 0;
        delusion->mobspell_dam = 0;

        // If the player is a Prestidigitator, the delusion is quite a bit more powerful..
        if(IS_PRESTIDIGITATOR(ch) && GET_ADVANCE_LEVEL(ch) >= SECOND_ADVANCE_SKILL) {
            // Mobs get the players' Skill Success Mod as a Percent Success.
            int skillSuccessMod = 0;
            int i;
            for(i = 0; i<250; i++) {
                if(equipmentSkillSuccess(ch)) {
                    skillSuccessMod += 1;
                }
            }
            delusion->mobskill_suc = skillSuccessMod/10;
            // Mobs get players' bonus spell damage
            delusion->mobspell_dam = spell_damage_gear(ch)/3;

            delusion->mobskill_suc += 70;
            delusion->mobspell_dam += 100;
        }
        
        sendChar( ch, "You vanish and reappear, leaving your shadow behind.\r\n" );

        GET_MANA(ch) -= MANA_COST;
        for( tch = world[ch->in_room].people; tch; tch = next_tch ){
            next_tch = tch->next_in_room;
            
            if(FIGHTING(tch) == ch) {
                if(chGroupedWithTarget(FIGHTING(tch), ch))
                    set_fighting(delusion, tch);

                if(IS_NPC(tch) && IS_SET_AR(MOB_FLAGS(tch), MOB_AWARE)) {
                    leaves_combat = FALSE;
                    act("$N is too alert to be fooled.\r\n", FALSE, ch, 0, victim, TO_CHAR );
                }
                else {
                    set_fighting(delusion, tch);
                    FIGHTING(tch) = delusion;
                }
            }
        }

        char_to_room(delusion, ch->in_room);
        
        if(!FIGHTING(delusion)) {
            sendChar(ch, "Your shadow can't get the better of your opponent.\r\n");
            extract_char(delusion);
            return;
        }

        // If the character is fighting a particular opponent, the delusion fights that opponent preferentially.
        set_fighting(delusion, FIGHTING(ch));

        if(leaves_combat)
            stop_fighting(ch);

        add_affect(ch, delusion, THIS_SKILL, 35, 0, 0, -1, 0, FALSE, FALSE, FALSE, FALSE);
        
        STUN_USER_MIN;
    }
    else {
        GET_MANA(ch) -= (MANA_COST >> 1);
        STUN_USER_MAX;
        act("You fail to dissapear from view.", FALSE, ch, 0, victim, TO_CHAR);
        act("$n flickers in the darkness, then becomes fully visible again.", FALSE, ch, 0, victim, TO_VICT);
        act("$n flickers into the darkness, but then becomes fully visible.", FALSE, ch, 0, victim, TO_NOTVICT);
    }

}/* do_SKILL */
