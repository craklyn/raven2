#ifndef __ZEDIT_H__
#define __ZEDIT_H__
/* ============================================================================
Header file for Zone Editor. Written by Vex of RavenMUD for RavenMUD.
============================================================================ */

/* Public functions. */
void zSetup(DescriptorData *d, int room_num);
void zSaveToDisk(DescriptorData *d);
void zNewZone(CharData *ch, int new_zone);
void zParse(DescriptorData *d, char *arg);

#endif
