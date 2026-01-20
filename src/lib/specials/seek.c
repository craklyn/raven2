/**************************************************************************
*   File: seek.c                      Author: Kenneth Morse              *
*  Usage: handling of seeking mobs    E-Mail: 76427.3305@compuserve.com  *
*                                                                         *
*  All rights reserved.  Copyright (C) 1995 by Kenneth Morse              *
*                                                                         *
**************************************************************************/

/**************************************************************************
*                    Version 1.0 - Expect Bugs!!!                         *
**************************************************************************/

/**************************************************************************
*  NOTE: This is prerelease beta code.  It is distributed to Liam and/or  *
*  Digger of RavenMud ONLY, and may be incorporated into RavenMud and     *
*  RavenMud ONLY. Please see seek.doc for features and installation      *
*  instructions.                                                          *
*                                                                         *
*  All rights reserved.  Copyright (C) 1995 by Kenneth Morse              *
**************************************************************************/

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "util/utils.h"
#include "general/class.h"
#include "general/color.h"
#include "general/comm.h"
#include "actions/interpreter.h"
#include "magic/spells.h"
#include "specials/seek.h"
#include "general/handler.h"
#include "actions/fight.h"        /* For hit function */
#include "util/weather.h"
#include "magic/sing.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */
#include "specials/mobact.h"      /* For forget function */

/* Control Constants */
#define CAN_SEEK_IMMORTS
#define CAN_ALL_ALL

#define DEBUG_NONE    0   /* char ch does nothing */
#define DEBUG_SAY     1   /* char ch SAYs [string]  */
#define DEBUG_IMMORT  2   /* char ch sends message [string] to all immorts in room */
#define DEBUG_TOCHAR  3   /* message is sent to char ch */

/* Set the following statments = to one of the above to control constants*/
/* debugging messages */
#define DEBUG_LOOT     DEBUG_NONE    /* when mob is looting corpses/rooms */
                                     /* may be none, say, or immort */
#define DEBUG_THINK    DEBUG_NONE  /* when mob is thinking */
                                     /* may be none, say, or immort */
#define DEBUG_COMMAND  DEBUG_TOCHAR  /* when imm is using seek commands */
                                     /* may be none, tochar, or immort */

const char *seek_status[] = {
  "Idle",
  "OK",
  "Blocked",
  "Fighting",
  "LostPrey",
  "BFSError",
  "NoPath",
  "Door",
  "Locked",
  "Busy",
  "Water",
  "Air",
  "*POOF*",
  "\n"
};

/* Forward defs for local functions */
int  seek_canland(struct char_data *ch, int room);
int  seek_cantakeoff(struct char_data *ch);
void seek_mobunlockdoor(struct char_data *ch, int dir);
void seek_mobopendoor(struct char_data *ch, int dir);
void seek_reallydopoof(struct char_data *ch, int room);
void seek_unlockmsg(struct char_data *ch, char *here, char *there, int dir, int openit);
int  ynnek_find_dots(char *str, char **real);
int  ynnek_isname(char *str, char *namelist);
char *two_words(char *argument, char *first_arg, char *second_arg);
RoomDirectionData *otherside(CharData *ch, int dir);

/* Procedure Uses are commented in SEEK.H!*/

char *two_words(char *argument, char *first_arg, char *second_arg)
{
    char *one_word(char *, char *);
    return one_word(one_word(argument,first_arg), second_arg); /* ;-) */
}

int ynnek_isname(char *str, char *namelist)
{
  /* returns true only if all of str is w/in namelist */
  /* (well, that's the theory at least....)           */

  char *curstr;
  char anarg[MAX_INPUT_LENGTH];

  curstr = str;
  for (;;) {
    if (!*curstr) break;

    curstr = one_argument(curstr,anarg);
    if (!isname(anarg,namelist)) return(0);
  }
  return(1);
}

int ynnek_find_dots(char *str, char **real)
{
  char *adot;
  int result=-1;

  /* Is there a dot? */
  adot = strchr(str, '.');

  if (adot)
  { /* there is a dot! */
    *real = adot+1;

    *adot = (char)0;
    if (is_number(str)) {
      /* is a X. */
      result = atoi(str);
    }
   else if (strcmp(str,"all")==0) result = YDOTS_ALLDOT;    /* is all. */
    else result = YDOTS_UNKNOWN;                            /* is ???. */
    if ((result == -2) && (strcmp(*real,"all")==0)) result=YDOTS_ALLDOTALL;
  }
  else
  {
    if (strcmp(str,"all")==0) result = YDOTS_ALL;
    else result=YDOTS_NODOTS;
    *real=str;
  }
  return result;
}

void seek_debug(struct char_data *ch, char *str, int mode)
{
  struct char_data *imm;
  char bufx[255];

  if (mode==DEBUG_NONE) return;
  if (mode==DEBUG_SAY) {
    do_say(ch,str,0,0);
  }
  if (mode==DEBUG_IMMORT) {
    for (imm=world[IN_ROOM(ch)].people;imm;imm=imm->next_in_room){
      if (GET_LEVEL(imm)>=LVL_GOD) {
        sprintf(bufx,"%s[%s] - %s%s\r\n",CCMAG(imm,C_NRM),GET_NAME(ch),str,
          CCNRM(imm,C_NRM));
        send_to_char(bufx,imm);
      }
    }
  }
  if (mode==DEBUG_TOCHAR) {
    sprintf(bufx,"%s%s%s\r\n",CCCYN(ch,C_NRM),str,CCNRM(ch,C_NRM));
    send_to_char(bufx,ch);
  }
}

