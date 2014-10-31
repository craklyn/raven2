
/* ============================================================================ 
*  _TwyliteMud_ by Rv.                          Based on CircleMud3.0bpl9 *
*    				                                          *
*  OasisOLC - qedit.c 		                                          *
*    				                                          *
*  Copyright 1996 Harvey Gilpin.                                          *
============================================================================ */

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/comm.h"
#include "util/utils.h"
#include "olc/olc.h"
#include "general/class.h"
#include "general/modify.h"
#include "olc/qedit.h"
#include "actions/interpreter.h"
#include "actions/quest.h"
#include "magic/spells.h"

/** Help buffer the global variable definitions */
#define __QEDIT_C__

//-------------------------------------------------------------------------
//
void qSaveInternally(DescriptorData *d);
void qMenu( DescriptorData * d );

// safely dup a string (ie, return NULL if str is NULL)
char *safe_dup(char *str)
{
  if (str == NULL) return NULL;
  return str_dup(str);
}

//-------------------------------------------------------------------------
//
// Utility Functions 
//
void
qSetupExisting( DescriptorData* d, int quest_num )
{
  int i;

  QuestData *quest = NULL;
  QuestData *QPTR = &qst_list[real_quest(OLC_NUM(d))];

  // Alloc some quest shaped space
  CREATE( quest, QuestData, 1 );

  // Copy in quest details
  quest->name           = safe_dup(QPTR->name);
  quest->speech         = safe_dup(QPTR->speech);
  quest->description    = safe_dup(QPTR->description);
  quest->minlvl         = QPTR->minlvl;
  quest->maxlvl         = QPTR->maxlvl;
  quest->flags          = QPTR->flags;
  quest->timelimit      = QPTR->timelimit;
  quest->waitcomplete   = QPTR->waitcomplete;
  quest->waitrefuse     = QPTR->waitrefuse;

  // this field is used as a 'has been modified' flag
  quest->virtual        = 0;

  // copy the various subfields
  for (i = 0; i < MAX_QST_PREREQ; i++)
    quest->prereqs[i] = QPTR->prereqs[i];
  for (i = 0; i < MAX_QST_GIVERS; i++)
    quest->givers[i] = QPTR->givers[i];
  for (i = 0; i < MAX_QST_TASKS; i++) {
    quest->tasks[i].type = QPTR->tasks[i].type;
    quest->tasks[i].identifier = QPTR->tasks[i].identifier;
  }
  for (i = 0; i < MAX_QST_REWARDS; i++) {
    quest->rewards[i].type = QPTR->rewards[i].type;
    quest->rewards[i].amount = QPTR->rewards[i].amount;
    quest->rewards[i].speech = safe_dup(QPTR->rewards[i].speech);
  }

  d->olc->quest = quest;

  // Display main menu 
  //
  qMenu(d);
}

// Create a new quest
void
qSetupNew(DescriptorData *d)
{
  int i;

  QuestData *quest = NULL;

  // Alloc some quest shaped space
  CREATE( quest, QuestData, 1 );

  // Copy in quest details
  quest->name           = safe_dup("unnamed quest");
  quest->speech         = NULL;
  quest->description    = NULL;
  quest->minlvl         = 1;
  quest->maxlvl         = 50;
  quest->flags          = 0;
  quest->timelimit      = 15;
  quest->waitcomplete   = 30;
  quest->waitrefuse     = 15;

  // this field is used as a 'has been modified' flag
  quest->virtual        = 0;

  // copy the various subfields
  for (i = 0; i < MAX_QST_PREREQ; i++)
    quest->prereqs[i] = 0;
  for (i = 0; i < MAX_QST_GIVERS; i++)
    quest->givers[i] = 0;
  for (i = 0; i < MAX_QST_TASKS; i++) {
    quest->tasks[i].type = QST_TASK_NONE;
    quest->tasks[i].identifier = 0;
  }
  for (i = 0; i < MAX_QST_REWARDS; i++) {
    quest->rewards[i].type = QST_REWARD_NONE;
    quest->rewards[i].amount = 0;
    quest->rewards[i].speech = NULL;
  }

  d->olc->quest = quest;

  // Display main menu 
  //
  qMenu(d);
}

void
qSaveInternally( DescriptorData *d )
{
  int rqst_num = real_quest(OLC_NUM(d));
  QuestData *new_list;
  CharData *ch;
  int i, j;

  OLC_QUEST(d)->virtual = OLC_NUM(d);
  if (rqst_num >= 0) {
    if (qst_list[rqst_num].name) free(qst_list[rqst_num].name);
    if (qst_list[rqst_num].speech) free(qst_list[rqst_num].speech);
    if (qst_list[rqst_num].description) free(qst_list[rqst_num].description);
    for (i = 0; i < MAX_QST_REWARDS; i++)
      if (qst_list[rqst_num].rewards[i].speech)
        free(qst_list[rqst_num].rewards[i].speech);
    OLC_QUEST(d)->number = rqst_num;
    qst_list[rqst_num] = *OLC_QUEST(d);
  } else {
    CREATE(new_list, QuestData, top_of_qstt + 2);
    // find the index of the first quest after the new one
    for (i = 0; i <= top_of_qstt; i++)
      if (qst_list[i].virtual > OLC_NUM(d)) break;
    OLC_QUEST(d)->number = i;
    // block copy the earlier quests, if any
    if (i > 0) memmove(new_list, qst_list, sizeof(QuestData) * i);
    // set up the new quest
    new_list[i] = *OLC_QUEST(d);
    // block copy the later quests, if any
    if (i <= top_of_qstt) memmove(new_list + i + 1, qst_list + i,
        sizeof(QuestData) * (top_of_qstt - i + 1));
    // and increment their numbers
    for (j = i; j <= top_of_qstt; j++) new_list[j+1].number++;
    // fix up any currently active player quests
    for (ch = character_list; ch; ch = ch->next) {
      if (GET_QUEST(ch) != NULL) {
        if (GET_QUEST(ch)->number >= i)
          GET_QUEST(ch) = new_list + GET_QUEST(ch)->number + 1;
        else
          GET_QUEST(ch) = new_list + GET_QUEST(ch)->number;
      }
    }
    // swap the new list into place
    free(qst_list);
    qst_list = new_list;
    top_of_qstt++;
  }

  olc_add_to_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_QUEST);
}

