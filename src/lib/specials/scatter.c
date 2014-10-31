/*
 * scatter drop
 *
 * A system to randomly place items around the MUD
 *
 */

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/handler.h"
#include "general/class.h"
#include "util/utils.h"
#include "specials/scatter.h"
#include "actions/interpreter.h"

#define MOB  0
#define ROOM 1

/* For each item to scatter, supply the item's vnum, the max # to have
 * loaded on the MUD at any time, and the % chance to load a new one
 * at each scatter pulse, and whether to load on a mob or in a room. */
struct scatter_data {
    int vnum, max, rate, on;
} *ScatterData = NULL;

/* Also, you can instruct a particular mob to tally the items given to
 * it by a particular player.  For each vnum/mob, there is a separate
 * file recording the current tallies. */
struct collect_data {
    int obj_vnum, mob_vnum;
    char file[128];
} *CollectData = NULL;

typedef struct collect_save {
    long id;
    short tally;
} COLLECTSave;

static int SCATTERS = 0;
static int COLLECTS = 0;

void scatter_pulse(void)
{
    int i, rnum, dest, r = 0;
    CharData *mob, *target = NULL;
    ObjData *obj;

    for (i = 0; i < SCATTERS; i++) {
        rnum = real_object(ScatterData[i].vnum);

        /* If there's space, and the random chance kicks in, ... */
        if (obj_index[rnum].number < ScatterData[i].max &&
            number(1, 100) < ScatterData[i].rate) {
            obj = read_object(rnum, REAL);
            if (ScatterData[i].on == ROOM) {
                dest = number(0, top_of_world);

                /* find a home for it */
                if (obj) {
                    obj_to_room(obj, dest);
                    mudlog( NRM, LVL_GOD, FALSE, "Scatter item %s loaded in room #%d, %s",
                            obj->short_description, world[dest].number,
                            world[dest].name);
                }
            } else {
                for (r = 0, mob = character_list; mob; mob = mob->next) {
                    if (IS_NPC(mob) && number(1, ++r) == 1 && mob->in_room > 0)
                        target = mob;
                }

                if (obj && target) {
                    obj_to_char(obj, target);
                    mudlog( NRM, LVL_GOD, FALSE, "Scatter item %s given to %s in %s [%d]",
                            obj->short_description, GET_NAME(target),
                            world[target->in_room].name,
                            world[target->in_room].number);
                }
            }
        }
    }
}

static void scatter_increment(CharData *ch, char *file)
{
    COLLECTSave rec;
    FILE *f = fopen(file, "rb+");
    int found = 0, max = 0, you = 0;
    long id = GET_IDNUM(ch);

    /* try to create a new file if none was found */
    if (!f) f = fopen(file, "wb+");

    if (!f) {
        mudlog( NRM, LVL_GOD, TRUE, "Scatter file '%s' could not be opened", file);
        return;
    }

    fseek(f, 0, SEEK_SET);
    while (!feof(f)) {
        if (fread(&rec, sizeof(rec), 1, f) == 1) {
            if (rec.id == id && !found) {
                you = ++rec.tally;
                fseek(f, -sizeof(rec), SEEK_CUR);
                fwrite(&rec, sizeof(rec), 1, f);
                found = 1;
            }
            if (rec.tally > max) max = rec.tally;
        }
    }

    /* append a new record? */
    if (!found) {
        rec.id = id;
        you = rec.tally = 1;
        fwrite(&rec, sizeof(rec), 1, f);
    }

    if (max <= you) {
        sendChar(ch, "You are in the lead!\r\n");
    } else {
        sendChar(ch, "You are %d items behind the leader!\r\n", max - you);
    }

    fclose(f);
}

