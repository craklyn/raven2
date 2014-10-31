/* ============================================================================
* Header file for advance.c
* Written by Arbaces of RavenMUD for RavenMUD.
*
* Most of this header file revised by Xiuhtecuhtli to fix the many mistakes
* made by the original author.
============================================================================ */
#ifndef __ADVANCE_H__
#define __ADVANCE_H__

/* ============================================================================ 
Public functions for advance.c.
============================================================================ */
int advanceshop(CharData *ch, void *me, int cmd, char *argument);

/* Defines */
#define MAX_ADVANCE_LEVEL 10

#endif /* __ADVANCE_H__ */