static char *tasks[] = {"NONE", "OBJ", "MOB"};
static char *rewards[] =
    {"NONE", "QP", "EXP", "OBJ", "GOLD", "PRAC", "SPELL", "REMORT"};

void
qSaveToDisk( DescriptorData* d )
{
  int counter, i;
  FILE *fp;

  sprintf(buf, "%s/%d.qst", QST_PREFIX, zone_table[OLC_ZNUM(d)].number);
  if (!(fp = fopen(buf, "w+"))) {
    mudlog(BRF, LVL_BUILDER, TRUE, "OLCERR: OLC: Cannot open quests file!");
    return;
  }

  for (counter = zone_table[OLC_ZNUM(d)].number * 100;
       counter <= zone_table[OLC_ZNUM(d)].top;
       counter++) {
    int real = real_quest(counter);

    if (real >= 0) {
      QuestData *qst = qst_list + real;
      char *speech = str_dup(qst->speech ? qst->speech : "");
      char *info = str_dup(qst->description ? qst->description : "");
      strip_string(speech);
      strip_string(info);
      fprintf(fp,
          "#%d\n"               // vnum
          "%s~\n"               // name
          "%s~\n"               // speech
          "%s~\n"               // info
          "%d %d %d "           // minlvl maxlvl flags
          "%d %d %d\n",         // timelimit waitcomplete waitrefuse
          qst->virtual,
          qst->name ? qst->name : "",
          speech, info,
          qst->minlvl, qst->maxlvl, qst->flags,
          qst->timelimit, qst->waitcomplete, qst->waitrefuse
      );
      free(speech);     // a noble cause!
      free(info);
      for (i = 0; i < MAX_QST_GIVERS; i++)
        if (qst->givers[i] > 0)
          fprintf(fp, "G %d\n", qst->givers[i]);
      for (i = 0; i < MAX_QST_PREREQ; i++)
        if (qst->prereqs[i] > 0)
          fprintf(fp, "P %d\n", qst->prereqs[i]);
      for (i = 0; i < MAX_QST_TASKS; i++)
        if (qst->tasks[i].type != QST_TASK_NONE) {
          fprintf(fp, "O %s %d\n", tasks[qst->tasks[i].type],
              qst->tasks[i].identifier);
        }
      for (i = 0; i < MAX_QST_REWARDS; i++)
        if (qst->rewards[i].type != QST_REWARD_NONE) {
          speech = str_dup(qst->rewards[i].speech ? qst->rewards[i].speech:"");
          strip_string(speech);
          fprintf(fp, "R %s %d\n%s~\n",
              rewards[qst->rewards[i].type], qst->rewards[i].amount, speech);
          free(speech);
        }
    }
  }

  fprintf(fp, "$~\n");
  fclose(fp);

  olc_remove_from_save_list(zone_table[OLC_ZNUM(d)].number, OLC_SAVE_QUEST);
}

void
qMenu( DescriptorData * d )
{
  QuestData *qst = OLC_QUEST(d);
  char *pbuf;
  int i;

  sprintbit(qst->flags, questflag_bits, buf1);
  sprintf(buf, "[H[J"
      "-- Quest number:  [%d]\r\n"
      "&021)&06 Name   : &00%s\r\n"
      "&022)&06 Speech : &00-\r\n%s"
      "&023)&06 Info   : &00-\r\n%s"
      "&024)&06 Minlvl : &03%-2d   &025)&06 Maxlvl : &03%-2d   "
      "&026)&06 Time limit   : &03%d &06ticks\r\n"
      "&027)&06 Accept pause  : &03%2d &06minutes     "
      "&028)&06 Refuse pause : &03%2d &06minutes\r\n"
      "&029)&06 Flags         : &03%s\r\n",
      OLC_NUM(d),
      qst->name ? qst->name : "<not set>",
      qst->speech ? qst->speech : "<not set>\r\n",
      qst->description ? qst->description : "<not set>\r\n",
      qst->minlvl, qst->maxlvl, qst->timelimit,
      qst->waitcomplete, qst->waitrefuse, buf1);
  send_to_char(buf, d->character);

  *buf1 = '\0';
  pbuf = buf1;
  for (i = 0; i < MAX_QST_PREREQ; i++) {
    if (qst->prereqs[i] > 0)
      pbuf += sprintf(pbuf, "#%d ", qst->prereqs[i]);
  }
  if (*buf1 == '\0') strcpy(buf1, "<none>");
  sprintf(buf, "&02A)&06 Prerequisites : &03%s\r\n", buf1);
  send_to_char(buf, d->character);

  *buf1 = '\0';
  pbuf = buf1;
  for (i = 0; i < MAX_QST_GIVERS; i++) {
    if (qst->givers[i] > 0)
      pbuf += sprintf(pbuf, "#%d ", qst->givers[i]);
  }
  if (*buf1 == '\0') strcpy(buf1, "<none>");
  sprintf(buf, "&02B)&06 Giver mobs    : &03%s\r\n", buf1);
  send_to_char(buf, d->character);

  sprintf(buf, "&02C)&06 Task Menu\r\n"
      "&02D)&06 Reward Menu\r\n"
      "&02Q)&06 Quit&00\r\n"
      "Enter choice : ");
  send_to_char(buf, d->character);

  OLC_MODE(d) = QEDIT_MAIN_MENU;
}

void
qFlagMenu(DescriptorData *d)
{
  bitTable(d, questflag_bits, NUM_QST_FLAGS, "quest", OLC_QUEST(d)->flags);
}

void
qPrereqMenu(DescriptorData *d)
{
  char *pbuf;
  int i;

  *buf1 = '\0';
  pbuf = buf1;
  for (i = 0; i < MAX_QST_PREREQ; i++) {
    if (OLC_QUEST(d)->prereqs[i] > 0)
      pbuf += sprintf(pbuf, "#%d ", OLC_QUEST(d)->prereqs[i]);
  }
  if (*buf1 == '\0') strcpy(buf1, "<none>");
  sprintf(buf, "&06Prerequisites : &03%s&00\r\n", buf1);
  send_to_char(buf, d->character);
  send_to_char("&02Add&00, &02remove&00, &02list&00 or &02help&00 ? ",
      d->character);
}

