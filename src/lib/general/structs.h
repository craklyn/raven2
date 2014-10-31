/* ************************************************************************
 *   File: structs.h                                     Part of CircleMUD *
 *  Usage: header file for central structures and contstants               *
 *                                                                         *
 *  All rights reserved.  See license.doc for complete information.        *
 *                                                                         *
 *  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
 *  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
 ************************************************************************ */

#ifndef __STRUCTS_H__
#define __STRUCTS_H__

/* room-related defines *************************************************
 * The cardinal directions: used as index to room_data.dir_option[] *****/
#define NORTH          0
#define EAST           1
#define SOUTH          2
#define WEST           3
#define UP             4
#define DOWN           5

/* Room flags: used in room_data.room_flags */
/* WARNING: In the world files, NEVER set the bits marked "R" ("Reserved") */
#define ROOM_DARK            0    /* Dark          */
#define ROOM_DEATH           1    /* Death trap        */
#define ROOM_NOMOB           2    /* MOBs not allowed      */
#define ROOM_INDOORS	     3    /* Indoors			*/
#define ROOM_PEACEFUL	     4    /* Violence not allowed	*/
#define ROOM_SOUNDPROOF	     5    /* Shouts, gossip blocked	*/
#define ROOM_NOTRACK	     6    /* Track won't go through	*/
#define ROOM_NOMAGIC	     7    /* Magic not allowed		*/
#define ROOM_TUNNEL          8    /* room for only 1 pers  */
#define ROOM_PRIVATE	     9    /* Can't teleport in		*/
#define ROOM_GODROOM	     10   /* LVL_GOD+ only allowed	*/
#define ROOM_HOUSE           11   /* (R) Room is a house   */
#define ROOM_HOUSE_CRASH     12   /* (R) House needs saving	*/
#define ROOM_ATRIUM          13   /* (R) The door to a house   */
#define ROOM_OLC             14   /* (R) Modifyable/!compress  */
#define ROOM_BFS_MARK	     15   /* (R) breath-first srch mrk	*/
#define ROOM_FALL            16   /* If fly runs you fall          */
#define ROOM_MANA_REGEN      17   /*                           */
#define ROOM_HEALTH_REGEN    18   /*                           */
#define ROOM_SUFFER          19   /*                           */
#define ROOM_CLAN            20   /* Clan members only.        */
#define ROOM_NORECALL        21   /* Can't recall from this room. */
#define ROOM_FOG             22   /* Room is foggy             */
#define ROOM_ONE_PERSON	     23   /* One person per room	    */
#define ROOM_NO_DISASTER     24   /* No disasters will happen in here */
#define ROOM_DISASTER_FIREBALL 25 /* Room has random fireballs */
#define ROOM_DISASTER_LIGHTNING 26 /* Room has random lightning */
#define ROOM_DISASTER_EARTHQUAKE 27 /* Room has random earthquakes */
#define ROOM_DISASTER_WIND   28	/* Room has random wind *fart*/
#define ROOM_SMALL	     29 /* Small races only */
#define ROOM_HOT	     30 /* Suffer room */
#define ROOM_COLD	     31 /* Suffer room */
#define ROOM_DRY	     32 /* Suffer room */
#define ROOM_FAMILIAR        33 /* Recall Point Room */
#define ROOM_NORELOCATE      34 
#define ROOM_SALTWATER_FISH  35 /* Saltwater fish be here */
#define ROOM_FRESHWATER_FISH 36 /* Freshwater fish be here */
#define ROOM_DISRUPTIVE      37 /* magic doesn't work quite right here */
#define ROOM_POISONED        38 /* Room will poison occupants */
/* The total number of Room Flags */
#define NUM_ROOM_FLAGS	     39

/* Exit info: used in room_data.dir_option.exit_info */
#define EX_ISDOOR		(1 << 0)   /* Exit is a door		*/
#define EX_CLOSED		(1 << 1)   /* The door is closed	*/
#define EX_LOCKED		(1 << 2)   /* The door is locked	*/
#define EX_PICKPROOF    	(1 << 3)   /* Lock can't be picked  */
#define EX_TOCLAN       	(1 << 4)   /* The exit leads to a clan  */
#define EX_DOORBASHED   	(1 << 5)   /* The exit was bashed open  */
/* The total number of exit bits */
#define NUM_EXIT_BITS           6

/* Sector types: used in room_data.sector_type */
#define SECT_INSIDE          0		   /* Indoors			*/
#define SECT_CITY            1		   /* In a city			*/
#define SECT_FIELD           2		   /* In a field		*/
#define SECT_FOREST          3		   /* In a forest		*/
#define SECT_HILLS           4		   /* In the hills		*/
#define SECT_MOUNTAIN        5		   /* On a mountain		*/
#define SECT_WATER_SWIM      6		   /* Swimmable water		*/
#define SECT_WATER_NOSWIM    7		   /* Water - need a boat	*/
#define SECT_UNDERWATER	     8		   /* Underwater		*/
#define SECT_FLYING          9         	   /* Wheee!            */
#define SECT_UNDERWATER_RIVER   10   /* Underwater River        */
#define SECT_CORPSE_ROOM        11   /* Corpse Room             */
#define SECT_ROAD               12   /* Road                    */
#define SECT_PLAIN              13   /* Plain                   */
#define SECT_ROCKY              14   /* Rocky                   */
#define SECT_MUDDY              15   /* Muddy                   */
#define SECT_SAND               16   /* Sand                    */
#define SECT_LIGHT_FOREST       17   /* Light Forest            */
#define SECT_THICK_FOREST       18   /* Thick Forest            */
/* The total number of room Sector Types */
#define NUM_ROOM_SECTORS     19

/* char and mob-related defines *****************************************/

/* Sex */
#define SEX_NEUTRAL   0
#define SEX_MALE      1
#define SEX_FEMALE    2
#define NUM_GENDERS   3

/* Positions */
#define POS_DEAD       0	/* dead			*/
#define POS_MORTALLYW  1	/* mortally wounded	*/
#define POS_INCAP      2	/* incapacitated	*/
#define POS_STUNNED    3	/* stunned		*/
#define POS_SLEEPING   4	/* sleeping		*/
#define POS_MEDITATING   5
#define POS_RESTING    6	/* resting		*/
#define POS_SITTING    7	/* sitting		*/
#define POS_FIGHTING   8	/* fighting		*/
#define POS_STANDING   9	/* standing		*/
#define NUM_POSITIONS  10

