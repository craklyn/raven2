
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/chores.h"
#include "util/utils.h"
#include "general/comm.h"
#include "actions/interpreter.h"
#include "general/handler.h"
#include "general/color.h"
#include "magic/magic.h"
#include "magic/spells.h"
#include "general/class.h"
#include "actions/quest.h"
#include "actions/act.clan.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */

/** Help buffer the global variable definitions */
#define __QUEST_C__

#define QFLAG(qst, flag) (IS_SET(qst->flags, flag))

char *questflag_bits[] = {
  "!GOOD", "!EVIL", "!NEUTRAL", "!MAGIC_USER", "!CLERIC", "!THIEF",
  "!WARRIOR", "!RANGER", "!ASSASSIN", "!SHOULIN", "!SOLAMNIC", "!DEATH_KN",
  "!SHAD_DANC", "!MINOTAUR", "!GNOME", "!ORC", "!ELF", "!DRACONIAN",
  "!HALFLING", "!OGRE", "!TROLL", "!DWARF", "!HUMAN", "ONCE_ONLY",
  "ONE_OBJECTIVE", "RANDOM_REWARD", "!DROW", "!NECRO", "COMMON_REWARD", "\n"
};

static int is_vowel(char ch)
{
    ch = tolower(ch);
    return (ch == 'a' || ch == 'e' || ch == 'i' || ch == 'o' || ch == 'u');
}

#define REAL_STR(ch) ((ch)->real_abils.str)
#define REAL_ADD(ch) ((ch)->real_abils.str_add)
#define REAL_INT(ch) ((ch)->real_abils.intel)
#define REAL_WIS(ch) ((ch)->real_abils.wis)
#define REAL_DEX(ch) ((ch)->real_abils.dex)
#define REAL_CON(ch) ((ch)->real_abils.con)
#define REAL_CHA(ch) ((ch)->real_abils.cha)

static int remort_costs[QST_NUM_REMORTS] = {
    50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
    25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 50, 25,
    50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50
};