/* if capable, handle a door */
int seek_handledoor(struct char_data *ch, int dir)
{
  struct obj_data *keyptr;

  /* ok, there is a door in the way, is it locked? */
  if (IS_SET(EXIT(ch,dir)->exit_info, EX_LOCKED)) {
    /* yup, its locked */
    /* do we have the key? */
    if( has_key( ch, EXIT(ch,dir)->key, &keyptr )) {
      /* yes, we do have the key! */
      seek_mobunlockdoor(ch,dir);
      return 1;
    } else {
      /* no key - Pickable? */
      if (!IS_SET(EXIT(ch,dir)->exit_info, EX_PICKPROOF)) {
        /* Pickable, have at it */
        switch (GET_RACE(ch))
        {
          case RACE_DRAGON:
            seek_unlockmsg(ch,"$n takes a deep breath and incinerates a $F with a belch of flame.",
                            "A %s bursts into flaming spliters behind you.\r\n",dir,1);
            return 1;
          case RACE_DWARF:
          case RACE_SDWARF:
            seek_unlockmsg(ch,"$n pulls out $s tools and skillfully unhinges the $F.",
                            "A %s falls off of it's hinges quietly.\r\n",dir,1);
            return 1;
          case RACE_WEREWOLF:
            seek_unlockmsg(ch,"$n slashes the $F into splinters with $s powerfull claws.",
                            "Razor sharp claws turn a %s into splintes from the other side.\r\n",dir,1);
            return 1;
          case RACE_GIANT:
            seek_unlockmsg(ch,"$n knocks the $F right off it's hinges with $s fist.",
                            "You hear a loud *BANG* as the %s flys off it's hinges.\r\n",dir,1);
            return 1;
          case RACE_MINOTAUR:
          case RACE_SMINOTAUR:
            seek_unlockmsg(ch,"$n prys the lock out of the $F with $s horns.",
                            "You hear a strange noise coming from the %s.\r\n",dir,0);
            return 1;
          case RACE_DEMON:
            seek_unlockmsg(ch,"$n waves $s hand and the $F vanishes into the mist.",
                            "The %s vanishes in a swirling mist.\r\n",dir,1);
            return 1;
          default:
            return 0;
        }
      } else {
        /* Non pickable, give it a shot */
        switch (GET_RACE(ch))
        {
          case RACE_DRAGON:
            seek_unlockmsg(ch,"$n carefully melts the lock on a $F with $s breath.",
                            "The %s behind you sizzles a bit.\r\n",dir,0);
            return 1;
          case RACE_GIANT:
            seek_unlockmsg(ch,"$n puts $s fist right through the $F's lock.",
                            "The lock on the %s suddenly flys across the room.\r\n",dir,0);
            return 1;
          case RACE_DEMON:
            seek_unlockmsg(ch,"$n waves $s hand and the $F vanishes into the mist.",
                            "The %s vanishes in a swirling mist.\r\n",dir,1);
            return 1;
          default:
            return 0;
        }
      }

    }
  } else {
    /* unlocked, open it */
    seek_mobopendoor(ch,dir);
    return 1;
  }

  return 0;
}

int seek_dopoof(struct char_data *ch, byte landonprey)
{
  int room;

  if (!seek_cantakeoff(ch)) {
    /* sorry, can't take off... */
    return 0;
  }

  /* can we land on our prey? */
  if (landonprey) {
    if (seek_canland(ch,IN_ROOM(SEEKING(ch)))) {
      seek_reallydopoof(ch,IN_ROOM(SEEKING(ch)));
      return 1;
    } else {
      return 0;
    }
  }

  /* pick a random room */
  room = 0;
  while (room==0) {
    room = number(1,top_of_world);
    if (seek_canland(ch,room)) {
      seek_reallydopoof(ch,room);
      return 1;
    } else {
      room = 0;
    }
  }
  return 0;
}


/* if allowed teleport */
int seek_pooftoprey(struct char_data *ch, byte force)
{
  int perland;  /* chance out of 1000 of landing on victim  */
  int perrand;  /* chance out of 1000 of landing randomly   */
  int magic;    /* and the magic number is...... */

  if (force) {          /* If we MUST poof */
    perrand = 1000;     /* 100% */       /* 100% - 5000% (caped at 100 of course) */
    perland = 2;        /* .2%  */       /* 0.2% - 10%   */
  } else {              /* If not.... */
    perrand = 0;        /* 0%   */       /* 0% - 0%      */
    perland = 1;        /* .1%  */       /* 0.1% - 5%    */
  }
  perrand = perrand * GET_LEVEL(ch);
  perland = perland * GET_LEVEL(ch);

  magic = number(1,1000);
  if (magic<=perland) {
    /* poof directly to them.... */
    if (!(seek_dopoof(ch,1))) {
      /* eek! we didn't get em! */
      /* try a random hop! */
      return seek_dopoof(ch,0);
    } else {
      /* got em! */
      return 1;
    }
  }
  if (magic<=perrand) {
    /* poof to a random sqyare */
    return seek_dopoof(ch,0);
  }
  return 0;
}


struct char_data *seek_gettarget(int dotmode, char *target,struct char_data *first, int *count, struct char_data *ch, int roomnum)
{
  struct char_data *mob;
  int total=0;
  int current=0;
  int firstrun=0;
  int valid;
  int done;

  /* other valid targets */
  if (strcmp(target,"quest")==0) {dotmode=YDOTS_QUEST;roomnum=-1;}
  else if (strcmp(target,"npc")==0) {dotmode=YDOTS_NPC;roomnum=-1;}
  else if (strcmp(target,"pc")==0) {dotmode=YDOTS_PC;roomnum=-1;}
  else if (strcmp(target,"mob")==0) {dotmode=YDOTS_MOB;roomnum=-1;}

  if (*count<1) {
    if (roomnum<=0) first=character_list;
      else first=world[roomnum].people;
    firstrun=1;
  }
  else total = number(1,*count);

  /* if this is not the firstrun, and dot>1.  (ie 30), the 30th matching */
  /* mob is already selected as firstmob.  No need to count 30 again...  */
  if (!firstrun && dotmode>1) dotmode=1;

