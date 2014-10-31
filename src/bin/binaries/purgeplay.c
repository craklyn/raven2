
/* ************************************************************************
*  file: purgeplay.c                                    Part of CircleMUD * 
*  Usage: purge useless chars from playerfile                             *
*  All Rights Reserved                                                    *
*  Copyright (C) 1992, 1993 The Trustees of The Johns Hopkins University  *
************************************************************************* */

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/class.h"
#include "util/utils.h"

#undef PURGE_GOLD
#undef PURGE_CLANS
#undef RENAME_A_CHAR
#undef SKILL_XFER
#undef CLAMP_EXP

#define PURGE_TIMEOUT
#define GOLD_PER_LEVEL 750000

typedef struct
{
  char* immortal_name;
  int   immortal_lvl;
} immortal_entry;

immortal_entry imm_list[] = {

/* 60 - LVL_IMMORT */
    { "Xiuhtecuhtli", LVL_IMPL}, /* + 01/27/09    */
    { "Arbaces",    LVL_IMPL },  /* ~ 01/01/95    */
/* 59 - LVL_GRGOD  */
    { "Enki",	    LVL_GRGOD  },  /* + 10/01/00 H. */
/* 58 - LVL_GOD         */
    { "Caleb", LVL_GOD}, /* + 01/27/09    */
    /*    { "Maestro", LVL_GOD},  + 01/27/09    */
/* 57 - LVL_LRGOD   */
/* 56 - LVL_DGOD     */
    { "Rashane", LVL_DGOD}, /* + 09/23/09    */
    { "Fenrir", LVL_DGOD}, /* + 06/09/09    */
/* 55 - LVL_CREATOR     */
    { "Vlad",    LVL_CREATOR     },  /* + 01/27/09    */
/* 54 - LVL_DEITY       */
/* 53 - LVL_ANGEL       */
    { "Maitreya",  LVL_ANGEL        },  /* + 01/27/09 */
/* 52 - LVL_SAINT       */
/* 51 - LVL_HERO        */
    { "", 0 }
};

void
swapSkills( struct char_file_u *plr, int skill1, int skill2 )
{
  plr->player_specials_saved.skills[skill2]      =
  plr->player_specials_saved.skills[skill1];

  plr->player_specials_saved.skill_usage[skill2] =
  plr->player_specials_saved.skill_usage[skill1];

  plr->player_specials_saved.skills[skill1]      = 0;
  plr->player_specials_saved.skill_usage[skill1] = 0;

  printf( "---moved skill %d level %d to skill %d\n", skill1,
  plr->player_specials_saved.skills[skill2], skill2 );
}

void
removeSkill( struct char_file_u *plr, int the_skill)
{
  if((plr->player_specials_saved.skills[the_skill] > 0) &&
     (plr->level < LVL_IMMORT))
  {
    plr->player_specials_saved.skills[the_skill] = 0;
    plr->player_specials_saved.skill_usage[the_skill] = 0;
    plr->player_specials_saved.spells_to_learn += 5;
    printf( "---removed skill %d\n", the_skill);
  }
}