static void remort(CharData *ch, int newrace, int costly)
{
    int i, oldrace, qp;

    /* Mobiles should never remort! */
    if (IS_NPC(ch)) return;

    if (costly && GET_LEVEL(ch) != 50) {
        send_to_char("Only the highest levels may be reborn!\r\n", ch);
        return;
    }

    // If you're already a remort race, no cost to remort again.
    if(IS_REMORT(ch))
        qp = 0;
    else
        qp = remort_costs[newrace];

    if (costly && GET_QP(ch) < qp) {
        sprintf(buf, "You must have %d quest points to be reborn.\r\n", qp);
        send_to_char(buf, ch);
        return;
    }

    // If you're already a remort, you do not need 100% exp to remort again.
    if (!IS_REMORT(ch) && costly && GET_EXP(ch) < titles[(int)GET_CLASS(ch)][51].exp) {
        send_to_char("You must have experience above and beyond your level "
                     "to be reborn.\r\n", ch);
        return;
    }

    oldrace = GET_RACE(ch);

    switch (newrace) {
        case QST_REMORT_DEMON:
            SET_RACE(ch, RACE_DEMON);
            SET_SUBRACE(ch, 0);
            break;
        case QST_REMORT_IZARTI:
            SET_RACE(ch, RACE_IZARTI);
            SET_SUBRACE(ch, 0);
            break;
        case QST_REMORT_WEREWOLF:
            SET_RACE(ch, RACE_WEREWOLF);
            SET_SUBRACE(ch, 0);
            break;
        case QST_REMORT_VAMPIRE:
            SET_RACE(ch, RACE_VAMPIRE);
            SET_SUBRACE(ch, 0);
            break;
        case QST_REMORT_GIANT:
            SET_RACE(ch, RACE_GIANT);
            SET_SUBRACE(ch, 0);
            break;
        case QST_REMORT_AMARA:
            SET_RACE(ch, RACE_AMARA);
            SET_SUBRACE(ch, 0);
            break;
        case QST_REMORT_FAERIE:
            SET_RACE(ch, RACE_FAERIE);
            SET_SUBRACE(ch, 0);
            break;
        case QST_REMORT_FIRE_ELEMENTAL:
            SET_RACE(ch, RACE_ELEMENTAL);
            SET_SUBRACE(ch, FIRE_ELEMENTAL);
            break;
        case QST_REMORT_WATER_ELEMENTAL:
            SET_RACE(ch, RACE_ELEMENTAL);
            SET_SUBRACE(ch, WATER_ELEMENTAL);
            break;
        case QST_REMORT_EARTH_ELEMENTAL:
            SET_RACE(ch, RACE_ELEMENTAL);
            SET_SUBRACE(ch, EARTH_ELEMENTAL);
            break;
        case QST_REMORT_AIR_ELEMENTAL:
            SET_RACE(ch, RACE_ELEMENTAL);
            SET_SUBRACE(ch, AIR_ELEMENTAL);
            break;
        case QST_REMORT_UNDEAD:
            SET_RACE(ch, RACE_UNDEAD);
            SET_SUBRACE(ch, 0);
            break;
        case QST_REMORT_HUMAN:
            SET_RACE(ch, RACE_HUMAN);       
            SET_SUBRACE(ch, 0);
            break;
        case QST_REMORT_HALFLING:
            SET_RACE(ch, RACE_HALFLING);
            SET_SUBRACE(ch, 0);
            break;
        case QST_REMORT_ELF:
            SET_RACE(ch, RACE_ELF);
            SET_SUBRACE(ch, 0);
            break;
        case QST_REMORT_DROW:
            SET_RACE(ch, RACE_DROW);
            SET_SUBRACE(ch, 0);
            break;
        case QST_REMORT_DWARF:
            SET_RACE(ch, RACE_DWARF);
            SET_SUBRACE(ch, 0);
            break;
        case QST_REMORT_MINOTAUR:
            SET_RACE(ch, RACE_MINOTAUR);
            SET_SUBRACE(ch, 0);
            break;
        case QST_REMORT_OGRE:
            SET_RACE(ch, RACE_OGRE);
            SET_SUBRACE(ch, 0);
            break;
        case QST_REMORT_TROLL:
            SET_RACE(ch, RACE_TROLL);
            SET_SUBRACE(ch, 0);
            break;
        case QST_REMORT_ORC:
            SET_RACE(ch, RACE_ORC);
            SET_SUBRACE(ch, 0);
            break;
        case QST_REMORT_GNOME:
            SET_RACE(ch, RACE_GNOME);           
            SET_SUBRACE(ch, 0);
            break;
        case QST_REMORT_DRACONIAN_RED:
        case QST_REMORT_DRACONIAN_GREEN:
        case QST_REMORT_DRACONIAN_WHITE:
        case QST_REMORT_DRACONIAN_BLACK:
        case QST_REMORT_DRACONIAN_BLUE:
            SET_RACE(ch, RACE_DRACONIAN);
            SET_SUBRACE(ch, newrace - QST_REMORT_DRACONIAN_RED + 1);
            break;
        case QST_REMORT_SHUMAN:
            SET_RACE(ch, RACE_SHUMAN);
            SET_SUBRACE(ch, 0);
            break;
        case QST_REMORT_SHALFLING:
            SET_RACE(ch, RACE_SHALFLING);
            SET_SUBRACE(ch, 0);
            break;
        case QST_REMORT_SELF:
            SET_RACE(ch, RACE_SELF);
            SET_SUBRACE(ch, 0);
            break;
        case QST_REMORT_SDROW:
            SET_RACE(ch, RACE_SDROW);
            SET_SUBRACE(ch, 0);
            break;
        case QST_REMORT_SDWARF:
            SET_RACE(ch, RACE_SDWARF);
            SET_SUBRACE(ch, 0);
            break;
        case QST_REMORT_SMINOTAUR:
            SET_RACE(ch, RACE_SMINOTAUR);
            SET_SUBRACE(ch, 0);
            break;
        case QST_REMORT_SOGRE:
            SET_RACE(ch, RACE_SOGRE);
            SET_SUBRACE(ch, 0);
            break;
        case QST_REMORT_STROLL:
            SET_RACE(ch, RACE_STROLL);
            SET_SUBRACE(ch, 0);
            break;
        case QST_REMORT_SDRACONIAN:
            // If player isn't already a drac, gets a random drac breathe
            if(!IS_DRACONIAN(ch))
                SET_SUBRACE(ch, number(1,5));
            SET_RACE(ch, RACE_SDRACONIAN);
            break;
        case QST_REMORT_SGNOME:
            SET_RACE(ch, RACE_SGNOME);
            SET_SUBRACE(ch, 0);
            break;
        case QST_REMORT_SORC:
            SET_RACE(ch, RACE_SORC);
            SET_SUBRACE(ch, 0);
            break;
        default:
            send_to_char("Uhoh!  I don't know the race you're set to become!"
                         "\r\n", ch);
            return;
    }

    /* unequip char */
    for (i = 0; i < NUM_WEARS; i++)
        if (ch->equipment[i])
            obj_to_char(unequip_char(ch, i), ch);

    /* unaffect char */
    while (ch->affected) affect_remove(ch, ch->affected, FALSE);

#define OLDLIM(x) (race_stat_limits[oldrace][x])
#define NEWLIM(x) (race_stat_limits[GET_RACE(ch)][x])

    /* set skills based on their old positions relative to max */
    i = OLDLIM(CHARISMA_INDEX) - REAL_CHA(ch);
    REAL_CHA(ch) = NEWLIM(CHARISMA_INDEX) - i;
    i = OLDLIM(CONSTITUTION_INDEX) - REAL_CON(ch);
    REAL_CON(ch) = NEWLIM(CONSTITUTION_INDEX) - i;
    i = OLDLIM(WISDOM_INDEX) - REAL_WIS(ch);
    REAL_WIS(ch) = NEWLIM(WISDOM_INDEX) - i;
    i = OLDLIM(DEXTERITY_INDEX) - REAL_DEX(ch);
    REAL_DEX(ch) = NEWLIM(DEXTERITY_INDEX) - i;
    i = OLDLIM(INTELLIGENCE_INDEX) - REAL_INT(ch);
    REAL_INT(ch) = NEWLIM(INTELLIGENCE_INDEX) - i;

    /* str is a little trickier, thanks to the str_add effect at 18 str */
    i = OLDLIM(STRENGTH_INDEX) - REAL_STR(ch);
    if (OLDLIM(STRENGTH_INDEX) == 18)
        i += OLDLIM(STRENGTH_ADD_INDEX)/10;
    if (OLDLIM(STRENGTH_INDEX) > 18)
        i += 10;
    if (REAL_STR(ch) == 18) i -= REAL_ADD(ch)/10;
    if (REAL_STR(ch) > 18) i -= 10;

    if (NEWLIM(STRENGTH_INDEX) < 18) {
        REAL_STR(ch) = NEWLIM(STRENGTH_INDEX) - i;
        REAL_ADD(ch) = 0;
    } else {
        int dist = NEWLIM(STRENGTH_INDEX) - 18;
        if (dist > i) {
            REAL_STR(ch) = NEWLIM(STRENGTH_INDEX) - i;
            REAL_ADD(ch) = 100;
        } else {
            i -= dist;
            if (i > 10) {
                REAL_STR(ch) = 18 - (i - 10);
                REAL_ADD(ch) = 0;
            } else {
                REAL_STR(ch) = 18;
                REAL_ADD(ch) = (10 - i) * 10;
            }
        }
    }

    if (REAL_STR(ch) < 0) REAL_STR(ch) = 0;
    if (REAL_INT(ch) < 0) REAL_INT(ch) = 0;
    if (REAL_WIS(ch) < 0) REAL_WIS(ch) = 0;
    if (REAL_DEX(ch) < 0) REAL_DEX(ch) = 0;
    if (REAL_CON(ch) < 0) REAL_CON(ch) = 0;
    if (REAL_CHA(ch) < 0) REAL_CHA(ch) = 0;

    if (REAL_STR(ch) > NEWLIM(STRENGTH_INDEX))
        REAL_STR(ch) = NEWLIM(STRENGTH_INDEX);
    if (REAL_INT(ch) > NEWLIM(INTELLIGENCE_INDEX))
        REAL_INT(ch) = NEWLIM(INTELLIGENCE_INDEX);
    if (REAL_WIS(ch) > NEWLIM(WISDOM_INDEX))
        REAL_WIS(ch) = NEWLIM(WISDOM_INDEX);
    if (REAL_DEX(ch) > NEWLIM(DEXTERITY_INDEX))
        REAL_DEX(ch) = NEWLIM(DEXTERITY_INDEX);
    if (REAL_CON(ch) > NEWLIM(CONSTITUTION_INDEX))
        REAL_CON(ch) = NEWLIM(CONSTITUTION_INDEX);
    if (REAL_CHA(ch) > NEWLIM(CHARISMA_INDEX))
        REAL_CHA(ch) = NEWLIM(CHARISMA_INDEX);

    if (REAL_STR(ch) < 18) REAL_ADD(ch) = 0;
    if (REAL_STR(ch) > 18) REAL_ADD(ch) = NEWLIM(STRENGTH_ADD_INDEX);
    if (REAL_STR(ch) == 18 && REAL_ADD(ch) > NEWLIM(STRENGTH_ADD_INDEX))
        REAL_ADD(ch) = NEWLIM(STRENGTH_ADD_INDEX);

#undef OLDLIM
#undef NEWLIM

    if (costly) GET_QP(ch) = GET_QP(ch) - qp;
    GET_PRACTICES(ch) = 0;
    GET_MAX_MANA(ch) = 100;

    ch->player.time.birth = time(0);

    for (i = 0; i <= MAX_SKILLS; i++)
        SET_SKILL(ch, i, 0);

    /* go initialise the char */
    do_initialise(ch);

    if (IS_REMORT(ch)) {
        switch (GET_CLASS(ch)) {
            case CLASS_WARRIOR:
                GET_MAX_HIT(ch) += 100;
                break;
            case CLASS_RANGER:
                GET_MAX_HIT(ch) += 70;
                GET_MAX_MANA(ch) += 10;
                GET_MAX_MOVE(ch) += 20;
                break;
            case CLASS_SOLAMNIC_KNIGHT:
            case CLASS_DEATH_KNIGHT:
                GET_MAX_HIT(ch) += 70;
                GET_MAX_MANA(ch) += 30;
                break;
            case CLASS_ASSASSIN:
            case CLASS_THIEF:
                GET_MAX_HIT(ch) += 60;
                GET_MAX_MOVE(ch) += 40;
                break;
            case CLASS_SHADOW_DANCER:
            case CLASS_SHOU_LIN:
                GET_MAX_HIT(ch) += 60;
                GET_MAX_MANA(ch) += 30;
                GET_MAX_MOVE(ch) += 10;
                break;
            case CLASS_MAGIC_USER:
            case CLASS_NECROMANCER:
                GET_MAX_HIT(ch) += 50;
                GET_MAX_MANA(ch) += 50;
                break;
            case CLASS_CLERIC:
                GET_MAX_HIT(ch) += 40;
                GET_MAX_MANA(ch) += 60;
                break;
        }
    }

    /* elementals get granted soak/sate */
    if (IS_ELEMENTAL(ch)) {
        GET_COND(ch, THIRST) = -1;
        GET_COND(ch, HUNGER) = -1;
    }
    /* Vampires get sated - Bean */
    if (IS_VAMPIRE(ch)) {
        GET_COND(ch, HUNGER) = -1;
    }

    mudlog( BRF, LVL_GOD, TRUE, "REMORT: %s has been reborn as a%s %s",
            GET_NAME(ch), is_vowel(race_types[GET_RACE(ch)][0]) ? "n" : "",
            race_types[GET_RACE(ch)]);

    char_from_room(ch);
    char_to_room(ch, real_room(3003));
    look_at_room(ch, 0);
}

