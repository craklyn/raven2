
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
#include "specials/lockers.h"
#include "olc/olc.h"
#include "general/color.h"

#define GOLD_COST       1000000
#define QP_COST         15

SPECIAL( rename_manager )
{
    char item_name[MAX_INPUT_LENGTH], *newname;
    int len, qpcost;
    char *aliases;
    ObjData *obj;

    struct char_data *manager = me;

    if( !ch->desc || IS_NPC(ch)) return FALSE;

    if( !CMD_IS("rename") ) return FALSE;

    if (top_of_rename_t == MAX_RENAMES) {
        act("$n tells you, 'Sorry, we've sold out!'", FALSE, manager, 0,
                ch, TO_VICT);
        return TRUE;
    }

    /* you have to be at least this tall to ride */
    if (GET_LEVEL(ch) < 45) {
        act("$n tells you, 'Earn some more experience first!'", FALSE,
                manager, 0, ch, TO_VICT);
        return TRUE;
    }

    newname = one_argument(argument, item_name);
    skip_spaces(&newname);

    if (*item_name == '\0') {
        act("$n tells you, 'Rename what item?'", FALSE, manager, 0,
                ch, TO_VICT);
        return TRUE;
    }

    len = strlen(newname);
    if (len > (MAX_OBJ_NAME - 4)) {
        act("$n tells you, 'That's too long a name!'", FALSE, manager, 0,
                ch, TO_VICT);
        return TRUE;
    }
    strcpy(newname + len, "&00");

    if (!mortal_color(newname, ch)) return TRUE;

    aliases = strdup(newname);
    procColor(aliases, 0);

    if (*aliases == '\0') {
        act("$n tells you, 'What do you want to call it?'", FALSE, manager, 0,
                ch, TO_VICT);
        return TRUE;
    }

    // find the item in question
    obj = get_obj_in_list_vis( ch, item_name, ch->carrying);
    if (!obj) {
        free(aliases);
        act("$n tells you, 'You don't seem to have that!'", FALSE, manager,
                0, ch, TO_VICT);
        return TRUE;
    }

    if (GOLD_COST > GET_GOLD(ch)) {
        free(aliases);
        act("$n tells you, 'You don't have the gold!'", FALSE, manager, 0,
                ch, TO_VICT);
        return TRUE;
    }

    // subsequent renames just cost gold
    qpcost = QP_COST;
    if (GET_OBJ_RENAME(obj)) qpcost = 0;

    if (qpcost > GET_QP(ch)) {
        free(aliases);
        act("$n tells you, 'You don't have the quest points!'", FALSE,
                manager, 0, ch, TO_VICT);
        return TRUE;
    }

    // If the object has already been renamed, reuse the slot
    if (!GET_OBJ_RENAME(obj)) {
        top_of_rename_t++;
        GET_OBJ_RENAME(obj) = top_of_rename_t;
    }

    strcpy(renames[GET_OBJ_RENAME(obj) - 1].name, newname);
    renames[GET_OBJ_RENAME(obj) - 1].renamer_id = GET_IDNUM(ch);

    // save the rename table
    save_renames();

    // charge it!
    GET_GOLD(ch) -= GOLD_COST;
    GET_QP(ch) -= qpcost;

    act("$n takes $p from you, and studies it briefly.", FALSE, manager,
            obj, ch, TO_VICT);

    // set the new name and aliases
    obj->short_description = strdup(newname);
    obj->name = aliases;

    // notify the user
    act("$n hands you $p.", FALSE, manager, obj, ch, TO_VICT);

    // log the action
    mudlog( NRM, LVL_GOD, TRUE, "%s has renamed #%d to '&00%s&02'", GET_NAME(ch),
            GET_OBJ_VNUM(obj), obj->short_description);

    return TRUE;
}