int check_scatter_give(CharData *ch, CharData *vict, ObjData *obj)
{
    int i, o_vnum, m_vnum;

    /* gotta be a player giving to a mob */
    if (IS_NPC(ch) || !IS_NPC(vict)) return 0;

    o_vnum = GET_OBJ_VNUM(obj);
    m_vnum = GET_MOB_VNUM(vict);

    for (i = 0; i < COLLECTS; i++) {
        if (o_vnum == CollectData[i].obj_vnum &&
            m_vnum == CollectData[i].mob_vnum) {
            scatter_increment(ch, CollectData[i].file);
            extract_obj(obj);
            return 1;
        }
    }
    return 0;
}

static void find_top_ten(CharData *ch, char *file)
{
}

void scatter_init(void)
{
    FILE *f;
    long sz;

    if (!(f = fopen("tally/scatters.dat", "rb"))) {
        exit(1); // No file bail out!
    }
    
    if (f) {
        fseek(f, 0, SEEK_END);
        sz = ftell(f);
        if (sz % sizeof (struct scatter_data) != 0) {
            mudlog(NRM, LVL_GOD, TRUE, "ERROR: Scatter data file is incorrect size!");
            exit(1); // Corrupted file, bail!
        } else {
            rewind(f);
            SCATTERS = sz / sizeof (struct scatter_data);
            CREATE(ScatterData, struct scatter_data, SCATTERS + 1);
            fread(ScatterData, sizeof (struct scatter_data), SCATTERS, f);
        }
        fclose(f); // Let's close out and go home!
    }

    /* But wait there is more! I had to laugh when I saw this, oh well...I
     * guess breaking up functions ain't for everyone.! */

    if (!(f = fopen("tally/tallies.dat", "rb"))) {
        exit(1);
   }
    if (f) {
        fseek(f, 0, SEEK_END);
        sz = ftell(f);
        if (sz % sizeof (struct collect_data) != 0) {
            mudlog(NRM, LVL_GOD, TRUE, "ERROR: Tally data file is incorrect size!");
            exit(1);
        } else {
            rewind(f);
            COLLECTS = sz / sizeof (struct collect_data);
            CREATE(CollectData, struct collect_data, COLLECTS +1);
            fread(CollectData, sizeof (struct collect_data), COLLECTS, f);
        }
        fclose(f);
    } /* Okay I guess we're done for real this time RAWR! */
}

void save_scatters(CharData *ch)
{
    FILE *f = fopen("tally/scatters.dat", "wb");

    if (!f) {
        if (ch) sendChar(ch, "Couldn't open scatter data file!\r\n");
        mudlog(BRF, LVL_GOD, FALSE, "ERROR: Cannot open scatter data file for writing!\r\n");
        return;
    }
    fwrite(ScatterData, sizeof(struct scatter_data), SCATTERS, f);
    fclose(f);
}

void save_collects(CharData *ch)
{
    FILE *f = fopen("tally/tallies.dat", "wb");

    if (!f) {
        if (ch) sendChar(ch, "Couldn't open tally data file!\r\n");
        mudlog(BRF, LVL_GOD, FALSE, "ERROR: Cannot open tally data file for writing!\r\n");
        return;
    }
    fwrite(CollectData, sizeof(struct collect_data), COLLECTS, f);
    fclose(f);
}

#define HELP_SCATTER_ANY \
"Usage: scatter add <vnum> <max> <rate> <type>\r\n" \
"       scatter edit <id> <vnum> <max> <rate> <type>\r\n" \
"       scatter del <id>\r\n"

#define HELP_SCATTER_ADD \
"Usage: scatter add <vnum> <max> <rate> <type>\r\n" \
"    vnum     the object to scatter\r\n" \
"    max      the maximum to have loaded at one time\r\n" \
"    rate     the \% chance to load an object at each scatter pulse\r\n" \
"    type     either MOB or ROOM, for where to load the objects\r\n"

