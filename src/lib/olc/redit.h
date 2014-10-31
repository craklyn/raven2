#ifndef __REDIT_H__
#define __REDIT_H__
/* ===========================================================================
Header file for Room Editor.
Vex.
=========================================================================== */

/* ===========================================================================
Public functions.
=========================================================================== */
void rSetupNew(DescriptorData *d, int number);
void rSetupExisting(DescriptorData *d, int real_num);
void rSaveToDisk(DescriptorData *d);
void rParse(DescriptorData *d, char *arg);
void rFreeRoom(RoomData *room);
void rCheckRoomFlags(RoomData *room);
#endif
