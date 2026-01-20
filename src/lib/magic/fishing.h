#define BASE_FISH 1497
#define FISH_TROPHY 1496
#define FISH_MASTER 18106

ACMD(do_castout);
ACMD(do_reelin);
void check_fishing(void);
int check_fishing_give(CharData *from, CharData *to, ObjData *obj);
void fishoff_ticker(void);
void final_fishoff_score(void);
extern int fishoff;

