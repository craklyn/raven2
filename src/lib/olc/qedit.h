#ifndef __QEDIT_H__
#define __QEDIT_H__
/* ============================================================================
 * Header file for Zone Editor. Written by Vex of RavenMUD for RavenMUD.
 *
 * Header Note: Well Vex old pal, let me give you a bit of a tip, even though
 * you'll never be around to see it...functions that reside in qedit.c won't
 * EVER appear in the public scope when you have an ifndef ZEDIT_H compiler
 * defines in this header file. Nice try though. -Xiuh 06.09.09
============================================================================ */

/* Public functions. */
void qSetupExisting(DescriptorData *d, int quest_num);
void qSaveToDisk(DescriptorData *d);
void qSetupNew(DescriptorData *d);
void qParse(DescriptorData *d, char *arg);
void qFreeQuest(QuestData *quest);
void qedit_string_cleanup(struct descriptor_data *d, int terminator);

/* ============================================================================
Public variables.
============================================================================ */
#ifndef __QEDIT_C__

extern char *qst_remort_races[];

#endif /* __QEDIT_C__ */

#endif /* _QEDIT_H_ */