static const char *remort_race_names[] = {
    "dem", "iza", "wer", "vam", "fae", "ele fire", "ele air", "ele water",
    "ele earth", "gia", "ama", "hum", "hlf", "elf", "dwf", "min", "ogr",
    "tro", "orc", "gnm", "drc fire", "drc poison", "drc frost",
    "drc acid", "drc lightning", "und", "drw", "shuman", "s-half",
    "s-elf", "s-drow", "s-dwarf", "s-mino", "s-ogre", "s-troll",
    "s-drac", "s-gnome", "s-orc", "terran", "zerg", "protoss", NULL
};

ACMD(do_remort)
{
    char name[MAX_INPUT_LENGTH], race[MAX_INPUT_LENGTH];
    CharData *vict;
    int i;

    if (IS_NPC(ch)) return;

    half_chop(argument, name, race);
    if (!(vict = get_player_vis(ch, name))) {
        send_to_char("There is no such player.\r\n", ch);
        return;
    }

    if (GET_LEVEL(vict) > MAX_MORTAL) {
        send_to_char("It's best not to remort immortals.\r\n", ch);
        return;
    }

    for (i = 0; remort_race_names[i]; i++)
        if (strcmp(race, remort_race_names[i]) == 0) break;

    if (!remort_race_names[i]) {
        send_to_char("Unknown remort race!\r\n", ch);
        return;
    }

    send_to_char("Remorting!\r\n", ch);

    remort(vict, i, 0);
}