#define HELP_SCATTER_EDIT \
"Usage: scatter edit <id> <vnum> <max> <rate> <type>\r\n" \
"    id       the scatter id to edit\r\n" \
"    vnum     the object to scatter\r\n" \
"    max      the maximum to have loaded at one time\r\n" \
"    rate     the \% chance to load an object at each scatter pulse\r\n" \
"    type     either MOB or ROOM, for where to load the objects\r\n"

#define HELP_SCATTER_DEL "Usage: scatter del <id>\r\n"

static int scatter_arguments(char *argument, struct scatter_data *dest)
{
    char vnum[20], max[20], rate[20], on[20];

    argument = two_arguments(argument, vnum, max);
    argument = two_arguments(argument, rate, on);

    /* Make sure all four arguments were supplied */
    if (!*vnum || !*max || !*rate || !*on) return 0;

    /* First three should be numbers */
    if (!is_number(vnum) || !is_number(max) || !is_number(rate)) return 0;

    /* Fourth should be (an abbreviation of) 'MOB' or 'ROOM' */
    *on = toupper(*on);
    if (*on != 'R' && *on != 'M') return 0;

    dest->vnum = atoi(vnum);
    dest->rate = atoi(rate);
    dest->max = atoi(max);
    dest->on = *on == 'R' ? ROOM : MOB;

    return 1;
}

ACMD(do_scatter)
{
    char action[40], idx[40];
    int i;

    argument = one_argument(argument, action);

    if (!*action) {
        sendChar(ch, "ID  VNUM  RATE  MAX  TYPE\r\n");
        for (i = 0; i < SCATTERS; i++) {
            sendChar(ch, "%-4d%-6d%-6d%-5d%s\r\n", i + 1,
                    ScatterData[i].vnum,
                    ScatterData[i].rate,
                    ScatterData[i].max,
                    ScatterData[i].on == ROOM ? "ROOM" : "MOB");
        }

        /* No change to data, no need to save it */
        return;
    } else if (is_abbrev(action, "add")) {
        struct scatter_data tmp;

        if (!scatter_arguments(argument, &tmp)) {
            sendChar(ch, HELP_SCATTER_ADD);
            return;
        }

        RECREATE(ScatterData, struct scatter_data, SCATTERS + 1);
        ScatterData[SCATTERS++] = tmp;
        sendChar(ch, "New scatter #%d added\r\n", SCATTERS);
    } else if (is_abbrev(action, "del")) {
        one_argument(argument, idx);

        if (!is_number(idx)) {
            sendChar(ch, HELP_SCATTER_DEL);
            return;
        }

        i = atoi(idx);
        if (i < 1 || i > SCATTERS) {
            sendChar(ch, "Invalid scatter ID!\r\n");
            return;
        }

        /* Need to shift the ones above it down in memory */
        if (i < SCATTERS) {
            memmove(ScatterData + i - 1, ScatterData + i,
                    sizeof(struct scatter_data) * (SCATTERS - i));
        }

        /* Then just resize the memory */
        if (SCATTERS > 1) {
            RECREATE(ScatterData, struct scatter_data, --SCATTERS);
        } else {
            free(ScatterData);
            ScatterData = NULL;
            SCATTERS = 0;
        }

        sendChar(ch, "Okay.\r\n");
    } else if (is_abbrev(action, "edit")) {
        argument = one_argument(argument, idx);

        if (!is_number(idx)) {
            sendChar(ch, HELP_SCATTER_EDIT);
            return;
        }

        i = atoi(idx);
        if (i < 1 || i > SCATTERS) {
            sendChar(ch, "Invalid scatter ID!\r\n");
            return;
        }

        if (!scatter_arguments(argument, &ScatterData[i - 1])) {
            sendChar(ch, HELP_SCATTER_EDIT);
            return;
        }

        sendChar(ch, "Okay.\r\n");
    } else {
        sendChar(ch, HELP_SCATTER_ANY);
    }

    save_scatters(ch);
}

