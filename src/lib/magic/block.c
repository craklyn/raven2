/*
 **++
 **  RCSID:     $Id: block.c,v 1.1 2002/01/14 12:56:42 raven Exp $
 **
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
 **      Digger (NOTE: this was probably copy-pasted in; Digger probably didn't write Block)
 **
 **  NOTES:
 **
 **--
 */

/*
 ** STANDARD U*IX INCLUDES
 */

#define THIS_SKILL                SKILL_BLOCK

#define SKILL_ADVANCES_WITH_USE   TRUE
#define SKILL_ADVANCE_STRING      "You've become notably more proficient."
#define SKILL_MAX_LEARN           90
#define DEX_AFFECTS               TRUE
#define INT_AFFECTS               TRUE
#define WIS_AFFECTS               FALSE

#define STUN_MIN                  2
#define STUN_MAX                  4

/*
 ** MUD SPECIFIC INCLUDES
 */
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/class.h"
#include "general/comm.h"
#include "general/handler.h"
#include "actions/interpreter.h"
#include "magic/skills.h"
#include "magic/spells.h"
#include "util/utils.h"
#include "magic/block.h"

/*
 ** EXTERNAL DEFINITIONS OF SKILLS/SPELLS CALLED FROM THIS FILE
 **
 **
 */

/*
 **++
 **  FUNCTIONAL DESCRIPTION:
 **
 **      do_block -
 **
 **  FORMAL PARAMETERS:
 **
 **      ch:
 **          A pointer to the character structure that is trying to use the skill/spell.
 **
 **  RETURN VALUE:
 **
 **      None
 **
 **  DESIGN:
 **
 **      Sets bit AFF_SHIELDBLOCk, next melee attack from each enemy is blocked.
 **
 */
ACMD(do_block)
{
    IF_UNLEARNED_SKILL("You wouldn't know where to begin.\r\n");

    if (!FIGHTING(ch))
    {
        send_to_char("You block a vicious gust of wind with your shield.\r\n", ch);
        return;
    }

    if (!(ch->equipment[WEAR_SHIELD]))
    {
        sendChar(ch, "You must be wearing a shield to block.\r\n");
        return;
    }

    if (skillSuccess(ch, THIS_SKILL) || IS_NPC(ch))
    {
        advanceSkill(ch, THIS_SKILL, SKILL_MAX_LEARN, SKILL_ADVANCE_STRING, DEX_AFFECTS, INT_AFFECTS, WIS_AFFECTS);
        send_to_char("You raise your shield to defend yourself.\r\n", ch);
        sprintf(buf, "$n lifts %s shield to deflect incoming blows.", HSHR(ch));
        act(buf, FALSE, ch, 0, 0, TO_ROOM);
        SET_BIT_AR(AFF_FLAGS(ch), AFF_SHIELDBLOCK);
        STUN_USER_MIN;
    }
    else
    {
        send_to_char("You fumble with your shield.\r\n", ch);
        STUN_USER_MAX;
    }
}/* do_block  */
