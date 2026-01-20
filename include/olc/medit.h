#ifndef __MEDIT_H__
#define __MEDIT_H__
/* ============================================================================
Header file for mob editor.
Written by Vex of RavenMUD for RavenMUD.
============================================================================ */

/* Public functions. */
void calculateXpAndGp(CharData *mob);
void mSetupNew(DescriptorData *d);
void mSetupExisting(DescriptorData *d, int rmob_num);
void mSaveToDisk(DescriptorData *d);
void mParse(DescriptorData *d, char *arg);
void mMenu(DescriptorData *d);
void mFreeMob(CharData *mob);
int  avgDamPerRound(CharData *mob, int nominal_ac);
int  spellAttackStrength(CharData *mob);

#endif
