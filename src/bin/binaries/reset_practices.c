
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
#include "magic/spells.h"
#include "util/utils.h"

#undef PURGE_GOLD
#undef PURGE_CLANS
#undef RENAME_A_CHAR
#undef SKILL_XFER
#undef CLAMP_EXP

void reset_practices(struct char_file_u *player);
void race_and_class_skills(struct char_file_u *plr);

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

    printf( "Reviewing %s.\n", player.name);

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


    if( okay )
    {
        reset_practices(&player);

        if( okay )
        {
            fwrite(&player, sizeof(struct char_file_u ), 1, outfile);
        }
        else if( player.level > -10 )
        {
            printf("%4d) deleted %-20s %s\n", ++num, player.name, reason);
        }

        if( okay == 2 )
        {
            fprintf(stderr, "%-20s %s\n", player.name, reason);
        }
    }
  } /* for each player */
  
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


// This function will reset all players' skills to 0.  It will calculate approximately
// how many practices the player deserves, and return quest points to players who exceed
// that amount.
void reset_practices(struct char_file_u *player) {
    int i;
    int debug = 0;

    /*
     * First, we calculate how many practices a player deserves based on their level.
     * Players get at least 2 practices for the first level...
     */
    if(strcmp( player->name, "Reid") == 0)
        debug = 0;

    int practicesDeserved = 0;

    if(debug) {
        printf( "Character's numeric race is %d\n", player->race);
        printf( "The maximum wisdom of this race is %d\n", (int)race_stat_limits[(int)player->race]);
    }

    practicesDeserved += 5*wis_app[(int)race_stat_limits[(int)player->race][WISDOM_INDEX]].bonus;
    if(debug)
        printf( "First level practices a): %d\n", practicesDeserved);
    practicesDeserved += wis_app[(int)race_stat_limits[(int)player->race][WISDOM_INDEX]].extra;
    if(debug)
        printf( "First level practices b): %d\n", practicesDeserved);
    practicesDeserved = MAX(10, practicesDeserved); // a value of 10 cooresponds with 2 practices

    // Now give them all subsequent levels.
    for(i = 1; i < player->level; i++) {
        practicesDeserved += 5*wis_app[(int)race_stat_limits[(int)player->race][WISDOM_INDEX]].bonus;
        if(debug)
            printf( "a) Practices after level %d: %d\n", i+1, practicesDeserved);
        practicesDeserved += wis_app[(int)race_stat_limits[(int)player->race][WISDOM_INDEX]].extra;
        if(debug)
            printf( "b) Practices after level %d: %d\n", i+1, practicesDeserved);
    }
    // Now reduce this number by a factor of 5 to get the actual practices deserved.
    practicesDeserved /= 5;

    /*
     * Calculate how much a character learns from practicing once with maximum racial intelligence.
     * Keep track of the total practices a player used on skills, then reset their skills.
     */
// These are a bunch of defined used when practicing skills.  This is bad for code maintenance...
#define MINGAIN(player) (prac_params[MIN_PER_PRAC][(int)player->class])
#define MAXGAIN(player) (prac_params[MAX_PER_PRAC][(int)player->class])
#define INT_LEARNING_RATIO 150.0
#define MAX_PER_PRAC	1	/* max percent gain in skill per practice */
#define MIN_PER_PRAC	2	/* min percent gain in skill per practice */
    int skill_delta = (int)(((float)race_stat_limits[(int)player->race][INTELLIGENCE_INDEX]/(float)INT_LEARNING_RATIO)*100.0);
    skill_delta = MIN(MAXGAIN(player), MAX(MINGAIN(player), skill_delta ));

    int practicesUsed = 0;

    practicesUsed += player->player_specials_saved.spells_to_learn;
    for (i = 0; i <= MAX_SKILLS; i++) {
        practicesUsed += MIN(MAX_PRACTICE_LEVEL, player->player_specials_saved.skills[i]) / skill_delta;
        if(MIN(MAX_PRACTICE_LEVEL, player->player_specials_saved.skills[i]) % skill_delta)
            practicesUsed += 1;

        player->player_specials_saved.skills[i] = 0;
        player->player_specials_saved.skill_usage[i] = 0;
    }
 
    // Give them the skills that every player of that class gets
    race_and_class_skills(player);

    if( TRUE ||
        strcmp( player->name, "Zechariah" ) == 0 ||
        strcmp( player->name, "Taufiq") == 0 ||
        strcmp( player->name, "Reid") == 0)
    {
        printf( "Information regarding %s:\n", player->name);
        printf( "Practices deserved: %d  Apparent practices used: %d\n", practicesDeserved, practicesUsed);
        printf( "QP reward: %d\n", 5 * MAX((practicesUsed - practicesDeserved), 0));
    }
    
    // Give them 5 QPs for each practice they have that outnumbers the number they deserve
    player->player_specials_saved.quest_pts += 5 * MAX((practicesUsed - practicesDeserved), 0);
    player->player_specials_saved.spells_to_learn = practicesDeserved;
}