/* Player flags: used by char_data.char_specials.act */
#define PLR_KILLER      0   /* Player is a player-killer     */
#define PLR_THIEF       1   /* Player is a player-thief      */
#define PLR_FROZEN      2   /* Player is frozen          */
#define PLR_DONTSET     3   /* Don't EVER set (ISNPC bit)    */
#define PLR_WRITING     4   /* Player writing (board/mail/olc)   */
#define PLR_MAILING     5   /* Player is writing mail        */
#define PLR_CRASH       6   /* Player needs to be crash-saved    */
#define PLR_SITEOK      7   /* Player has been site-cleared  */
#define PLR_NOSHOUT     8   /* Player not allowed to shout/goss  */
#define PLR_NOTITLE     9   /* Player not allowed to set title   */
#define PLR_DELETED     10  /* Player deleted - space reusable   */
#define PLR_LOADROOM	11  /* Player uses nonstandard loadroom	*/
#define PLR_NOWIZLIST	12  /* Player shouldn't be on wizlist	*/
#define PLR_NODELETE	13  /* Player shouldn't be deleted	*/
#define PLR_INVSTART	14  /* Player should enter game wizinvis	*/
#define PLR_CRYO        15  /* Player is cryo-saved (purge prog) */
#define PLR_SHUNNED     16  /* Player is shunned                 */
#define PLR_BUILDING	17  /* player is currently editing something */
#define PLR_RMC         18  /* Player is a member of the RMC      */
#define PLR_HUNTED      19  /* Player has looted another player   */
#define PLR_ATTACKER    20  /* Player attacked another player     */
#define PLR_JAILED      21  /* Player is Jailed			 */
#define PLR_RAWLOG      22  /* Player is being logged             */
#define PLR_REIMBURSED  23  /* Player has been reimbursed	 */
#define PLR_CONJ_TMR    24  /* conj_countdown is running		*/
#define PLR_NORECALL    25  /* Moved to prefrence bits, this can be reused. */
#define PLR_RETIRED	26  /* Immortal is retired so they can relax */
#define PLR_NOCOMM	27  /* Total silence on a player */
#define PLR_POULTICE    28  /* Player has herbs for preparing a poultice */
#define PLR_UBER        29  /* Removed code, this bit can be reused */
#define PLR_GRIEF       30  /* Player is generally hated */
#define PLR_FISHING     31  /* Player has a line in the water */
#define PLR_FISH_ON     32  /* Player's got a fish on the line */
/* Total number of player flags */
#define NUM_PLAYER_FLAGS 33

/* Once per tick state flags */
#define STATE_FORTIFIED  1  /* Player has had SPELL_FORTIFY kick in */
#define STATE_NOXIOUS    2  /* Player has had SPELL_NOXIOUS_SKIN kick in */
#define STATE_LOVER      4  /* Player has hugged a mob this tick! */

/* Mobile flags: used by char_data.char_specials.act */
#define MOB_SPEC         0  /* Mob has a callable spec-proc	*/
#define MOB_SENTINEL     1  /* Mob should not move		*/
#define MOB_SCAVENGER    2  /* Mob picks up stuff on the ground	*/
#define MOB_ISNPC        3  /* (R) Automatically set on all Mobs	*/
#define MOB_AWARE        4  /* Mob can't be backstabbed      */
#define MOB_AGGRESSIVE   5  /* Mob hits players in the room	*/
#define MOB_STAY_ZONE    6  /* Mob shouldn't wander out of zone	*/
#define MOB_WIMPY        7  /* Mob flees if severely injured	*/
#define MOB_AGGR_EVIL	 8  /* auto attack evil PC's		*/
#define MOB_AGGR_GOOD	 9  /* auto attack good PC's		*/
#define MOB_AGGR_NEUTRAL 10 /* auto attack neutral PC's		*/
#define MOB_MEMORY       11 /* remember attackers if attacked    */
#define MOB_HELPER       12 /* attack PCs fighting other NPCs    */
#define MOB_NOCHARM      13 /* Mob can't be charmed      */
#define MOB_NOSUMMON	 14 /* Mob can't be summoned		*/
#define MOB_NOSLEEP      15 /* Mob can't be slept        */
#define MOB_NOBASH       16 /* Mob can't be bashed (e.g. trees)  */
#define MOB_NOBLIND      17 /* Mob can't be blinded      */
#define MOB_CONJURED     18 /* Mob has been conjured/summoned    */
#define MOB_SEEKTORMENTOR 19
#define MOB_SUPERAGG     20 /* Mob is REALLY pissed off          */
#define MOB_MOUNT        21 /* Mob can be ridden.                */
#define MOB_CLONE        22 /* Mob is a clone.                   */
#define MOB_PREDATOR     23 /* The mob attacks the weakest player in the room */
#define MOB_GUARD_CLASS  24 /* Mob will block players not of his class */
#define MOB_GUARD_RACE   25 /* Mob will block players not of his race */
#define MOB_GUARD_BOTH   26 /* Mob will block players !his race/class */
#define MOB_QUESTMASTER  27 /* Mob is allowed to give out quests */
#define MOB_TELEPORTS    28 /* Mob can teleport instead of walk */
#define MOB_TERRORIZE    29 /* Mob is a bully and a braggart! */
#define MOB_NONOX	 30 /* Mob not affected by Nox - Bean */
#define MOB_GRENADER     31 /* Mob lobs flashbangs at adjacent rooms */
#define MOB_EVASIVE      32
/* Total number of Mob Flags */
#define NUM_MOB_FLAGS  33

