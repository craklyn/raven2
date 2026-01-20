
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/class.h"
#include "general/comm.h"
#include "actions/interpreter.h"
#include "util/utils.h"
#include "specials/special.h"
#include "magic/spells.h"

SPECIAL(unicorn)
{
    struct char_data *the_unicorn, *attacker;

    if( cmd == 0 && GET_POS(ch) == POS_STANDING){ }

    if( GET_POS(ch) != POS_FIGHTING && !FIGHTING(ch) ) return FALSE;

    else if( cmd == 151 ){ /* fleeing */
        the_unicorn = FIGHTING(ch);     attacker = ch;
        if( number( 0, 1 ) == 0 ){
            char *to_room   = "The Unicorn trips $n with his horn sending $m sprawling.";
            char *to_victim = "The Unicorn trips you with his horn.";

            act( to_room, FALSE, the_unicorn, 0, 0, TO_ROOM );
            send_to_char( to_victim, attacker );

            GET_POS( attacker ) = POS_SITTING;
            return TRUE;

        }/* if */
    }/* else if */

    else if( cmd == 0 ){
        the_unicorn = ch; attacker = FIGHTING(ch);

        if( number( 0, 5 ) == 0 ){
            char *to_room   = "The Unicorn's horn glows bright white!";
            char *to_victim = "The Unicorn points his horn at you!";

            act( to_room, FALSE, ch, 0, 0, TO_ROOM );
            send_to_char( to_victim, ch );
            cast_spell(the_unicorn, attacker, NULL, SPELL_TELEPORT);

            return TRUE;
        }/* if */

        else if( GET_HIT( ch ) < 300 ){
            char *to_room   = "The Unicorn glows with a bright light!\n\rThere is a sudden flash of light!\n\r";
            act( to_room, FALSE, ch, 0, 0, TO_ROOM );
            cast_spell(the_unicorn, the_unicorn, NULL, SPELL_TELEPORT);
            return TRUE;
        }/* else */

    }/* else */

    return FALSE;

}/* unicorn */


