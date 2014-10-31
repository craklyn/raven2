/* ************************************************************************
*   File: spec_assign.c                                 Part of CircleMUD *
*  Usage: Functions to assign function pointers to objs/mobs/rooms        *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/comm.h"
#include "actions/interpreter.h"
#include "util/utils.h"
#include "specials/reward.h"
#include "specials/beholder.h"
#include "specials/castle.h"
#include "specials/guard.h"
#include "specials/healer.h"
#include "specials/legend.h"
#include "specials/ten_trials.h"
#include "specials/lands_of_chaos.h"
#include "specials/mail.h"
#include "specials/metaphys.h"
#include "specials/portal.h"
#include "specials/superAggr.h"
#include "specials/unicorn.h"
#include "specials/combspec.h"
#include "actions/ban.h"
#include "specials/boards.h"
#include "general/objsave.h"
#include "specials/special.h"
#include "specials/perceptory.h"
#include "specials/torment.h"
#include "specials/bloodbowl.h"
#include "specials/dice.h"
#include "specials/lockers.h"
#include "specials/renames.h"
#include "specials/qpshop.h"
#include "specials/forger.h"
#include "specials/elie.h"
#include "specials/advance.h"

/* local (file scope only) functions */
static void ASSIGNROOM(room_vnum room, SPECIAL(fname));
static void ASSIGNMOB(mob_vnum mob, SPECIAL(fname));
static void ASSIGNOBJ(obj_vnum obj, SPECIAL(fname));

/* functions to perform assignments */
static void ASSIGNMOB(mob_vnum mob, SPECIAL(fname))
{
  mob_rnum rnum;

  if ((rnum = real_mobile(mob)) != NOBODY)
    mob_index[rnum].func = fname;
  else if (!mini_mud)
    mlog("SYSERR: Attempt to assign spec to non-existant mob #%d", mob);
  }

static void ASSIGNOBJ(obj_vnum obj, SPECIAL(fname))
{
  obj_rnum rnum;

  if ((rnum = real_object(obj)) != NOTHING)
    obj_index[rnum].func = fname;
  else if (!mini_mud)
    mlog("SYSERR: Attempt to assign spec to non-existant obj #%d", obj);
}

static void ASSIGNROOM(room_vnum room, SPECIAL(fname))
{
  room_rnum rnum;

  if ((rnum = real_room(room)) != NOWHERE)
    world[rnum].func = fname;
  else if (!mini_mud)
    mlog("SYSERR: Attempt to assign spec to non-existant room #%d", room);
}


/* ********************************************************************
*  Assignments                                                        *
******************************************************************** */

