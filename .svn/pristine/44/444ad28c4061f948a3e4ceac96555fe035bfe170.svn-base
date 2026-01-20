/*
** This is the collection of player torment functions that I always threatened
** to write. This file contains the mob launcher and clonemaster that has long
** since been needed to spice things up for the high level players who get bored.
**
**                                                   Digger 9/1/98
*/

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/handler.h"
#include "actions/act.h"          /* ACMDs located within the act*.c files */
#include "general/class.h"
#include "general/comm.h"
#include "actions/interpreter.h"
#include "util/utils.h"
#include "specials/special.h"
#include "magic/spells.h"

/*
** Locally Global Variables (gotta love that phrase)
*/
#define DEFAULT_COUNT 50
#define DEFAULT_SALVO 6
#define DEFAULT_MOB 11208

static int launcherOn = FALSE;
static int targetRoom = 18001;
static int maxSalvo   = DEFAULT_SALVO;
static int maxLaunch  = DEFAULT_COUNT;
static int totLaunch  = 0;
static int currentMob = DEFAULT_MOB;
char currentPlayer[80] = "";

static CharData* targetPC = NULL;

/*
** The launcher command structures
*/
typedef struct
{
  int (*launchFunc)(CharData *, int, char *);
  char *cmd;        /* The launch command */
  int   minLevel;   /* The min level to issue this command */
} launchCmd;

static int launchClear(  CharData*, int, char* );
static int launchHelp(   CharData*, int, char* );
static int launchList(   CharData*, int, char* );
static int launchStart(  CharData*, int, char* );
static int launchStop(   CharData*, int, char* );
static int launchTarget( CharData*, int, char* );
static int launchLaunchee( CharData*, int, char* );

launchCmd launchCmds[] = {
  { launchClear,  "clear",  51},
  { launchHelp,   "help",   51},
  { launchList,   "list",   51},
  { launchStart,  "start",  51}, /* MaxMobs MaxSalvo */
  { launchStop,   "stop",   51},
  { launchTarget, "target", 51},
  { launchLaunchee, "mob", 51},
  { NULL,         "",        0}

};

/*
** The Launcher Functions
**
** These functions are written this way to provide a more object oriented approach
** to programming these little command processors.
*/
static int
launchHelp( CharData *ch, int cmd, char* args )
{
  sendChar( ch, "\r\n"
	            "\r\n"
			"&09Usage: &10launch <cmnd> <args> &09where &10<cmnd> &09is one of:&00          \r\n"
			"&10help        &09show this screen&00                                          \r\n"
			"&10clear       &09halts and clears the launch sequence&00                      \r\n"
			"&10list        &09lists the current firing sequence&00                         \r\n"
			"&10target   &08<&10PC&08|&10Room&08>&10  &09targets a player, group, or area&00\r\n"
			"&10start    &08<&10Cnt&08> <&10MaxSalvo&08>&00                                 \r\n"
			"&09begins the loaded firing sequence&00                                        \r\n"
			"&10stop        &09pauses the active sequence&00                                \r\n"
			"&10mob <vnum>  &09sets the current mob&00                                      \r\n"
			"\r\n" );

  return(1);
}


/*
**
*/
void
resetLauncher()
{
  launcherOn = FALSE;
  currentMob = DEFAULT_MOB;
  maxLaunch  = DEFAULT_COUNT;
  maxSalvo   = DEFAULT_SALVO;
  totLaunch  = 0;
  strcpy(currentPlayer,"");
}


/*
**
*/
static int
launchClear( CharData *ch, int cmd, char* args )
{
  sendChar( ch, "\n&10Launch sequence cleared.&00\n", args );
  mudlog(NRM, LVL_IMMORT, TRUE, "TORMENT: Launch sequence cleared by %s", GET_NAME(ch) );

  launcherOn = FALSE;
  return(1);
}


/*
**
*/
static int
launchList( CharData *ch, int cmd, char* args )
{
  sendChar( ch, 
	  "&09   Launcher [&10%3s&09]&00    &09Target Room [&10%5d&09]&00\r\n"
	  "&09 Launch Cnt [&10%3d&09]&00    &09  Salvo Max [&10%3d&09]&00\r\n"
	  "&09 Launch Max [&10%3d&09]&00    &09  Mob       [&10%5d&09]&00\r\n"
	  "&09 Launch plr [&10%s&09]&00                                  \r\n"
	  "\r\n", 
	  (launcherOn ? "On":"Off"), targetRoom, totLaunch, maxSalvo, maxLaunch, currentMob, currentPlayer );

  return(1);
}


/*
**
*/
static int
launchTarget( CharData *ch, int cmd, char* args )
{
  int rNum = -1;

  if( ch == NULL || args == NULL || strlen(args) == 0 ) return(0);


  rNum = find_target_room( ch, args );

  if( rNum < 0 )
  {
    mudlog(NRM, LVL_IMMORT, TRUE, "TORMENT: %s cannot target [%s]", GET_NAME(ch), args );
    return(0);
  }
  else
  {
    CharData* tmpPC;

    skip_spaces(&args);
    tmpPC = get_char_vis( ch, args , 1 );

    targetRoom = world[rNum].number;

    if( tmpPC == NULL )
    {
      mudlog(NRM, LVL_IMMORT, TRUE, "TORMENT: %s targetting room r:%d v:%d arg:[%s]",
                GET_NAME(ch), rNum, targetRoom, args );
      strcpy( currentPlayer, "");
    }
    else
    {
      mudlog(NRM, LVL_IMMORT, TRUE, "TORMENT: %s targetting PC %s arg:[%s]",
                GET_NAME(ch), GET_NAME(tmpPC), args );
      strcpy( currentPlayer, GET_NAME(tmpPC));
    }

    targetPC = tmpPC;

    return(1);
  }
}
/*
**
*/   
static int
launchLaunchee( CharData *ch, int cmd, char* args )
{
  int mob;
  if( ch == NULL || args == NULL || strlen(args) == 0 ) return(0);
  mob=atoi(args);
  if (mob==0) return 0;

  mudlog(NRM, LVL_IMMORT, TRUE, "TORMENT: Launch Mob changed by %s, mob=%d", GET_NAME(ch), mob );
  
  currentMob=mob;
    
  return(1);
}

