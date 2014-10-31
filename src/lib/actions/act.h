/*
* Header file for the core act functions.
*
* I got this idea from stock tbaMUD. Previously RavenMUD has the command array
* and all the ACMD functions grouped together. This seemd to cause more problems
* than it was worth. As an alternative to that method I've added this act file
* and turned the command_list file into a .c file so it remains out of the
* global scope.
* Header File Author: Xiuhtecuhtli
* Date: August 29th 2009
*/
#ifndef _ACT_H_
#define _ACT_H_

#include "util/utils.h"           /* for the ACMD macro */

/*****************************************************************************
 * Begin ACMD Functions from  auctions.c
 ****************************************************************************/
ACMD(do_bid);
ACMD(do_auction);
ACMD(do_auction_stat);

/*****************************************************************************
 * Begin ACMD Functions from  act.create.c
 ****************************************************************************/
ACMD(do_brew);
ACMD(do_forge);
ACMD(do_scribe);

/*****************************************************************************
 * Begin public functions for objact.c
 ****************************************************************************/
void     do_give(CharData *ch, char *argument, int cmd, int subcmd);
CharData *give_find_vict(CharData *ch, char *arg);
void     perform_give(CharData *ch, CharData *vict, ObjData *obj);
void     perform_get_from_container(CharData *ch, ObjData *obj, ObjData *cont, int mode);

/*****************************************************************************
 * Begin public functions for combat.c
 ****************************************************************************/
void smart_NPC_combat( CharData *the_mob );
int  calculate_attack_count( CharData * ch );

/*****************************************************************************
 * Begin Functions and defines for act.movement.c
 ****************************************************************************/
int  find_door(CharData * ch, char *type, char *dir);
int  mob_block(struct char_data *ch, struct char_data *mob, int dir);
int  exit_guarded( CharData *ch, int dir);
void awkward_dismount(CharData *ch);
void aggroRoomCheck(CharData *ch, int sneaking);
void deathTrapKill(CharData *ch);

/*****************************************************************************
 * Begin Functions and defines for other.c
 ****************************************************************************/
int chGroupedWithTarget(CharData *ch, CharData *target);
int has_owner(ObjData *blade);
int owns_item(CharData *ch, ObjData *blade);
int set_owner(CharData *ch, ObjData *blade);
void logItem(int vnum, char *fileName);
ACMD(do_recall);
ACMD(do_locate);

/*****************************************************************************
 * Begin Functions and defines for act.wizard.c
 ****************************************************************************/
void    poll_clear(void);
sh_int  find_target_room( CharData * ch, char *rawroomstr );
void    restore( CharData *ch, CharData *vict );
void    do_stat_room(CharData * ch);
void    do_stat_character(CharData * ch, CharData * k);
void    do_stat_object(CharData * ch, ObjData * j);

/*****************************************************************************
 * Begin Functions and defines for act.informative.c
 ****************************************************************************/
char *find_exdesc(char *word, struct extra_descr_data * list);
void diag_char_to_char(CharData *ch, CharData *victim);

/*****************************************************************************
 * Begin ACMD Functions from  act.social.c
 ****************************************************************************/
char *fread_action(FILE * fl, int nr);
ACMD(do_insult);
ACMD(do_action);

/*****************************************************************************
 * Begin ACMD Functions for class skiills
 ****************************************************************************/
ACMD(do_aggressive);
ACMD(do_backstab);
ACMD(do_flashbang);
ACMD(do_charge);
ACMD(do_envenom);
ACMD(do_turn);
ACMD(do_throw);
ACMD(do_sting);
ACMD(do_steal);
ACMD(do_dust);
ACMD(do_calm);
ACMD(do_breathe);
ACMD(do_blind_strike);
ACMD(do_befriend);
ACMD(do_shadowdance);
ACMD(do_pickpocket);
ACMD(do_instant_poison);
ACMD(do_retarget);
ACMD(do_rescue);
/* From epic.c */
ACMD(do_battlecry);
ACMD(do_warcry);
ACMD(do_stance);
ACMD(do_powerstrike);
/* From holidays.c */
ACMD(do_hug);

ACMD(do_advance);
ACMD(do_affected);
ACMD(do_alias);
ACMD(do_ambush);
ACMD(do_area);
ACMD(do_arenarank);
ACMD(do_assist);
ACMD(do_aspect);
ACMD(do_at);
ACMD(do_ban);
ACMD(do_berserk);
ACMD(do_block);
ACMD(do_brain);
ACMD(do_blackjack);
ACMD(do_chown);
ACMD(do_convert);
ACMD(do_demote);
ACMD(do_devour);
ACMD(do_doorbash);
ACMD(do_butcher);
ACMD(do_cast);
ACMD(do_castout);
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
ACMD(do_dump);
ACMD(do_eat);
ACMD(do_echo);
ACMD(do_enter);
ACMD(do_entrance);
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
ACMD(do_info);
ACMD(do_inventory);
ACMD(do_invis);
ACMD(do_ipcheck);
ACMD(do_innereye);
ACMD(do_invigorate);
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
ACMD(do_fishoff);
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
ACMD(do_rest);
ACMD(do_restore);
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
ACMD(do_peace);
ACMD(do_pseek);
ACMD(do_scatter);
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
ACMD(do_steadfastness);
ACMD(do_switch);
ACMD(do_tally);
ACMD(do_tedit);
ACMD(do_teleport);
ACMD(do_tell);
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


#endif	/* _ACT_H */