/*--------------------------------------------------------------------------*/
/* Quest Loading and Unloading Functions                                    */
/*--------------------------------------------------------------------------*/
/* Can't believe I missed this but I was tryign to pass a pointer on to another
 * function for malloc. Only problem is I was trying to pass a literal string
 * Removing this guy temporarily -Xiuh 06.09.09
void destroy_quests(void)
{
  qst_rnum rnum = 0;

  if (!qst_list)
    return;

  for (rnum = 0; rnum < top_of_qstt; rnum++){
    qFreeQuest(&qst_list[rnum]);
  }
  free(qst_list);
  qst_list = NULL;
  top_of_qstt = 0;

  return;
}
*/
// check if this kill was a quest kill
void check_quest_kill(CharData *ch, CharData *vict)
{
  ObjData *token;
  int i;

  // make sure we have a killer and a victim
  if (ch == NULL || vict == NULL) return;

  // check here for a mob doing a kill for its master
  if (IS_NPC(ch) && ch->master != NULL && IN_ROOM(ch) == IN_ROOM(ch->master))
    ch = ch->master;

  // check that it's a PC killing an NPC
  if (IS_NPC(ch) || !IS_NPC(vict)) return;

  // check that player is on a quest
  if (GET_QUEST(ch) == NULL || GET_QUEST_TIME(ch) < 0) {
    // check if this player is following someone on a non solo quest
    if (ch->master == NULL) return;

    if (IN_ROOM(ch) != IN_ROOM(ch->master)) return;
    // just quietly pretend the quester did the kill
    ch = ch->master;
    if (IS_NPC(ch) || GET_QUEST(ch) == NULL || GET_QUEST_TIME(ch) < 0)
      return;
    // check for solo quests here.
  }

  // search the list of tasks for a kill-mob task with this mob's vnum
  for (i = 0; i < MAX_QST_TASKS; i++) {
    if (GET_QUEST(ch)->tasks[i].type == QST_TASK_MOB &&
        GET_QUEST(ch)->tasks[i].identifier == GET_MOB_VNUM(vict))
      break;
  }

  // huzzah, a quest kill, create the reward object
  if (i < MAX_QST_TASKS) {
    token = read_perfect_object(QUEST_KILL_TOKEN, VIRTUAL);
    if (!token) return;         // damn, eh.
    GET_OBJ_RENT(token) = GET_MOB_VNUM(vict);
    send_to_char("You take the head of your victim as proof of the kill.\r\n",
        ch);
    obj_to_char(token, ch);
  }
}