/* assign special procedures to mobiles */
void assign_mobiles(void)
{
   assign_kings_castle();

   ASSIGNMOB(19911, ten_trials);        /* Ten Trials, Nereus, With Key */

   ASSIGNMOB(102, janitor);    /* Maid */

   /* God Rooms */
   ASSIGNMOB( 1230, guild_guard ); /* Xi-Ssiin */
   ASSIGNMOB( 1236, mobLauncher ); /* Mob Launcher */

   /* The Chessboard */
   ASSIGNMOB( 1699, guild_guard ); /* Sisyphus  */
   /* Garamond */
   ASSIGNMOB( 1928, superAggr ); /* guardian */
   /* Shire */
   ASSIGNMOB( 2431, receptionist); /* receptionist in green dragon */
   /* Abyss */
   ASSIGNMOB( 2702, superAggr ); /* The Juggernaut */
   ASSIGNMOB( 2707, superAggr ); /* The Black Mage */
   ASSIGNMOB( 2711, superAggr ); /* Tanar'ri Lord */
   ASSIGNMOB( 2716, superAggr ); /* Hiddukel */

   /* City of Everwar  */
   ASSIGNMOB(27718, receptionist); /*  Gnimwodilus  */

   /* Whatever Jimmy & Rag's Area Is */
   ASSIGNMOB(2818, reward);	/* Loki */
   ASSIGNMOB(2819, guild_guard); /* Heimdall */
   ASSIGNMOB(2832, superAggr);  /* TeethGnasher */
   ASSIGNMOB(2833, superAggr);  /* TeethGrinder */
   ASSIGNMOB(2842, humanAggr); /* giant thrym */

   /* Northern Midgaard */
   ASSIGNMOB(3005, receptionist);
   ASSIGNMOB(3010, postmaster);

   ASSIGNMOB(3014, guild);
   ASSIGNMOB(3015, guild);
   ASSIGNMOB(3016, guild);
   ASSIGNMOB(3017, guild);
   ASSIGNMOB(3018, guild);
   ASSIGNMOB(3019, guild);
   ASSIGNMOB(3020, guild);
   ASSIGNMOB(3021, guild);
   ASSIGNMOB(3022, guild);
   ASSIGNMOB(3023, guild);

   ASSIGNMOB(3024, guild_guard);
   ASSIGNMOB(3025, guild_guard);
   ASSIGNMOB(3026, guild_guard);
   ASSIGNMOB(3027, guild_guard);
   ASSIGNMOB(3028, guild_guard);
   ASSIGNMOB(3029, guild_guard);
   ASSIGNMOB(3030, guild_guard);
   ASSIGNMOB(3031, guild_guard);
   ASSIGNMOB(3032, guild_guard);
   ASSIGNMOB(3033, guild_guard);
   ASSIGNMOB(3099, guild_guard);

   ASSIGNMOB(3060, cityguard);
   ASSIGNMOB(3061, janitor);
   ASSIGNMOB(3062, fido);
   ASSIGNMOB(3066, fido);
   ASSIGNMOB(3067, cityguard);
   ASSIGNMOB(3068, janitor);
   ASSIGNMOB(3095, cryogenicist);

   ASSIGNMOB(3705, guild_guard); /* red super giant */
   ASSIGNMOB(3713, superAggr);  /* Draco */
   ASSIGNMOB(3729, guild_guard); /* Ursa major */

   /* Tuatha */
   ASSIGNMOB(3818, guild_guard );
   ASSIGNMOB(3819, guild_guard );
   ASSIGNMOB(3820, guild_guard );

   /* The Mystic Sea Guardians */
   ASSIGNMOB(3901, guild_guard );
   ASSIGNMOB(3902, guild_guard );
   ASSIGNMOB(3903, guild_guard );

   /* Moria 1-2 */
   ASSIGNMOB(4000, snake);
   ASSIGNMOB(4001, snake);
   ASSIGNMOB(4053, snake);
   ASSIGNMOB(4102, snake);

   /* The Astral giths */
   ASSIGNMOB(4200, guild_guard ); /* Astral guardian */
   ASSIGNMOB(4209, guild_guard ); /* Githyanki protector */

   /* The Elven Forest */
   ASSIGNMOB( 4409, receptionist );

   /* The Great Eastern Desert */
   ASSIGNMOB( 5005, guild_guard );

   /* Pyramid of the Serpent */
   ASSIGNMOB(5712, superAggr);	/* Serpent-man assassin */
   ASSIGNMOB(5715, superAggr);	/* Serpent-man assassin */
   ASSIGNMOB(5710, superAggr);	/* Giant asp */
   ASSIGNMOB(5803, superAggr);  /* King Szithra */
   ASSIGNMOB(5816, superAggr);  /* Serpent-man assassin */
   ASSIGNMOB(5818, superAggr);  /* Child of the serpent */
   ASSIGNMOB(5821, superAggr);  /* Serpent-man assassin */
   ASSIGNMOB(5809, snake);      /*the amphisbaena */

   /* Haon-Dor Dark */
   ASSIGNMOB(6113, snake);

   /* Dwarven Kingdom */
   ASSIGNMOB(6500, cityguard);

   /* The Sewers of Midgaard */
   ASSIGNMOB(7006, snake);

   /* Myth-Try */
   ASSIGNMOB(8417, receptionist); /* Isstvaan */

   /* Mt. V-something :p */
   ASSIGNMOB(9705, superAggr);	/* Hail */


   /* LupusVille Day */
   ASSIGNMOB(10002, null_proc); /* Shopkeeper */
   ASSIGNMOB(10005, fido);
   ASSIGNMOB(10007, null_proc); /* Widow */
   ASSIGNMOB(10012, null_proc); /* Bartender */
   ASSIGNMOB(10013, receptionist);
   ASSIGNMOB(10023, null_proc); /* Citizen */
   ASSIGNMOB(10025, null_proc); /* Shopkeeper */
   ASSIGNMOB(10026, cityguard);
   ASSIGNMOB(10027, cityguard);

   /* LupusVille Night */
   ASSIGNMOB(10112, null_proc); /* Bartender */
   ASSIGNMOB(10113, receptionist);
   ASSIGNMOB(10125, null_proc); /* Shopkeeper */
   ASSIGNMOB(10126, cityguard);

   /* The Monk Monestary */
   ASSIGNMOB( 11012, receptionist );

   /* Shaden */
   ASSIGNMOB(11507, guild_guard);
   ASSIGNMOB(11513, rogue_cryogenicist);
   ASSIGNMOB(11514, rogue_receptionist);

   /* Town of freehold */
   ASSIGNMOB(12240, healer);
   ASSIGNMOB(12241, receptionist);

   /* Thieves Guild - Tough Guard */
   ASSIGNMOB(12504, guild_guard);

   /* Black Hand - Morgoth */
   ASSIGNMOB(12607, guild_guard);

   /* Astral plane */
   ASSIGNMOB(13200, superAggr); /* Takhisis */
   ASSIGNMOB(13205, guild_guard); /* Highlord */
   ASSIGNMOB(13210, superAggr); /* White Dragon wyrm */
   ASSIGNMOB(13220, superAggr); /* Black dragon wyrm */
   ASSIGNMOB(13230, superAggr); /* Green Dragon wyrm */
   ASSIGNMOB(13240, superAggr); /* Blue Dragon wyrm */
   ASSIGNMOB(13250, superAggr); /* Red Dragon wyrm */
   ASSIGNMOB(13290, guild_guard); /* githyanki knights */
   ASSIGNMOB(13385, superAggr); /* gith chief knight */
   ASSIGNMOB(13420, superAggr); /* Orcus */

   /* Chisha Village */
   ASSIGNMOB(42910, superAggr); /* Yun */

   /* New Thalos */
   ASSIGNMOB( 13504, receptionist );

   /* Olympus */
   ASSIGNMOB( 13842, receptionist );
   /* The War Camp */
   ASSIGNMOB( 13920, guild_guard );
   ASSIGNMOB( 13927, superAggr );

   /* Xerxes */
   ASSIGNMOB(14306, superAggr );	/* Night Stalker */
   ASSIGNMOB(14308, superAggr );	/* Life Stealer */
   ASSIGNMOB(14316, superAggr );	/* Drelnza */
   ASSIGNMOB(14322, superAggr );	/* Darkness */
   ASSIGNMOB(14501, superAggr );	/* Astaroth */
   ASSIGNMOB(14503, superAggr );	/* Soul Stealer */
   ASSIGNMOB(14505, superAggr );	/* Mist Stalker */
   ASSIGNMOB(14507, superAggr );	/* Deep Dweller */
   ASSIGNMOB(14517, superAggr );	/* The Mist */
   ASSIGNMOB(14701, superAggr );	/* King Xerxes */
   ASSIGNMOB(14704, superAggr );	/* Skathor */
   ASSIGNMOB(14713, superAggr );	/* Royal Amazon */
   ASSIGNMOB(14731, superAggr );	/* Nabassu */
   ASSIGNMOB(14741, superAggr );	/* Marilith */
   ASSIGNMOB(14742, superAggr );	/* Rehnaremme */
   ASSIGNMOB(14743, superAggr );	/* Aisapra */
   ASSIGNMOB(14744, superAggr );	/* Kevokuli */
   ASSIGNMOB(14745, superAggr );	/* Nalfeshnee */
   ASSIGNMOB(14746, superAggr );	/* Johud */
   ASSIGNMOB(14747, superAggr );	/* Bilwhr */
   ASSIGNMOB(14748, superAggr );	/* Junarga */
   ASSIGNMOB(14749, superAggr );	/* Grontlar */
   ASSIGNMOB(14750, superAggr );	/* Jumundgar */
   ASSIGNMOB(14112, guild_guard);
   ASSIGNMOB(14518, superAggr);          /* Zilanthor - dragon form */
   ASSIGNMOB(14519, guild_guard);
   ASSIGNMOB(14520, guild_guard);
   ASSIGNMOB(14521, guild_guard);
   ASSIGNMOB(14712, guild_guard);
   ASSIGNMOB(14739, guild_guard);
   ASSIGNMOB(14752, guild_guard);
   ASSIGNMOB(14753, guild_guard);

   /* The Tombs of Tarin */
   ASSIGNMOB( 15612, null_proc );
   ASSIGNMOB( 15624, guild_guard); /* huge ancient tree */
   ASSIGNMOB( 15628, null_proc );

   /* Legend */
   ASSIGNMOB( 15714, una );
   ASSIGNMOB( 15723, alicorn );

   /* ??(Hasana)?? */
   ASSIGNMOB( 15902, superAggr );

   /* The Isle of Mysts */
   ASSIGNMOB( 16213, guild_guard );
   ASSIGNMOB( 16214, guild_guard );

   /* The Jungle */
   ASSIGNMOB( 16407, guild_guard );

   /* Shanarra */
   /* ASSIGNMOB( 17721, receptionist ); */
   /* Samsera */
   ASSIGNMOB( 18001, cityguard );
   ASSIGNMOB( 18002, cityguard );
   ASSIGNMOB( 18003, cityguard );
   ASSIGNMOB( 18004, cityguard );
   ASSIGNMOB( 18005, cityguard );
   ASSIGNMOB( 18010, guild );
   ASSIGNMOB( 18011, guild );
   ASSIGNMOB( 18012, guild );
   ASSIGNMOB( 18013, guild );
   ASSIGNMOB( 18014, guild );
   ASSIGNMOB( 18015, guild );
   ASSIGNMOB( 38801, guild );  /* New Th guildleader */
   ASSIGNMOB( 18017, guild );
   ASSIGNMOB( 18018, guild );
   ASSIGNMOB( 18019, guild );
 /* ASSIGNMOB( 18030, guild_guard );commented out blocking enabled via flag
   ASSIGNMOB( 18031, guild_guard );
   ASSIGNMOB( 18032, guild_guard );
   ASSIGNMOB( 18033, guild_guard );
   ASSIGNMOB( 18034, guild_guard );
   ASSIGNMOB( 18035, guild_guard );
   ASSIGNMOB( 18036, guild_guard );
   ASSIGNMOB( 18037, guild_guard );
   ASSIGNMOB( 18038, guild_guard );
   ASSIGNMOB( 18039, guild_guard );
  */
   ASSIGNMOB( 18054, fido );
   ASSIGNMOB( 18055, fido );
   ASSIGNMOB( 18056, janitor );
   ASSIGNMOB( 18072, receptionist );
   ASSIGNMOB( 18073, postmaster);
   ASSIGNMOB( 18098, cryogenicist);
   ASSIGNMOB( 18099, metaphysician);
   ASSIGNMOB( 18078, healer );
   ASSIGNMOB( 18150, cityguard );
   ASSIGNMOB( 41101, locker_manager );
   ASSIGNMOB( 41102, rename_manager );

   /* The Under Dark */
   ASSIGNMOB( 20606, beholder);	/*  Scheming beholder */
   ASSIGNMOB( 20689, guild); /* Meliadus */
   ASSIGNMOB( 20690, guild); /* Thief trainer */
   ASSIGNMOB( 20700, guild); /* Assassin trainer */

   /* The Forgotten City */
   ASSIGNMOB( 21201, cityguard );
   ASSIGNMOB( 21202, cityguard );
   ASSIGNMOB( 21203, guild );
   ASSIGNMOB( 21207, receptionist );
   ASSIGNMOB( 21213, janitor );
   ASSIGNMOB( 21214, cityguard );
   ASSIGNMOB( 21230, guild );
   ASSIGNMOB( 21231, guild );
   ASSIGNMOB( 21232, guild );
   ASSIGNMOB( 21235, guild_guard ); /* Guards East */
   ASSIGNMOB( 21236, guild_guard ); /* Guards North */
   ASSIGNMOB( 21237, guild_guard ); /* Guards West */
   ASSIGNMOB( 21238, guild_guard ); /* Guards South */
   ASSIGNMOB( 21269, superAggr ); /* Fenris */

   /* The Forgotten Forest */
   ASSIGNMOB( 21315, unicorn); /* Unicorn */

   /* Avalon */
   ASSIGNMOB( 22301, guild_guard ); /* Guards North */
   ASSIGNMOB( 22302, guild_guard ); /* Guards North */
   ASSIGNMOB( 22307, guild_guard ); /* Guards Up    */
   ASSIGNMOB( 22309, guild_guard ); /* Guards North */

   /* Free port */
   ASSIGNMOB( 22940, receptionist); /* receptionist in green dragon */

   /* Yun */
   ASSIGNMOB( 23000, ibm);
   ASSIGNMOB( 23006, stjohn);

   // The BloodBowl
   //
   ASSIGNMOB( 30301, bloodref );

   // Fairhaven
   //
   ASSIGNMOB( 30715, guild );
   ASSIGNMOB( 30706, guild );
   ASSIGNMOB( 30701, guild );
   ASSIGNMOB( 30713, guild );
   ASSIGNMOB( 30711, guild );
   ASSIGNMOB( 30709, guild );
   ASSIGNMOB( 30704, guild );

   ASSIGNMOB( 30714, guild_guard );
   ASSIGNMOB( 30705, guild_guard );
   ASSIGNMOB( 30710, guild_guard );
   ASSIGNMOB( 30708, guild_guard );
   ASSIGNMOB( 30702, guild_guard );
   ASSIGNMOB( 30712, guild_guard );
   ASSIGNMOB( 30703, guild_guard );
   ASSIGNMOB( 30922, receptionist );
   ASSIGNMOB( 33127, receptionist );
   ASSIGNMOB( 38001, guild );
   ASSIGNMOB( 38902, guild );
  // ASSIGNMOB( 41299, receptionist);

   /* clans */
   ASSIGNMOB( 19301, guild_guard );
   ASSIGNMOB( 19305, guild_guard );
   ASSIGNMOB( 19306, healer );
   ASSIGNMOB( 19302, receptionist );
   ASSIGNMOB(  1298, receptionist );
   ASSIGNMOB( 19311, receptionist );
   ASSIGNMOB( 19310, guild_guard );
   ASSIGNMOB( 19314, guild_guard );
   ASSIGNMOB( 19315, receptionist );
   ASSIGNMOB( 19321, receptionist );
   ASSIGNMOB( 19320, guild_guard );
   ASSIGNMOB( 19324, guild_guard );
   ASSIGNMOB( 19330, guild_guard );
   ASSIGNMOB( 19331, receptionist );
   ASSIGNMOB( 19340, guild_guard );
   ASSIGNMOB( 19341, receptionist );
   ASSIGNMOB( 19350, guild_guard );
   ASSIGNMOB( 19351, guild_guard );
   ASSIGNMOB( 19352, receptionist );
   ASSIGNMOB( 19353, guild_guard );
   ASSIGNMOB( 19370, receptionist );
   ASSIGNMOB( 19371, guild_guard );
   ASSIGNMOB( 19380, guild_guard );
   ASSIGNMOB( 19381, receptionist );
   ASSIGNMOB( 19390, guild_guard );
   ASSIGNMOB( 19391, receptionist );
   ASSIGNMOB( 19386, guild_guard );
   ASSIGNMOB( 19396, guild_guard );
   ASSIGNMOB( 19397, receptionist );
   ASSIGNMOB( 37806, guild_guard );
   ASSIGNMOB( 37808, receptionist );
   ASSIGNMOB( 19800, guild_guard );
   ASSIGNMOB( 19801, receptionist );
   ASSIGNMOB( 19303, bank );
   ASSIGNMOB( 19322, bank );
   ASSIGNMOB( 19317, guild_guard );
   ASSIGNMOB( 19319, receptionist );

   //Grim Marauders
   ASSIGNMOB( 19334, receptionist );
   ASSIGNMOB( 19333, guild_guard );

   /* Guilds */
   ASSIGNMOB( 39001, guild_guard );
   ASSIGNMOB( 39002, guild );
   ASSIGNMOB( 38502, guild_guard );
   ASSIGNMOB( 38505, guild );

   /* Transylvania */
   ASSIGNMOB( 43916, guild_guard );

   /* Forgers  */
   ASSIGNMOB( 4850,  forgeshop); // level 30 forger
   ASSIGNMOB( 13912, forgeshop); // level 38 forger
   ASSIGNMOB( 13808, forgeshop); // level 42 forger
   ASSIGNMOB( 27135, forgeshop); // level 45 forger
   ASSIGNMOB( 40132, forgeshop); // level 50 forger
   ASSIGNMOB( 3811,  forgeshop); // level 50 forger

   /* Elie the prophet */
   ASSIGNMOB(18083,  identifyshop);

   /* remort questmaster */
   ASSIGNMOB(2227 ,  remort_questmaster); // gnome wife
   
   /* Advancer */
   ASSIGNMOB(18084, advanceshop);
}