void
qAddPrereq(DescriptorData *d, int vnum)
{
  int i;

  for (i = 0; i < MAX_QST_PREREQ; i++) {
    if (OLC_QUEST(d)->prereqs[i] == 0) {
      OLC_QUEST(d)->prereqs[i] = vnum;
      break;
    }
  }

  if (i == MAX_QST_PREREQ) {
    send_to_char("Prerequisite limit reached!\r\n", d->character);
  }
}

void
qRemPrereq(DescriptorData *d, int vnum)
{
  int i;

  for (i = 0; i < MAX_QST_PREREQ; i++) {
    if (OLC_QUEST(d)->prereqs[i] == vnum) {
      OLC_QUEST(d)->prereqs[i] = 0;
      break;
    }
  }

  if (i == MAX_QST_PREREQ) {
    send_to_char("No such prerequisite.\r\n", d->character);
  }
}

void
qListQuests(DescriptorData *d)
{
  int i;

  send_to_char("Quests defined: \r\n", d->character);
  for (i = 0; i <= top_of_qstt; i++) {
    if (qst_list[i].virtual < zone_table[OLC_ZNUM(d)].number * 100) continue;
    if (qst_list[i].virtual > zone_table[OLC_ZNUM(d)].top) break;
    sprintf(buf, "#%-5d   %s\r\n", qst_list[i].virtual, qst_list[i].name);
    send_to_char(buf, d->character);
  }
  send_to_char("\r\n", d->character);
}

void
qGiverMenu(DescriptorData *d)
{
  char *pbuf;
  int i;

  *buf1 = '\0';
  pbuf = buf1;
  for (i = 0; i < MAX_QST_GIVERS; i++) {
    if (OLC_QUEST(d)->givers[i] > 0)
      pbuf += sprintf(pbuf, "#%d ", OLC_QUEST(d)->givers[i]);
  }
  if (*buf1 == '\0') strcpy(buf1, "<none>");
  sprintf(buf, "&06Givers : &03%s&00\r\n", buf1);
  send_to_char(buf, d->character);
  send_to_char("&02Add&00, &02remove&00, &02list&00 or &02help&00 ? ",
      d->character);
}

void
qAddGiver(DescriptorData *d, int vnum)
{
  int i;

  for (i = 0; i < MAX_QST_GIVERS; i++) {
    if (OLC_QUEST(d)->givers[i] == 0) {
      OLC_QUEST(d)->givers[i] = vnum;
      break;
    }
  }

  if (i == MAX_QST_GIVERS) {
    send_to_char("Giver limit reached!\r\n", d->character);
  }
}

void
qRemGiver(DescriptorData *d, int vnum)
{
  int i;

  for (i = 0; i < MAX_QST_GIVERS; i++) {
    if (OLC_QUEST(d)->givers[i] == vnum) {
      OLC_QUEST(d)->givers[i] = 0;
      break;
    }
  }

  if (i == MAX_QST_GIVERS) {
    send_to_char("No such mobile in the list.\r\n", d->character);
  }
}

void
qListGivers(DescriptorData *d)
{
  int i;

  send_to_char("Questmaster mobiles in zone: \r\n", d->character);
  for (i = 0; i <= top_of_mobt; i++) {
    if (mob_index[i].virtual < zone_table[OLC_ZNUM(d)].number * 100) continue;
    if (mob_index[i].virtual > zone_table[OLC_ZNUM(d)].top) break;
    if (!MOB_FLAGGED(&mob_proto[i], MOB_QUESTMASTER)) continue;
    sprintf(buf, "#%-5d   %s\r\n", mob_index[i].virtual,
        GET_NAME(&mob_proto[i]));
    send_to_char(buf, d->character);
  }
  send_to_char("\r\n", d->character);
}

void
qTaskMenu(DescriptorData *d)
{
  int i, j = 1, rnum;

  sprintf(buf, "Tasks for quest #%d\r\n", OLC_NUM(d));
  send_to_char(buf, d->character);
  for (i = 0; i < MAX_QST_TASKS; i++) {
    switch (OLC_QUEST(d)->tasks[i].type) {
      case QST_TASK_NONE:
        break;
      case QST_TASK_MOB:
        rnum = real_mobile(OLC_QUEST(d)->tasks[i].identifier);
        sprintf(buf, "&06 %2d  - Kill &03%s &06[&03%d&06]\r\n",
          j++, (rnum != -1) ? GET_NAME(&mob_proto[rnum]) : "<invalid>",
          OLC_QUEST(d)->tasks[i].identifier);
        send_to_char(buf, d->character);
        break;
      case QST_TASK_OBJ:
        rnum = real_object(OLC_QUEST(d)->tasks[i].identifier);
        sprintf(buf, "&06 %2d  - Find &03%s &06[&03%d&06]\r\n",
          j++, (rnum != -1) ? obj_proto[rnum].short_description : "<invalid>",
          OLC_QUEST(d)->tasks[i].identifier);
        send_to_char(buf, d->character);
        break;
      default:
        mudlog(BRF, LVL_BUILDER, FALSE, "OLCERR: OLC: Invalid quest task type!");
        OLC_QUEST(d)->tasks[i].type = QST_TASK_NONE;
        break;
    }
  }
  if (j == 1) send_to_char("(none)\r\n", d->character);
  send_to_char("&02N)&06ew  &02D)&06elete  &02E)&06dit  &02Q)&06uit&00\r\n"
               "Make your choice : ", d->character);
}

int
qCanAddTask(DescriptorData *d)
{
  int i;

  for (i = 0; i < MAX_QST_TASKS; i++)
    if (OLC_QUEST(d)->tasks[i].type == QST_TASK_NONE) break;

  return (i != MAX_QST_TASKS);
}

int
qCanRemTask(DescriptorData *d)
{
  int i;

  for (i = 0; i < MAX_QST_TASKS; i++)
    if (OLC_QUEST(d)->tasks[i].type != QST_TASK_NONE) break;

  return (i != MAX_QST_TASKS);
}

