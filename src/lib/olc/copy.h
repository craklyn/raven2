/* ============================================================================
Header file for coping rooms/objects.
Written by Mortius of RavenMUD for RavenMUD.
============================================================================ */
#ifndef __COPY_H__
#define __COPY_H__

/* Defines */
#define ROOM 	0
#define OBJECT	1
#define MOBILE	2

/* Public functions */
int   can_edit_zone(CharData *ch, int zone_num);
void  copy_room(int rnum_src, int rnum_targ);
void  copy_object(int rnum_src, int rnum_targ);

#endif /* __COPY_H__ */