/* assign special procedures to objects */
void assign_objects(void)
{
#ifdef OLD_PORTAL
    ASSIGNOBJ(1299,  portal_proc);     /* the portals   */
    ASSIGNOBJ(2838,  portal_proc);     /* Painting in Asgard */
    ASSIGNOBJ(13211, portal_proc);     /* Abyss color pool */
    ASSIGNOBJ(13212, portal_proc);     /* Galaxy color pool */
    ASSIGNOBJ(13215, portal_proc);     /* Color pool in abyss, back to astral */
    ASSIGNOBJ(21532, portal_proc);     /* Portal to drow city */
    ASSIGNOBJ(23621, portal_proc);     /* Portal to Errtu */
#endif

    ASSIGNOBJ(3097, gen_board); /* freeze board */
    ASSIGNOBJ(3098, gen_board); /* immortal board */
    ASSIGNOBJ(18111, gen_board); /* Sam Advent board */
    ASSIGNOBJ(3099, gen_board); /* New Hope Guild */
    ASSIGNOBJ(1201, gen_board); /* building board */
    ASSIGNOBJ(20150, gen_board); /* quest board */
    ASSIGNOBJ(18123, gen_board); /* Ad board */
    ASSIGNOBJ(1203, gen_board); /* Vex's old board */
    ASSIGNOBJ(1204, gen_board); /* Policy Board */
    ASSIGNOBJ(1208, gen_board); /* Arbaces board */
    ASSIGNOBJ(1209, gen_board); /* diggers laptop */
    ASSIGNOBJ(1210, gen_board); /* available */
    ASSIGNOBJ(1211, gen_board); /* Caleb's Board */
    ASSIGNOBJ(1215, gen_board); /* available */
    ASSIGNOBJ(1216, gen_board); /* Coder's board */
    ASSIGNOBJ(1249, gen_board); /* another imm board */
    ASSIGNOBJ(1280, gen_board); /* Xiuh's board */
    ASSIGNOBJ(1266, gen_board); /* Nicor's board */
    ASSIGNOBJ(1288, gen_board); /* Enki's Board */
    ASSIGNOBJ(1289, gen_board); /* IMM Quest Board */
    ASSIGNOBJ(1290, gen_board); /* available */
    ASSIGNOBJ(1291, gen_board); /* available */
    ASSIGNOBJ(1292, gen_board); /* available */
    ASSIGNOBJ(1294, gen_board); /* available */
    ASSIGNOBJ(1295, gen_board); /* Th board */
    ASSIGNOBJ(1296, gen_board); /* available */
    ASSIGNOBJ(1297, gen_board); /* available */
    ASSIGNOBJ(38000, gen_board); /* Wa board*/
    ASSIGNOBJ(38100, gen_board); /* Ra Board */
    ASSIGNOBJ(38200, gen_board); /* Sl Board */
    ASSIGNOBJ(38300, gen_board); /*Cl Board*/
    ASSIGNOBJ(38400, gen_board); /* Sk board */
    ASSIGNOBJ(38500, gen_board); /* Dk board */
    ASSIGNOBJ(38600, gen_board); /* Mu Board */
    ASSIGNOBJ(38700, gen_board); /* sd board */
    ASSIGNOBJ(38900, gen_board); /*As board */
    ASSIGNOBJ(38001, gen_board); /* Wa challenge board */
    ASSIGNOBJ(38101, gen_board); /* Ra challenge board */
    ASSIGNOBJ(38201, gen_board); /* Sl challenge board */
    ASSIGNOBJ(38301, gen_board); /* Cl challenge board */
    ASSIGNOBJ(38401, gen_board); /* Sk challenge board */
    ASSIGNOBJ(38601, gen_board); /* Mu challenge board */
    ASSIGNOBJ(38701, gen_board); /* Sd challenge board */
    ASSIGNOBJ(38801, gen_board); /* Th challenge board */
    ASSIGNOBJ(38901, gen_board); /* Bounty Hunter board */
    ASSIGNOBJ(41299, gen_board); /* Nm Board */

    ASSIGNOBJ(3034,  bank);	/* atm */
    ASSIGNOBJ(18112, bank);
    ASSIGNOBJ(22940, bank);
    ASSIGNOBJ(30602, bank);  // Fairhaven

    ASSIGNOBJ(21822, mobLauncher);

    ASSIGNOBJ(896, random_dice);	/* The Dice Of Fate */

    ASSIGNOBJ(30000, blood_ball);	/* Blood Bowl Ball */


    /* clans */
    ASSIGNOBJ(1287, gen_board); /* Genesis Clan */
    ASSIGNOBJ(19301, gen_board); /* Forsaken */
    ASSIGNOBJ(1275, gen_board); /* Highlanders */
    ASSIGNOBJ(19310, gen_board); /* Order Darkness #1 */
    ASSIGNOBJ(19314, gen_board); /* available */
    ASSIGNOBJ(19320, gen_board); /* Aftermath */
    ASSIGNOBJ(19324, gen_board); /* available */
    ASSIGNOBJ(19325, gen_board); /* available */
    ASSIGNOBJ(19330, gen_board); /* The Occult */
    ASSIGNOBJ(19340, gen_board); /* Coven */
    ASSIGNOBJ(19353, gen_board); /* The Wanderers */
    ASSIGNOBJ(19355, gen_board); /* The Wanderers */
    ASSIGNOBJ(19370, gen_board); /* The Brethren */
    ASSIGNOBJ(19380, gen_board); /* The Tarsis Shriners */
    ASSIGNOBJ(19390, gen_board); /* Eternal Damnation */
    ASSIGNOBJ(19397, gen_board); /* Order Darkness #2 */
    ASSIGNOBJ(19318, gen_board); /*The Republic */
}