/* Preference flags: used by char_data.player_specials.pref */
#define PRF_BRIEF       0  /* Room descs won't normally be shown	*/
#define PRF_COMPACT     1  /* No extra CRLF pair before prompts	*/
#define PRF_DEAF        2  /* Can't hear shouts          */
#define PRF_NOTELL      3  /* Can't receive tells        */
#define PRF_DISPHP      4  /* Display hit points in prompt   */
#define PRF_DISPMANA	5  /* Display mana points in prompt	*/
#define PRF_DISPMOVE	6  /* Display move points in prompt	*/
#define PRF_AUTOEXIT	7  /* Display exits in a room		*/
#define PRF_NOHASSLE	8  /* Aggr mobs won't attack		*/
#define PRF_QUEST       9  /* On quest               */
#define PRF_SUMMONABLE	10 /* Can be summoned			*/
#define PRF_NOREPEAT	11 /* No repetition of comm commands	*/
#define PRF_HOLYLIGHT	12 /* Can see in dark			*/
#define PRF_COLOR_1     13 /* Color (low bit)            */
#define PRF_COLOR_2     14 /* Color (high bit)           */
#define PRF_NOARENA     15 /* Can't hear arena status    */
#define PRF_LOG1        16 /* On-line System Log (low bit)   */
#define PRF_LOG2        17 /* On-line System Log (high bit)  */
#define PRF_NOAUCT      18 /* Can't hear auction channel     */
#define PRF_NORPLAY     19 /* Can't hear roleplay channel	*/
#define PRF_NOGRATZ     20 /* Can't hear grats channel       */
#define PRF_SHOWVNUMS	21 /* Can see vnum flags (ROOM_x more to come!)	*/
#define PRF_NOTANON  	22 /* Not anonymous in who list          */
#define PRF_CONSENT  	23 /* Player has given consent.          */
#define PRF_NOSPAM   	24 /* Player has given consent.          */
#define PRF_DISPEXP     25 /* Display percent to level.          */
#define PRF_OLCV        26 /* Verbose or not verbose entries?    */
#define PRF_GOLD_TEAM   27 /* Member of gold team */
#define PRF_BLACK_TEAM  28 /* Member of black team */
#define PRF_NOOOC       29 /* Can't hear OOC channel             */
#define PRF_NOQUERY     30 /* Can't hear query channel           */
#define PRF_NOGUILD     31 /* Can't hear guild channel           */
#define PRF_AUTOSPLIT   32 /* Will auto split looted gold	*/
#define PRF_AUTOLOOT	33 /* Will auto loot corpses		*/
#define PRF_ROGUE_TEAM   34 /* Member of blue team */
#define PRF_RED_TEAM	35 /* Member of red team */
#define PRF_NOCLAN	36 /* Can't hear clan channel */
#define PRF_AUTOGOLD	37 /* Will auto loot gold only */
#define PRF_NORECALL    38 
#define PRF_SHOWTANK    39 /* Show your tank's HP in your prompt */
#define PRF_NOWHO       40 /* Player doesn't show up on WHO */
#define PRF_SHOWDAM     41 /* Players see their damage in arena  */
#define PRF_CLS         42 /* Clear screen in OLC */
#define PRF_NOWIZ       43 /* Can't hear wizline */
#define PRF_AFK         44 /**< AFK flag */
#define PRF_PASSIVE_PET 45 /* Pets will not take action in combat */
/* Total number of available PRF flags */
#define NUM_PRF_FLAGS 46

/* Affect bits: used in char_data.char_specials.saved.affected_by */
/* WARNING: In the world files, NEVER set the bits marked "R" ("Reserved") */
#define AFF_DONTSET           0	   	/* (R) Char is blind		*/
#define AFF_INVISIBLE         1	   	/* Char is invisible		*/
#define AFF_DETECT_ALIGN      2	   	/* Char is sensitive to align*/
#define AFF_DETECT_INVIS      3	   	/* Char can see invis chars  */
#define AFF_DETECT_MAGIC      4	   	/* Char is sensitive to magic*/
#define AFF_SENSE_LIFE        5	   	/* Char can sense hidden life*/
#define AFF_SHADOW_WALK       6	   	/* Char moves easily in darkness */
#define AFF_SANCTUARY         7	   	/* Char protected by sanct.	*/
#define AFF_GROUP             8	   	/* (R) Char is grouped	*/
#define AFF_CURSE             9    	/* Char is cursed		*/
#define AFF_INFRAVISION       10   	/* Char can see in dark	*/
#define AFF_POISON            11	/* (R) Char is poisoned	*/
#define AFF_PROTECT_EVIL      12	/* Char protected from evil  */
#define AFF_PROTECT_GOOD      13	/* Char protected from good  */
#define AFF_SLEEP             14	/* (R) Char magically asleep	*/
#define AFF_NOTRACK           15        /* Char can't be tracked */
#define AFF_FLY	              16	/* Char is flying            */
#define AFF_REGENERATE	      17	/* Char is regenerating     	*/
#define AFF_SNEAK             18   	/* Char can move quietly	*/
#define AFF_HIDE              19	/* Char is hidden		*/
#define AFF_HASTE             20    	/* Char is hasted/slowed */
#define AFF_CHARM             21    	/* Char is charmed		*/
#define AFF_SHIELD            22    	/* Char is shielded          */
#define AFF_PARALYZE	      23    	/* Char is paralyzed         */
#define AFF_AIRSPHERE	      24    	/* Char can breathe water    */
#define AFF_PLAGUE            25    	/* It has he PLAGUE!         */
#define AFF_SHIELDBLOCK       26    	/* Char is shieldblocking    */
#define AFF_SILENCE           27    	/* Char is silent            */
#define AFF_MOUNTED           28    	/* Char is riding a mount    */
#define AFF_WARD              29    	/* Char is warded            */
#define AFF_SHADOW_SPHERE     30    	/* Char is in shadow sphere  */
#define AFF_BERSERK           31    	/* Char is berserk!          */
#define AFF_NO_HOT	          32	/* Char can withstand hot  */
#define AFF_NO_COLD	          33	/* Char can withstand cold */
#define AFF_NO_DRY	          34	/* Char can withstand dry */
#define AFF_BLIND	          35        /* Char cannot see */
#define AFF_WEB               36        /* Char is Webbed */
#define AFF_BLINK             37    /* Char has dodge for a while */
#define AFF_FEEBLE            38
#define AFF_ASSISTANT         39        /* Char has a divine assistant */
#define AFF_FORTIFY           40        /* Char is being fortified */
#define AFF_HAMSTRUNG         41        /* Char has been hamstrung */
#define AFF_PULSE_HIT         42        /* Pulse heal hitpoints */
#define AFF_PULSE_MANA        43        /* Pulse heal mana */
#define AFF_DISTRACT          44        /* distracted */
#define AFF_CRUSADE           45        /* crusading */
#define AFF_APOCALYPSE        46        /* Char has an Aura of Apocalypse */
#define AFF_MISSION           47        /* Char is on a Divine Mission */
#define AFF_UNUSED            48        /* UNUSED! ex-SHIELDSTUN */
#define AFF_LORE              49        /* Char is using forest lore */
#define AFF_LEARNING          50        /* Char gets a 2x boost to exp */
#define AFF_FLAME_BLADE       51        /* 1d4+level/10 to damage, type burn */
#define AFF_UNUSED2           52        /* Not currently used.  Used to be sunvisor*/
#define AFF_WRAITHFORM        53        /* Not entirely pyhsically present */
#define AFF_DISEASE           54        /* A much nastier form of poison */
/* Total number of affect flags **/
#define NUM_AFF_FLAGS     55