int
qCompressTasks(DescriptorData *d)
{
  int i, j;

  // find holes, shuffle tasks into them
  for (i = 0; i < MAX_QST_TASKS - 1; i++) {
    if (OLC_QUEST(d)->tasks[i].type != QST_TASK_NONE) continue;
    for (j = i+1; j < MAX_QST_TASKS; j++) {
      if (OLC_QUEST(d)->tasks[j].type != QST_TASK_NONE) {
        OLC_QUEST(d)->tasks[i] = OLC_QUEST(d)->tasks[j];
        OLC_QUEST(d)->tasks[j].type = QST_TASK_NONE;
        break;
      }
    }
    if (j == MAX_QST_TASKS) break; // no more tasks to shuffle up
  }
  return i;
}

void
qTaskTypeMenu(DescriptorData *d)
{
  send_to_char("&02K)&06ill a mob    &02F)&06ind an object\r\n"
               "Which task type shall this be? ", d->character);
  OLC_MODE(d) = QEDIT_TASK_TYPE;
}

void
qAddTask(DescriptorData *d, int task)
{
  int i;

  task -= 1;
  if (task < 0) task = 0;
  i = qCompressTasks(d);
  if (task > i) task = i;

  // move each task after the new insert one along a point if necessary
  if (task < i)
    memmove(OLC_QUEST(d)->tasks + task + 1, OLC_QUEST(d)->tasks + task,
        sizeof(QuestTaskData) * (i - task));
  OLC_QUEST(d)->virtual = task;
  qTaskTypeMenu(d);
}

void
qEditTask(DescriptorData *d, int task)
{
  int i, j = 0;
  for (i = 0; i < MAX_QST_TASKS; i++) {
    if (OLC_QUEST(d)->tasks[i].type != QST_TASK_NONE) j++;
    if (j == task) {
      OLC_QUEST(d)->virtual = i;
      qTaskTypeMenu(d);
      return;
    }
  }
  send_to_char("Invalid task number.\r\n", d->character);
  OLC_MODE(d) = QEDIT_TASK_MENU;
  qTaskMenu(d);
}

void
qRemTask(DescriptorData *d, int task)
{
  int i, j = 0;

  for (i = 0; i < MAX_QST_TASKS; i++) {
    if (OLC_QUEST(d)->tasks[i].type != QST_TASK_NONE) j++;
    if (j == task) {
      OLC_QUEST(d)->tasks[i].type = QST_TASK_NONE;
      return;
    }
  }
}

char *qst_remort_races[QST_NUM_REMORTS] = {
    "demon", "izarti", "werewolf", "vampire", "faerie", "fire elemental",
    "air elemental", "water elemental", "earth elemental", "giant", "amara",
    "human", "halfing", "elf", "dwarf", "minotaur", "ogre", "troll", "orc",
    "gnome", "red draconian", "green draconian", "white draconian", 
    "black draconian", "blue draconian", "undead", "drow", "shuman",
    "shalfling", "self", "sdrow", "sdwarf", "sminotaur", "sogre",
    "stroll", "sdraconian", "sgnome", "sorc"
};

void
qRewardMenu(DescriptorData *d)
{
  int i, j = 1, rnum;

  sprintf(buf, "Rewards for quest #%d\r\n", OLC_NUM(d));
  send_to_char(buf, d->character);
  for (i = 0; i < MAX_QST_REWARDS; i++) {
    switch (OLC_QUEST(d)->rewards[i].type) {
      case QST_REWARD_NONE:
        continue;
      case QST_REWARD_QP:
        sprintf(buf, "&06 %2d  - Give &03%d&06 quest points\r\n",
          j++, OLC_QUEST(d)->rewards[i].amount);
        send_to_char(buf, d->character);
        break;
      case QST_REWARD_EXP:
        sprintf(buf, "&06 %2d  - Give &03%d&06 experience points\r\n",
          j++, OLC_QUEST(d)->rewards[i].amount);
        send_to_char(buf, d->character);
        break;
      case QST_REWARD_OBJ:
        rnum = real_object(OLC_QUEST(d)->rewards[i].amount);
        sprintf(buf, "&06 %2d  - Give object &03%s &06[&03%d&06]\r\n", j++,
            (rnum == -1) ? "<invalid>" : obj_proto[rnum].short_description,
            OLC_QUEST(d)->rewards[i].amount);
        send_to_char(buf, d->character);
        break;
      case QST_REWARD_GOLD:
        sprintf(buf, "&06 %2d  - Give &03%d&06 gold\r\n",
          j++, OLC_QUEST(d)->rewards[i].amount);
        send_to_char(buf, d->character);
        break;
      case QST_REWARD_PRAC:
        sprintf(buf, "&06 %2d  - Give &03%d&06 practices\r\n",
          j++, OLC_QUEST(d)->rewards[i].amount);
        send_to_char(buf, d->character);
        break;
      case QST_REWARD_SPELL:
        rnum = OLC_QUEST(d)->rewards[i].amount;
        sprintf(buf1, "<invalid: %d>", rnum);
        sprintf(buf, "&06 %2d  - Cast &03%s&06\r\n", j++,
            (rnum > 0 && rnum < NUM_SPELLS) ? spells[rnum] : buf1);
        send_to_char(buf, d->character);
        break;
      case QST_REWARD_REMORT:
        rnum = OLC_QUEST(d)->rewards[i].amount;
        sprintf(buf, "&06 %2d  - Remort to race &03%s&06\r\n", j++,
                qst_remort_races[rnum]);
        send_to_char(buf, d->character);
        break;
      default:
        mudlog(BRF, LVL_BUILDER, FALSE, "OLCERR: OLC: Invalid quest reward type!");
        OLC_QUEST(d)->rewards[i].type = QST_REWARD_NONE;
        if (OLC_QUEST(d)->rewards[i].speech)
          free(OLC_QUEST(d)->rewards[i].speech);
        continue;
    }
    sprintf(buf, "&06With message:\r\n&00%s&00",
        OLC_QUEST(d)->rewards[i].speech ? OLC_QUEST(d)->rewards[i].speech :
        "<none>\r\n");
    send_to_char(buf, d->character);
  }
  if (j == 1) send_to_char("(none)\r\n", d->character);
  send_to_char("&02N)&06ew  &02D)&06elete  &02E)&06dit  &02Q)&06uit&00\r\n"
               "Make your choice : ", d->character);
  OLC_MODE(d) = QEDIT_REWARD_MENU;
}

