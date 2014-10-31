
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/comm.h"
#include "actions/interpreter.h"
#include "util/utils.h"
#include "specials/special.h"
#include "general/class.h"
#include "actions/act.clan.h"
#include "specials/guard.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */

SPECIAL(guild_guard)
{
  static char *to_room   = "The guard humiliates $n, and blocks $s way.";

  static char *guard_actions[] = {
    "grabs you by the collar",
    "grabs your ear and twists",
    "steps in front of you",
    "trips you up",
    "blocks your path",
    "gives you a wedgie and takes your lunch money",
    "punches you in the mouth and sends you reeling",
    "grabs your buttcheek and makes you think twice",
    "shoves you up against the wall"
  };

  /*static guarded_exit_t clanPaths[] = {
    {  19310, CMD_SOUTH, CLAN_LOTUS,       "Undertaker"   },
    {  19320, CMD_NORTH, CLAN_CURSED,      "Kabal"        },
    {  19330, CMD_NORTH, CLAN_OCCULT,      "King Mob"     },
    {  19340, CMD_NORTH, CLAN_COVEN,       "Tikbalang"    },
    {   3528, CMD_SOUTH, CLAN_WANDERERS,   "Spot"         },
    {  19353, CMD_SOUTH, CLAN_WANDERERS,   "Fluffy"       },
    {  19355, CMD_UP,    CLAN_WANDERERS,   "flowing darkness"        },
    {  19370, CMD_NORTH, CLAN_BRETHREN,    "William"      },
    {  19380, CMD_NORTH, CLAN_SHRINERS,    "Bodie"        },
    {  19301, CMD_NORTH, CLAN_FORSAKEN,    "guardian"     },
    {  19802, CMD_DOWN,  CLAN_GENESIS,     "Jeno"         },
	{  19317, CMD_NORTH, CLAN_JUSTICE,     "Calcifer"     },
	{  19305, CMD_EAST, CLAN_FORSAKEN,     "Caleb"        },
    {  0, 0, 0 }
  };*/

  static guarded_exit_t guild_paths[] = {

    {  1450, CMD_EAST,  CLASS_SOLAMNIC_KNIGHT, "knight"            },

  /* Midgaard Guilds */
    {  3004, CMD_NORTH, CLASS_CLERIC,          "knight templar"    },
    {  3017, CMD_NORTH, CLASS_MAGIC_USER,      "sorcerer"          },
    {  3021, CMD_SOUTH, CLASS_WARRIOR,         "knight"            },
    {  3027, CMD_SOUTH, CLASS_THIEF,           "assassin"          },
    {  3074, CMD_NORTH, CLASS_RANGER,          "ranger lord"       },
    {  3076, CMD_NORTH, CLASS_DEATH_KNIGHT,    "unholy guardian"   },
    {  3072, CMD_SOUTH, CLASS_SOLAMNIC_KNIGHT, "knight"            },
    {  3082, CMD_SOUTH, CLASS_SHADOW_DANCER,   "myrddraal"         },
    {  3080, CMD_NORTH, CLASS_ASSASSIN,        "butcher"           },
    {  3078, CMD_SOUTH, CLASS_SHOU_LIN,        "wise old man"      },

  /* Shaden */
    { 11522, CMD_EAST,  CLASS_THIEF,           "guard"             },

  /* Thieves Guild */
    { 12516, CMD_EAST,  CLASS_THIEF,           "tough guard"       },

  /* Samsera's Guilds */
    { 18104, CMD_SOUTH, CLASS_WARRIOR,         "warrior champion"  },
    { 18101, CMD_UP,    CLASS_RANGER,          "ranger lord"       },
    { 18101, CMD_DOWN,  CLASS_RANGER,          "ranger lord"       },
    { 18107, CMD_SOUTH, CLASS_SHOU_LIN,        "wise old man"      },
    { 18113, CMD_NORTH, CLASS_CLERIC,          "knight templar"    },
    { 18116, CMD_NORTH, CLASS_DEATH_KNIGHT,    "unholy guardian"   },
    { 18110, CMD_EAST,  CLASS_SOLAMNIC_KNIGHT, "knight"            },
    { 18122, CMD_UP,    CLASS_MAGIC_USER,      "sorcerer"          },
    { 18131, CMD_DOWN,  CLASS_THIEF,           "mercenary"         },
    { 18128, CMD_DOWN,  CLASS_SHADOW_DANCER,   "myrddraal"         },
    { 18134, CMD_DOWN,  CLASS_ASSASSIN,        "butcher"           },
    { 38501, CMD_NORTH, CLASS_DEATH_KNIGHT,    "Crypt Dragon"      },
  /* The Forgotten City Guilds */
    { 21208, CMD_WEST,  CLASS_THIEF,           "ninja"             },
    { 21216, CMD_SOUTH, CLASS_WARRIOR,         "animated armor"    },
    { 21225, CMD_EAST,  CLASS_CLERIC,          "spectre"           },
    { 21233, CMD_NORTH, CLASS_MAGIC_USER,      "young mage"        },
  /* Fairhaven */
    { 30725, CMD_UP,    CLASS_THIEF,           "Thug"              },
    { 30773, CMD_UP,    CLASS_SHADOW_DANCER,   "Shadow Dancer"     },
    { 30743, CMD_NORTH, CLASS_WARRIOR,         "Warrior"           },
    { 30747, CMD_NORTH, CLASS_RANGER,          "Ranger"            },
    { 30751, CMD_EAST,  CLASS_MAGIC_USER,      "Sorcerer"          },
    { 30739, CMD_WEST,  CLASS_CLERIC,          "Knight Templar"    },
    { 30888, CMD_NORTH, CLASS_ASSASSIN,        "Assassin"          },
  /* Necromancer Guilds */
    { 39001, CMD_WEST,  CLASS_NECROMANCER,     "necromancer"       },
  /* Necropolis */
	{ 43945, CMD_EAST, CLASS_NECROMANCER,     "Writhing Mass"     },
  /* End Of Guarded Paths */
    {  0, 0, 0 }
  };

  static guarded_exit_t level_paths[] = {
  /* Digger's Stuff */
    {  1230,  CMD_DOWN,  LVL_IMMORT,    "Xi-Ssxin"            },
  /* The Entrance to the Chessboard */
    {  1699,  CMD_WEST,      -10,     "King of Corinth"     },
  /* The East Gate of Midgaard */
    {  3053,  CMD_WEST,      -15,     "Gergory the Watcher" },
  /* Asgard */
    {  2859, CMD_UP,    LVL_IMMORT,     "Heimdall"            },
    {  3703, CMD_NORTH, LVL_IMMORT,     "The Red Super Giant" },
    {  3703, CMD_WEST,  LVL_IMMORT,     "The Red Super Giant" },
    {  3703, CMD_EAST,  LVL_IMMORT,     "The Red Super Giant" },
    {  3770, CMD_SOUTH, LVL_IMMORT,     "Ursa Major" },

  /* Tuatha */
    {  3832,  CMD_NORTH, LVL_IMMORT,    "Sentinel of Lir"   },
    {  3867,  CMD_WEST,  LVL_IMMORT,    "Bansidhe"          },
    {  3869,  CMD_WEST,  LVL_IMMORT,    "Bansidhe"          },
  /* Mystic Sea - Level 1 */
    {  3911,  CMD_SOUTH, LVL_IMMORT,    "Sapphire Guardian" },
    {  3917,  CMD_NORTH, LVL_IMMORT,    "Sapphire Guardian" },
    {  3927,  CMD_EAST,  LVL_IMMORT,    "Sapphire Guardian" },
    {  3930,  CMD_WEST,  LVL_IMMORT,    "Sapphire Guardian" },
  /* Mystic Sea - Level 2*/
    {  3941,  CMD_EAST,  LVL_IMMORT,    "Emerald Guardian"  },
    {  3949,  CMD_NORTH, LVL_IMMORT,    "Emerald Guardian"  },
    {  3963,  CMD_NORTH, LVL_IMMORT,    "Emerald Guardian"  },
    {  3964,  CMD_EAST,  LVL_IMMORT,    "Emerald Guardian"  },
  /* Mystic Sea - Level 3*/
    {  3998,  CMD_SOUTH, LVL_IMMORT,    "Obsidian Guardian" },
    {  3993,  CMD_EAST,  LVL_IMMORT,    "Obsidian Guardian" },
    {  3988,  CMD_NORTH, LVL_IMMORT,    "Obsidian Guardian" },
    {  3985,  CMD_NORTH, LVL_IMMORT,    "Obsidian Guardian" },
  /* Astral Plane */
    {  4200,  CMD_NORTH, LVL_IMMORT,    "Astral Guardian"   },
    {  4276,  CMD_NORTH, LVL_IMMORT,    "Githyanki Protector"   },
  /* The Brass Dragon */
    {  5065,   CMD_WEST, LVL_IMMORT,    "Brass Dragon"      },
  /* Shaden */
    { 11522,  CMD_EAST,  25,          "guard"             },
  /* Black Hand */
    { 12627,  CMD_DOWN,  LVL_IMMORT,    "Morgoth"           },
  /* Astral Extensions */
    { 13359, CMD_SOUTH,  LVL_IMMORT,	   "Githyanki Knight"},
    { 13386, CMD_SOUTH,  LVL_IMMORT,	   "Githyanki Knight"},
    { 13446, CMD_UP,     LVL_IMMORT,     "Highlord" },
    { 13446, CMD_EAST,   LVL_IMMORT,     "Highlord" },
  /* The War Camp */
    { 13956, CMD_UP,   LVL_IMMORT, "The Elite Sentry"	},
  /* Xerxes */
    { 14101,  CMD_NORTH, LVL_IMMORT,    "guard"             },
    { 14634,  CMD_NORTH, LVL_IMMORT,    "guard"             },
    { 14634,  CMD_SOUTH, LVL_IMMORT,    "guard"             },
    { 14707,  CMD_EAST,  LVL_IMMORT,    "guard"             },
    { 14764,  CMD_EAST,  LVL_IMMORT,    "guard"             },
    { 14818,  CMD_UP,    LVL_IMMORT,    "guard"             },
  /* The Tombs of Tarin */
    { 15684, CMD_NORTH,  LVL_IMMORT,    "huge ancient tree" },

  /* The Isle of Mysts */
    { 16261, CMD_NORTH, LVL_IMMORT,	  "the guardian" },

  /* The Jungle */
    { 16486, CMD_DOWN,   LVL_IMMORT,    "soldier ant"       },
  /* Avalon */
    { 22301,  CMD_NORTH, LVL_IMMORT,    "Design Guardian"   },
    { 22302,  CMD_NORTH, LVL_IMMORT,    "Vortex"            },
    { 22307,  CMD_UP,    LVL_IMMORT,    "Canth Guard"       },
    { 22309,  CMD_NORTH, LVL_IMMORT,    "Bwoine"            },
  /* End of Guard List */
    {  0, 0, 0 }
  };

  int guard_id = FALSE;
  char to_victim[ 132 ];
  char *guard_name = NULL;

  // Just because we're a guard doesn't mean we're dumb.
  //
  smart_NPC_combat(ch);

  if( CMD_IS( "enter" ) ) return TRUE;  // Just eat enter

  if( cmd > CMD_DOWN || cmd < CMD_NORTH ) return FALSE;

  if(( guard_id = exitIsGuarded( guild_paths, GUARD_CLASS, cmd, ch )) >= 0 )
  {
    sprintf( to_victim,
            "The %s %s, 'No admittance, SCUM!'\n\r",
             guild_paths[ guard_id ].guard_name,
             guard_actions[ number( 0, 8 ) ] );

    act( to_room, FALSE, ch, 0, 0, TO_ROOM );
    sendChar( ch, to_victim );
    return TRUE;
  }

  if(( guard_id = exitIsGuarded( level_paths, GUARD_LEVEL, cmd, ch )) >= 0 )
  {
    sprintf( to_victim,
            "The %s %s, 'No admittance, SCUM!'\n\r",
             level_paths[ guard_id ].guard_name,
             guard_actions[ number( 0, 8 ) ] );

    act( to_room, FALSE, ch, 0, 0, TO_ROOM );
    sendChar( ch, to_victim );
    return TRUE;
  }/* if */
  
  if(( guard_name = exitIsClanGuarded(cmd, ch)) != NULL )
  {
    sprintf( to_victim,
            "%s %s, 'No admittance, SCUM!'\n\r", guard_name,
             guard_actions[number(0, 8)]);

    act( to_room, FALSE, ch, 0, 0, TO_ROOM );
    sendChar( ch, to_victim );
    return TRUE;
  }/* if */

  return FALSE;
}