/* Modes of connectedness: used by descriptor_data.state */
#define CON_PLAYING      0      /* Playing - Nominal state  */
#define CON_CLOSE        1      /* Disconnecting        */
#define CON_GET_NAME	 2		/* By what name ..?		*/
#define CON_NAME_CNFRM	 3		/* Did I get that right, x?	*/
#define CON_PASSWORD	 4		/* Password:			*/
#define CON_NEWPASSWD	 5		/* Give me a password for x	*/
#define CON_CNFPASSWD	 6		/* Please retype password:	*/
#define CON_QSEX         7      /* Sex?             */
#define CON_QAGE         8
#define CON_QCLASS       9      /* Class?           */
#define CON_QRACE        10     /* Race?            */
#define CON_RMOTD        11     /* PRESS RETURN after MOTD  */
#define CON_MENU         12     /* Your choice: (main menu) */
#define CON_PLR_DESC     13     /* Enter a new description: */
#define CON_CHPWD_GETOLD 14		/* Changing passwd: get old	*/
#define CON_CHPWD_GETNEW 15		/* Changing passwd: get new	*/
#define CON_CHPWD_VRFY   16		/* Verify new password		*/
#define CON_DELCNF1      17     /* Delete confirmation 1    */
#define CON_DELCNF2      18     /* Delete confirmation 2    */
#define CON_SHOWWHO      19     /* Show users w/o loggin on */
#define CON_QABILS       20     /* These stats good enough? */
#define CON_QBREATH      21     /* selectbreath weapon      */
#define CON_OEDIT        22     /* OLC mode - object edit   */
#define CON_REDIT        23     /* OLC mode - room edit     */
#define CON_ZEDIT        24     /* OLC mode - zone info edit */
#define CON_MEDIT        25     /* OLC mode - mobile edit   */
#define CON_SEDIT        26     /* OLC mode - shop edit     */
#define CON_TEDIT	 27     /* Edit backgrnd, news, policy etc. */
#define CON_TRIGEDIT     28     /* OLC mode - trigger edit */
#define CON_QEDIT        29     /* OLC mode - quest edit */
#define CON_CEDIT        30 /**< OLC mode - conf editor		*/
#define CON_HEDIT        31 /**< OLC mode - help edit */

/* Depreciated -Xiuh */
#define NUM_CONNECTED_TYPES 32

/* OLC States range - used by IS_IN_OLC and IS_PLAYING */
#define FIRST_OLC_STATE CON_OEDIT     /**< The first CON_ state that is an OLC */
#define LAST_OLC_STATE  CON_HEDIT  /**< The last CON_ state that is an OLC  */

/* Zone info: Used in zone_data.zone_flags */
#define ZONE_OPEN               (1 << 0)
#define ZONE_CLOSED             (1 << 1)
#define ZONE_NORECALL           (1 << 2)
#define ZONE_NOSUMMON           (1 << 3)
#define ZONE_NOPORTAL		(1 << 4)
#define ZONE_NORELOCATE		(1 << 5)
#define ZONE_NOMORTAL	        (1 << 6)
#define ZONE_REMORT_ONLY        (1 << 7)
#define ZONE_DIS_LIGHTNING	(1 << 8)
#define ZONE_DIS_FIREBALL	(1 << 9)
#define ZONE_DIS_WIND		(1 << 10)
#define ZONE_DIS_EARTHQUAKE	(1 << 11)
#define ZONE_DIS_LAVA		(1 << 12)
#define ZONE_DIS_FLOOD		(1 << 13)
#define ZONE_PEACEFUL		(1 << 14)
#define ZONE_SLEEPTAG		(1 << 15)
#define ZONE_ARENA		(1 << 16)
/* Total number of zone flags */
#define NUM_ZONE_FLAGS 17

/* Character equipment positions: used as index for char_data.equipment[]
   NOTE: Don't confuse these constants with the ITEM_ bitvectors
   which control the valid places you can wear a piece of equipment
   For example, there are two neck positions on the player, and items
   only get the generic neck type. */
#define WEAR_LIGHT      0
#define WEAR_FINGER_R   1
#define WEAR_FINGER_L   2
#define WEAR_CLOAK      3
#define WEAR_NECK       4
#define WEAR_BODY       5
#define WEAR_HEAD       6
#define WEAR_EARS       7
#define WEAR_FACE       8
#define WEAR_LEGS       9
#define WEAR_ANKLES    10
#define WEAR_FEET      11
#define WEAR_HANDS     12
#define WEAR_ARMS      13
#define WEAR_SHIELD    14
#define WEAR_ABOUT     15
#define WEAR_WAIST     16
#define WEAR_WRIST_R   17
#define WEAR_WRIST_L   18
#define WEAR_WIELD     19
#define WEAR_HOLD      20
#define WEAR_ORBIT     21

/* object-related defines ********************************************
 * Item types: used by obj_data.obj_flags.type_flag ******************/