/*
**
*/
static int
launchStart( CharData *ch, int cmd, char* args )
{
  mudlog(NRM, LVL_IMMORT, TRUE, "TORMENT: Launch sequence initiated by %s", GET_NAME(ch) );

  launcherOn = TRUE;

  return(1);
}


/*
**
*/
static int
launchStop( CharData *ch, int cmd, char* args )
{
  mudlog(NRM, LVL_IMMORT, TRUE, "TORMENT: Launch sequence was terminted by %s", GET_NAME(ch) );

  launcherOn = FALSE;
  return(1);
}


/*
** In its simplest form this function could just launch orcs into an area
** much the same way I raid town from time to time. However, it is hoped
** that eventually this code can launch all sorts of groups of mobs at
** various players all over the mud.
**
** FYI: If spells need to be added try:
**
** AffectedType spDetInv = { SPELL_DETECT_INVIS, 50, AFF_DETECT_INVIS, APPLY_NONE, 0 };
**
** affect_join( mob, &spDetInv, FALSE, FALSE, FALSE, FALSE);
**
*/
static void
doLaunch()
{
  int numMobs  = dice( 2, maxSalvo );
  int vMobNum  = currentMob; // 11208;
  int rMobNum  = 0;
  int rRoomNum = 0;
  int idx      = 0;
  int lCount   = 0;
  int tgtArea  = 0;

  if( targetPC != NULL )
  {
    int rNum = find_target_room( targetPC, GET_NAME(targetPC) );

    if( rNum < 0 )
    {
      mudlog(NRM, LVL_IMMORT, TRUE, "TORMENT: lost target [%s]", GET_NAME(targetPC) );
      resetLauncher();
      return;
    }

    targetRoom = world[rNum].number;
  }

  tgtArea = targetRoom + dice( 0, 2 );

  for( idx = 0; idx < numMobs; idx++ )
  {
    CharData* mob = NULL;
    rMobNum   = real_mobile( vMobNum );
    rRoomNum  = real_room( tgtArea );

    if( rMobNum < 0 )
    {
      mudlog(NRM, LVL_IMMORT, TRUE, "TORMENT: doLaunch failed to load mob number %d", vMobNum );
      resetLauncher();
      return;
    }

    if( rRoomNum < 0 )
    {
      mudlog(NRM, LVL_IMMORT, TRUE, "TORMENT: doLaunch failed to validate target room %d", tgtArea );
      resetLauncher();
      return;
    }

    mob = read_mobile( rMobNum, REAL );
    if( mob != NULL )
    {
      SET_BIT_AR(AFF_FLAGS(mob), AFF_INFRAVISION);
      SET_BIT_AR(AFF_FLAGS(mob), AFF_DETECT_INVIS);
      char_to_room( mob, rRoomNum );
      lCount += 1;
    }
  }

  if( lCount > 0 )
  {
    send_to_outdoor( 
		"&09You hear a muffled &08*WHOOMF*&00\r\n"
		"&09A blazing ball of cobalt blue energy shrieks past overhead.&00\r\n");

    send_to_room( 
		"&08A blazing ball of energy &10explodes&08, tossing monsters everywhere!&00\r\n", rRoomNum );

    totLaunch += lCount;

    mudlog(NRM, LVL_IMMORT, TRUE, "TORMENT: Launched %d mobs into room %d [%d:%d]",
              lCount, tgtArea, totLaunch, maxLaunch );

    if( totLaunch >= maxLaunch )
    {
      resetLauncher();

      mudlog(NRM, LVL_IMMORT, TRUE, "TORMENT: Launch sequence complete - weapons tight" );
    }

  }

}


static int
validateLaunchCmd( char* args )
{
  if( args != NULL )
  {
    char theCmd[80] = "";
    int  i = 0;

    args = one_argument( args, theCmd );

    for( i = 0; launchCmds[i].launchFunc != NULL; i++ )
    {
      if( strcmp( launchCmds[i].cmd, theCmd ) == 0 )
        return(i);
    }
  }

  return(-1);
}

/*
**
*/
int
mobLauncher( CharData* ch, void* me, int cmd, char* args )
{
  int cmdIdx = validateLaunchCmd( args );

  if( launcherOn && cmd == 0 ) { doLaunch(); }

  if( cmdIdx < 0 ) return( FALSE );

  else
  {
    /*
    ** Strip the command, leave the args, and invoke the func.
    */
    char theCmd[80] = "";
    args = one_argument( args, theCmd );

    launchCmds[cmdIdx].launchFunc( ch, cmd, args );
  }

  return( TRUE );
}


SPECIAL(hitSquad)
{
#if 0
  char      tgtname[80] = "";
  char      objname[80] = "";
  ObjData*  obj         = NULL;
  
  if( !ch->desc || IS_NPC(ch) ) return FALSE;

#endif
  return( FALSE );
}