void race_and_class_skills(struct char_file_u *plr) {

    switch (plr->class) {
        case CLASS_THIEF:
            plr->player_specials_saved.skills[SKILL_HIDE] = 10;
            plr->player_specials_saved.skills[SKILL_STEAL] = 15;
            plr->player_specials_saved.skills[SKILL_PICK_LOCK] = 30;
            break;
        case CLASS_RANGER:
            plr->player_specials_saved.skills[SKILL_SNEAK] = 10;
            plr->player_specials_saved.skills[SKILL_HIDE] = 15;
            plr->player_specials_saved.skills[SKILL_TRACK] = 20;
            break;
        case CLASS_MAGIC_USER:
            plr->player_specials_saved.skills[SPELL_MAGIC_MISSILE] = 30;
            break;
        case CLASS_CLERIC:
            plr->player_specials_saved.skills[SPELL_CURE_CRITIC] = 30;
            break;
        case CLASS_WARRIOR:
            plr->player_specials_saved.skills[SKILL_INVIGORATE] = 25;
            break;
        case CLASS_ASSASSIN:
            plr->player_specials_saved.skills[SKILL_BACKSTAB] = 25;
            break;
        case CLASS_SHOU_LIN:
            plr->player_specials_saved.skills[SKILL_SWEEP] = 10;
            plr->player_specials_saved.skills[SKILL_LAY_HANDS] = 10;
            break;
        case CLASS_SOLAMNIC_KNIGHT:
            plr->player_specials_saved.skills[SPELL_CURE_LIGHT] = 25;
            break;
        case CLASS_DEATH_KNIGHT:
            plr->player_specials_saved.skills[SPELL_CAUSE_WOUND] = 30;
            break;
        case CLASS_SHADOW_DANCER:
            plr->player_specials_saved.skills[SPELL_SHADOW_BLADES] = 15;
            plr->player_specials_saved.skills[SPELL_SHADOW_SPHERE] = 10;
            break;
        case CLASS_NECROMANCER:
            plr->player_specials_saved.skills[SPELL_LIFE_DRAIN] = 15;
            plr->player_specials_saved.skills[SPELL_BLINDNESS] = 10;
            break;
        default:
            break;
    }

    switch (plr->race) {
        case RACE_MINOTAUR:
            plr->player_specials_saved.skills[SKILL_GORE] = 60;
            break;
        case RACE_DRACONIAN:
            plr->player_specials_saved.skills[SKILL_BREATHE] = 60;
            break;
        case RACE_DEMON:
            plr->player_specials_saved.skills[SKILL_MIST] = 60;
            break;
        case RACE_IZARTI:
            plr->player_specials_saved.skills[SKILL_CALM] = 60;
            break;
        case RACE_VAMPIRE:
            plr->player_specials_saved.skills[SKILL_FEED] = 60;
            break;
        case RACE_AMARA:
            plr->player_specials_saved.skills[SKILL_STING] = 60;
            break;
        case RACE_ELEMENTAL:
            switch (plr->player_specials_saved.sub_race) {
                case FIRE_ELEMENTAL:
                    plr->player_specials_saved.skills[SPELL_WALL_OF_FIRE] = 60;
                    break;
                case AIR_ELEMENTAL:
                    plr->player_specials_saved.skills[SPELL_TYPHOON] = 60;
                    break;
                case EARTH_ELEMENTAL:
                    plr->player_specials_saved.skills[SPELL_TREMOR] = 60;
                    break;
                case WATER_ELEMENTAL:
                    plr->player_specials_saved.skills[SPELL_TSUNAMI] = 60;
                    break;
            }
            break;
        case RACE_UNDEAD:
            plr->player_specials_saved.skills[SPELL_TERROR] = 60;
            break;
        default:
            break;
    }
}