#define ITEM_UNDEFINED  0		/* Item type isn't defined         */
#define ITEM_LIGHT      1		/* Item is a light source	   */
#define ITEM_SCROLL     2		/* Item is a scroll		   */
#define ITEM_WAND       3		/* Item is a wand		   */
#define ITEM_STAFF      4       	/* Item is a staff      	   */
#define ITEM_WEAPON     5		/* Item is a weapon		   */
#define ITEM_FIREWEAPON 6		/* Item fires missiles		   */
#define ITEM_MISSILE    7		/* Item is a missile		   */
#define ITEM_TREASURE   8		/* Item is a treasure, not gold	   */
#define ITEM_ARMOR      9		/* Item is armor		   */
#define ITEM_POTION    10 		/* Item is a potion		   */
#define ITEM_WORN      11		/* Unimplemented		   */
#define ITEM_OTHER     12		/* Misc object			   */
#define ITEM_TRASH     13		/* Trash - shopkeeps won't buy	   */
#define ITEM_UNUSED    14		/* Item is a trap: DEPRECATED      */
#define ITEM_CONTAINER 15		/* Item is a container		   */
#define ITEM_NOTE      16		/* Item is note 		   */
#define ITEM_DRINKCON  17		/* Item is a drink container	   */
#define ITEM_KEY       18		/* Item is a key		   */
#define ITEM_FOOD      19		/* Item is food			   */
#define ITEM_MONEY     20		/* Item is money (gold)		   */
#define ITEM_PEN       21		/* Item is a pen		   */
#define ITEM_BOAT      22		/* Item is a boat		   */
#define ITEM_FOUNTAIN  23       	/* Item is a fountain              */
#define ITEM_PORTAL    24		/* Item is a portal of some kind.  */
#define ITEM_SCRIBE    25		/* Item is paper for scribing	   */
#define ITEM_AFFECT    26               /* Item is has a room/spell affect */
#define ITEM_DUST      27               /* Item is a pile of dust          */
#define ITEM_POLE      28               /* Item is a fishing pole          */
#define ITEM_DEPRECATED 29               /* Item is a trophy                */
/* Total number of item types. */
#define NUM_ITEM_TYPES 30

/* Take/Wear flags: used by obj_data.obj_flags.wear_flags */
#define ITEM_WEAR_TAKE		0  /* Item can be takes		*/
#define ITEM_WEAR_FINGER	1  /* Can be worn on finger	*/
#define ITEM_WEAR_CLOAK 	2  /* Can be worn as a cloak     */
#define ITEM_WEAR_BODY		3  /* Can be worn on body 	*/
#define ITEM_WEAR_HEAD		4  /* Can be worn on head 	*/
#define ITEM_WEAR_LEGS		5  /* Can be worn on legs	*/
#define ITEM_WEAR_FEET		6  /* Can be worn on feet	*/
#define ITEM_WEAR_HANDS		7  /* Can be worn on hands	*/
#define ITEM_WEAR_ARMS		8  /* Can be worn on arms	*/
#define ITEM_WEAR_SHIELD	9  /* Can be used as a shield	*/
#define ITEM_WEAR_ABOUT		10 /* Can be worn about body 	*/
#define ITEM_WEAR_WAIST 	11 /* Can be worn around waist 	*/
#define ITEM_WEAR_WRIST		12 /* Can be worn on wrist 	*/
#define ITEM_WEAR_WIELD		13 /* Can be wielded		*/
#define ITEM_WEAR_HOLD		14 /* Can be held		*/
#define ITEM_WEAR_NECK		15 /* Can be worn around neck    */
#define ITEM_WEAR_ORBIT     	16 /* Orbits your head :) */
#define ITEM_WEAR_ANKLES    	17 /* Can be worn on ankles */
#define ITEM_WEAR_EARS       	18 /* Can be worn on ears */
#define ITEM_WEAR_FACE       	19 /* Can be worn on face */
/* Total number of item wears */
#define NUM_ITEM_WEARS          20

/* Extra object flags: used by obj_data.obj_flags.extra_flags */
#define ITEM_GLOW                 0	/* Item is glowing		*/
#define ITEM_HUM                  1	/* Item is humming		*/
#define ITEM_NORENT               2	/* Item cannot be rented	*/
#define ITEM_NODONATE             3	/* Item cannot be donated	*/
#define ITEM_NOINVIS	          4	/* Item cannot be made invis	*/
#define ITEM_INVISIBLE            5	/* Item is invisible		*/
#define ITEM_MAGIC                6	/* Item is magical		*/
#define ITEM_CURSED               7	/* Item is cursed: can't drop	*/
#define ITEM_BLESS                8	/* Item is blessed		*/
#define ITEM_ANTI_GOOD            9	/* Not usable by good people	*/
#define ITEM_ANTI_EVIL            10	/* Not usable by evil people	*/
#define ITEM_ANTI_NEUTRAL         11	/* Not usable by neutral people	*/
#define ITEM_ANTI_MAGIC_USER      12	/* Not usable by mages		*/
#define ITEM_ANTI_CLERIC          13	/* Not usable by clerics	*/
#define ITEM_ANTI_THIEF           14	/* Not usable by thieves	*/
#define ITEM_ANTI_WARRIOR         15	/* Not usable by warriors	*/
#define ITEM_NOSELL               16	/* Shopkeepers won't touch it	*/
#define ITEM_ANTI_RANGER          17	/* Not usable by rangers	*/
#define ITEM_ANTI_ASSASSIN        18	/* Not usable by assassins	*/
#define ITEM_ANTI_SHOU_LIN        19	/* Not usable by shou lin	*/
#define ITEM_ANTI_SOLAMNIC_KNIGHT 20	/* Not usable by solamnic knights	*/
#define ITEM_ANTI_DEATH_KNIGHT    21	/* Not usable by death knights	*/
#define ITEM_ANTI_SHADOW_DANCER   22	/* Not usable by shadow dancers	*/
#define ITEM_TIMED                23 	/* Item will disappear when its timer reaches 0 */
#define ITEM_EXPLODES             24 	/* Explodable */
#define ITEM_ARTIFACT             25	/* This is an artifact */
#define ITEM_NOLOCATE             26	/* Item can't be located */
#define ITEM_ANTI_MINOTAUR	  27	/* Item can't be worn by minotaurs */
#define ITEM_ANTI_GNOME		  28	/* Item can't be worn by gnomes */
#define ITEM_ANTI_ORC		  29	/* Item can't be worn by orcs */
#define ITEM_ANTI_ELF		  30	/* Item can't be worn by elfs */
#define ITEM_ANTI_DRACONIAN	  31	/* Item can't be worn by draconians */
#define ITEM_ANTI_HALFLING	  32	/* Item can't be worn by halflings */
#define ITEM_ANTI_OGRE		  33	/* Item can't be worn by ogres */
#define ITEM_ANTI_TROLL		  34	/* Item can't be worn by trolls */
#define ITEM_ANTI_DWARF		  35	/* Item can't be worn by dwarfs */
#define ITEM_ANTI_HUMAN		  36	/* Item can't be worn by humans */
#define ITEM_ANTI_NECROMANCER     37    /* Item can't be worn by necros */
#define ITEM_MAIN_PART		  38    /* Item is main part for inserts */
#define ITEM_INSERT		  39    /* Item can be inserted into other items*/
#define ITEM_ARENA                40    /* Item is for arena use */
#define ITEM_ANTI_DEMON           41    /* Item can't be worn by demons */
#define ITEM_ANTI_IZARTI          42    /* Item can't be worn by izarti */
#define ITEM_ANTI_VAMPIRE         43    /* Item can't be worn by vampires */
#define ITEM_ANTI_WEREWOLF        44    /* Item can't be worn by werewolves */
#define ITEM_ANTI_ELEMENTAL       45    /* Item can't be worn by elementals */
#define ITEM_ANTI_GIANT           46    /* Item can't be worn by giants */
#define ITEM_ANTI_FAERIE          47    /* Item can't be worn by faeries */
#define ITEM_ANTI_AMARA           48    /* Item can't be worn by amara */
#define ITEM_ANTI_UNDEAD          49    /* Item can't be worn by undead */
#define ITEM_ANTI_DROW		  50	/* Item can't be worn by drow */
#define ITEM_LOOTED		  51	/* Corpse has been looted by a player */
#define ITEM_IDENTIFIED           52    /* Item has been identified   */
#define ITEM_RANDOMIZED           53    /* Items' stats are randomized */
#define ITEM_SOULBOUND            54    /* Item cannot be transfered   */
#define ITEM_ANTI_REMORT          55    /* Item cannot be worn by remorts */
#define ITEM_ANTI_PREMORT         56    /* Item cannot be worn by premorts */
#define ITEM_TROPHY               57    /* Item is a trophy, and will dissapear eventually */
/* Total number of item flags */
#define NUM_ITEM_FLAGS            58

