
#include "magic/stun.h"
#include "magic/knock.h"
#include "magic/backstab.h"
#include "magic/aggressive.h"
#include "magic/epic.h"
#include "magic/charge.h"
#include "magic/flashbang.h"


ACMD(do_action);
ACMD(do_advance);
ACMD(do_affected);
ACMD(do_alias);
ACMD(do_ambush);
ACMD(do_area);
ACMD(do_arenarank);
ACMD(do_assist);
ACMD(do_aspect);
ACMD(do_at);
ACMD(do_auction);
ACMD(do_auction_stat);
ACMD(do_ban);
ACMD(do_befriend);
ACMD(do_berserk);
ACMD(do_bid);
ACMD(do_block);
ACMD(do_brain);
ACMD(do_blackjack);
ACMD(do_breathe);
ACMD(do_brew);
ACMD(do_chown);
ACMD(do_convert);
ACMD(do_demote);
ACMD(do_devour);
ACMD(do_doorbash);
ACMD(do_blind_strike);
ACMD(do_butcher);
ACMD(do_cast);
ACMD(do_castout);
ACMD(do_calm);
ACMD(do_camp);
ACMD(do_circle);
ACMD(do_clone);
ACMD(do_close);
ACMD(do_clans);
ACMD(do_commands);
ACMD(do_consider);
ACMD(do_copy);
ACMD(do_cower);
ACMD(do_corpse);
ACMD(do_credits);
ACMD(do_cut_throat);
ACMD(do_date);
ACMD(do_dc);
ACMD(do_delusion);
ACMD(do_diagnose);
ACMD(do_disarm);
ACMD(do_display);
ACMD(do_distract);
ACMD(do_drink);
ACMD(do_drop);
ACMD(do_dust);
ACMD(do_dump);
ACMD(do_eat);
ACMD(do_echo);
ACMD(do_enter);
ACMD(do_entrance);
ACMD(do_envenom);
ACMD(do_escape);
ACMD(do_equipment);
ACMD(do_examine);
ACMD(do_expose);
ACMD(do_exit);
ACMD(do_exits);
ACMD(do_false_trail);
ACMD(do_feign_death);
ACMD(do_feed);
ACMD(do_familiarize);
ACMD(do_file);
ACMD(do_flag);
ACMD(do_flee);
ACMD(do_follow);
ACMD(do_force);
ACMD(do_forge);
ACMD(do_gecho);
ACMD(do_gen_comm);
ACMD(do_gen_ps);
ACMD(do_gen_tog);
ACMD(do_gen_write);
ACMD(do_get);
ACMD(do_give);
ACMD(do_glance);
ACMD(do_showchores);
ACMD(do_gold);
ACMD(do_gore);
ACMD(do_goto);
ACMD(do_grab);
ACMD(do_group);
ACMD(do_guard);
ACMD(do_gut);
ACMD(do_gsay);
ACMD(do_hands);
ACMD(do_hcontrol);
ACMD(do_oasis_hedit);
ACMD(do_help);
ACMD(do_hide);
ACMD(do_hit);
ACMD(do_house);
ACMD(do_hug);
ACMD(do_info);
ACMD(do_insult);
ACMD(do_inventory);
ACMD(do_invis);
ACMD(do_ipcheck);
ACMD(do_innereye);
ACMD(do_invigorate);
ACMD(do_instant_poison);
ACMD(do_kill);
ACMD(do_land);
ACMD(do_last);
ACMD(do_leave);
ACMD(do_levels);
ACMD(do_liblist);
ACMD(do_load);
ACMD(do_lock);
ACMD(do_logfile);
ACMD(do_look);
ACMD(do_meditate);
ACMD(do_meld);
ACMD(do_mist);
ACMD(do_mix);
ACMD(do_move);
ACMD(do_mount);
ACMD(do_muckle);
ACMD(do_multi);
ACMD(do_noteam);
ACMD(do_nowho);
ACMD(do_not_here);
ACMD(do_offer);
ACMD(do_olc);
ACMD(do_open);
ACMD(do_order);
ACMD(do_page);
ACMD(do_palm);
ACMD(do_pardon);
ACMD(do_pick);
ACMD(do_pickpocket);
ACMD(do_players);
ACMD(do_poll);
ACMD(do_poofset);
ACMD(do_poultice);
ACMD(do_pour);
ACMD(do_advance_toggle);
ACMD(do_practice);
ACMD(do_purge);
ACMD(do_put);
ACMD(do_prayer);
ACMD(do_qecho);
ACMD(do_quest);
ACMD(do_questPnts);
ACMD(do_qcomm);
ACMD(do_quit);
ACMD(do_rawDamage);
ACMD(do_reelin);
ACMD(do_redoubt);
ACMD(do_reload);
ACMD(do_reboot);
ACMD(do_reimb);
ACMD(do_remove);
ACMD(do_remort);
ACMD(do_rent);
ACMD(do_reply);
ACMD(do_report);
ACMD(do_rescue);
ACMD(do_rest);
ACMD(do_restore);
ACMD(do_retarget);
ACMD(do_retreat);
ACMD(do_return);
// ACMD(do_rub);
ACMD(do_save);
ACMD(do_say);
ACMD(do_scan);
ACMD(do_score);
ACMD(do_send);
ACMD(do_set);
ACMD(do_seek);
ACMD(do_sing);
ACMD(do_stop);
ACMD(do_sting);
ACMD(do_peace);
ACMD(do_pseek);
ACMD(do_scatter);
ACMD(do_scribe);
ACMD(do_shadowdance);
ACMD(do_shadowmist);
ACMD(do_shadowstep);
ACMD(do_shadowjump);
ACMD(do_shoot);
ACMD(do_show);
ACMD(do_shutdown);
ACMD(do_sit);
ACMD(do_skillset);
ACMD(do_sleep);
ACMD(do_sneak);
ACMD(do_snoop);
ACMD(do_spec_comm);
ACMD(do_split);
ACMD(do_spy);
ACMD(do_stalk);
ACMD(do_stand);
ACMD(do_stat);
ACMD(do_steal);
ACMD(do_steadfastness);
ACMD(do_switch);
ACMD(do_tally);
ACMD(do_tedit);
ACMD(do_teleport);
ACMD(do_tell);
ACMD(do_throw);
ACMD(do_tick);
ACMD(do_time);
ACMD(do_title);
ACMD(do_toggle);
ACMD(do_track);
ACMD(do_trans);
ACMD(do_transquest);
ACMD(do_trap);
ACMD(do_detect_trap);
ACMD(do_disable_trap);
ACMD(do_tsay);
ACMD(do_turn);
ACMD(do_unban);
ACMD(do_ungroup);
ACMD(do_dismount);
ACMD(do_unlock);
ACMD(do_update);
ACMD(do_use);
ACMD(do_users);
ACMD(do_valentine);
ACMD(do_vote);
ACMD(do_visible);
ACMD(do_vnum);
ACMD(do_vstat);
ACMD(do_qstat);
ACMD(do_qlist);
ACMD(do_wake);
ACMD(do_wear);
ACMD(do_weather);
ACMD(do_where);
ACMD(do_who);
ACMD(do_wield);
ACMD(do_wizlock);
ACMD(do_wiznet);
ACMD(do_wizslap);
ACMD(do_wizutil);
ACMD(do_write);
ACMD(do_xname);
ACMD(do_zombies);
ACMD(do_zombiegift);
ACMD(do_zreset);

