/* ============================================================================ 
Header files for objsave. Theres not a lot here atm.
Vex.
============================================================================ */
#ifndef __OBJSAVE_H__
#define __OBJSAVE_H__

/* ============================================================================ 
Public functions from objsave.c.
============================================================================ */
int     receptionist(CharData *ch, void *me, int cmd, char *argument);
int     cryogenicist(CharData *ch, void *me, int cmd, char *argument);
int     rogue_receptionist(CharData *ch, void *me, int cmd, char *argument);
int     rogue_cryogenicist(CharData *ch, void *me, int cmd, char *argument);

int     Obj_to_store(ObjData * obj, FILE * fl);
ObjData *Obj_from_store(ObjFileElem store);
int     Obj_into_store(ObjData * obj, ObjFileElem *store);

/* Eternal Locker global functions */
void    save_locker( ObjData *obj );
void    save_locker_of( CharData *ch );
void    load_locker(ObjData *obj);

void    add_saved_corpse(time_t created, ObjData *obj);
void    del_saved_corpse(ObjData *obj);
void    boot_corpses(void);
ObjData *find_player_corpse(CharData *ch);

int     deleteCrashFile( CharData * ch );
void	Crash_listrent(CharData *ch, char *name);
int	Crash_load(CharData *ch);
void	Crash_crashsave(CharData *ch);
void	Crash_save_all(void);
int     Crash_is_unrentable(ObjData * obj);
int     crashDeleteFile( char *name );
void    crashRentSave( CharData *ch, int cost );

void desc_to_aliases(ObjData *obj);

/* ============================================================================ 
Rent codes. Rent code structure in db.h.
============================================================================ */
#define RENT_UNDEF      0
#define RENT_CRASH      1
#define RENT_RENTED     2
#define RENT_CRYO       3
#define RENT_FORCED     4
#define RENT_TIMEDOUT   5


#endif  /* End of _OBJSAVE_H_ */