/* Modifier constants used with obj affects ('A' fields) */
#define APPLY_NONE              0	/* No effect			*/
#define APPLY_STR               1	/* Apply to strength		*/
#define APPLY_DEX               2	/* Apply to dexterity		*/
#define APPLY_INT               3	/* Apply to constitution	*/
#define APPLY_WIS               4	/* Apply to wisdom		*/
#define APPLY_CON               5	/* Apply to constitution	*/
#define APPLY_CHA               6   	/* Apply to charisma        	*/
#define APPLY_CLASS             7	/* Reserved			*/
#define APPLY_LEVEL             8	/* Reserved			*/
#define APPLY_AGE               9	/* Apply to age			*/
#define APPLY_CHAR_WEIGHT      10	/* Apply to weight		*/
#define APPLY_CHAR_HEIGHT      11	/* Apply to height		*/
#define APPLY_MANA             12	/* Apply to max mana		*/
#define APPLY_HIT              13	/* Apply to max hit points	*/
#define APPLY_MOVE             14	/* Apply to max move points	*/
#define APPLY_GOLD             15	/* Reserved			*/
#define APPLY_EXP              16	/* Reserved			*/
#define APPLY_AC               17	/* Apply to Armor Class		*/
#define APPLY_HITROLL          18	/* Apply to hitroll		*/
#define APPLY_DAMROLL          19	/* Apply to damage roll		*/
#define APPLY_SAVING_PARA      20	/* Apply to save throw: paralz	*/
#define APPLY_SAVING_ROD       21	/* Apply to save throw: rods	*/
#define APPLY_SAVING_PETRI     22	/* Apply to save throw: petrif	*/
#define APPLY_SAVING_BREATH    23	/* Apply to save throw: breath	*/
#define APPLY_SAVING_SPELL     24	/* Apply to save throw: spells	*/
#define APPLY_POISON           25   	/* Item can poison vistim       */
#define APPLY_PLAGUE           26   	/* Item can plague victim       */
#define APPLY_SPELL_COST       27	/* Item reduces cost of spells  */
#define APPLY_SPELL_SAVE       28       /* Affects victims saving throw */
#define APPLY_SPELL_DAMAGE     29	/* Will increase spell damage   */
#define APPLY_SPELL_DURATION   30	/* Will increase spell duration */
#define APPLY_SKILL_SUCCESS    31	/* Affects chance of spells working */
#define APPLY_SKILL_SUCCESS_DEPRECATED    32	/* Affects chance of skills working */
#define APPLY_USELEVEL         33       /* min/max level to use an object */
/* Total number of applies */
#define NUM_APPLIES        34

/* Container flags - value[1] */
#define CONT_CLOSEABLE      (1 << 0)	/* Container can be closed	*/
#define CONT_PICKPROOF      (1 << 1)	/* Container is pickproof	*/
#define CONT_CLOSED         (1 << 2)	/* Container is closed		*/
#define CONT_LOCKED         (1 << 3)	/* Container is locked		*/
#define CONT_LOCKER         (1 << 4)    /* Container is a locker        */
#define CONT_DIRTY          (1 << 5)    /* Locker contents have changed */
/* Total number of container bits */
#define NUM_CONTAINER_BITS   6

/* Some different kind of liquids for use in values of drink containers */
#define LIQ_WATER      0
#define LIQ_BEER       1
#define LIQ_WINE       2
#define LIQ_ALE        3
#define LIQ_DARKALE    4
#define LIQ_WHISKY     5
#define LIQ_LEMONADE   6
#define LIQ_FIREBRT    7
#define LIQ_LOCALSPC   8
#define LIQ_SLIME      9
#define LIQ_MILK       10
#define LIQ_TEA        11
#define LIQ_COFFE      12
#define LIQ_BLOOD      13
#define LIQ_SALTWATER  14
#define LIQ_CLEARWATER 15
/* Total number of liquid types */
#define NUM_LIQ_TYPES     16

/* other miscellaneous defines *******************************************/

/* Player conditions */
#define DRUNK        0
#define HUNGER         1
#define THIRST       2

/* other #defined constants */
/* **DO**NOT** blindly change the number of levels in your MUD merely by
 * changing these numbers and without changing the rest of the code to match.
 * Other changes throughout the code are required.  See coding.doc for details.
 *
 * LVL_IMPL should always be the HIGHEST possible immortal level, and
 * LVL_IMMORT should always be the LOWEST immortal level.  The number of
 * mortal levels will always be LVL_IMMORT - 1. */

#define LVL_HERO                   51
#define LVL_SAINT                  52
#define LVL_ANGEL                  53
#define LVL_DEITY                  54
#define LVL_CREATOR                55
#define LVL_DGOD                   56
#define LVL_LRGOD                  57
#define LVL_GOD                    58
#define LVL_GRGOD                  59
#define LVL_IMPL                   60

#define MAX_MORTAL                 50
#define LVL_IMMORT        (MAX_MORTAL+1)
#define MAX_IMMORTAL      (MAX_MORTAL+10)