// check if this give completes a quest task
int check_quest_give(CharData *ch, CharData *to, ObjData *obj)
{
  int i, done = 1, wanted = 0, prize = -1;

  // check that it's a PC doing the give
  if (IS_NPC(ch)) return 0;

  // check that player is on a quest
  if (GET_QUEST(ch) == NULL || GET_QUEST_TIME(ch) < 0) return 0;

  // check that the recipient was the quest giver
  if (!IS_NPC(to) || to->nr != GET_QUEST_GIVER(ch)) return 0;

  // by now, the player's either given an item to the right mob, or they've
  // gotten freakily lucky with rnums.  time to check their tasks.
  for (i = 0; i < MAX_QST_TASKS; i++) {
    if ((GET_QUEST_TASKS(ch) & (1 << i)) == 0) { // find incomplete tasks
      if (wanted && GET_QUEST(ch)->tasks[i].type != QST_TASK_NONE) {
        done = 0;
        break;
      }
      switch (GET_QUEST(ch)->tasks[i].type) {
        case QST_TASK_OBJ:
          if (GET_OBJ_VNUM(obj) == GET_QUEST(ch)->tasks[i].identifier) {
            GET_QUEST_TASKS(ch) |= 1 << i;
            extract_obj(obj);
            wanted = 1;
          } else done = 0;
          break;
        case QST_TASK_MOB:              // unimped!
          if (GET_OBJ_VNUM(obj) == QUEST_KILL_TOKEN) {
            // make sure this is the right token
            if (GET_OBJ_RENT(obj) == GET_QUEST(ch)->tasks[i].identifier) {
              GET_QUEST_TASKS(ch) |= 1 << i;
              extract_obj(obj);
              wanted = 1;
            } else done = 0;
          } else done = 0;
        default:
			break;
	  }
    }
  }

#define REWARD GET_QUEST(ch)->rewards[i].amount
  // either finish up the quest or tell the player there's more to go
  if (done || (wanted == 1 && QFLAG(GET_QUEST(ch), QST_ONE_OBJECTIVE))) {
    ObjData *reward;

    /* If the quest has a random reward, select it here */
    if (QFLAG(GET_QUEST(ch), QST_RANDOM_REWARD)) {
        int r = 0;
        for (i = 0; i < MAX_QST_REWARDS; i++)
            if (GET_QUEST(ch)->rewards[i].type != QST_REWARD_NONE &&
                    number(1, ++r) == 1)
                prize = i;
    }

    for (i = 0; i < MAX_QST_REWARDS; i++) {
      if (GET_QUEST(ch)->rewards[i].type == QST_REWARD_NONE) continue;
      if (prize >= 0 && prize != i) continue;

      chore_check_quest(ch, GET_QUEST(ch)->virtual);

      reward = GET_QUEST(ch)->rewards[i].type == QST_REWARD_OBJ ?
        read_object(REWARD, VIRTUAL) : NULL;
      if (GET_QUEST(ch)->rewards[i].speech != NULL)
        act(GET_QUEST(ch)->rewards[i].speech, FALSE, ch, reward, to, TO_CHAR);
      switch (GET_QUEST(ch)->rewards[i].type) {
        case QST_REWARD_QP:
          GET_QP(ch) += REWARD;
          mudlog( BRF, LVL_IMMORT, TRUE, "%s has received %d quest points as a quest reward for quest #%d.", GET_NAME(ch), REWARD, GET_QUEST(ch)->virtual);
          break;
        case QST_REWARD_EXP:
          gain_exp(ch, REWARD);
          mudlog( BRF, LVL_IMMORT, TRUE, "%s has received %d experience as a quest reward for quest #%d.", GET_NAME(ch), REWARD, GET_QUEST(ch)->virtual);
          break;
        case QST_REWARD_OBJ:
          if (reward) {
              obj_to_char(reward, ch);
              // In general, quest rewards must be owned by the player who completes the quest. (flagged soulbound)
              // The builder may bypass this with a COMMON_REWARD flag
              if(!QFLAG(GET_QUEST(ch), QST_COMMON_REWARD))
                  SET_BIT_AR(GET_OBJ_EXTRA(obj), ITEM_SOULBOUND);
              mudlog( BRF, LVL_IMMORT, TRUE, "%s has received the item %s (vnum %d) as a quest reward for quest #%d.",
                      GET_NAME(ch), reward->short_description, GET_OBJ_VNUM(reward), GET_QUEST(ch)->virtual);
          }
          break;
        case QST_REWARD_GOLD:
          GET_GOLD(ch) += REWARD;
          mudlog( BRF, LVL_IMMORT, TRUE, "%s has received %d gold as a quest reward for quest #%d.", GET_NAME(ch), REWARD, GET_QUEST(ch)->virtual);
          break;
        case QST_REWARD_PRAC:
          GET_PRACTICES(ch) += REWARD;
          mudlog( BRF, LVL_IMMORT, TRUE, "%s has received %d PRACTICE SESSIONS as a quest reward for quest #%d.", GET_NAME(ch), REWARD, GET_QUEST(ch)->virtual);
          break;
        case QST_REWARD_SPELL:
          call_magic(to, ch, NULL, REWARD, 50, NULL, CAST_SPELL);
          break;
        case QST_REWARD_REMORT:
          remort(ch, REWARD, 1);
          break;
      }
    }
    GET_QUEST_WAIT(ch) = time(NULL) + GET_QUEST(ch)->waitcomplete * 60;
    for (i = 0; i < GET_QUEST_COUNT(ch); i++)
      if (GET_QUEST_DONE(ch, i) == GET_QUEST(ch)->virtual) break;
    if (i == GET_QUEST_COUNT(ch)) {
      RECREATE(ch->player_specials->quest.saved.quests, int, i+1);
      GET_QUEST_DONE(ch, i) = GET_QUEST(ch)->virtual;
      GET_QUEST_COUNT(ch) = i + 1;
    }
    GET_QUEST(ch) = NULL;
  } else if (wanted) {
    do_say(to, "Excellent.  You still have more tasks to do, though!", 0, 0);
  }
  return 1;
}