  done=0;
  for (mob=first;mob && !done;(roomnum<=0) ? (mob = mob->next) : (mob = mob->next_in_room)) {
#ifdef CAN_SEEK_IMMORTS
    if (CAN_SEE(ch,mob)) {
#else
    if (CAN_SEE(ch,mob) && (IS_NPC(mob) || (!IS_NPC(mob) && GET_LEVEL(mob)<LVL_IMMORT))) {
#endif
      valid=0;

      if (dotmode == YDOTS_NODOTS && ynnek_isname(target,mob->player.name)) valid=1;
      else if (dotmode == YDOTS_ALLDOT && ynnek_isname(target,mob->player.name)) valid=1;
      else if (dotmode == YDOTS_ALL || dotmode ==YDOTS_ALLDOTALL) valid = 1;
      else if (dotmode == YDOTS_ZERO && ynnek_isname(target,mob->player.name) && !IS_NPC(mob)) valid =1;
      else if (dotmode == 1 && ynnek_isname(target,mob->player.name)) valid = 1;
      else if (dotmode >= 1 && ynnek_isname(target,mob->player.name)) dotmode--;
      else if (dotmode == YDOTS_QUEST && (PRF_FLAGGED(mob,PRF_QUEST))) valid = 1;
      else if (dotmode == YDOTS_PC && !IS_NPC(mob)) valid = 1;
      else if (dotmode == YDOTS_NPC && IS_NPC(mob)) valid = 1;
      else if (dotmode == YDOTS_MOB && IS_MOB(mob)) valid = 1;

      if (valid) {
        if (firstrun) {
          total++;
          if (total==1) first=mob;
        }
        else {
          current++;
          if (current==total) return mob;
        }
        if (dotmode>=0) done=1;
      }
    }
  }
  if (!firstrun)
  {
    mlog("SYSERR: logic flaw in seeking code - seek_gettarget ");
    return NULL;
  }

  *count = total;
  return first;
}

ACMD(do_seek)
{
    char target[MAX_INPUT_LENGTH];
    char seeker[MAX_INPUT_LENGTH];
    char buffer[MAX_INPUT_LENGTH];
    char *realseeker, *realtarget;
    int  seekerdot,targetdot;
    struct char_data *mob,*firstmob;
    int  done,count,doit,mobcount,location,tries;
    int ishunting,ishuntingtarget;

    /* Fetch the seeker and target names from the command line */
    two_words(argument,seeker,target);

    /* If no seeker is specified, show usage: */
    if( !*seeker && subcmd!=SCMD_QUERY ){
        sendChar( ch, "Usage: SEEK    MOB   TARGET\r\n"  );
        sendChar( ch, "Usage: QSEEK  [MOB] [TARGET]\r\n" );
        sendChar( ch, "Usage: UNSEEK  MOB  [TARGET]\r\n" );
        return;
    }

    if(( seekerdot = ynnek_find_dots(seeker,&realseeker)) == YDOTS_UNKNOWN ){
      sendChar( ch, "I don't understand [%s.%s].\r\n", seeker, realseeker );
      return;
    }

    if ((targetdot = ynnek_find_dots(target,&realtarget)) == YDOTS_UNKNOWN) {
        sendChar( ch, "I don't understand [%s.%s].\r\n", target, realtarget );
        return;
    }

    /* If there is no target, and we are setting, treat as a query */
    if( (!*target) && (subcmd == SCMD_SET)) subcmd = SCMD_QUERY;

    /* If we are querying, and no dots in seeker, treat it as ALL.SEEKER */
    if( subcmd==SCMD_QUERY && seekerdot==YDOTS_NONE) seekerdot = YDOTS_ALLDOT;

    /* If we are querying and there is no seeker specified, treat as all */
    if( subcmd==SCMD_QUERY && !*seeker) seekerdot = YDOTS_ALL;
    /* Debugging ... */
    switch (subcmd)
    {
        case SCMD_SET:   sprintf(buffer,"Set Stalk: "); break;
        case SCMD_UNSET: sprintf(buffer,"UnSet Stalk: "); break;
        case SCMD_QUERY: sprintf(buffer,"Query Stalk: "); break;
        default: sprintf(buffer,"INTERNAL ERROR - UNDEFINED SEEK SUBCOMMAND: "); break;
    }

    sprintf(buf,"%sStalkdot = %d; Stalker = [%s]; RealStalker = [%s]",buffer,seekerdot,seeker,realseeker);
    seek_debug(ch,buf,DEBUG_COMMAND);
    sprintf(buf,"%sTargetdot = %d; Target = [%s]; RealTarget = [%s]",buffer,targetdot,target,realtarget);
    seek_debug(ch,buf,DEBUG_COMMAND);

    done=0;
    count=0;
    mobcount=0;

    if (subcmd==SCMD_SET) {
      location=-1;
      if( targetdot == YDOTS_NONE ){
          location=ch->in_room;
          firstmob = seek_gettarget(targetdot,realtarget,NULL,&mobcount,ch,location);
          if( mobcount==0){ targetdot=YDOTS_ALLDOT; location=-1; }
      }
      if( mobcount==0 ) firstmob = seek_gettarget(targetdot,realtarget,NULL,&mobcount,ch,-1);
      if( mobcount>0 ){
          sprintf(buf,"Valid targets: %d",mobcount);
          seek_debug(ch,buf,DEBUG_COMMAND);
      }
      else {
          sendChar( ch, "No valid targets found!\r\n" );
          return;
      }

   sprintf(buf, "First valid target: %s",GET_NAME(firstmob));
   seek_debug(ch,buf,DEBUG_COMMAND);
  }

  /* So, what mob do we pick?? */
  for (mob=character_list;mob && !done;mob = mob->next) {
    if (IS_MOB(mob) && CAN_SEE(ch,mob)) {
      doit=0;
      ishunting = (SEEKING(mob) != NULL);
      if (ishunting) {
        if (*target)
           ishuntingtarget = ynnek_isname(realtarget,(SEEKING(mob))->player.name);
        else
          ishuntingtarget = 1;
      }
      else
        ishuntingtarget = 0;

      /* Source is formated with no dots..... */
      if (((seekerdot == YDOTS_NONE) && (ynnek_isname(realseeker,mob->player.name))) &&
        (((subcmd == SCMD_SET) && !ishunting) || ((subcmd == SCMD_UNSET) && ishunting))) doit=1;

     /* Source is formatted all.name */
      if (((seekerdot == YDOTS_ALLDOT) && (ynnek_isname(realseeker,mob->player.name))) &&
        (((subcmd == SCMD_SET) && !ishunting) ||
         ((subcmd == SCMD_UNSET) && ishuntingtarget) ||
         ((subcmd == SCMD_QUERY) && ishuntingtarget))) doit=1;
     /* Source is formatted x.name (x=1) */
      if ((seekerdot == 1) && (ynnek_isname(realseeker,mob->player.name))) doit = 1;

      /* Source is formatted x.name (x>1) */
      if ((seekerdot >1) && (ynnek_isname(realseeker,mob->player.name))) seekerdot--;

      /* Source is formatted all */
      if (seekerdot == YDOTS_ALL && subcmd!=SCMD_SET && ishuntingtarget) doit=1;

      /* Source is formatted all.all */
      if ((seekerdot == YDOTS_ALLDOTALL) && (
#ifdef CAN_ALL_ALL
         ((subcmd == SCMD_SET) && !ishunting) ||
#endif
         ((subcmd == SCMD_UNSET) && ishuntingtarget) ||
         ((subcmd == SCMD_QUERY) && ishuntingtarget))) doit=1;

      if (doit==1) {
       switch (subcmd)
        {
          case SCMD_SET:
            tries=1;
            do {
              tries++;
              SEEKING(mob) = seek_gettarget(targetdot,realtarget,firstmob,&mobcount,ch,location);
            } while (tries<=5 && SEEKING(mob)==mob);
            if (SEEKING(mob) == mob) SEEKING(mob)=NULL;
            SEEK_STATUS(mob)=SEEK_IDLE;
            /* give mob Infravision*/
            SET_BIT_AR(AFF_FLAGS(mob),AFF_INFRAVISION);
            break;
          case SCMD_UNSET:
            SEEKING(mob) = NULL;
            SEEK_STATUS(mob)=SEEK_IDLE;
            break;
        }

        if (count<100) {
          if (!SEEKING(mob)) sprintf(buffer,"Nobody");
          else sprintf(buffer,"%23s [%s%5d%s]",GET_NAME(SEEKING(mob)),
            CCGRN(ch,C_NRM),world[SEEKING(mob)->in_room].number,CCNRM(ch,C_NRM));
          sprintf(buf,"[%s%5d%s] %23s -> %s - ",CCGRN(ch,C_NRM),
            world[mob->in_room].number,CCNRM(ch,C_NRM),GET_NAME(mob),buffer);
          sprintf(buffer,"%s%s%s\r\n",CCBLU(ch,C_NRM),
            seek_status[(int)SEEK_STATUS(mob)],CCNRM(ch,C_NRM));
          strcat(buf,buffer);
          send_to_char(buf,ch);
        }
         else if (count == 100) send_to_char("Display aborted at 100... Sorry.\r\n",ch);

        if (seekerdot >= -1) done = 1;
        count++;
      }
    }
  }

  if (count>0) {
    sprintf(buf,"%d seekers. \r\n",count);
    send_to_char(buf,ch);
 }
  else
  {
    sprintf(buf,"Nothing done.\r\n");
    send_to_char(buf,ch);
    sprintf(buf,"[Place Error Message Here]\r\n");
    send_to_char(buf,ch);
  }
}

ACMD(do_pseek)
{
  char target[MAX_INPUT_LENGTH];
  char seeker[MAX_INPUT_LENGTH];
  char buffer[MAX_INPUT_LENGTH];
  char *realseeker;
  int  seekerdot;
  int ishunting,ishuntingtarget;
  struct char_data *mob;
  int  done,doit,count;

  /* Fetch the seeker and target names from the command line */
  two_words(argument,seeker,target);

  /* If no seeker is specified, show usage: */
    if (!*seeker && subcmd!=SCMD_QUERY) {
    send_to_char("Use: PSEEK    MOB   TARGET  \r\n",ch);
    send_to_char("Use: QPSEEK  [MOB] [TARGET] \r\n",ch);
    send_to_char("Use: UNPSEEK  MOB  [TARGET] \r\n",ch);
    return;
  }

  if ((seekerdot = ynnek_find_dots(seeker,&realseeker)) == -3) {
    sprintf(buf,"I don't understand [%s.%s].\r\n",seeker,realseeker);
    send_to_char(buf,ch);
    return;
  }

  /* If there is no target, and we are setting, treat as a query */
  if ((!*target) && (subcmd == SCMD_SET)) subcmd = SCMD_QUERY;

  /* If we are querying, and no dots in seeker, treat it as ALL.SEEKER */
  if (subcmd==SCMD_QUERY && seekerdot==YDOTS_NONE) seekerdot = YDOTS_ALLDOT;

  /* If we are querying and there is no seeker specified, treat as all */
  if (subcmd==SCMD_QUERY && !*seeker) seekerdot = YDOTS_ALL;

  /* For debugging porpoises only.... */
  switch (subcmd)
  {
    case SCMD_SET:
      sprintf(buffer,"Set Persistant Stalk: ");
      break;
    case SCMD_UNSET:
      sprintf(buffer,"UnSet Persistant Stalk: ");
      break;
    case SCMD_QUERY:
      sprintf(buffer,"Query Persistant Stalk: ");
      break;
    default:
      sprintf(buffer,"INTERNAL ERROR - UNDEFINED SEEK SUBCOMMAND: ");
      break;
  }

  sprintf(buf,"%sStalkdot = %d; Stalker = [%s]; RealStalker = [%s]",buffer,seekerdot,seeker,realseeker);
  seek_debug(ch,buf,DEBUG_COMMAND);

  count=0;
  done=0;

  /* So, what mob do we pick?? */
  for (mob=character_list;mob && !done;mob = mob->next) {
    if (IS_MOB(mob) && CAN_SEE(ch,mob)) {
      doit=0;
      ishuntingtarget = 0;
      ishunting = (SEEK_TARGETSTR(mob) != NULL);
      if (ishunting) {
        if (*target)
           ishuntingtarget = (strcmp((char *)SEEK_TARGETSTR(mob),target)==0);
        else
          ishuntingtarget = 1;
      }

      /* Source is formated with no dots..... */
      if (((seekerdot == YDOTS_NONE) && (ynnek_isname(realseeker,mob->player.name))) &&
        (((subcmd == SCMD_SET) && !ishunting) || ((subcmd == SCMD_UNSET) && ishunting))) doit=1;

     /* Source is formatted all.name */
      if (((seekerdot == YDOTS_ALLDOT) && (ynnek_isname(realseeker,mob->player.name))) &&
        (((subcmd == SCMD_SET) && !ishunting) ||
         ((subcmd == SCMD_UNSET) && ishuntingtarget) ||
         ((subcmd == SCMD_QUERY) && ishuntingtarget))) doit=1;
     /* Source is formatted x.name (x=1) */
      if ((seekerdot == 1) && (ynnek_isname(realseeker,mob->player.name))) doit = 1;

      /* Source is formatted x.name (x>1) */
      if ((seekerdot >1) && (ynnek_isname(realseeker,mob->player.name))) seekerdot--;

      /* Source is formatted all */
      if (seekerdot == YDOTS_ALL && subcmd!=SCMD_SET && ishuntingtarget) doit=1;

      /* Source is formatted all.all */
      if ((seekerdot == YDOTS_ALLDOTALL) &&
        (((subcmd == SCMD_SET) && !ishunting) ||
         ((subcmd == SCMD_UNSET) && ishuntingtarget) ||
         ((subcmd == SCMD_QUERY) && ishuntingtarget))) doit=1;

      if (doit==1) {
       switch (subcmd)
        {
          case SCMD_SET:
            /* remove old string */
            if (SEEK_TARGETSTR(mob) != NULL) free(SEEK_TARGETSTR(mob));
            SEEK_TARGETSTR(mob) = str_dup(target);
            /* give mob Infravision*/
            SET_BIT_AR(AFF_FLAGS(mob),AFF_INFRAVISION);
            break;
          case SCMD_UNSET:
            if (SEEK_TARGETSTR(mob) != NULL) free(SEEK_TARGETSTR(mob));
            SEEK_TARGETSTR(mob)=NULL;
            break;
        }

        if (count<100) {
          if (SEEK_TARGETSTR(mob)==NULL) sprintf(buffer,"Nobody");
          else sprintf(buffer,"%23s",SEEK_TARGETSTR(mob));
          sprintf(buf,"[%s%5d%s] %23s -> %s - ",CCGRN(ch,C_NRM),
            world[mob->in_room].number,CCNRM(ch,C_NRM),GET_NAME(mob),buffer);
          sprintf(buffer,"%s%s%s\r\n",CCBLU(ch,C_NRM),
            seek_status[(int)SEEK_STATUS(mob)],CCNRM(ch,C_NRM));
          strcat(buf,buffer);
          send_to_char(buf,ch);
        }
         else if (count == 100) send_to_char("Display aborted at 100... Sorry.\r\n",ch);

        if (seekerdot >= -1) done = 1;
        count++;
      }
    }
  }

  if (count>0) {
    sprintf(buf,"%d seekers. \r\n",count);
    send_to_char(buf,ch);
 }
  else
  {
    sprintf(buf,"Nothing done.\r\n");
    send_to_char(buf,ch);
    sprintf(buf,"[Place Error Message Here]\r\n");
    send_to_char(buf,ch);
  }
}


void seek_lootlist(struct char_data *ch, struct obj_data *objlist, struct obj_data *cont)
{
  /* Look for anything interesting in  */
  /* target's corpse.                  */
  struct obj_data *tmp,*obj,*nextobj;
  int want;
  char bufx[255];

  for (obj=objlist;obj;obj=nextobj) {
    /* save next object to avoid corruption */
    nextobj = obj->next_content;

    if (!CAN_SEE_OBJ(ch,obj)) continue;

    /* do we want the object? */
    want=0;

    /* Always pick up keys & cash */
    if (GET_OBJ_TYPE(obj) == ITEM_MONEY) want=1;
    else if (GET_OBJ_TYPE(obj) == ITEM_KEY) want=1;

    /* Is it a boat? Do we have a boat? Yes & No? Get a boat...*/
    else if (GET_OBJ_TYPE(obj) == ITEM_BOAT) {
      want=1;
      for(tmp=ch->carrying;tmp && want;tmp=tmp->next_content) {
        if (GET_OBJ_TYPE(obj) == ITEM_BOAT) want=0;
      }
    }

    if (want==0) {
      /* Anything with your race name in it */
#if 0
      if( isname( &race_types[(int)GET_RACE(ch)], obj->name )) want=1;
#else
      if( search_block(obj->name, race_types, FALSE) >= 0) want = 1;
#endif

      /* Dragons all "gold" and treasure */
      else if (GET_RACE(ch)==RACE_DRAGON) {
        if (isname("gold",obj->name) || GET_OBJ_TYPE(obj)==ITEM_TREASURE) want=1;
      }

      /* Undead all "silver" */
      else if (IS_VAMPIRE(ch) || GET_RACE(ch)==RACE_UNDEAD || GET_RACE(ch)==RACE_WEREWOLF) {
       if (isname("silver",obj->name)) want=1;
      }

      /* Animals, all food */
      else if (GET_RACE(ch)==RACE_ANIMAL) {
        if (GET_OBJ_TYPE(obj)==ITEM_FOOD) want=1;
      }
    }

    if (want) {

      sprintf(bufx,"Getting %s.",obj->short_description);
      seek_debug(ch,bufx,DEBUG_LOOT);

      /* get the item from wherever it is... */

      if (!cont) {
        /* get item from room */
        obj_from_room(obj);
        obj_to_char(obj,ch);
        act("You snatch $p.",FALSE,ch,obj,NULL,TO_CHAR);
        act("$n snatches $p.",TRUE,ch,obj,NULL,TO_ROOM);
        get_check_money(ch,obj);
      } else {
        /* get item from object */
        
          /* Players are losing items out of lockers (both personal and clanned).
           * So disabling this.  Ideally, at a later date could sit down and make
           * sure mobiles don't take things out of closed containers (at least
           * until after opening them).  Then, always disabling lockers and 
           * perhaps also player corpses.
          obj_from_obj(obj);
          obj_to_char(obj,ch);
          act("You snatch $p from $P.",FALSE,ch,obj,cont,TO_CHAR);
          act("$n snatches $p from $P.",TRUE,ch,obj,cont,TO_ROOM);
          get_check_money(ch,obj);
           * /
      }

    } else {
      if (GET_OBJ_TYPE(obj) == ITEM_CONTAINER) {
        /*
        sprintf(bufx,"Looking in %s.",obj->short_description);
        seek_debug(ch,bufx,DEBUG_LOOT);
        */

        /* let's not magically remove items from locked containers anymore... */
        if (!IS_SET(GET_OBJ_VAL(obj,1), CONT_LOCKED))
          seek_lootlist(ch,obj->contains,obj);

        /*
        sprintf(bufx,"Done with %s.",obj->short_description);
        seek_debug(ch,bufx,DEBUG_LOOT);
        */
      }
    }
  }
}


/* Whack current target */
/* NOTE: Assumes prey is in same room as target!!! */
void seek_whackprey(struct char_data *ch)
{
  char str[80], str2[80];
  /* Sen't mages charmies out to find their masters, but lets not hit them! */
  if( IS_SET_AR(AFF_FLAGS(ch), AFF_CHARM) && ch->master && SEEKING(ch) == ch->master)
  {
	/* Hurrah, we found our master again. */
	if (GET_SEX(ch->master) == 2)
		sprintf(str2, "Mistress");
	else
		sprintf(str2, "Master");
	sprintf(str, "We are united again, %s!", str2);
	do_say(ch, str, 0, 0);
	SEEKING(ch) = NULL;
	SEEK_STATUS(ch) = SEEK_IDLE;
	SEEK_TARGETSTR(ch) = NULL;
	return;
  }

  if( CAN_SEE( ch, SEEKING(ch)) && !PRF_FLAGGED( SEEKING(ch), PRF_NOHASSLE )) {
    if( IS_SET_AR( ROOM_FLAGS( ch->in_room ), ROOM_PEACEFUL )){
        if( number( 1, 3 ) == 1 ){
            act("You get the feeling that $N is here for you!",FALSE,SEEKING(ch),NULL,ch,TO_CHAR);
            act("$N glares at $n with a vengeful look.",TRUE,SEEKING(ch),NULL,ch,TO_ROOM);
        }
    }
    else {
        hit(ch, SEEKING(ch), TYPE_UNDEFINED);
        SEEK_STATUS(ch)=SEEK_FIGHTING;
    }
  }
}


/* Stalk current target */
void seek_seekprey(struct char_data *ch)
{
    CharData *tmp;
    byte found;
    int dir, dest;
    char bufx[255];

    /* is the mob in a valid room? */
    if (IN_ROOM(ch) < 0) return;

    /* is mob seeking someone right now? */
    if( SEEKING(ch) ){
        /* mob is currently hunting someone */
        /* Perhaps I just killed my prey? */
        seek_lootlist( ch, world[IN_ROOM(ch)].contents, NULL );

        /* is that person a valid target? */
        for( found=0, tmp=character_list; tmp && !found; tmp=tmp->next ) {
            if( SEEKING(ch) == tmp ) found=1;
        }
        if( !found ){
            do_say( ch, "Darn! My prey is gone!!", 0, 0 );
            SEEK_STATUS(ch) = SEEK_LOSTPREY;
            SEEKING(ch)     = NULL;
            seek_newtarget(ch);
            return;
        }
        /* person is a valid target - look for person */
        dest = SEEKING(ch)->in_room;
        if (SEEKING(ch)->false_trail) {
            if (SEEKING(ch)->false_trail == ch->in_room) {
                SEEKING(ch)->false_trail = 0;
                do_say(ch, "Where did my prey go?", 0, 0);
                forget(ch, SEEKING(ch));
                seek_newtarget(ch);
                return;
            } else {
                dest = SEEKING(ch)->false_trail;
            }
        }
        dir = find_first_step(ch->in_room,dest);

        sprintf( bufx, "dir = %d", dir );
        seek_debug( ch, bufx, DEBUG_THINK );

        switch (dir) {
            case BFS_ERROR:
                do_say(ch,"ACK! Error in seeking code! I'm lost!",0,0);
                SEEK_STATUS(ch)=SEEK_BFSERROR;
                seek_newtarget(ch);
                return;
            case BFS_ALREADY_THERE: seek_whackprey(ch); return;
            case BFS_NO_PATH:
                if( !seek_pooftoprey(ch,TRUE) )
                    SEEK_STATUS(ch)=SEEK_NOPATH;
                else
                    SEEK_STATUS(ch)=SEEK_POOFED;
                return;
            default:
                /* They are in the mud, and accessable, but not here.... */
                if (!seek_pooftoprey(ch,FALSE)) {
                    /* Either incapable of poofing, or not in the mood, so walk */
                    /* Door in the way? */
                if( IS_SET(EXIT(ch,dir)->exit_info, EX_CLOSED )){
                    SEEK_STATUS(ch)=SEEK_DOOR;
            /* EEK, closed door in the way... */
            if (!seek_handledoor(ch,dir)) {
              /* no progress on door */
              /* now we really need to poof */
              if (!seek_pooftoprey(ch,TRUE)) {
                /* but we Can't, find new prey then */
                SEEK_STATUS(ch)=SEEK_LOCKED;
                seek_newtarget(ch);
                return;
              }
              else
              { /* we poofed */
                 SEEK_STATUS(ch) = SEEK_POOFED;
              }
            }
            else
            {

              /* we made progress on the door */

            }
          }
           else
          {
             int zid = world[EXIT(ch,dir)->to_room].zone;
             if( zone_table[zid].tournament_room != 0 ){
                 SEEK_STATUS(ch) = SEEK_BLOCKED;
             }
            /* Nope no door in the way */
             else if( perform_move( ch, dir, 1 ) == 0 ){
               /* move failed, maybe we're dead */
               if (IN_ROOM(ch) == -1) return;
               /* poof, please? */
               if (!seek_pooftoprey(ch,TRUE)) {
                 /* Let's find out why... */
                 /* is it because we can't fly? */
                 if (world[EXIT(ch,dir)->to_room].sector_type==SECT_FLYING &&
                     !CAN_FLY(ch)) {
                     SEEK_STATUS(ch) = SEEK_AIR;
                 } else if (world[EXIT(ch,dir)->to_room].sector_type==SECT_WATER_NOSWIM &&
                     !CAN_SWIM(ch)) {
                     SEEK_STATUS(ch) = SEEK_WATER;
                 } else {
                     SEEK_STATUS(ch) = SEEK_BLOCKED;
                 }
               } else {
                 /* we poofied! */
                 SEEK_STATUS(ch) = SEEK_POOFED;
               }
             } else {
               /* we moved */
               SEEK_STATUS(ch) = SEEK_OK;
             }
          }
        }
         else
        {
          /* We did poof! */
          SEEK_STATUS(ch) = SEEK_POOFED;
        }
    }
  }
  else
  { /* mob is not currently hunting someone */
    mlog("SYSERR: Stalkprey called for mob with no target");
  }

  /* Ok, we moved, so is prey here? */
  if (ch->in_room == SEEKING(ch)->in_room) {
    /* Feeling tetchy? 1 in 5 chance */
    if (number(1,5)==1) {
      /* Wack em! */
      seek_whackprey(ch);
    }
  }
}

int seek_cantakeoff(struct char_data *ch)
{
  switch(GET_RACE(ch)) {
    case RACE_DRACONIAN:
    case RACE_SDRACONIAN:
    case RACE_DRAGON:
      if (world[IN_ROOM(ch)].sector_type == SECT_INSIDE ||
          ROOM_FLAGGED(IN_ROOM(ch),ROOM_INDOORS)) {
        return 0;
      } else {
        return 1;
      }
    case RACE_VAMPIRE:
      if (world[IN_ROOM(ch)].sector_type == SECT_INSIDE ||
          ROOM_FLAGGED(IN_ROOM(ch),ROOM_INDOORS)) {
        return 0;
      } else {
        return 1;
      }
    case RACE_DEMON:
      return 1;
    default:
      return 0;
  }
}


int seek_canland(struct char_data *ch, int room)
{
  switch(GET_RACE(ch)) {
    case RACE_VAMPIRE:
      if (world[room].sector_type == SECT_INSIDE ||
          ROOM_FLAGGED(room, ROOM_INDOORS) || ROOM_FLAGGED(room, ROOM_NOMOB) ||
	  ROOM_FLAGGED(room, ROOM_PRIVATE) || ROOM_FLAGGED(room, ROOM_GODROOM) ||
	  ROOM_FLAGGED(room, ROOM_HOUSE) || ROOM_FLAGGED(room, ROOM_ATRIUM)) {
          return 0;
      } else {
           return 1;
      }
    case RACE_DRACONIAN:
    case RACE_SDRACONIAN:
    case RACE_DRAGON:
      if (world[room].sector_type == SECT_INSIDE ||
          ROOM_FLAGGED(room, ROOM_INDOORS) || ROOM_FLAGGED(room, ROOM_NOMOB) ||
	  ROOM_FLAGGED(room, ROOM_PRIVATE) || ROOM_FLAGGED(room, ROOM_GODROOM) ||
	  ROOM_FLAGGED(room, ROOM_HOUSE) || ROOM_FLAGGED(room, ROOM_ATRIUM)) {
          return 0;
      } else {
           return 1;
      }
    case RACE_DEMON:
      if (ROOM_FLAGGED(room, ROOM_NOMOB) || ROOM_FLAGGED(room, ROOM_PRIVATE) ||
	  ROOM_FLAGGED(room, ROOM_GODROOM) || ROOM_FLAGGED(room, ROOM_HOUSE) ||
	  ROOM_FLAGGED(room, ROOM_ATRIUM)) {
          return 0;
      } else {
           return 1;
      }
    default:
      mlog("SYSERR: seek_canland called for non flying race! ");
      return 0;
  }
}


void seek_reallydopoof(struct char_data *ch, int room)
{
  /* ok, lets do it... */
  switch (GET_RACE(ch)) {
    case RACE_DRACONIAN:
    case RACE_SDRACONIAN:
      act("$n flaps $s wings and flies into the sky.",FALSE,ch,0,0,TO_ROOM);
      sendChar(ch, "You flap your wings andfly into the sky.\r\n");
      break;
    case RACE_DRAGON:
      act("$n beats $s wings and takes off in search of prey.",FALSE,ch,0,0,TO_ROOM);
      sendChar(ch, "You beat your wings and fly in search of prey.\r\n");
      break;
    case RACE_VAMPIRE:
      act("In a flash of smoke $n vanishes, and a bat flies off into the sky.",FALSE,ch,0,0,TO_ROOM);
      sendChar(ch, "You assume bat form and fly into the sky.\r\n");
      break;
    case RACE_DEMON:
      act("$n vanishes in a swirling mist.",FALSE,ch,0,0,TO_ROOM);
      sendChar(ch, "You vanish in a swirling mist.\r\n");
      break;
    default:
      mlog("SYSERR: seek_reallydopoof called for non flying race! ");
      return;
  }

  char_from_room(ch);
  char_to_room(ch,room);
  look_at_room(ch, 0);  /* For switched imms - Vex. */

  switch (GET_RACE(ch)) {
    case RACE_DRACONIAN:
    case RACE_SDRACONIAN:
      act("$n flies down from the sky.",FALSE,ch,0,0,TO_ROOM);
      break;
    case RACE_DRAGON:
      act("$n lands in front of you with a *THUMP*.",FALSE,ch,0,0,TO_ROOM);
      break;
    case RACE_VAMPIRE:
      act("You watch with interest as a small bat arrives. ",FALSE,ch,0,0,TO_ROOM);
      break;
    case RACE_DEMON:
      act("$n arrives in a swirling mist.",FALSE,ch,0,0,TO_ROOM);
      break;
    default:
      mlog("SYSERR: seek_reallydopoof/2 called for non flying race! ");
      return;
  }
}



RoomDirectionData *
otherside( CharData* ch, int dir )
{
  RoomDirectionData *back;
  int other_room;

  if( (other_room = EXIT(ch,dir)->to_room) != NOWHERE )
  {
    if( (back = world[other_room].dir_option[rev_dir[dir]]) )
    {
      if( back->to_room == ch->in_room )
      {
        return back;
      }
    }
  }

  return NULL;
}

void seek_mobopendoor(struct char_data *ch, int dir)
{
  struct room_direction_data *back;

  REMOVE_BIT(EXIT(ch,dir)->exit_info, EX_CLOSED);
  if (EXIT(ch, dir)->keyword)
    act("$n opens the $F.", FALSE, ch, 0, EXIT(ch,dir)->keyword,TO_ROOM);
  else
    act("$n open the door.", FALSE, ch, 0, 0, TO_ROOM);
  /* and the other side...*/
  back = otherside(ch,dir);
  if( back != NULL && back->to_room == ch->in_room ){
    REMOVE_BIT(back->exit_info, EX_CLOSED);
    if (back->keyword) {
      sprintf(buf, "The %s is opened from the other side.\r\n",
        fname(back->keyword));
      send_to_room(buf,EXIT(ch,dir)->to_room);
    } else
      send_to_room("The door is opened from the other side.\r\n",
        EXIT(ch,dir)->to_room);
  }
}

/* assumes the mob DOES have the key */
void
seek_mobunlockdoor( CharData* ch, int dir )
{
  RoomDirectionData *back;

  REMOVE_BIT(EXIT(ch,dir)->exit_info, EX_LOCKED);

  if (EXIT(ch, dir)->keyword)
    act("$n unlocks the $F.", FALSE, ch, 0, EXIT(ch,dir)->keyword,TO_ROOM);
  else
    act("$n unlocks the door.", FALSE, ch, 0, 0, TO_ROOM);

  send_to_room("*Click*\r\n",ch->in_room);

  /*
  ** and the other side...
  */
  back = otherside(ch,dir);

  if( back != NULL &&  back->to_room == ch->in_room )
  {
    REMOVE_BIT(back->exit_info, EX_LOCKED);
    send_to_room("*Click*\r\n",EXIT(ch,dir)->to_room);
  }
}

/* handle the messages associated with a door */
void
seek_unlockmsg( CharData* ch, char *here, char *there, int dir, int openit )
{
  RoomDirectionData *back;
  char bufx[255];

  REMOVE_BIT(EXIT(ch,dir)->exit_info, EX_LOCKED);

  if( openit ) REMOVE_BIT(EXIT(ch,dir)->exit_info, EX_CLOSED);

  /*
  ** Lets see if we  can let a switched imm know whats  going on.
  */
  if (EXIT(ch, dir)->keyword)
  {
    act(here, FALSE, ch, 0, EXIT(ch,dir)->keyword,TO_ROOM);
    act(here, FALSE, ch, 0, EXIT(ch,dir)->keyword,TO_CHAR);
  }

  else
  {
    act(here, FALSE, ch, 0, "door", TO_ROOM);
    act(here, FALSE, ch, 0, "door", TO_CHAR);
  }

  /*
  ** and the other side...
  */
  back = otherside( ch, dir );
  if( back != NULL && back->to_room==ch->in_room )
  {
    REMOVE_BIT(back->exit_info, EX_LOCKED);
    if (openit) REMOVE_BIT(back->exit_info, EX_CLOSED);
    if (back->keyword) {
      sprintf(bufx,there,back->keyword);
      send_to_room(bufx,EXIT(ch,dir)->to_room);
    } else {
      sprintf(bufx,there,"door");
      send_to_room(bufx,EXIT(ch,dir)->to_room);
    }
  }
}

/* assign a new target to ch */
void seek_newtarget(struct char_data *ch)
{
  char bufx[255];
  char target[MAX_INPUT_LENGTH];
  char *realtarget;
  int dotmode;
  struct char_data *oldhunt,*firstmob,*amob;
  int location,tries,mobcount,i,found;
  memory_rec *names;

  oldhunt=SEEKING(ch);

  /* first things first, are we PStalking anyone? */
  if (SEEK_TARGETSTR(ch)!=NULL) {
    /* Yes, we are, so reprocess the string.... */

    strcpy(target,SEEK_TARGETSTR(ch));
    dotmode=ynnek_find_dots(target,&realtarget);

    /*DEBUGGING*/
    sprintf(bufx,"Stalkdot = %d; Stalker = [%s]; RealStalker = [%s]\r\n",dotmode,target,realtarget);
    seek_debug(ch,bufx,DEBUG_THINK);

    mobcount=0;
    location=-1;
    if (dotmode==YDOTS_NONE)
    {
      seek_debug(ch,"Looking in room first\r\n",DEBUG_THINK);
      location=ch->in_room;
      firstmob = seek_gettarget(dotmode,realtarget,NULL,&mobcount,ch,location);
      if (mobcount==0) {dotmode=YDOTS_ALLDOT;location=-1;}
    }
    if (mobcount==0) firstmob = seek_gettarget(dotmode,realtarget,NULL,&mobcount,ch,-1);

    if (mobcount>0) {
      sprintf(bufx,"Valid targets: %d\r\n",mobcount);
      seek_debug(ch,bufx,DEBUG_THINK);
    }
    else {
      seek_debug(ch,"No valid targets found!\r\n",DEBUG_THINK);
    }

    if (mobcount!=0) {
      sprintf(bufx, "First valid target: %s\r\n",GET_NAME(firstmob));
      seek_debug(ch,bufx,DEBUG_THINK);
      tries=1;
      do {
        tries++;
        SEEKING(ch) = seek_gettarget(dotmode,realtarget,firstmob,&mobcount,ch,location);
      } while (tries<=5 && SEEKING(ch)==ch);
      /*SEEK_STATUS(ch)=SEEK_IDLE;*/
      if (SEEKING(ch) == ch)
        SEEKING(ch)=oldhunt;
      else {
        sprintf(bufx,"I think I'll go eat %s.\r\n",GET_NAME(SEEKING(ch)));
        do_say(ch,bufx,0,0);
        return;
      }
    }
  }
  /* do we autoseek memory? */
  if (oldhunt==SEEKING(ch) && !MOB_FLAGGED(ch, MOB_SEEKTORMENTOR) && 
      MOB_FLAGGED(ch,MOB_MEMORY)) {
    /* Why yes, we do... */
    /* So how many targets we have in memory */
    for (mobcount=0,names=MEMORY(ch); names; names=names->next,mobcount++) ;
    if (mobcount>0) {
      mobcount=number(1,mobcount);
      for (i=1,names=MEMORY(ch);names && i!=mobcount;names=names->next,mobcount++);
      found=0;
      for (amob=character_list;amob && !found; amob=amob->next) {
        if(( names != NULL ) && ( names->id == GET_IDNUM(amob))){
          found=1;
          SEEKING(ch) = amob;
        }
      }
    }
  }
}



