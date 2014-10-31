#ifndef __DISASTERS_H__
#define __DISASTERS_H__
/* ============================================================================ 
disasters.h
Header file for disaster rooms
Written by Mortius of RavenMUD for RavenMUD.
============================================================================ */

/* ============================================================================ 
Public functions.
============================================================================ */
extern void disaster_activity();
extern void disaster_fireball(struct char_data *ch, int dam);
extern void disaster_lightning(struct char_data *ch, int dam);
extern void disaster_earthquake(struct char_data *ch, int dam);
extern void disaster_wind(struct char_data *ch, int dam);

/* Externs */

extern struct char_data *character_list;
extern struct room_data *world;
extern struct zone_data *zone_table;
extern void send_to_zone(char *messg, int zone_rnum);
extern void create_ghost(struct char_data *ch);

#endif