void update_char_quests(CharData *ch)
{
  // if they can quest again, let them know
  if (GET_QUEST_WAIT(ch) > 0 && GET_QUEST_WAIT(ch) <= time(NULL)) {
    GET_QUEST_WAIT(ch) = 0;
    send_to_char("You may now take another quest.\r\n", ch);
  }

  if (GET_QUEST(ch) == NULL) return;

  GET_QUEST_TIME(ch) -= 1;

  // players get 3 ticks to make up their minds about a quest
  if (GET_QUEST_TIME(ch) < -3) {
    send_to_char("The quest offer has expired!\r\n", ch);
    GET_QUEST(ch) = NULL;
  }

  // if the time's run out, this quest is over, man!
  if (GET_QUEST_TIME(ch) == 0) {
    send_to_char("You have run out of time to complete your quest!\r\n", ch);
    GET_QUEST(ch) = NULL;
  }
}

int can_take_quest(CharData *ch, QuestData *qst, CharData *mob)
{
  int i;

  if (GET_LEVEL(ch) < qst->minlvl || GET_LEVEL(ch) > qst->maxlvl) return 0;
  // check align
  if ((QFLAG(qst, QST_ANTI_GOOD) && IS_GOOD(ch)) ||
      (QFLAG(qst, QST_ANTI_NEUT) && IS_NEUTRAL(ch)) ||
      (QFLAG(qst, QST_ANTI_EVIL) && IS_EVIL(ch))) return 0;
  // check class
  if ((QFLAG(qst, QST_ANTI_MAGE) && GET_CLASS(ch) == CLASS_MAGIC_USER) ||
      (QFLAG(qst, QST_ANTI_CLERIC) && GET_CLASS(ch) == CLASS_CLERIC) ||
      (QFLAG(qst, QST_ANTI_THIEF) && GET_CLASS(ch) == CLASS_THIEF) ||
      (QFLAG(qst, QST_ANTI_WARRIOR) && GET_CLASS(ch) == CLASS_WARRIOR) ||
      (QFLAG(qst, QST_ANTI_RANGER) && GET_CLASS(ch) == CLASS_RANGER) ||
      (QFLAG(qst, QST_ANTI_ASSASSIN) && GET_CLASS(ch) == CLASS_ASSASSIN) ||
      (QFLAG(qst, QST_ANTI_SHOULIN) && GET_CLASS(ch) == CLASS_SHOU_LIN) ||
      (QFLAG(qst, QST_ANTI_SOLAMNIC) && GET_CLASS(ch) == CLASS_SOLAMNIC_KNIGHT) ||
      (QFLAG(qst, QST_ANTI_DEATH_KN) && GET_CLASS(ch) == CLASS_DEATH_KNIGHT) ||
      (QFLAG(qst, QST_ANTI_SHAD_DANC) && GET_CLASS(ch) == CLASS_SHADOW_DANCER) ||
      (QFLAG(qst, QST_ANTI_NECRO)     && GET_CLASS(ch) == CLASS_NECROMANCER))
    return 0;
  // check race
  if ((QFLAG(qst, QST_ANTI_MINOTAUR) && IS_MINOTAUR(ch)) ||
      (QFLAG(qst, QST_ANTI_GNOME) && IS_GNOME(ch)) ||
      (QFLAG(qst, QST_ANTI_ORC) && IS_ORC(ch)) ||
      (QFLAG(qst, QST_ANTI_ELF) && IS_ELF(ch)) ||
      (QFLAG(qst, QST_ANTI_DROW) && IS_DROW(ch)) ||
      (QFLAG(qst, QST_ANTI_DRACONIAN) && IS_DRACONIAN(ch)) ||
      (QFLAG(qst, QST_ANTI_HALFING) && IS_HALFLING(ch)) ||
      (QFLAG(qst, QST_ANTI_OGRE) && IS_OGRE(ch)) ||
      (QFLAG(qst, QST_ANTI_TROLL) && IS_TROLL(ch)) ||
      (QFLAG(qst, QST_ANTI_DWARF) && IS_DWARF(ch)) ||
      (QFLAG(qst, QST_ANTI_HUMAN) && IS_HUMAN(ch)))
    return 0;

  // check the player hasn't already done this quest
  if (QFLAG(qst, QST_ONCE_ONLY)) for (i = 0; i < GET_QUEST_COUNT(ch); i++) {
    if (GET_QUEST_DONE(ch, i) == qst->virtual) return 0;
  }

  // check that prerequisites are all satisfied
  for (i = 0; i < MAX_QST_PREREQ; i++) {
    int j, done = 1;
    if (qst->prereqs[i] != 0) {
      int did = 0;
      for (j = 0; j < GET_QUEST_COUNT(ch); j++) {
        if (GET_QUEST_DONE(ch, j) == qst->prereqs[i]) did = 1;
      }
      if (!did) done = 0;
    }
    if (!done) return 0;
  }

  // check that the supplied mob is a giver, if there's one at all
  if (mob) for (i = 0; i < MAX_QST_GIVERS; i++) {
    if (qst->givers[i] == GET_MOB_VNUM(mob)) break;
  }

  if (i == MAX_QST_GIVERS) return 0;

  return 1;
}