void
purge( char* filename )
{
  FILE* fl;
  FILE* outfile;

  int   num = 0;
  long  timeout;
  char* ptr;

  if( !(fl = fopen(filename, "r+")))
  {
    printf("Can't open %s.", filename);
        exit(1);
  }

  outfile = fopen( "players.new", "w" );

  for(;;)
  {
    char reason[80] = "";
    int  okay       = 1;
    int  guarded    = 0;

    struct char_file_u player;

    fread( &player, sizeof(struct char_file_u ), 1, fl );

    if( feof(fl) )
    {
      fclose(fl);
      fclose(outfile);
      puts("Done.");
      exit(0);
    }

    //printf( "Reviewing %s level %d\n", player.name, player.level );

    for( ptr = player.name; *ptr; ptr++ )
    {
      if( !isalpha(*ptr) || *ptr == ' ' )
      {
        strcpy( reason, "Invalid name" );
        okay = 0;
      }
    }

    if( player.level == 0 )
    {
      strcpy( reason, "Never entered game" );
      okay = 0;
    }

    if( (player.level < 1) || (player.level > LVL_IMPL) )
    {
      char buf[80];
      sprintf( buf, "Invalid level [%d]", player.level );
      strcpy( reason, buf );
      player.level = 1;
    }

    // Validate the IMMs
    //
    if( player.level >= LVL_IMMORT )
    {
      immortal_entry *imm_ptr = imm_list;
      int valid_immort = 0;

      //printf( "Verifying %s at level %d.\n", player.name, player.level );

      while( imm_ptr->immortal_lvl > 0 )
      {
        if( strcmp( player.name, imm_ptr->immortal_name ) == 0 )
        {
          if( player.level != imm_ptr->immortal_lvl )
          {
            printf( "Immortal %s adjusted from level %d to %d.\n",
                     imm_ptr->immortal_name, player.level,
                     imm_ptr->immortal_lvl );
            player.level = imm_ptr->immortal_lvl;
          }

          valid_immort = 1;
        }

        imm_ptr++;
      }

      if( !valid_immort )
      {
        printf( "IMMORTAL NUKED -> %s\n", player.name );
        okay = 0;
      }
    }

    // Make certain the following two clowns always end up on top.
    //
    if( strcmp( player.name, "xiuhtecuhtli" ) == 0 )
    {
      player.level = LVL_IMPL;
    }

# if RENAME_A_CHAR
    // Rename a character.
    //
    if( strcmp( player.name, "OldName" ) == 0 )
    {
      printf( "Renaming %s ", player.name );
      strcpy( player.name, "NewName" );
      printf( "to %s\n", player.name );
    }
# endif

    if( okay )
    {

# ifdef PURGE_TIMEOUT
      if( IS_SET_AR( player.char_specials_saved.act, PLR_CRYO ))
      {
        timeout = 180; // Char in Cryogenic State
        guarded = 1;
      }
      else if (player.level <= 1)        // Lev 1: 5 days
      {
          timeout = 5;
      }
      else if (player.level <= 10 )      // Lev 2-10: 30 days
      {
          timeout = 30;
      }
            else if (player.level <= 25) // Lev 11-35: 180 days
      {
        timeout = 180;
                // guarded = 1;
      }
      else if( player.level <= 45 )      // Lev 36-45: 180 days
      {
        timeout = 180;
        guarded = 1;
      }
      else if( player.level < LVL_IMMORT ) // Lev 46-50: 360 days
      {
        timeout = 360;
        guarded = 1;
      }
      else
      {
        timeout = 365;
                //    guarded = 1;
      }
# endif

      timeout *= SECS_PER_REAL_DAY;

      if( (time(0) - player.last_logon) > timeout )
      {
        okay = 0;
        if( guarded )
        {
          okay = 1;
          sprintf( reason, "Level %2d idle for %3d days - GUARDED",
                   player.level,
                  (int)((time(0) - player.last_logon) / SECS_PER_REAL_DAY));
        }
        else if( player.level > 0 )
        {
          sprintf( reason, "Level %2d idle for %3d days", player.level,
                  (int)((time(0) - player.last_logon) / SECS_PER_REAL_DAY));
        }


          if (IS_SET_AR(player.char_specials_saved.act, PLR_DELETED))
          {
            sprintf( reason, "Level %2d SELF-DELETED", player.level );
            okay = 0;
          }

        if( IS_SET_AR(player.char_specials_saved.act, PLR_SHUNNED))
        {
          sprintf( reason, " *** SHUNNED purging! ***" );
          okay = 0;
        }
      }

# ifdef LOG_EXCESSIVE_GOLD
      if( player.level <= LVL_IMMORT )
      {
        if( player.points.gold > ( player.level * GOLD_PER_LEVEL ))
        {
          printf( "Clamping %s gold(%d)?\n", player.name,
                                             player.points.gold );
# ifdef PURGE_GOLD
          player.points.gold = player.level * GOLD_PER_LEVEL;
# endif
        }

        if( player.points.bank_gold > ( player.level * GOLD_PER_LEVEL ))
        {
          printf( "Clamping %s bank acct(%d)?\n", player.name,
                                                  player.points.bank_gold );
# ifdef PURGE_GOLD
          player.points.bank_gold = player.level * GOLD_PER_LEVEL;
# endif
        }
      }
# endif

# ifdef PURGE_CLANS
      // Reset the clan settings
      //
      if( player.player_specials_saved.clan_id != 0 )
        printf( "Clearing the clan setting for %s \n", player.name );

      player.player_specials_saved.clan_id         = 0;
      player.player_specials_saved.clan_level      = 0;
      player.player_specials_saved.clan_experience = 0;
      player.player_specials_saved.clan_donations  = 0;
# endif

# ifdef SKILL_XFER
      // Xfer skills
      //
      switch( player.class )
      {
        case CLASS_RANGER:
          break;
        case CLASS_DEATH_KNIGHT:
          break;
        case CLASS_SOLAMNIC_KNIGHT:
          break;
        case CLASS_MAGIC_USER:
          //swapSkills(&player, 25, 76);
          break;
        case CLASS_CLERIC:
          break;
        case CLASS_SHADOW_DANCER:
          break;
        case CLASS_ASSASSIN:
          break;
        default:
          break;
      }
# endif

#define CLAMP_LOSTLEVELS
#ifdef CLAMP_LOSTLEVELS
      player.lostlevels = 0;
#endif

#ifdef CLAMP_EXP
      /* anyone below -90% exp gets set to -90% exp */
      if (player.level < 51) {
          long thisLvl = (titles[player.class][player.level + 0].exp);
          long nextLvl = (titles[player.class][player.level + 1].exp);
          long neg90 = thisLvl - (nextLvl - thisLvl) * 9 / 10;
          if (player.points.exp < neg90) {
            printf("%s has below -90%% exp - CLAMPED\n", player.name);
            player.points.exp = neg90;
          }
      }
#endif

      if( okay )
      {
        fwrite(&player, sizeof(struct char_file_u ), 1, outfile);
      }

      else if( player.level > 0 )
      {
        printf("%4d) deleted %-20s %s\n", ++num, player.name, reason);
      }

      if( okay == 2 )
      {
        fprintf(stderr, "%-20s %s\n", player.name, reason);
      }
    }
  }
}


int
main( int argc, char** argv )
{
  if (argc != 2)
    printf("Usage: %s playerfile-name\n", argv[0]);
  else
    purge(argv[1]);
  return 0;
}