void
qRaceMenu(DescriptorData *d)
{
    int i, j = 0;

    for (i = 0; i < QST_NUM_REMORTS; i++) {
        sprintf(buf,"&02%2d)&06 %-20s", i, qst_remort_races[i]);
        send_to_char(buf, d->character);
        if ((j = 1 - j) == 0) send_to_char("\r\n", d->character);
    }
    if (j == 1) send_to_char("\r\n", d->character);
    send_to_char("Make your choice : ", d->character);
}

int
qCanAddReward(DescriptorData *d)
{
  int i;

  for (i = 0; i < MAX_QST_REWARDS; i++)
    if (OLC_QUEST(d)->rewards[i].type == QST_REWARD_NONE) break;

  return (i != MAX_QST_REWARDS);
}

int
qCanRemReward(DescriptorData *d)
{
  int i;

  for (i = 0; i < MAX_QST_REWARDS; i++)
    if (OLC_QUEST(d)->rewards[i].type != QST_REWARD_NONE) break;

  return (i != MAX_QST_REWARDS);
}

int
qCompressRewards(DescriptorData *d)
{
  int i, j;

  // find holes, shuffle rewards into them
  for (i = 0; i < MAX_QST_REWARDS - 1; i++) {
    if (OLC_QUEST(d)->rewards[i].type != QST_REWARD_NONE) continue;
    for (j = i+1; j < MAX_QST_REWARDS; j++) {
      if (OLC_QUEST(d)->rewards[j].type != QST_REWARD_NONE) {
        OLC_QUEST(d)->rewards[i] = OLC_QUEST(d)->rewards[j];
        OLC_QUEST(d)->rewards[j].type = QST_REWARD_NONE;
        OLC_QUEST(d)->rewards[j].speech = NULL;
        break;
      }
    }
    if (j == MAX_QST_REWARDS) break; // no more rewards to shuffle up
  }
  return i;
}

void
qRewardTypeMenu(DescriptorData *d)
{
  send_to_char("&02S)&06pell      &02Q)&06uest points  &02G)&06old\r\n"
               "&02P)&06ractices  &02E)&06xperience    &02O)&06bject\r\n",
               d->character);
  if (GET_LEVEL(d->character) >= LVL_GOD)
               send_to_char("&02R)&06emort\r\n", d->character);
  send_to_char("Which reward type shall this be? ", d->character);
  OLC_MODE(d) = QEDIT_REWARD_TYPE;
}

void
qAddReward(DescriptorData *d, int reward)
{
  int i;

  reward -= 1;
  if (reward < 0) reward = 0;
  i = qCompressRewards(d);
  if (reward > i) reward = i;

  // move each reward after the new insert one along a point if necessary
  if (reward < i)
    memmove(OLC_QUEST(d)->rewards + reward + 1, OLC_QUEST(d)->rewards + reward,
        sizeof(QuestRewardData) * (i - reward));
  OLC_QUEST(d)->virtual = reward;
  OLC_QUEST(d)->rewards[reward].speech = NULL;
  qRewardTypeMenu(d);
}

void
qEditReward(DescriptorData *d, int reward)
{
  int i, j = 0;
  for (i = 0; i < MAX_QST_REWARDS; i++) {
    if (OLC_QUEST(d)->rewards[i].type != QST_REWARD_NONE) j++;
    if (j == reward) {
      OLC_QUEST(d)->virtual = i;
      qRewardTypeMenu(d);
      return;
    }
  }
  send_to_char("Invalid reward number.\r\n", d->character);
  OLC_MODE(d) = QEDIT_REWARD_MENU;
  qRewardMenu(d);
}

void
qRemReward(DescriptorData *d, int reward)
{
  int i, j = 0;

  for (i = 0; i < MAX_QST_REWARDS; i++) {
    if (OLC_QUEST(d)->rewards[i].type != QST_REWARD_NONE) j++;
    if (j == reward) {
      OLC_QUEST(d)->rewards[i].type = QST_REWARD_NONE;
      return;
    }
  }
}

void
qLimitReward(DescriptorData *d, int reward)
{
#define AMOUNT OLC_QUEST(d)->rewards[reward].amount
  switch (OLC_QUEST(d)->rewards[reward].type) {
    case QST_REWARD_NONE:
      break;
    case QST_REWARD_QP:
      if (AMOUNT < 1) AMOUNT = 1;
      if (AMOUNT > 10) AMOUNT = 10;
      break;
    case QST_REWARD_EXP:
      if (AMOUNT < 1) AMOUNT = 1;
      if (AMOUNT > 1000000) AMOUNT = 1000000;
      break;
    case QST_REWARD_GOLD:
      if (AMOUNT < 1) AMOUNT = 1;
      if (AMOUNT > 1000000) AMOUNT = 1000000;
      break;
    case QST_REWARD_PRAC:
      if (AMOUNT < 1) AMOUNT = 1;
      if (AMOUNT > 3) AMOUNT = 3;
      break;
    case QST_REWARD_REMORT:
      if (AMOUNT < 0) AMOUNT = 0;
      if (AMOUNT > QST_REMORT_AMARA) AMOUNT = QST_REMORT_AMARA;
      break;
  }
#undef AMOUNT
}