static void do_quest_offer(CharData *ch, char *argument)
{
  int i, q = -1, r = 0;
  char msgbuf[200];
  CharData *mob;
  time_t pause;

  if (ch->in_room == -1 || ch->in_room >= top_of_world) return;

  if(GET_POS(ch) <= POS_SLEEPING) {
      send_to_char("You have to wake up first.\r\n", ch);
      return;
  }

  // find a questgiving mob
  for (mob = world[ch->in_room].people; mob; mob = mob->next_in_room) {
    if (MOB_FLAGGED(mob, MOB_QUESTMASTER)) break;
  }

  if (!mob) {
    send_to_char("You cannot do that here!\r\n", ch);
    return;
  }

  // make sure player doesn't have a quest on offer or in progress
  if (GET_QUEST(ch) != NULL) {
    if (GET_QUEST_TIME(ch) < 0) {
      send_to_char("You already have a quest on offer.\r\n", ch);
    } else {
      send_to_char("You haven't finished your last quest.\r\n", ch);
    }
    return;
  }

  // check that the player isn't on an enforced break
  pause = GET_QUEST_WAIT(ch) - time(NULL);
  if (pause > 0) {
    sendChar(ch, "You must wait %d minute%s before taking another quest.\r\n",
        (pause / 60) + 1, pause > 59 ? "s" : "");
    return;
  }

  // pick a random quest offerable by this mob
  for (i = 0; i <= top_of_qstt; i++) {
    if (can_take_quest(ch, qst_list + i, mob) && number(1, ++r) == 1) q = i;
  }

  if (q == -1) {
    sprintf(msgbuf, "%s tells you, 'I don't have any quests for you.'\r\n",
        GET_NAME(mob));
    send_to_char(msgbuf, ch);
    return;
  }
  give_quest(ch, qst_list + q, mob);
}

void give_quest(CharData *ch, QuestData *q, CharData *mob)
{
  char *speech;
  char msgbuf[200];
  // give the player the speech about the quest
  speech = q->speech;

  while (*speech) {
    char *ptr = strchr(speech, '\n');

    if (ptr) *ptr = '\0';
    //do_say(mob, speech, 0, 0);
    sprintf(msgbuf, "&18$N tells you, '%s'&00", speech);
    act(msgbuf, FALSE, ch, 0, mob, TO_CHAR);
    *ptr = '\n'; speech = ptr + 1;
  }

  // assign the quest to the player and set up variables
  GET_QUEST(ch) = q;
  GET_QUEST_TIME(ch) = -1;
  GET_QUEST_GIVER(ch) = mob->nr;

  // if they rent, they have to wait refusal time before getting a new offer
  GET_QUEST_WAIT(ch) = time(NULL) + q->waitrefuse * 60;
}

static void do_quest_info(CharData *ch, char *argument)
{
  time_t pause;
  
  // make sure player doesn't have a quest on offer or in progress
  if (GET_QUEST(ch) == NULL) {
    send_to_char("You don't currently have a quest.\r\n", ch);

	pause = GET_QUEST_WAIT(ch) - time(NULL);
	//report how long until they can take another quest
	if (pause > 0) {
		sendChar(ch, "You must wait %d minute%s before taking another quest.\r\n",
			(pause / 60) + 1, pause > 59 ? "s" : "");
	}
		
		return;
  }

  // show them the info
  send_to_char(GET_QUEST(ch)->description, ch);

  // tell them how to take it, or how long they have to complete it
  if (GET_QUEST_TIME(ch) < 0) {
    send_to_char("Type 'quest take' to take this quest, or 'quest refuse' "
                 "to refuse it.\r\n", ch);
  } else {
    char timebuf[200];
    sprintf(timebuf, "You have %d ticks left to finish this quest.\r\n",
        GET_QUEST_TIME(ch));
    send_to_char(timebuf, ch);
  }
}