/* assign special procedures to rooms */
void assign_rooms(void)
{
  ASSIGNROOM(3031,  pet_shops);
  ASSIGNROOM(5765,  roomAggro);
  ASSIGNROOM(5843,  roomFall);
  ASSIGNROOM(5814,  roomFall);
  ASSIGNROOM(16321, roomFall);
  ASSIGNROOM(18141, pet_shops);
  ASSIGNROOM(23003, heavens_door);
  ASSIGNROOM(30780, pet_shops);
  ASSIGNROOM(9084,  xactuachak);  /* Spec proc for lands of chaos */
  ASSIGNROOM(18083, qpshop);
}
/* Soon to replace some of the old functions in specials.c
struct spec_func_data {
   char *name;
   SPECIAL(*func);
};

struct spec_func_data spec_func_list[] = {
  {"Mayor",          mayor },
  {"Snake",          snake },
  {"Thief",          thief },
  {"Magic User",     magic_user },
  {"Puff",           puff },
  {"Fido",           fido },
  {"Janitor",        janitor },
  {"Cityguard",      cityguard },
  {"Postmaster",     postmaster },
  {"Receptionist",   receptionist },
  {"Cryogenicist",   cryogenicist},
  {"Bulletin Board", gen_board },
  {"Bank",           bank },
  {"Pet Shop",       pet_shops },
  {"Dump",           dump },
  {"Guildmaster",    guild },
  {"Guild Guard",    guild_guard },
  {"Questmaster",    questmaster },
  {"Shopkeeper",     shop_keeper },
  {"\n", NULL}
}; 

const char *get_spec_func_name(SPECIAL(*func))
{
  int i;
  for (i=0; *(spec_func_list[i].name) != '\n'; i++) {
    if (func == spec_func_list[i].func) return (spec_func_list[i].name);
  }
  return NULL;
}
*/