/* DG Script ACMD's */
ACMD(do_attach);
ACMD(do_detach);
ACMD(do_tlist);
ACMD(do_tstat);
ACMD(do_masound);
ACMD(do_mkill);
ACMD(do_mjunk);
ACMD(do_mdoor);
ACMD(do_mechoaround);
ACMD(do_msend);
ACMD(do_mecho);
ACMD(do_mload);
ACMD(do_mpurge);
ACMD(do_mgoto);
ACMD(do_mat);
ACMD(do_mteleport);
ACMD(do_mforce);
ACMD(do_mexp);
ACMD(do_mgold);
ACMD(do_mhunt);
ACMD(do_mremember);
ACMD(do_mforget);
ACMD(do_mtransform);
ACMD(do_vdelete);

/* This is the Master Command List(tm).
 *
 * You can put new commands in, take commands out, change the order
 * they appear in, etc.  You can adjust the "priority" of commands
 * simply by changing the order they appear in the command list.
 * (For example, if you want "as" to mean "assist" instead of "ask",
 * just put "assist" above "ask" in the Master Command List(tm).
 *
 * In general, utility commands such as "at" should have high priority;
 * infrequently used and dangerously destructive commands should have low
 * priority.
 */

#define NONE 0
#define HIDE AFF_HIDE