#define NUM_GOD_LABELS (MAX_IMMORTAL - MAX_MORTAL)

/** Minimum level to build and to run the saveall command */
#define LVL_BUILDER	LVL_IMMORT

#define LVL_FREEZE	LVL_GRGOD

/* Shou-lin comb and def stances */
#define STANCE_NEUTRAL          0
#define STANCE_OFFENSIVE        1
#define STANCE_DEFENSIVE        2
#define NUM_STANCES             3

#define ASPECT_NONE             0
#define ASPECT_WIND             1
#define ASPECT_TIGER            2
#define ASPECT_SNAKE            3
#define ASPECT_MONKEY           4
#define ASPECT_CRANE            5
#define ASPECT_FLOWER           6
#define ASPECT_DRAGON           7
#define NUM_ASPECTS             8

/* quest related defines * ***********************************************/

/* quest rewards */
#define QST_REWARD_NONE   0
#define QST_REWARD_QP     1
#define QST_REWARD_EXP    2
#define QST_REWARD_OBJ    3
#define QST_REWARD_GOLD   4
#define QST_REWARD_PRAC   5
#define QST_REWARD_SPELL  6
#define QST_REWARD_REMORT 7

/* quest tasks */
#define QST_TASK_NONE    0      /* no task              */
#define QST_TASK_OBJ     1      /* retrieve an object   */
#define QST_TASK_MOB     2      /* kill a mob           */
#define QST_TASK_ROOM    3      /* enter a room         */

/* quest flags */
#define QST_ANTI_GOOD      (1 << 0)
#define QST_ANTI_EVIL      (1 << 1)
#define QST_ANTI_NEUT      (1 << 2)
#define QST_ANTI_MAGE      (1 << 3)
#define QST_ANTI_CLERIC    (1 << 4)
#define QST_ANTI_THIEF     (1 << 5)
#define QST_ANTI_WARRIOR   (1 << 6)
#define QST_ANTI_RANGER    (1 << 7)
#define QST_ANTI_ASSASSIN  (1 << 8)
#define QST_ANTI_SHOULIN   (1 << 9)
#define QST_ANTI_SOLAMNIC  (1 << 10)
#define QST_ANTI_DEATH_KN  (1 << 11)
#define QST_ANTI_SHAD_DANC (1 << 12)
#define QST_ANTI_MINOTAUR  (1 << 13)
#define QST_ANTI_GNOME     (1 << 14)
#define QST_ANTI_ORC       (1 << 15)
#define QST_ANTI_ELF       (1 << 16)
#define QST_ANTI_DRACONIAN (1 << 17)
#define QST_ANTI_HALFING   (1 << 18)
#define QST_ANTI_OGRE      (1 << 19)
#define QST_ANTI_TROLL     (1 << 20)
#define QST_ANTI_DWARF     (1 << 21)
#define QST_ANTI_HUMAN     (1 << 22)
#define QST_ONCE_ONLY      (1 << 23)
#define QST_ONE_OBJECTIVE  (1 << 24)
#define QST_RANDOM_REWARD  (1 << 25)
#define QST_ANTI_DROW      (1 << 26)
#define QST_ANTI_NECRO     (1 << 27)
#define QST_COMMON_REWARD  (1 << 28)
/* Total number of qst flags. */
#define NUM_QST_FLAGS      29

#define QST_REMORT_DEMON                0
#define QST_REMORT_IZARTI               1
#define QST_REMORT_WEREWOLF             2
#define QST_REMORT_VAMPIRE              3
#define QST_REMORT_FAERIE               4
#define QST_REMORT_FIRE_ELEMENTAL       5
#define QST_REMORT_AIR_ELEMENTAL        6
#define QST_REMORT_WATER_ELEMENTAL      7
#define QST_REMORT_EARTH_ELEMENTAL      8
#define QST_REMORT_GIANT                9
#define QST_REMORT_AMARA                10
#define QST_REMORT_HUMAN                11
#define QST_REMORT_HALFLING             12
#define QST_REMORT_ELF                  13
#define QST_REMORT_DWARF                14
#define QST_REMORT_MINOTAUR             15
#define QST_REMORT_OGRE                 16
#define QST_REMORT_TROLL                17
#define QST_REMORT_ORC                  18
#define QST_REMORT_GNOME                19
#define QST_REMORT_DRACONIAN_RED        20
#define QST_REMORT_DRACONIAN_GREEN      21
#define QST_REMORT_DRACONIAN_WHITE      22
#define QST_REMORT_DRACONIAN_BLACK      23
#define QST_REMORT_DRACONIAN_BLUE       24
#define QST_REMORT_UNDEAD               25
#define QST_REMORT_DROW                 26
#define QST_REMORT_SHUMAN               27
#define QST_REMORT_SHALFLING            28
#define QST_REMORT_SELF                 29
#define QST_REMORT_SDROW                30
#define QST_REMORT_SDWARF               31
#define QST_REMORT_SMINOTAUR            32
#define QST_REMORT_SOGRE                33
#define QST_REMORT_STROLL               34
#define QST_REMORT_SDRACONIAN           35
#define QST_REMORT_SGNOME               36
#define QST_REMORT_SORC                 37
/* Total number of qst remort races. */
#define QST_NUM_REMORTS                 38

typedef struct dex_skill_type {
    signed short int p_pocket;
    signed short int p_locks;
    signed short int traps;
    signed short int sneak;
    signed short int hide;
} DexSkillType;

typedef struct dex_app_type {
    signed short int reaction;
    signed short int miss_att;
    signed short int defensive;
} DexAppType;

typedef struct str_app_type {
    signed short int tohit; /* To Hit (THAC0) Bonus/Penalty        */
    signed short int todam; /* Damage Bonus/Penalty                */
    signed short int carry_w; /* Maximum weight that can be carrried */
    signed short int wield_w; /* Maximum weight that can be wielded  */
} StrAppType;

typedef struct wis_app_type {
    char bonus; /* how many practices player gains per lev */
    signed short int extra; /* Number between 1 and 5 which reflects chance
                     ** of 1 extra practice. */
} WisAppType;

typedef struct con_app_type {
    signed short int hitp;
    signed short int shock;
    signed short int extra; /* Number between 1 and 3, reflects chance of
                    ** 1 extra hit point. */
} ConAppType;

