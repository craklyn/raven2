#ifndef __PORTAL_H__
#define __PORTAL_H__
/* ============================================================================ 
portal.h
============================================================================ */

/* Public Functions from portal.c */
int   portal_proc(CharData *ch, void *me, int cmd, char *argument);
void  enter_portal(CharData *ch, ObjData *portal);
void  look_in_portal(CharData *ch, ObjData *obj);

#endif  /* __PORTAL_H__ */
