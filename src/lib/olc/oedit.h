#ifndef __OEDIT_H__
#define __OEDIT_H__
/* Header file for object editor. I've had enough of this extern nonsense. Vex. */

/* oedit structures. */
typedef struct apply_restrict_type{
    char min; /* smallest value apply can have. */
    char max; /* largest value apply can have. */
} ApplyRestrictType;

typedef struct value_restrict_type{
    int min;
    int max;
} ValueRestrictType;

/* Public functions */
void oSetupNew(DescriptorData *d);
void oSetupExisting(DescriptorData *d, int real_num);
void oSaveToDisk(DescriptorData *d);
void oParse(DescriptorData *d, char *arg);
void oPrintValues(ObjData *obj, char *result);
void oCheckAffectFlags(ObjData *obj);
char applyModLimit(int location, int theMod);
int  objValueLimit(int itemType, int valIndex, int theValue);
void calculateRent(ObjData *obj);

/*. Macros .*/
#define S_PRODUCT(s, i) ((s)->producing[(i)])
#define MAX_OBJ_WEIGHT 10000
#define MAX_OBJ_COST 1000000
#define MAX_OBJ_RENT 50000
#define MAX_OBJ_TIMER 100000

#endif /* __OEDIT_H__ */