typedef struct int_app_type {
    signed short int manap; /* extra mana points. */
    signed short int extra; /* Chance of 1 extra mana point, number between
                    ** 1 and 3. */
} IntAppType;

/*
 ** All of the extern forward defs are here.
 */
extern const char *circlemud_version;
extern char *action_bits[];
extern char *affected_bits[];
extern char *zone_bits[];
extern const char *apply_types[];
extern const char *dirs[];
extern char *drinks[];
extern char *god_labels[];
extern const char *equipment_types[];
extern char *extra_bits[];
extern char *genders[];
extern const char *item_types[];
extern const char *position_types[];
extern char *none_room_bits[];
extern char **room_bits;
extern const char *sector_types[];
extern char *player_bits[];
extern char *preference_bits[];

extern char *wear_bits[];
extern const char *connected_types[];
extern const char *wear_where[];
extern char *stances[];
extern char *aspects[];
extern char *mastery_types[];
extern const char *color_liquid[];
extern const char *fullness[];
extern const DexAppType dex_app[];
extern const DexSkillType dex_app_skill[];
extern const IntAppType int_app[];
extern const StrAppType str_app[];
extern const WisAppType wis_app[];
extern const ConAppType con_app[];
extern const char *strString[];
extern const char *intString[];
extern const char *wisString[];
extern const char *dexString[];
extern const char *conString[];
extern const char *chaString[];
extern const int rev_dir[];

/* Config structs */

/** The game configuration structure used for configurating the game play
 * variables. */
struct game_data {
    int pk_allowed; /**< Is player killing allowed?    */
    int pt_allowed; /**< Is player thieving allowed?   */
    int level_can_shout; /**< Level player must be to shout.   */
    int holler_move_cost; /**< Cost to holler in move points.    */
    int tunnel_size; /**< Number of people allowed in a tunnel.*/
    int max_exp_gain; /**< Maximum experience gainable per kill.*/
    int max_exp_loss; /**< Maximum experience losable per death.*/
    int max_npc_corpse_time; /**< Num tics before NPC corpses decompose*/
    int max_pc_corpse_time; /**< Num tics before PC corpse decomposes.*/
    int idle_void; /**< Num tics before PC sent to void(idle)*/
    int idle_rent_time; /**< Num tics before PC is autorented.   */
    int idle_max_level; /**< Level of players immune to idle.     */
    int dts_are_dumps; /**< Should items in dt's be junked?   */
    int load_into_inventory; /**< Objects load in immortals inventory. */
    int track_through_doors; /**< Track through doors while closed?    */
    int no_mort_to_immort; /**< Prevent mortals leveling to imms?    */
    int disp_closed_doors; /**< Display closed doors in autoexit?    */
    int script_players; /**< Is attaching scripts to players allowed? */
    char *OK; /**< When player receives 'Okay.' text.    */
    char *NOPERSON; /**< 'Nobody by that name is here.'   */
    char *NOEFFECT; /**< 'Nothing seems to happen.'            */
    int plague_is_contagious; /**< Global MUD plague contagious amount players  */
    int quest_active; /**< Global toggle for quest flag activation.   */
};

/** The rent and crashsave options. */
struct crash_save_data {
    int free_rent; /**< Should the MUD allow rent for free?   */
    int max_obj_save; /**< Max items players can rent.           */
    int min_rent_cost; /**< surcharge on top of item costs.       */
    int auto_save; /**< Does the game automatically save ppl? */
    int autosave_time; /**< if auto_save=TRUE, how often?         */
    int crash_file_timeout; /**< Life of crashfiles and idlesaves.     */
    int rent_file_timeout; /**< Lifetime of normal rent files in days */
};

/** Important room numbers. This structure stores vnums, not real array
 ** numbers. */
struct room_numbers {
    sh_int mortal_start_room; /**< vnum of room that mortals enter at.  */
    sh_int immort_start_room; /**< vnum of room that immorts enter at.  */
    sh_int frozen_start_room; /**< vnum of room that frozen ppl enter.  */
    sh_int donation_room_1; /**< vnum of donation room #1.            */
    sh_int donation_room_2; /**< vnum of donation room #2.            */
    sh_int donation_room_3; /**< vnum of donation room #3.            */
};

/** Operational game options. */
struct game_operation {
    ush_int DFLT_PORT; /**< The default port to run the game.  */
  char *DFLT_IP; /**< Bind to all interfaces.     */
    char *DFLT_DIR; /**< The default directory (lib).    */
    char *LOGNAME; /**< The file to log messages to.    */
    int max_playing; /**< Maximum number of players allowed. */
    int max_filesize; /**< Maximum size of misc files.   */
  int max_bad_pws; /**< Maximum number of pword attempts.  */
  int siteok_everyone; /**< Everyone from all sites are SITEOK.*/
    int nameserver_is_slow; /**< Is the nameserver slow or fast?   */
  int use_new_socials; /**< Use new or old socials file ?      */
    int auto_save_olc; /**< Does OLC save to disk right away ? */
    char *MENU; /**< The MAIN MENU. */
    char *GREETINGS; /**< The RavenMUD ascii login picture.      */
    char *WELC_MESSG; /**< The welcome message.      */
    char *START_MESSG; /**< The start msg for new characters.  */
};

/** The Autowizard options. */
struct autowiz_data {
    int use_autowiz; /**< Use the autowiz feature?   */
    int min_wizlist_lev; /**< Minimun level to show on wizlist.  */
};

/**
 Main Game Configuration Structure.
 Global variables that can be changed within the game are held within this
 structure. During gameplay, elements within this structure can be altered,
 thus affecting the gameplay immediately, and avoiding the need to recompile
 the code.
 If changes are made to values of the elements of this structure during game
 play, the information will be saved to disk.
 */
struct config_data {
    /** Path to on-disk file where the config_data structure gets written. */
    char *CONFFILE;
    /** In-game specific global settings, such as allowing player killing. */
    struct game_data play;
    /** How is renting, crash files, and object saving handled? */
    struct crash_save_data csd;
    /** Special designated rooms, like start rooms, and donation rooms. */
    struct room_numbers room_nums;
    /** Basic operational settings, like max file sizes and max players. */
    struct game_operation operation;
    /** Autowiz specific settings, like turning it on and minimum level */
    struct autowiz_data autowiz;
};

#ifdef MEMORY_DEBUG
#include "util/zmalloc.h"
#endif

#endif /* _STRUCTS_H_ */
