
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/comm.h"
#include "general/handler.h"
#include "actions/interpreter.h"
#include "util/utils.h"
#include "specials/special.h"
#include "general/class.h"
#include "magic/spells.h"
#include "scripts/dg_scripts.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */
#include "specials/qpshop.h"

/* No longer used for shop.
static void buy_prac(CharData *ch, int userdata)
{
    //GET_PRACTICES(ch) += 1;
    sendChar(ch, "You feel ready to learn anything!\r\n");
    sendChar(ch, "Unfortunately, this has no effect!\r\n");
}
*/

static void buy_align(CharData *ch, int userdata)
{
    GET_ALIGNMENT(ch) = userdata;
    sendChar(ch, "Your disposition suddenly changes.\r\n");
}

#define CASTER_PROXY 1
static void buy_spell(CharData *ch, int userdata)
{
    CharData *caster = read_mobile(CASTER_PROXY, VIRTUAL);

    if (!caster) {
        sendChar(ch, "You feel cheated by a bug.\r\n");
        return;
    }

    sendChar(ch, "Energy crackles around the room.\r\n");

    caster->player.short_descr = str_dup("the gods");
    char_to_room(caster, ch->in_room);
    call_magic(caster, ch, NULL, userdata, 51, NULL, CAST_SPELL);
    extract_char(caster);
}

static void qpbuy_item(CharData *ch, int userdata)
{
   ObjData *obj;
   obj = read_perfect_object(real_object(userdata), REAL);
   SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_SOULBOUND);
   load_otrigger(obj);
   obj_to_char(obj, ch);
   sendChar(ch, "You feel strange, and then suddenly realize you have something new.\r\n");
}

static struct qpshop_cmds {
    char *command;
    int qpcost;
    void (*func)(CharData *ch, int userdata);
    int userdata;
} qpshop_cmds[] = {
    {"good"        , 4, buy_align  , 1000           },
    {"neutral"     , 4, buy_align  , 0              },
    {"evil"        , 4, buy_align  , -1000          },
    {"sanc"        , 2, buy_spell  , SPELL_SANCTUARY},
    {"haste"       , 2, buy_spell  , SPELL_HASTE    },
    {"air"         , 2, buy_spell  , SPELL_AIRSPHERE},
//    {"fastlearn"   , 0, buy_spell  , SPELL_FAST_LEARNING},
    {"damage"      , 1, buy_spell  , SKILL_SHOWDAM},

    {"divinity"    , 25, qpbuy_item, 24801          },
    {"destruction" , 25, qpbuy_item, 24802          },
    {"momentum"    , 25, qpbuy_item, 24803          },
    {"protection"  , 25, qpbuy_item, 24804          },
    {"regenerate", 25, qpbuy_item, 24805          },
    {"vision"      , 25, qpbuy_item, 24806          },
    {"charisma"    , 20, qpbuy_item, 24807          },
    {NULL          ,  0, NULL      , 0               }
};

SPECIAL (qpshop) {
    if (!ch->desc || IS_NPC(ch)) return FALSE;

    if (!CMD_IS("list") && !CMD_IS("buy")) return FALSE;

    if (CMD_IS("list")) {
        sendChar(ch,
"You hear nothing, and see nothing, but suddenly gain understanding:\r\n\r\n"
"   NAME   &14|&00 QP COST &14|&00 SERVICE\r\n"
"&14----------+---------+-----------------------------------------&00\r\n"
"  good    &14|&00    4    &14|&00 Set alignment to Holy\r\n"
"  neutral &14|&00    4    &14|&00 Set alignment to Neutral\r\n"
"  evil    &14|&00    4    &14|&00 Set alignment to Satanic\r\n"
"  sanc    &14|&00    2    &14|&00 Gain a temporary sanctuary\r\n"
"  haste   &14|&00    2    &14|&00 Gain a temporary haste\r\n"
"  air     &14|&00    2    &14|&00 Gain a temporary airsphere\r\n"
//" fastlearn&14|&00    0    &14|&00 Learn at a faster rate\r\n"
"  damage  &14|&00    1    &14|&00 Gain ability to see your damage\r\n"
"&14----------+---------+-----------------------------------------&00\r\n"
"\r\n"
"The following orbitals are also available for purchase:\r\n"
"(Type read orbitals for more details)\r\n"
"&14--------------------------------------------------------------&00\r\n"
"    divinity   &14|&00   25\r\n"
"  destruction  &14|&00   25\r\n"
"    momentum   &14|&00   25\r\n"
"   protection  &14|&00   25\r\n"
"   regenerate  &14|&00   25\r\n"
"    vision     &14|&00   25\r\n"
"   charisma    &14|&00   20\r\n"
"&14--------------------------------------------------------------&00\r\n"
);

        return TRUE;
    }

    if (CMD_IS("buy")) {
        struct qpshop_cmds *qpcmd = qpshop_cmds;
        char cmd[80];

        argument = one_argument(argument, cmd);

        /* hunt down the command they seek */
        while (qpcmd->command && !is_abbrev(cmd, qpcmd->command))
            qpcmd++;

        /* if there's none, they're confused */
        if (!qpcmd->command) {
            sendChar(ch, "You feel a sense of confusion.\r\n");
            return TRUE;
        }

        /* can they afford it? */
        if (qpcmd->qpcost > GET_QP(ch)) {
            sendChar(ch, "You get the feeling you need to quest more.\r\n");
            return TRUE;
        }

        GET_QP(ch) -= qpcmd->qpcost;

        /* fire it off! */
        qpcmd->func(ch, qpcmd->userdata);

        return TRUE;
    }

    return FALSE;
}