static void do_quest_take(CharData *ch, char *argument)
{
  char acceptbuf[200];
  CharData *mob;

  if(GET_POS(ch) <= POS_SLEEPING) {
      send_to_char("You have to wake up first.\r\n", ch);
      return;
  }

  // make sure the player has a quest on offer
  if (GET_QUEST(ch) == NULL) {
    send_to_char("You aren't being offered a quest.\r\n", ch);
    return;
  }

  // make sure the quest is offered, not already accepted
  if (GET_QUEST_TIME(ch) > -1) {
    send_to_char("You have already accepted a quest.\r\n", ch);
    return;
  }

  // make sure the player isn't trying to accept dead or somewhere strange
  if (ch->in_room == -1 || ch->in_room >= top_of_world) return;

  // make sure the offering mob is still in the same room
  for (mob = world[ch->in_room].people; mob; mob = mob->next_in_room) {
    if (IS_NPC(mob) && mob->nr == GET_QUEST_GIVER(ch)) break;
  }

  if (!mob) {
    send_to_char("Noone here is offering you a quest.\r\n", ch);
    return;
  }

  // set the refusal time in case they rent
  GET_QUEST_WAIT(ch) = time(NULL) + GET_QUEST(ch)->waitcomplete * 60;
  GET_QUEST_TIME(ch) = GET_QUEST(ch)->timelimit;

  // clear the completed tasks bitfield
  GET_QUEST_TASKS(ch) = 0;

  sprintf(acceptbuf, "You have %d ticks to complete your quest.\r\n",
      GET_QUEST_TIME(ch));
  if (GET_QUEST_TIME(ch) != 0) {
    send_to_char(acceptbuf, ch);
  } else {
    send_to_char("You have accepted the quest -- good luck!\r\n", ch);
  }
}

static void do_quest_refuse(CharData *ch, char *argument)
{
    if(GET_POS(ch) <= POS_SLEEPING) {
        send_to_char("You have to wake up first.\r\n", ch);
        return;
    }

    // make sure the player has a quest on offer
    if (GET_QUEST(ch) == NULL) {
        send_to_char("You aren't being offered a quest.\r\n", ch);
        return;
    }
    
    // make sure the quest is offered, not already accepted
    if (GET_QUEST_TIME(ch) > -1) {
        send_to_char("You have already accepted a quest.\r\n", ch);
        return;
    }

    // set the refusal time, and remove the quest
    GET_QUEST_WAIT(ch) = time(NULL) + GET_QUEST(ch)->waitrefuse * 60;
    GET_QUEST(ch) = NULL;
    send_to_char("You have cowardly turned down the quest.\r\n", ch);
}

static void do_quest_abort(CharData *ch, char *argument)
{
  // make sure the player has a quest
  if (GET_QUEST(ch) == NULL) {
    send_to_char("You don't have a quest right now.\r\n", ch);
    return;
  }

  // make sure the quest is underway, not just offered
  if (GET_QUEST_TIME(ch) < 0) {
    send_to_char("You haven't taken a quest yet -- try quest refuse.\r\n", ch);
    return;
  }

  // set the finish time, and remove the quest
  GET_QUEST_WAIT(ch) = time(NULL) + GET_QUEST(ch)->waitcomplete * 60;
  GET_QUEST(ch) = NULL;
  send_to_char("You have admitted defeat, and surrendered your quest.\r\n", ch);
}

ACMD(do_quest)
{
    char scmd[50];

    argument = one_argument(argument, scmd);
    if (*scmd == '\0') {
        if (CONFIG_QUEST_ACTIVE) do_gen_tog(ch, argument, cmd, SCMD_QUEST);
        else {
            REMOVE_BIT_AR(PRF_FLAGS(ch), PRF_QUEST);
            sendChar(ch, "There are no immortal run quests active.\r\n");
        }
        return;
    }
    if (is_abbrev(scmd, "offer")) do_quest_offer(ch, argument);
    else if (is_abbrev(scmd, "info")) do_quest_info(ch, argument);
    else if (is_abbrev(scmd, "take")) do_quest_take(ch, argument);
    else if (is_abbrev(scmd, "refuse")) do_quest_refuse(ch, argument);
    else if (is_abbrev(scmd, "abort")) do_quest_abort(ch, argument);
    else if (GET_LEVEL(ch) >= LVL_IMMORT && is_abbrev(scmd, "active")) {
        CONFIG_QUEST_ACTIVE = !CONFIG_QUEST_ACTIVE;
        sendChar(ch, "Questing is now %s.\r\n",
                CONFIG_QUEST_ACTIVE ? "&09ON&00" : "&08OFF&00");

        /* Remove all quest flags now! */
        if (!CONFIG_QUEST_ACTIVE) {
            DescriptorData *dsc;

            for (dsc = descriptor_list; dsc; dsc = dsc->next) {
                if (dsc->character && !IS_NPC(dsc->character) &&
                        PRF_FLAGGED(dsc->character, PRF_QUEST)) {
                    sendChar(dsc->character, "The quest is over.\r\n");
                    REMOVE_BIT_AR(PRF_FLAGS(dsc->character), PRF_QUEST);
                }
            }
        }
    } else {
        send_to_char("Quest what now?\r\n", ch);
    }
}