void
qParse( DescriptorData *d, char *arg )
{
  int number;
  char cmd[50];

  char chIn = toupper(*arg);
  switch( d->olc->mode )
  {
/*-------------------------------------------------------------------*/
  case QEDIT_MAIN_MENU:
    d->olc->tableDisp = TRUE;
    switch( chIn )
    {
      case '1':
        send_to_char("Enter name : ", d->character);
        OLC_MODE(d) = QEDIT_NAME;
        break;
      case '2':
        send_to_char("Enter quest speech: (/s saves /h for help)\r\n\r\n",
            d->character);
        SEND_RULER(d);
        d->backstr = NULL;
        if (OLC_QUEST(d)->speech) {
          send_to_char(OLC_QUEST(d)->speech, d->character);
          d->backstr = str_dup(OLC_QUEST(d)->speech);
        }
        d->str = &OLC_QUEST(d)->speech;
        d->max_str = 3000;
        d->mail_to = 0;
        OLC_QUEST(d)->virtual = 1;
        OLC_MODE(d) = QEDIT_SPEECH;
        break;
      case '3':
        send_to_char("Enter quest info: (/s saves /h for help)\r\n\r\n",
            d->character);
        SEND_RULER(d);
        d->backstr = NULL;
        if (OLC_QUEST(d)->description) {
          send_to_char(OLC_QUEST(d)->description, d->character);
          d->backstr = str_dup(OLC_QUEST(d)->description);
        }
        d->str = &OLC_QUEST(d)->description;
        d->max_str = 3000;
        d->mail_to = 0;
        OLC_QUEST(d)->virtual = 1;
        OLC_MODE(d) = QEDIT_INFO;
        break;
      case '4':
        send_to_char("Enter minimum level of questers : ", d->character);
        OLC_MODE(d) = QEDIT_MINLVL;
        break;
      case '5':
        send_to_char("Enter maximum level of questers : ", d->character);
        OLC_MODE(d) = QEDIT_MAXLVL;
        break;
      case '6':
        send_to_char("Enter time limit in ticks : ", d->character);
        OLC_MODE(d) = QEDIT_TIMELIMIT;
        break;
      case '7':
        send_to_char("Enter pause after quest completion in minutes : ",
            d->character);
        OLC_MODE(d) = QEDIT_ACCEPT_WAIT;
        break;
      case '8':
        send_to_char("Enter pause after quest refusal in minutes : ",
            d->character);
        OLC_MODE(d) = QEDIT_REFUSE_WAIT;
        break;
      case '9': qFlagMenu(d); OLC_MODE(d) = QEDIT_FLAGS; break;
      case 'A': qPrereqMenu(d); OLC_MODE(d) = QEDIT_PREREQUISITES; break;
      case 'B': qGiverMenu(d); OLC_MODE(d) = QEDIT_GIVERS; break;
      case 'C': qTaskMenu(d); OLC_MODE(d) = QEDIT_TASK_MENU; break;
      case 'D': qRewardMenu(d); OLC_MODE(d) = QEDIT_REWARD_MENU; break;
      case 'Q':
        if (OLC_QUEST(d)->virtual) {
          send_to_char("Do you wish to save this quest internally? : ",
              d->character);
          OLC_MODE(d) = QEDIT_CONFIRM_SAVESTRING;
        } else {
          sendChar(d->character, "No changes to save.\r\n");
          cleanup_olc(d, CLEANUP_ALL);
        }
        return;
      default:
        sendChar(d->character, "Invalid choice!\r\n");
        break;
    }
    d->olc->tableDisp = FALSE;
    return; /* end of QEDIT_MAIN_MENU */
/*-------------------------------------------------------------------*/
  case QEDIT_NAME:
    if (OLC_QUEST(d)->name) free(OLC_QUEST(d)->name);
    OLC_QUEST(d)->name = str_dup(arg);
    break;
  case QEDIT_MINLVL:
    OLC_QUEST(d)->minlvl = atoi(arg);
    if (OLC_QUEST(d)->minlvl < 1) OLC_QUEST(d)->minlvl = 1;
    if (OLC_QUEST(d)->minlvl > MAX_IMMORTAL)
      OLC_QUEST(d)->minlvl = MAX_IMMORTAL;
    break;
  case QEDIT_MAXLVL:
    OLC_QUEST(d)->maxlvl = atoi(arg);
    if (OLC_QUEST(d)->maxlvl < 1) OLC_QUEST(d)->maxlvl = 1;
    if (OLC_QUEST(d)->maxlvl > MAX_IMMORTAL)
      OLC_QUEST(d)->maxlvl = MAX_IMMORTAL;
    break;
  case QEDIT_TIMELIMIT:
    OLC_QUEST(d)->timelimit = atoi(arg);
    if (OLC_QUEST(d)->timelimit < 2) OLC_QUEST(d)->timelimit = 2;
    if (OLC_QUEST(d)->timelimit > 200) OLC_QUEST(d)->timelimit = 200;
    break;
  case QEDIT_ACCEPT_WAIT:
    OLC_QUEST(d)->waitcomplete = atoi(arg);
    if (OLC_QUEST(d)->waitcomplete < 1) OLC_QUEST(d)->waitcomplete = 1;
    if (OLC_QUEST(d)->waitcomplete > 60) OLC_QUEST(d)->waitcomplete = 60;
    break;
  case QEDIT_REFUSE_WAIT:
    OLC_QUEST(d)->waitrefuse = atoi(arg);
    if (OLC_QUEST(d)->waitrefuse < 1) OLC_QUEST(d)->waitrefuse = 1;
    if (OLC_QUEST(d)->waitrefuse > 60) OLC_QUEST(d)->waitrefuse = 60;
    break;
  case QEDIT_FLAGS:
    if (*arg == '\0') break;
    number = toggleBit(d, arg, questflag_bits, NUM_QST_FLAGS, "quest flag",
        &OLC_QUEST(d)->flags);
    qFlagMenu(d);
    return;
/*-------------------------------------------------------------------*/
  case QEDIT_PREREQUISITES:
    arg = one_argument(arg, cmd);
    if (is_abbrev(cmd, "add")) {
      int vnum = atoi(arg);
      if (real_quest(vnum) == -1) {
        send_to_char("There's no quest of that number.\r\n", d->character);
      } else {
        qAddPrereq(d, vnum);
      }
      qPrereqMenu(d); return;
    } else if (is_abbrev(cmd, "remove")) {
      int vnum = atoi(arg);
      qRemPrereq(d, vnum);
      qPrereqMenu(d); return;
    } else if (is_abbrev(cmd, "list")) {
      qListQuests(d);
      qPrereqMenu(d); return;
    } else if (is_abbrev(cmd, "help")) {
      send_to_char("&06Add <vnum>&00 to add a quest to the list\r\n",
          d->character);
      send_to_char("&06Remove <vnum>&00 to remove a quest from the list\r\n",
          d->character);
      send_to_char("&06List&00 to list quests in this zone\r\n",
          d->character);
      send_to_char("&06Help&00 to see this help\r\n",
          d->character);
      qPrereqMenu(d); return;
    }
    break;
/*-------------------------------------------------------------------*/
  case QEDIT_GIVERS:
    arg = one_argument(arg, cmd);
    if (is_abbrev(cmd, "add")) {
      int vnum = atoi(arg);
      if (real_mobile(vnum) == -1) {
        send_to_char("There's no mobile of that number.\r\n", d->character);
      } else {
        qAddGiver(d, vnum);
      }
      qGiverMenu(d); return;
    } else if (is_abbrev(cmd, "remove")) {
      int vnum = atoi(arg);
      qRemGiver(d, vnum);
      qGiverMenu(d); return;
    } else if (is_abbrev(cmd, "list")) {
      qListGivers(d);
      qGiverMenu(d); return;
    } else if (is_abbrev(cmd, "help")) {
      send_to_char("&06Add <vnum>&00 to add a mobile to the list\r\n",
          d->character);
      send_to_char("&06Remove <vnum>&00 to remove a mobile from the list\r\n",
          d->character);
      send_to_char("&06List&00 to list mobiles in this zone\r\n",
          d->character);
      send_to_char("&06Help&00 to see this help\r\n",
          d->character);
      qGiverMenu(d); return;
    }
    break;
/*-------------------------------------------------------------------*/
  case QEDIT_CONFIRM_SAVESTRING:
    switch( chIn )
    {
    case 'Y':
      /*
      ** Save the quest in memory
      */
      sendChar(d->character, "Saving quest info in memory.\r\n" );
      qSaveInternally(d);
      mudlog( CMP, LVL_BUILDER, TRUE, "OLC: %s edits quest %d", GET_NAME(d->character),
          OLC_NUM(d));
      cleanup_olc(d, CLEANUP_STRUCTS);
      break;
    case 'N':
      sendChar(d->character, "Throwing away changes.\r\n");
      cleanup_olc(d, CLEANUP_ALL);
      break;
    default:
      sendChar( d->character, "Invalid choice!\r\n" );
      sendChar( d->character, "Do you wish to save the quest info? " );
      break;
    }
    return; /* end of QEDIT_CONFIRM_SAVESTRING */
/*-------------------------------------------------------------------*/
  case QEDIT_TASK_MENU:
    switch (chIn) {
      case 'Q':
        qMenu(d); break;
      case 'N':
        if (qCanAddTask(d)) {
          send_to_char("Which number should this task be? ", d->character);
          OLC_MODE(d) = QEDIT_TASK_NEW;
        } else {
          send_to_char("Maximum number of tasks reached.\r\n", d->character);
          send_to_char("Make your choice : ", d->character);
        }
        break;
      case 'D':
        if (qCanRemTask(d)) {
          send_to_char("Remove which task? ", d->character);
          OLC_MODE(d) = QEDIT_TASK_DELETE;
        } else {
          send_to_char("No tasks to delete.\r\n", d->character);
          send_to_char("Make your choice : ", d->character);
        }
        break;
      case 'E':
        if (qCanRemTask(d)) { // check if a task exists
          send_to_char("Edit which task? ", d->character);
          OLC_MODE(d) = QEDIT_TASK_CHANGE;
        } else {
          send_to_char("No tasks to edit.\r\n", d->character);
          send_to_char("Make your choice : ", d->character);
        }
        break;
    }
    return; /* end of QEDIT_TASK_MENU */
/*-------------------------------------------------------------------*/
  case QEDIT_TASK_NEW:
    if (!isdigit(*arg)) {
      OLC_MODE(d) = QEDIT_TASK_MENU;
      qTaskMenu(d);
      return;
    }
    qAddTask(d, atoi(arg));
    return;
  case QEDIT_TASK_CHANGE:
    if (!isdigit(*arg)) {
      OLC_MODE(d) = QEDIT_TASK_MENU;
      qTaskMenu(d);
      return;
    }
    qEditTask(d, atoi(arg));
    return;
  case QEDIT_TASK_DELETE:
    if (isdigit(*arg)) qRemTask(d, atoi(arg));
    OLC_QUEST(d)->virtual = 1;
    OLC_MODE(d) = QEDIT_TASK_MENU;
    qTaskMenu(d);
    return;
  case QEDIT_TASK_TYPE:
    switch (chIn) {
      case 'K':
        OLC_QUEST(d)->tasks[OLC_QUEST(d)->virtual].type = QST_TASK_MOB;
        send_to_char("Enter vnum of mob to kill : ", d->character);
        break;
      case 'F':
        OLC_QUEST(d)->tasks[OLC_QUEST(d)->virtual].type = QST_TASK_OBJ;
        send_to_char("Enter vnum of obj to find : ", d->character);
        break;
      default:
        send_to_char("Invalid choice!\r\nWhat task type should this be : ",
            d->character);
        return;
    }
    OLC_MODE(d) = QEDIT_TASK_ARG;
    return;
  case QEDIT_TASK_ARG:
    if (isdigit(*arg)) {
      OLC_QUEST(d)->tasks[OLC_QUEST(d)->virtual].identifier = atoi(arg);
      OLC_QUEST(d)->virtual = 1;
      OLC_MODE(d) = QEDIT_TASK_MENU;
      qTaskMenu(d);
    } else {
      send_to_char("Invalid vnum, try again : ", d->character);
    }
    return;
/*-------------------------------------------------------------------*/
  case QEDIT_REWARD_MENU:
    switch (chIn) {
      case 'Q':
        qMenu(d); break;
      case 'N':
        if (qCanAddReward(d)) {
          send_to_char("Which number should this reward be? ", d->character);
          OLC_MODE(d) = QEDIT_REWARD_NEW;
        } else {
          send_to_char("Maximum number of rewards reached.\r\n", d->character);
          send_to_char("Make your choice : ", d->character);
        }
        break;
      case 'D':
        if (qCanRemReward(d)) {
          send_to_char("Remove which reward? ", d->character);
          OLC_MODE(d) = QEDIT_REWARD_DELETE;
        } else {
          send_to_char("No tasks to delete.\r\n", d->character);
          send_to_char("Make your choice : ", d->character);
        }
        break;
      case 'E':
        if (qCanRemReward(d)) { // check if a reward exists
          send_to_char("Edit which reward? ", d->character);
          OLC_MODE(d) = QEDIT_REWARD_CHANGE;
        } else {
          send_to_char("No rewards to edit.\r\n", d->character);
          send_to_char("Make your choice : ", d->character);
        }
        break;
    }
    return; /* end of QEDIT_REWARD_MENU */
/*-------------------------------------------------------------------*/
  case QEDIT_REWARD_NEW:
    if (!isdigit(*arg)) {
      OLC_MODE(d) = QEDIT_REWARD_MENU;
      qRewardMenu(d);
      return;
    }
    qAddReward(d, atoi(arg));
    return;
  case QEDIT_REWARD_CHANGE:
    if (!isdigit(*arg)) {
      OLC_MODE(d) = QEDIT_REWARD_MENU;
      qRewardMenu(d);
      return;
    }
    qEditReward(d, atoi(arg));
    return;
  case QEDIT_REWARD_DELETE:
    if (isdigit(*arg)) qRemReward(d, atoi(arg));
    OLC_QUEST(d)->virtual = 1;
    OLC_MODE(d) = QEDIT_REWARD_MENU;
    qRewardMenu(d);
    return;
  case QEDIT_REWARD_TYPE:
    switch (chIn) {
      case 'S':
        OLC_QUEST(d)->rewards[OLC_QUEST(d)->virtual].type = QST_REWARD_SPELL;
        send_to_char("Enter spell name : ", d->character);
        break;
      case 'Q':
        OLC_QUEST(d)->rewards[OLC_QUEST(d)->virtual].type = QST_REWARD_QP;
        send_to_char("Enter amount : ", d->character);
        break;
      case 'G':
        OLC_QUEST(d)->rewards[OLC_QUEST(d)->virtual].type = QST_REWARD_GOLD;
        send_to_char("Enter amount : ", d->character);
        break;
      case 'P':
        OLC_QUEST(d)->rewards[OLC_QUEST(d)->virtual].type = QST_REWARD_PRAC;
        send_to_char("Enter amount : ", d->character);
        break;
      case 'E':
        OLC_QUEST(d)->rewards[OLC_QUEST(d)->virtual].type = QST_REWARD_EXP;
        send_to_char("Enter amount : ", d->character);
        break;
      case 'O':
        OLC_QUEST(d)->rewards[OLC_QUEST(d)->virtual].type = QST_REWARD_OBJ;
        send_to_char("Enter object vnum (-1 to list) : ", d->character);
        break;
      case 'R':
        if (GET_LEVEL(d->character) >= LVL_GOD) {
            OLC_QUEST(d)->rewards[OLC_QUEST(d)->virtual].type =
                QST_REWARD_REMORT;
            qRaceMenu(d);
            break;
        }
      default:
        send_to_char("Invalid choice!\r\nWhat reward type should this be : ",
            d->character);
        return;
    }
    OLC_MODE(d) = QEDIT_REWARD_ARG;
    return;
  case QEDIT_REWARD_ARG:
    switch (OLC_QUEST(d)->rewards[OLC_QUEST(d)->virtual].type) {
      case QST_REWARD_QP:
      case QST_REWARD_EXP:
      case QST_REWARD_GOLD:
      case QST_REWARD_PRAC:
        if (isdigit(*arg)) {
          OLC_QUEST(d)->rewards[OLC_QUEST(d)->virtual].amount = atoi(arg);
          qLimitReward(d, OLC_QUEST(d)->virtual);
        } else {
          send_to_char("Invalid amount, try again : ", d->character);
          return;
        }
        break;
      case QST_REWARD_OBJ:
        if (isdigit(*arg) && real_object(atoi(arg)) != -1) {
          OLC_QUEST(d)->rewards[OLC_QUEST(d)->virtual].amount = atoi(arg);
        } else {
          // check for -1 here
          send_to_char("Invalid object, try again : ", d->character);
          return;
        }
        break;
      case QST_REWARD_SPELL: {
        int rnum = find_skill_num(arg);
        if (rnum < 1 || rnum >= NUM_SPELLS) {
          send_to_char("That spell is unknown, try again : ", d->character);
          return;
        }
        OLC_QUEST(d)->rewards[OLC_QUEST(d)->virtual].amount = rnum;
        } break;
      case QST_REWARD_REMORT: {
          int race = atoi(arg);
          if (race < 0 || race > QST_NUM_REMORTS) {
              send_to_char("That race is unknown, try again : ", d->character);
              return;
          }
          OLC_QUEST(d)->rewards[OLC_QUEST(d)->virtual].amount = race;
        }
        break;
    }
    send_to_char("Enter action string: \r\n", d->character);
    SEND_RULER(d);
    d->backstr = NULL;
    if (OLC_QUEST(d)->rewards[OLC_QUEST(d)->virtual].speech) {
      send_to_char(OLC_QUEST(d)->rewards[OLC_QUEST(d)->virtual].speech,
          d->character);
      d->backstr = str_dup(OLC_QUEST(d)->rewards[OLC_QUEST(d)->virtual].speech);
    }
    d->str = &OLC_QUEST(d)->rewards[OLC_QUEST(d)->virtual].speech;
    d->max_str = 3000;
    d->mail_to = 0;
    OLC_QUEST(d)->virtual = 1;
    OLC_MODE(d) = QEDIT_REWARD_ACT;
    return;
  }
  OLC_QUEST(d)->virtual = 1;
  qMenu(d);
}
/*. End of parse_qedit() .*/

void qFreeQuest(QuestData *quest)
{
  int i;

  if (quest->name) free(quest->name);
  if (quest->speech) free(quest->speech);
  if (quest->description) free(quest->description);
  for (i = 0; i < MAX_QST_REWARDS; i++)
    if (quest->rewards[i].speech) free(quest->rewards[i].speech);
  free(quest);
}

void qedit_string_cleanup(struct descriptor_data *d, int terminator)
{
  switch (OLC_MODE(d)) {
  case QEDIT_SPEECH:
  case QEDIT_INFO:
    qMenu(d);
    break;
  case QEDIT_REWARD_ACT:
    qRewardMenu(d);
    break;
  }
}