const struct command_info cmd_info[] = {
  { "RESERVED", 0, 0, 0, 0, 0 },/* this must be first -- for specprocs */

/*
**     CMD          MIN_POSITION    FUNCNAME        MINLVL          SUBCMD          UNAFFECT
*/
    { "north"     , POS_STANDING,   do_move       , 1             , SCMD_NORTH    , NONE },
    { "east"      , POS_STANDING,   do_move       , 1             , SCMD_EAST     , NONE },
    { "south"     , POS_STANDING,   do_move       , 1             , SCMD_SOUTH    , NONE },
    { "west"      , POS_STANDING,   do_move       , 1             , SCMD_WEST     , NONE },
    { "up"        , POS_STANDING,   do_move       , 1             , SCMD_UP       , NONE },
    { "down"      , POS_STANDING,   do_move       , 1             , SCMD_DOWN     , NONE },
/*
**     CMD          MIN_POSITION    FUNCNAME        MINLVL          SUBCMD          UNAFFECT
*/
    { "at"        , POS_DEAD      , do_at         , LVL_SAINT     , 0             , NONE },
    { "advance"   , POS_DEAD      , do_advance    , LVL_GRGOD     , 0             , NONE },
    { "affected"  , POS_DEAD      , do_affected   , LVL_CREATOR   , 0             , NONE },
    { "aggressive", POS_DEAD	  , do_aggressive , 0             , 0             , NONE },
    { "alias"     , POS_DEAD      , do_alias      , 1             , 0             , NONE },
    { "ambush"    , POS_STANDING  , do_ambush     , 0             , 0             , NONE },
    { "area"      , POS_DEAD      , do_area       , 1             , 0             , NONE },
    { "arenarank" , POS_DEAD      , do_arenarank  , 1             , 0             , NONE },
    { "assist"    , POS_FIGHTING  , do_assist     , 1             , 0             , HIDE },
    { "aspect"    , POS_FIGHTING  , do_aspect     , 0             , 0             , HIDE },
    { "ask"       , POS_RESTING   , do_spec_comm  , 1             , SCMD_ASK      , HIDE },
    { "auction"   , POS_RESTING   , do_auction    , 1             , 0             , NONE },
    { "autoexit"  , POS_DEAD      , do_gen_tog    , 1             , SCMD_AUTOEXIT , NONE },
    { "autoloot"  , POS_DEAD	  , do_gen_tog    , 1	          , SCMD_AUTOLOOT , NONE },
    { "autogold"  , POS_DEAD      , do_gen_tog	  , 1 	          , SCMD_AUTOGOLD , NONE },
    { "autosplit" , POS_DEAD	  , do_gen_tog    , 1             , SCMD_AUTOSPLIT, NONE },
    { "anon"      , POS_DEAD      , do_gen_tog    , 1             , SCMD_ANON     , NONE },
/*
**     CMD          MIN_POSITION    FUNCNAME        MINLVL          SUBCMD          UNAFFECT
*/
    { "backstab"  , POS_FIGHTING  , do_backstab   , 0             , 0             , HIDE },
    { "battlecry" , POS_FIGHTING  , do_battlecry  , 0             , 0             , HIDE },
    { "ban"       , POS_DEAD      , do_ban        , LVL_GRGOD    , 0             , NONE },
    { "bash"      , POS_FIGHTING  , do_bash       , 0             , 0             , HIDE },
    { "befriend"  , POS_STANDING  , do_befriend   , 0             , 0             , HIDE },
    { "berserk"   , POS_FIGHTING  , do_berserk    , 0             , 0             , HIDE },
    { "bid"       , POS_SLEEPING  , do_bid        , 1             , 0             , NONE },
    { "brain"	  , POS_FIGHTING  , do_brain	  , LVL_IMMORT    , 0             , HIDE },
    { "breathe"	  , POS_FIGHTING  , do_breathe    , 0	          , 0             , HIDE },
    { "brew"      , POS_STANDING  , do_brew       , 1             , 0             , HIDE },
    { "blind"     , POS_FIGHTING  , do_blind_strike, 0            , 0             , HIDE },
    { "blackjack" , POS_FIGHTING  , do_blackjack  , 0             , 0             , HIDE },
    { "brief"     , POS_DEAD      , do_gen_tog    , 1             , SCMD_BRIEF    , NONE },
    { "buy"       , POS_STANDING  , do_not_here   , 1             , 0             , HIDE },
    { "butcher"   , POS_STANDING  , do_butcher    , 0             , 0             , HIDE },
    { "bug"       , POS_DEAD      , do_gen_write  , 1             , SCMD_BUG      , NONE },
/*
**     CMD          MIN_POSITION    FUNCNAME        MINLVL          SUBCMD          UNAFFECT
*/
    { "cast"      , POS_SITTING   , do_cast       , 1             , 0             , HIDE },
    { "castout"   , POS_SITTING   , do_castout    , 0             , 0             , HIDE },
    { "calm"      , POS_FIGHTING  , do_calm       , 0             , 0             , HIDE },
    { "camp"      , POS_STANDING  , do_camp       , 0             , 0             , HIDE },
    { "capture"   , POS_RESTING   , do_drop       , 1             , SCMD_CAPT     , NONE },
    { "charge"    , POS_STANDING  , do_charge     , 0             , 0             , HIDE },
    { "check"     , POS_STANDING  , do_not_here   , 1             , 0             , HIDE },
    { "chown"	  , POS_DEAD	  , do_chown	  , LVL_IMMORT	  , 0		  , NONE },
    { "circle"    , POS_FIGHTING  , do_circle     , 0             , 0             , HIDE },
    { "clear"     , POS_DEAD      , do_gen_ps     , 1             , SCMD_CLEAR    , NONE },
    { "clone"     , POS_SITTING   , do_clone      , LVL_GRGOD     , 0             , NONE },
    { "close"     , POS_SITTING   , do_close      , 1             , 0             , NONE },
    { "clan"      , POS_SLEEPING  , do_gen_comm   , 1             , SCMD_CLAN     , NONE },
    { "clanquit"  , POS_STANDING  , do_clanquit   , 1             , 0             , NONE },
    { "clanadv"   , POS_RESTING   , do_clanadv    , 1             , 0             , NONE },
    { "clanlist"  , POS_SLEEPING  , do_clanlist   , 1             , 0             , NONE },
    { "clandem"   , POS_RESTING   , do_clandem    , 1             , 0             , NONE },
    { "clanwho"   , POS_SLEEPING  , do_clanwho    , 1             , 0             , NONE },
    { "clans"     , POS_SLEEPING  , do_clans      , LVL_GRGOD     , 0             , NONE },
    { "consider"  , POS_MEDITATING, do_consider   , 1             , 0             , NONE },
    { "consent"   , POS_MEDITATING, do_gen_tog    , 1             , SCMD_CONSENT  , HIDE },
    { "commands"  , POS_DEAD      , do_commands   , 1             , SCMD_COMMANDS , NONE },
    { "commandingshout", POS_FIGHTING, do_advance_toggle, 0       , SCMD_COM_SHOUT, HIDE },
    { "compact"   , POS_DEAD      , do_gen_tog    , 1             , SCMD_COMPACT  , NONE },
    { "copy"	  , POS_DEAD	  , do_copy	  , LVL_DEITY	  , 0             , NONE },
    { "cower"     , POS_FIGHTING  , do_cower      , 0             , 0             , HIDE },
    { "corpse"	  , POS_DEAD	  , do_corpse     , LVL_CREATOR	  , 0             , NONE },
    { "convert"   , POS_STANDING  , do_convert    , 0             , 0             , HIDE },
    { "credits"   , POS_DEAD      , do_gen_ps     , 1             , SCMD_CREDITS  , NONE },
    { "cutthroat" , POS_STANDING  , do_cut_throat , 0		      , 0             , HIDE },
/*
**     CMD          MIN_POSITION    FUNCNAME        MINLVL          SUBCMD          UNAFFECT
*/
    { "date"      , POS_DEAD      , do_date       , LVL_HERO      , SCMD_DATE     , NONE },
    { "dc"        , POS_DEAD      , do_dc         , LVL_HERO      , 0             , NONE },
    { "deposit"   , POS_STANDING  , do_not_here   , 1             , 0             , HIDE },
    { "destroy"   , POS_STANDING  , do_drop       , 1             , SCMD_DEST     , HIDE },
    { "devour"    , POS_FIGHTING  , do_devour     , 0             , 0             , HIDE },
    { "delusion"  , POS_FIGHTING  , do_delusion   , 0             , 0             , HIDE },
    { "deterrence", POS_FIGHTING  , do_advance_toggle, 0          , SCMD_DETERRENCE, HIDE },
    { "demote"    , POS_DEAD      , do_demote     , LVL_GOD       , 0            , NONE },
    { "disable"   , POS_STANDING  ,do_disable_trap, 0             , 0             , HIDE },
    { "diagnose"  , POS_MEDITATING, do_diagnose   , 1             , 0             , NONE },
    { "disarm"    , POS_FIGHTING  , do_disarm     , 0             , 0             , HIDE },
    { "dismount"  , POS_SITTING   , do_dismount   , 0             , 0             , HIDE },
    { "display"   , POS_DEAD      , do_display    , 1             , 0             , NONE },
    { "distract"  , POS_STANDING  , do_distract   , 0             , 0             , HIDE },
    { "donate"    , POS_RESTING   , do_drop       , 1             , SCMD_DONATE   , HIDE },
    { "doorbash"  , POS_FIGHTING  , do_doorbash   , 0             , 0             , HIDE },
    { "drink"     , POS_RESTING   , do_drink      , 1             , SCMD_DRINK    , NONE },
    { "drop"      , POS_RESTING   , do_drop       , 1             , SCMD_DROP     , NONE },
    { "dust"      , POS_FIGHTING  , do_dust       , 0             , 0             , HIDE },
    { "dump"      , POS_FIGHTING  , do_dump       , LVL_GRGOD    , 0             , NONE },
/*
**     CMD          MIN_POSITION    FUNCNAME        MINLVL          SUBCMD          UNAFFECT
*/
    { "eat"       , POS_RESTING   , do_eat        , 1             , SCMD_EAT      , NONE },
    { "echo"      , POS_SLEEPING  , do_echo       , LVL_ANGEL     , SCMD_ECHO     , NONE },
    { "emote"     , POS_SLEEPING  , do_echo       , 1            , SCMD_EMOTE    , HIDE },
    { ":"         , POS_RESTING   , do_echo       , 1             , SCMD_EMOTE    , HIDE },
    { "enter"     , POS_STANDING  , do_enter      , 1           , 0             , HIDE },
    { "entrance"  , POS_SLEEPING  , do_entrance   , LVL_IMMORT    , 0             , NONE },
    { "envenom"   , POS_STANDING  , do_envenom    , 0             , 0             , HIDE },
    { "escape"    , POS_FIGHTING  , do_escape     , 0             , 0             , NONE },
    { "equipment" , POS_SLEEPING  , do_equipment  , 1             , 0             , NONE },
    { "exits"     , POS_RESTING   , do_exits      , 1             , 0             , NONE },
    { "examine"   , POS_RESTING   , do_examine    , 1             , 0             , NONE },
    { "expose"    , POS_STANDING  , do_expose     , 0             , 0             , HIDE },
/*
**     CMD          MIN_POSITION    FUNCNAME        MINLVL          SUBCMD          UNAFFECT
*/
    { "falsetrail", POS_STANDING  , do_false_trail, 0             , 0             , HIDE },
    { "force"     , POS_SLEEPING  , do_force      , LVL_DGOD       , 0             , NONE },
    { "forge"     , POS_STANDING  , do_forge      , 30            , 0             , HIDE },
    { "feign"     , POS_FIGHTING  , do_feign_death, 0             , 0             , HIDE },
    { "feed"      , POS_FIGHTING  , do_feed       , 0             , 0             , HIDE },
    { "fill"      , POS_STANDING  , do_pour       , 1             , SCMD_FILL     , HIDE },
    { "fists"     , POS_FIGHTING  , do_fists      , 0             , 0             , HIDE },
    { "flag"      , POS_DEAD      , do_flag       , LVL_HERO      , 0             , NONE },
    { "flee"      , POS_FIGHTING  , do_flee       , 1             , 0             , HIDE },
    { "flashbang" , POS_STANDING  , do_flashbang  , 0             , 0             , HIDE },
    { "follow"    , POS_RESTING   , do_follow     , 1             , 0             , HIDE },
    { "freeze"    , POS_DEAD      , do_wizutil    , LVL_LRGOD     , SCMD_FREEZE   , NONE },
    { "file"	  , POS_DEAD	  , do_file	  , LVL_ANGEL     , 0	          , NONE },
    { "familiarize", POS_STANDING, do_familiarize , 0             , 0             , NONE },
/*
**     CMD          MIN_POSITION    FUNCNAME        MINLVL          SUBCMD          UNAFFECT
*/
    { "guild"     , POS_SLEEPING  , do_gen_comm   , 1             , SCMD_GUILD    , HIDE },
    { "get"       , POS_RESTING   , do_get        , 1             , 0             , HIDE },
    { "gecho"     , POS_DEAD      , do_gecho      , LVL_HERO      , 0             , NONE },
    { "give"      , POS_RESTING   , do_give       , 1             , 0             , HIDE },
//  { "goals"     , POS_DEAD 	  , do_showchores , LVL_IMPL       , 0             , NONE},
    { "goto"  	  , POS_SLEEPING  , do_goto 	  , LVL_HERO 	  , 0 	          , NONE },
    { "gold"      , POS_SLEEPING  , do_gold       , 1             , 0             , NONE },
    { "gore"	  , POS_FIGHTING  , do_gore       , 0             , 0             , HIDE },
    { "group"     , POS_SLEEPING  , do_group      , 1             , 0             , NONE },
    { "grats"     , POS_SLEEPING  , do_gen_comm   , 3             , SCMD_GRATZ    , HIDE },
    { "gut"	  , POS_FIGHTING  , do_gut	  , 0		  , 0	          , HIDE },
    { "gsay"      , POS_SLEEPING  , do_gsay       , 1             , 0             , NONE },
    { "gtell"     , POS_SLEEPING  , do_gsay       , 1             , 0             , NONE },
    { "guard"     , POS_STANDING  , do_guard      , 0             , 0             , NONE },
/*
**     CMD          MIN_POSITION    FUNCNAME        MINLVL          SUBCMD          UNAFFECT
*/
    { "help"      , POS_DEAD      , do_help       , 0             , 0             , NONE },
    { "hedit"     , POS_DEAD      , do_oasis_hedit, LVL_CREATOR   , 0             , NONE },
    { "cedit"     , POS_DEAD      , do_oasis_cedit , LVL_GRGOD    , 0             , NONE },
    { "hindex"    , POS_DEAD      , do_hindex     , 0             , 0             , NONE },
    { "helpcheck" , POS_DEAD      , do_helpcheck  , LVL_CREATOR   , 0             , NONE },
    { "hands"     , POS_MEDITATING, do_hands      , 0             , 0             , HIDE },
    { "hamstring" , POS_STANDING  , do_hamstring  , 0             , 0             , HIDE },
#ifdef US_HANDBOOK
    { "handbook"  , POS_DEAD      , do_gen_ps     , LVL_IMMORT      , SCMD_HANDBOOK , NONE },
#endif
    { "hcontrol"  , POS_DEAD      , do_hcontrol   , LVL_LRGOD     , 0             , NONE },
    { "hide"      , POS_RESTING   , do_hide       , 0             , 0             , NONE },
    { "hit"       , POS_FIGHTING  , do_hit        , 1             , SCMD_HIT      , HIDE },
    { "hippocratic",POS_STANDING  , do_advance_toggle, 0          , SCMD_HIPP     , HIDE },
    { "hold"      , POS_RESTING   , do_grab       , 1             , 0             , NONE },
    { "holler"    , POS_RESTING   , do_gen_comm   , LVL_GOD       , SCMD_HOLLER   , HIDE },
    { "holylight" , POS_DEAD      , do_gen_tog    , LVL_IMMORT    , SCMD_HOLYLIGHT, NONE },
    { "house"     , POS_RESTING   , do_house      , 1             , 0             , NONE },
    { "hug"       , POS_RESTING   , do_hug        , 0             , 0             , NONE },
/*
**     CMD          MIN_POSITION    FUNCNAME        MINLVL          SUBCMD          UNAFFECT
*/
    { "inventory" , POS_DEAD      , do_inventory  , 1             , 0             , NONE },
    { "idea"      , POS_DEAD      , do_gen_write  , 5             , SCMD_IDEA     , NONE },
    { "imotd"     , POS_DEAD      , do_gen_ps     , LVL_IMMORT    , SCMD_IMOTD    , NONE },
    { "immlist"   , POS_DEAD      , do_gen_ps     , 1             , SCMD_IMMLIST  , NONE },
    { "info"      , POS_SLEEPING  , do_gen_ps     , 1             , SCMD_INFO     , NONE },
    { "innereye"  , POS_STANDING  , do_innereye   , 0             , 0             , NONE },
    { "invigorate", POS_FIGHTING  , do_invigorate , 0             , 0             , HIDE },
    { "insult"    , POS_RESTING   , do_insult     , 0             , 0             , HIDE },
    { "ipcheck"   , POS_DEAD      , do_ipcheck    , LVL_GRGOD     , 0             , NONE },
    { "invis"     , POS_DEAD      , do_invis      , LVL_IMMORT    , 0             , NONE },
/*
**     CMD          MIN_POSITION    FUNCNAME        MINLVL          SUBCMD          UNAFFECT
*/
    { "junk"      , POS_RESTING   , do_drop       , 1             , SCMD_JUNK     , HIDE },
/*
**     CMD          MIN_POSITION    FUNCNAME        MINLVL          SUBCMD          UNAFFECT
*/
    { "kill"      , POS_FIGHTING  , do_kill       , 1             , 0             , HIDE },
    { "knock"     , POS_FIGHTING  , do_knock      , 0             , 0             , HIDE },
    { "kick"      , POS_FIGHTING  , do_kick       , 0             , 0             , HIDE },
/*
**     CMD          MIN_POSITION    FUNCNAME        MINLVL          SUBCMD          UNAFFECT
*/
    { "look"      , POS_MEDITATING, do_look       , 1             , SCMD_LOOK     , NONE },
    { "land"	  , POS_STANDING  , do_land       , 1             , 0             , HIDE },
    { "last"      , POS_DEAD      , do_last       , LVL_IMMORT    , 0             , NONE },
    { "launch"    , POS_DEAD      , do_not_here   , LVL_SAINT     , 0             , NONE },
    { "leave"     , POS_STANDING  , do_leave      , 1             , 0             , HIDE },
    { "levels"    , POS_DEAD      , do_levels     , 1             , 0             , NONE },
    { "list"      , POS_STANDING  , do_not_here   , 1             , 0             , HIDE },
    { "reelin"    , POS_SITTING   , do_reelin     , 0             , 0             , HIDE },
    { "rename"    , POS_STANDING  , do_not_here   , 0             , 0             , NONE },
    { "lock"      , POS_SITTING   , do_lock       , 1             , 0             , HIDE },
    { "load"      , POS_DEAD      , do_load       , LVL_SAINT     , 0             , NONE },
    { "logfile"   , POS_DEAD      , do_logfile    , LVL_GRGOD     , 0             , NONE },
/*
**     CMD          MIN_POSITION    FUNCNAME        MINLVL          SUBCMD          UNAFFECT
*/
    { "metamorphosis", POS_FIGHTING, do_advance_toggle, 0         , SCMD_METAMORPH, HIDE },
    { "medit"     , POS_DEAD      , do_olc        , LVL_SAINT     , SCMD_OLC_MEDIT, NONE },
    { "meditate"  , POS_RESTING   , do_meditate   , 1             , 0             , HIDE },
    { "meld"	  , POS_STANDING  , do_meld	  , 0 	          , 0             , HIDE },
    { "mendpet"   , POS_STANDING  , do_advance_toggle, 0          , SCMD_MEND_PET , HIDE },
    { "mist"      , POS_STANDING  , do_mist       , 0             , 0             , HIDE },
    { "mix"       , POS_STANDING  , do_mix        , 30            , 0             , HIDE },
    { "mlist"     , POS_DEAD      , do_liblist    , LVL_IMMORT    , SCMD_MLIST    , NONE },
    { "motd"      , POS_DEAD      , do_gen_ps     , 1             , SCMD_MOTD     , NONE },
    { "mount"     , POS_STANDING  , do_mount      , 1             , 0             , HIDE },
    { "mail"      , POS_STANDING  , do_not_here   , 1             , 0             , HIDE },
    { "muckle"    , POS_DEAD      , do_muckle     , 1      , 0             , NONE },
    { "mute"      , POS_DEAD      , do_wizutil    , LVL_CREATOR   , SCMD_SQUELCH  , NONE },
    { "murder"    , POS_FIGHTING  , do_hit        , 1             , SCMD_MURDER   , HIDE },
    { "multi"     , POS_DEAD      , do_multi      , LVL_GOD       , 0             , NONE },
    { "mug"       , POS_STANDING  , do_steal      , 0             , SCMD_MUG      , NONE },
/*
**     CMD          MIN_POSITION    FUNCNAME        MINLVL          SUBCMD          UNAFFECT
*/
    { "news"      , POS_SLEEPING  , do_gen_ps     , 1             , SCMD_NEWS     , NONE },
    { "noarena"   , POS_DEAD      , do_gen_tog    , 1             , SCMD_NOARENA  , NONE },
    { "noauction" , POS_DEAD      , do_gen_tog    , 1             , SCMD_NOAUCTION, NONE },
    { "noclan"    , POS_DEAD	  , do_gen_tog    , 1             , SCMD_NOCLAN  ,  NONE },
    { "noooc"     , POS_DEAD      , do_gen_tog    , 1             , SCMD_NOOOC    , NONE },
    { "noquery"   , POS_DEAD      , do_gen_tog    , 1             , SCMD_NOQUERY  , NONE },
    { "noroleplay"   , POS_DEAD      , do_gen_tog    , 1             , SCMD_NORPLAY  , NONE },
    { "nograts"   , POS_DEAD      , do_gen_tog    , 1             , SCMD_NOGRATZ  , NONE },
    { "noguild"   , POS_DEAD      , do_gen_tog    , 1             , SCMD_NOGUILD  , NONE },
    { "nohassle"  , POS_DEAD      , do_gen_tog    , LVL_IMMORT    , SCMD_NOHASSLE , NONE },
    { "norepeat"  , POS_DEAD      , do_gen_tog    , 1             , SCMD_NOREPEAT , NONE },
    { "noshout"   , POS_SLEEPING  , do_gen_tog    , 1             , SCMD_DEAF     , NONE },
    { "nospam"    , POS_SLEEPING  , do_gen_tog    , 1             , SCMD_SPAM     , NONE },
    { "norecall"  , POS_DEAD      , do_gen_tog    , 1             , SCMD_NORECALL , NONE },
    { "nosummon"  , POS_DEAD      , do_gen_tog    , 1             , SCMD_NOSUMMON , NONE },
    { "noteam"    , POS_DEAD      , do_noteam     , 1		  , 0             , NONE },
    { "notell"    , POS_DEAD      , do_gen_tog    , 1             , SCMD_NOTELL   , NONE },
    { "notitle"   , POS_DEAD      , do_wizutil    , LVL_CREATOR   , SCMD_NOTITLE  , NONE },
    { "nowiz"     , POS_DEAD      , do_gen_tog    , LVL_IMMORT    , SCMD_NOWIZ    , NONE },
    { "nowho"     , POS_DEAD      , do_gen_tog    , LVL_SAINT     , SCMD_NOWHO    , NONE },
/*
**     CMD          MIN_POSITION    FUNCNAME        MINLVL          SUBCMD          UNAFFECT
*/
    { "order"     , POS_RESTING   , do_order      , 1             , 0             , HIDE },
    { "ooc"       , POS_SLEEPING  , do_gen_comm   , 5             , SCMD_OOC      , HIDE },
    { "offer"     , POS_STANDING  , do_not_here   , 1             , 0             , HIDE },
    { "olc"       , POS_DEAD      , do_olc        , LVL_SAINT     , SCMD_OLC_SAVEINFO , NONE },
    { "oedit"     , POS_DEAD      , do_olc        , LVL_SAINT     , SCMD_OLC_OEDIT , NONE },
    { "olist"     , POS_DEAD      , do_liblist    , LVL_IMMORT    , SCMD_OLIST    , NONE },
    { "open"      , POS_SITTING   , do_open       , 1             , 0             , HIDE },
/*
**     CMD          MIN_POSITION    FUNCNAME        MINLVL          SUBCMD          UNAFFECT
*/
    { "put"       , POS_RESTING   , do_put        , 1             , 0             , NONE },
    { "palm"      , POS_STANDING  , do_palm       , 0             , 0             , NONE },
    { "page"      , POS_DEAD      , do_page       , LVL_DGOD      , 0             , HIDE },
    { "pardon"    , POS_MEDITATING, do_pardon     , 1             , SCMD_PARDON   , NONE },
    { "phaseshift", POS_FIGHTING  , do_advance_toggle, 0          ,SCMD_PHASESHIFT, NONE },
    { "pick"      , POS_STANDING  , do_pick       , 0             , 0             , HIDE },
    { "players"   , POS_FIGHTING  , do_players    , LVL_IMMORT    , 0             , NONE },
    { "policy"    , POS_DEAD      , do_gen_ps     , 1             , SCMD_POLICIES , NONE },
    { "poll"      , POS_FIGHTING  , do_poll       , LVL_IMMORT    , 0             , NONE },
    { "poof"	  , POS_DEAD	  , do_poofset    , LVL_IMMORT    , SCMD_POOF	  , NONE },
    { "poofin"    , POS_DEAD      , do_poofset    , LVL_IMMORT    , SCMD_POOFIN   , NONE },
    { "poofout"   , POS_DEAD      , do_poofset    , LVL_IMMORT    , SCMD_POOFOUT  , NONE },
    { "poultice"  , POS_STANDING  , do_poultice   , 0             , 0             , HIDE },
    { "poison"	  , POS_FIGHTING  , do_instant_poison, 0          , 0             , HIDE },
    { "pour"      , POS_STANDING  , do_pour       , 1             , SCMD_POUR     , HIDE },
    { "potency"   , POS_STANDING  , do_advance_toggle , 0         , SCMD_POTENCY  , HIDE },
    { "powerstrike",POS_STANDING  , do_powerstrike, 0             , 0             , NONE },
    { "prompt"    , POS_DEAD      , do_display    , 1             , 0             , NONE },
    { "practice"  , POS_SLEEPING  , do_practice   , 1             , 0             , HIDE },
    { "purge"     , POS_DEAD      , do_purge      , LVL_SAINT     , 0             , NONE },
    { "push"      , POS_SITTING   , do_open       , 1             , 0             , HIDE },
    { "prayer"    , POS_SITTING   , do_prayer     , 1             , 0             , NONE },
/*
**     CMD          MIN_POSITION    FUNCNAME        MINLVL          SUBCMD          UNAFFECT
*/
    { "qecho"     , POS_DEAD      , do_qecho      , LVL_SAINT     , 0             , NONE },
    { "qedit"     , POS_DEAD      , do_olc        , LVL_DGOD      , SCMD_OLC_QEDIT, NONE },
    { "qpadd"     , POS_DEAD      , do_questPnts  , LVL_SAINT     , SCMD_QPADD    , NONE },
    { "qpdec"     , POS_DEAD      , do_questPnts  , LVL_SAINT     , SCMD_QPDEC    , NONE },
    { "qpshow"    , POS_DEAD      , do_questPnts  , LVL_SAINT     , SCMD_QPSHOW   , NONE },
    { "qpset"     , POS_DEAD      , do_questPnts  , LVL_SAINT     , SCMD_QPSET    , NONE },
    { "qpclr"     , POS_DEAD      , do_questPnts  , LVL_SAINT     , SCMD_QPCLR    , NONE },
    { "quaff"     , POS_RESTING   , do_use        , 1             , SCMD_QUAFF    , NONE },
    { "query"     , POS_SLEEPING  , do_gen_comm   , 1             , SCMD_QUERYSAY , HIDE },
    { "quest"     , POS_DEAD      , do_quest      , 1             , 0             , HIDE },
    { "qui"       , POS_DEAD      , do_quit       , 1             , 0             , HIDE },
    { "quit"      , POS_DEAD      , do_quit       , 1             , SCMD_QUIT     , HIDE },
    { "qlist"     , POS_DEAD      , do_qlist      , LVL_ANGEL     , 0             , NONE },
    { "qsay"      , POS_SLEEPING  , do_qcomm      , 1             , 0             , HIDE },
    { "qstat"     , POS_DEAD      , do_qstat      , LVL_ANGEL     , 0             , NONE },
/*
**     CMD          MIN_POSITION    FUNCNAME        MINLVL          SUBCMD          UNAFFECT
*/
    { "roleplay"  , POS_SLEEPING  , do_gen_comm   , 5             , SCMD_ROLEPLAY , HIDE },
    { "rawdamage" , POS_DEAD      , do_rawDamage  , LVL_SAINT     , 0             , NONE },
    { "rest"      , POS_MEDITATING, do_rest       , 1             , 0             , NONE },
    { "read"      , POS_RESTING   , do_look       , 1             , SCMD_READ     , NONE },
    { "reboo"     , POS_DEAD      , do_reboot     , LVL_DGOD      , 0             , NONE },
    { "reboot"    , POS_DEAD      , do_reboot     , LVL_DGOD      , SCMD_REBOOT   , NONE },
    { "reload"    , POS_DEAD      , do_reload     , LVL_GOD       , 0             , NONE },
    { "recite"    , POS_RESTING   , do_use        , 1             , SCMD_RECITE   , HIDE },
    { "receive"   , POS_STANDING  , do_not_here   , 0             , 0             , HIDE },
    { "redoubt"   , POS_FIGHTING  , do_redoubt    , 0             , 0             , HIDE },
    { "redit"     , POS_DEAD      , do_olc        , LVL_SAINT     , SCMD_OLC_REDIT, NONE },
    { "reimb"     , POS_DEAD      , do_reimb      , LVL_CREATOR   , 0             , NONE },
    { "remove"    , POS_RESTING   , do_remove     , 1             , 0             , HIDE },
    { "remort"    , POS_DEAD      , do_remort     , LVL_GRGOD     , 0             , NONE },
    { "rent"      , POS_STANDING  , do_not_here   , 1             , 0             , HIDE },
    { "report"    , POS_SLEEPING  , do_report     , 1            , 0             , HIDE },
//  { "reply"     , POS_SLEEPING  , do_reply      , 0           , 0             , HIDE },
    { "reroll"    , POS_DEAD      , do_wizutil    , LVL_GOD       , SCMD_REROLL   , NONE },
    { "rescue"    , POS_FIGHTING  , do_rescue     , 0             , 0             , HIDE },
    { "restore"   , POS_DEAD      , do_restore    , LVL_ANGEL     , 0             , NONE },
    { "return"    , POS_DEAD      , do_return     , 0             , 0             , HIDE },
    { "retarget"  , POS_FIGHTING  , do_retarget   , 0             , 0             , HIDE },
    { "retreat"   , POS_FIGHTING  , do_retreat    , 0             , 0             , HIDE },
    { "rlist"     , POS_DEAD      , do_liblist    , LVL_IMMORT     , SCMD_RLIST    , NONE },
    { "roomflags" , POS_DEAD      , do_gen_tog    , LVL_IMMORT     , SCMD_ROOMFLAGS, NONE },
//  { "rub"       , POS_FIGHTING  , do_rub        , 1             , 0             , NONE },
/*
**     CMD          MIN_POSITION    FUNCNAME        MINLVL          SUBCMD          UNAFFECT
*/
    { "say"       , POS_MEDITATING, do_say        , 1             , 0             , HIDE },
    { "'"         , POS_MEDITATING, do_say        , 1             , 0             , HIDE },
    { "save"      , POS_SLEEPING  , do_save       , 1             , 0             , NONE },
    { "sacrifice" , POS_FIGHTING  , do_advance_toggle, 0          , SCMD_SACRIFICE, HIDE },
    { "scatter"   , POS_DEAD      , do_scatter    , LVL_ANGEL     , 0             , NONE },
    { "score"     , POS_DEAD      , do_score      , 1             , 0             , NONE },
    { "scribe"    , POS_STANDING  , do_scribe     , 1             , 0             , HIDE },
    { "scan"      , POS_FIGHTING  , do_scan       , 0             , 0             , NONE },
    { "search"    , POS_STANDING  , do_detect_trap, 0             , 0             , NONE },
    { "sell"      , POS_STANDING  , do_not_here   , 1             , 0             , HIDE },
    { "send"      , POS_SLEEPING  , do_send       , LVL_DGOD      , 0             , NONE },
    { "set"       , POS_DEAD      , do_set        , LVL_SAINT     , 0             , NONE },
    { "sedit"     , POS_DEAD      , do_olc        , LVL_DGOD      , SCMD_OLC_SEDIT, NONE },
    { "shadow dance",POS_FIGHTING , do_shadowdance, 0             , 0             , HIDE },
    { "shadowmist" , POS_STANDING , do_shadowmist , 0             , 0             , HIDE },
    { "shadowstep", POS_STANDING  , do_shadowstep , 0		  , 0             , HIDE },
    { "shadowjump", POS_STANDING  , do_shadowjump , 0             , 0             , NONE },
    { "block"     , POS_FIGHTING  , do_block      , 0             , 0             , HIDE },
    { "shieldbash", POS_FIGHTING  , do_shieldbash , 0             , 0             , HIDE },
    { "shoot"     , POS_FIGHTING  , do_shoot      , 0             , 0             , HIDE },
    { "shout"     , POS_MEDITATING, do_gen_comm   , 0             , SCMD_SHOUT    , HIDE },
    { "shadowform", POS_STANDING  , do_advance_toggle, 0          , SCMD_SHADOWFORM,HIDE },
    { "shove"     , POS_SITTING   , do_open       , 1             , 0             , HIDE },
    { "show"      , POS_DEAD      , do_show       , LVL_HERO      , 0             , NONE },
    { "showdamage", POS_DEAD      , do_gen_tog    , 1             , SCMD_SHOWDAM  , NONE },
    { "shutdow"   , POS_DEAD      , do_shutdown   , LVL_ANGEL     , 0             , NONE },
    { "shutdown"  , POS_DEAD      , do_shutdown   , LVL_ANGEL     , SCMD_SHUTDOWN , NONE },
    { "sign"      , POS_STANDING  , do_not_here   , 1             , 0             , HIDE },
    { "sip"       , POS_RESTING   , do_drink      , 1             , SCMD_SIP      , NONE },
  //{ "sit"       , POS_MEDITATING, do_sit        , 1             , 0             , NONE },
  //{ "sing"      , POS_FIGHTING  , do_sing       , 1             , 0             , HIDE },
    { "skillset"  , POS_SLEEPING  , do_skillset   , LVL_GOD       , 0             , NONE },
    { "sleep"     , POS_SLEEPING  , do_sleep      , 1             , 0             , NONE },
    { "sneak"     , POS_STANDING  , do_sneak      , 0             , 0             , NONE },
    { "snoop"     , POS_DEAD      , do_snoop      , LVL_GOD       , 0             , NONE },
    { "soulpyre"  , POS_FIGHTING  , do_advance_toggle, 0          , SCMD_PYRE     , NONE },
    { "socials"   , POS_DEAD      , do_commands   , 1             , SCMD_SOCIALS  , NONE },
    { "split"     , POS_SITTING   , do_split      , 1             , 0             , HIDE },
    { "spy"       , POS_SITTING   , do_spy        , 0             , 0             , NONE },
    { "stand"     , POS_MEDITATING, do_stand      , 1             , 0             , NONE },
    { "stance"    , POS_STANDING  , do_stance     , 0             , 0             , NONE },
    { "stat"      , POS_DEAD      , do_stat       , 1             , 0             , NONE },
    { "steal"     , POS_STANDING  , do_steal      , 0             , SCMD_STEAL    , NONE },
    { "steadfastness", POS_FIGHTING, do_steadfastness, 0          , 0             , HIDE },
    { "stalk"     , POS_STANDING  , do_stalk      , 0             , 0             , NONE },
    { "sting"     , POS_FIGHTING  , do_sting      , 0             , 0             , HIDE },
    { "stop"      , POS_RESTING   , do_stop       , 1             , 0             , HIDE },
    { "sweep"     , POS_FIGHTING  , do_sweep      , 0             , 0             , HIDE },
    { "switch"    , POS_DEAD      , do_switch     , LVL_SAINT     , 0             , NONE },
/*
**     CMD          MIN_POSITION    FUNCNAME        MINLVL          SUBCMD          UNAFFECT
*/
    { "seek"      , POS_DEAD      , do_seek       , LVL_CREATOR       , SCMD_SET      , NONE },
    { "unseek"    , POS_DEAD      , do_seek       , LVL_CREATOR       , SCMD_UNSET    , NONE },
    { "unpseek"   , POS_DEAD      , do_pseek      , LVL_CREATOR       , SCMD_UNSET    , NONE },
    { "qseek"     , POS_DEAD      , do_seek       , LVL_CREATOR       , SCMD_QUERY    , NONE },
    { "peace"     , POS_DEAD	  , do_peace	  , LVL_HERO	      , 0             , NONE },
    { "qpseek"    , POS_DEAD      , do_pseek      , LVL_CREATOR       , SCMD_QUERY    , NONE },
    { "pseek"     , POS_DEAD      , do_pseek      , LVL_CREATOR       , SCMD_SET      , NONE },
/*
**     CMD          MIN_POSITION    FUNCNAME        MINLVL          SUBCMD          UNAFFECT
*/
    { "tell"      , POS_DEAD      , do_tell       , 1             , 0             , HIDE },
    { "teamsay"   , POS_DEAD      , do_tsay       , 1             , 0             , NONE },
    { "terms"     , POS_STANDING  , do_not_here   , 1             , 0             , HIDE },
    { "take"      , POS_RESTING   , do_get        , 1             , 0             , HIDE },
    { "taste"     , POS_RESTING   , do_eat        , 0             , SCMD_TASTE    , NONE },
    { "tally"     , POS_DEAD      , do_tally      , LVL_ANGEL     , 0             , NONE },
    { "tedit"     , POS_DEAD      , do_tedit      , LVL_DGOD      , 0             , NONE },
    { "teleport"  , POS_DEAD      , do_teleport   , LVL_ANGEL     , 0             , NONE },
    { "throw"     , POS_STANDING  , do_throw      , 1             , 0             , HIDE },
    { "thaw"      , POS_DEAD      , do_wizutil    , LVL_LRGOD     , SCMD_THAW     , NONE },
    { "tickupdate", POS_DEAD	  , do_tick	  , LVL_GRGOD     , 0             , NONE },
    { "title"     , POS_DEAD      , do_title      , 1            , 0             , NONE },
    { "time"      , POS_DEAD      , do_time       , 1             , 0             , NONE },
    { "toggle"    , POS_DEAD      , do_toggle     , 1             , 0             , NONE },
    { "toss"      , POS_RESTING   , do_use        , 1             , SCMD_TOSS     , HIDE },
    { "track"     , POS_STANDING  , do_track      , 0             , 0             , NONE },
    { "transfer"  , POS_SLEEPING  , do_trans      , LVL_ANGEL         , 0             , NONE },
    { "transquest", POS_SLEEPING  , do_transquest , LVL_SAINT	  , 0             , NONE },
    { "trap"      , POS_STANDING  , do_trap       , 0             , 0             , NONE },
    { "trip"      , POS_FIGHTING  , do_trip       , 0             , 0             , HIDE },
    { "tsay"      , POS_DEAD      , do_tsay       , 1             , 0             , NONE },
    { "turn"      , POS_FIGHTING  , do_turn       , 0             , 0             , HIDE },
    { "typo"      , POS_DEAD      , do_gen_write  , 5             , SCMD_TYPO     , NONE },
    { "twist"     , POS_SITTING   , do_open       , 1             , 0             , HIDE },
/*
**     CMD          MIN_POSITION    FUNCNAME        MINLVL          SUBCMD          UNAFFECT
*/
    { "unlock"    , POS_SITTING   , do_unlock     , 1             , 0             , HIDE },
    { "ungroup"   , POS_DEAD      , do_ungroup    , 1             , 0             , NONE },
    { "unban"     , POS_DEAD      , do_unban      , LVL_GRGOD     , 0             , NONE },
    { "unaffect"  , POS_DEAD      , do_wizutil    , LVL_ANGEL     , SCMD_UNAFFECT , NONE },
    { "update"    , POS_DEAD      , do_update     , LVL_DGOD      , 0             , NONE },
    { "uptime"    , POS_DEAD      , do_date       , 1             , SCMD_UPTIME   , NONE },
    { "use"       , POS_SITTING   , do_use        , 1             , SCMD_USE      , NONE },
    { "users"     , POS_DEAD      , do_users      , LVL_HERO      , 0             , NONE },
/*
**     CMD          MIN_POSITION    FUNCNAME        MINLVL          SUBCMD          UNAFFECT
*/
    { "vote"      , POS_SLEEPING  , do_vote       , 1             , 0             , NONE },
    { "valentine" , POS_DEAD      , do_valentine  , LVL_DGOD      , 0             , NONE },
    { "value"     , POS_STANDING  , do_not_here   , 0             , 0             , HIDE },
    { "version"   , POS_SLEEPING  , do_gen_ps     , 1             , SCMD_VERSION  , NONE },
    { "visible"   , POS_RESTING   , do_visible    , 1             , 0             , HIDE },
    { "vnum"      , POS_DEAD      , do_vnum       , LVL_HERO          , 0             , NONE },
    { "vstat"     , POS_DEAD      , do_vstat      , LVL_HERO          , 0             , NONE },
/*
**     CMD          MIN_POSITION    FUNCNAME        MINLVL          SUBCMD          UNAFFECT
*/
    { "wake"      , POS_SLEEPING  , do_wake       , 1             , 0             , NONE },
    { "warcry"    , POS_FIGHTING  , do_warcry     , 0             , 0             , HIDE },
    { "wear"      , POS_RESTING   , do_wear       , 1             , 0             , NONE },
    { "weather"   , POS_RESTING   , do_weather    , 1             , 0             , NONE },
    { "who"       , POS_DEAD      , do_who        , 1             , 0             , NONE },
    { "whoami"    , POS_DEAD      , do_gen_ps     , 1             , SCMD_WHOAMI   , NONE },
    { "where"     , POS_SLEEPING  , do_where      , 1             , 0             , NONE },
    { "whisper"   , POS_RESTING   , do_spec_comm  , 1             , SCMD_WHISPER  , HIDE },
    { "wield"     , POS_RESTING   , do_wield      , 1             , 0             , NONE },
    { "withdraw"  , POS_STANDING  , do_not_here   , 1             , 0             , HIDE },
    { "wiznet"    , POS_DEAD      , do_wiznet     , LVL_IMMORT      , 0             , NONE },
    { ";"         , POS_DEAD      , do_wiznet     , LVL_IMMORT      , 0             , NONE },
    { "wizhelp"   , POS_SLEEPING  , do_commands   , LVL_IMMORT      , SCMD_WIZHELP  , NONE },
    { "wizlist"   , POS_DEAD      , do_gen_ps     , 1             , SCMD_WIZLIST  , NONE },
    { "wizlock"   , POS_DEAD      , do_wizlock    , LVL_GRGOD   , 0             , NONE },
    { "wizslap"   , POS_DEAD      , do_wizslap    , LVL_IMMORT      , 0             , NONE },
    { "write"     , POS_STANDING  , do_write      , 1             , 0             , NONE },
/*
**     CMD          MIN_POSITION    FUNCNAME        MINLVL          SUBCMD          UNAFFECT
*/
    { "xname"     , POS_DEAD      , do_xname      , LVL_DGOD	      , 0             , NONE },
    { "zedit"     , POS_DEAD      , do_olc        , LVL_SAINT         , SCMD_OLC_ZEDIT, NONE },
    { "zlist"	  , POS_DEAD      , do_liblist    , LVL_IMMORT      , SCMD_ZLIST    , NONE },
    { "zombies"   , POS_DEAD      , do_zombies    , LVL_GOD           , 0             , NONE },
    { "zombiegift", POS_DEAD      , do_zombiegift , LVL_GOD           , 0             , NONE },
    { "zreset"    , POS_DEAD      , do_zreset     , LVL_SAINT         , 0             , NONE },

/*
** Now let's load up the socials.
*/
#include "interp_socials.c"
  /* DG trigger commands */
  { "attach"   , POS_DEAD    , do_attach   , LVL_ANGEL   , 0 },
  { "detach"   , POS_DEAD    , do_detach   , LVL_ANGEL   , 0 },
  { "tlist"    , POS_DEAD    , do_tlist    , LVL_ANGEL   , 0 },
  { "tstat"    , POS_DEAD    , do_tstat    , LVL_ANGEL   , 0 },
  { "masound"  , POS_DEAD    , do_masound  , -1, 0 },
  { "mkill"    , POS_STANDING, do_mkill    , -1, 0 },
  { "mjunk"    , POS_SITTING , do_mjunk    , -1, 0 },
  { "mdoor"    , POS_DEAD    , do_mdoor    , -1, 0 },
  { "mecho"    , POS_DEAD    , do_mecho    , -1, 0 },
  { "mechoaround" , POS_DEAD , do_mechoaround, -1, 0 },
  { "msend"    , POS_DEAD    , do_msend    , -1, 0 },
  { "mload"    , POS_DEAD    , do_mload    , -1, 0 },
  { "mpurge"   , POS_DEAD    , do_mpurge   , -1, 0 },
  { "mgoto"    , POS_DEAD    , do_mgoto    , -1, 0 },
  { "mat"      , POS_DEAD    , do_mat      , -1, 0 },
  { "mteleport", POS_DEAD    , do_mteleport, -1, 0 },
  { "mforce"   , POS_DEAD    , do_mforce   , -1, 0 },
  { "mexp"     , POS_DEAD    , do_mexp     , -1, 0 },
  { "mgold"    , POS_DEAD    , do_mgold    , -1, 0 },
  { "mhunt"    , POS_DEAD    , do_mhunt    , -1, 0 },
  { "mremember", POS_DEAD    , do_mremember, -1, 0 },
  { "mforget"  , POS_DEAD    , do_mforget  , -1, 0 },
  { "mtransform",POS_DEAD    , do_mtransform,-1, 0 },
  { "vdelete"  , POS_DEAD    , do_vdelete  , LVL_ANGEL   , 0 },

  { "trigedit" , POS_DEAD    , do_olc , LVL_ANGEL   , SCMD_OLC_TRIGEDIT, NONE },

/*
**   This MUST be last.
*/
  { "\n", 0, 0, 0, 0, 0 }
};