static int collect_arguments(char *argument, struct collect_data *dest)
{
    char obj[20], mob[20], file[128];

    argument = two_arguments(argument, obj, mob);
    argument = one_argument(argument, file);

    /* Make sure all three arguments were supplied */
    if (!*obj || !*mob || !*file) return 0;

    /* First two should be numbers */
    if (!is_number(obj) || !is_number(mob)) return 0;

    dest->obj_vnum = atoi(obj);
    dest->mob_vnum = atoi(mob);
    strcpy(dest->file, file);

    return 1;
}

#define HELP_COLLECT_ADD \
"Usage: tally add <obj> <mob> <file>\r\n" \
"    obj      the object vnum to tally\r\n" \
"    mob      the mobile vnum who will collect the tally\r\n" \
"    file     the file to store the tallies in\r\n"

#define HELP_COLLECT_EDIT \
"Usage: tally edit <id> <vnum> <max> <rate> <type>\r\n" \
"    id       the tally id to edit\r\n" \
"    obj      the object vnum to tally\r\n" \
"    mob      the mobile vnum who will collect the tally\r\n" \
"    file     the file to store the tallies in\r\n"

#define HELP_COLLECT_DEL "Usage: tally del <id>\r\n"

ACMD(do_tally)
{
    char action[40], idx[40];
    int i;

    argument = one_argument(argument, action);

    if (!*action) {
        sendChar(ch, "#  Name   Obj   Mob\r\n");
        for (i = 0; i < COLLECTS; i++) {
            sendChar(ch, "%d  %-6s %-5d %d\r\n", i + 1, CollectData[i].file,
                    CollectData[i].obj_vnum, CollectData[i].mob_vnum);
        }
        return; /* No change to data, no need to save it */
    } else if (is_number(action)) {
        int tally = atoi(action);
        if (tally >= 0 && tally < COLLECTS) {
            sendChar(ch, "Tally %s top ten:\r\n", CollectData[tally].file);
            find_top_ten(ch, CollectData[tally].file);
        } else {
            sendChar(ch, "Tally id %d is out of range\r\n", tally);
        }
        return; /* No change to data, no need to save it */
    } else if (is_abbrev(action, "add")) {
        struct collect_data tmp;

        if (!collect_arguments(argument, &tmp)) {
            sendChar(ch, HELP_COLLECT_ADD);
            return;
        }

        RECREATE(CollectData, struct collect_data, COLLECTS + 1);
        CollectData[COLLECTS++] = tmp;
        sendChar(ch, "New tally #%d added\r\n", COLLECTS);
    } else if (is_abbrev(action, "del")) {
        argument = one_argument(argument, idx);

        if (!is_number(idx)) {
            sendChar(ch, HELP_COLLECT_DEL);
            return;
        }

        i = atoi(idx);
        if (i < 1 || i > COLLECTS) {
            sendChar(ch, "Invalid tally ID!\r\n");
            return;
        }

        /* Need to shift the ones above it down in memory */
        if (i < COLLECTS) {
            memmove(CollectData + i - 1, CollectData + i,
                    sizeof(struct collect_data) * (COLLECTS - i));
        }

        /* Then just resize the memory */
        if (COLLECTS > 1) {
            RECREATE(CollectData, struct collect_data, --COLLECTS);
        } else {
            free(CollectData);
            CollectData = NULL;
            COLLECTS = 0;
        }

        sendChar(ch, "Okay.\r\n");
    } else if (is_abbrev(action, "edit")) {
        one_argument(argument, idx);

        if (!is_number(idx)) {
            sendChar(ch, HELP_COLLECT_EDIT);
            return;
        }

        i = atoi(idx);
        if (i < 1 || i > COLLECTS) {
            sendChar(ch, "Invalid tally ID!\r\n");
            return;
        }

        if (!collect_arguments(argument, &CollectData[i - 1])) {
            sendChar(ch, HELP_COLLECT_EDIT);
            return;
        }

        sendChar(ch, "Okay.\r\n");
    }

    save_collects(ch);
}
