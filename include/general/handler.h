/* ************************************************************************
*   File: handler.h                                     Part of CircleMUD *
*  Usage: header file: prototypes of handling and utility functions       *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */
#ifndef __HANDLER_H__
#define __HANDLER_H__

/* handling the affected-structures */
void affect_total( CharData *ch );			
void affect_to_char(CharData *ch, AffectedType *af);
/* Added byte save to affect_remove for fall rooms */
void affect_remove(CharData *ch, AffectedType *af, int save);
void affect_from_char(CharData *ch, int skill);
bool affected_by_spell(CharData *ch, int skill);
int  aff_level(CharData *ch, int skill);
void affect_join( CharData     *ch,
                         AffectedType *af,
                         bool          add_dur,
                         bool          avg_dur,
                         bool          add_mod,
                         bool          avg_mod );
int  spell_level(CharData *ch, int skill);
int  spell_duration(CharData *ch, int skill);
void doFallRoom(CharData*);

/* utility */
char    *money_desc(int amount);
ObjData *create_money(int amount);
int     isname(char *str, char *namelist);
char    *fname(const char *namelist);
int	get_number(char **name);

/* ******** objects *********** */
void	obj_to_char(ObjData *object, CharData *ch);
void	obj_from_char(ObjData *object);

void	equip_char(CharData *ch, ObjData *obj, int pos);
ObjData *unequip_char(CharData *ch, int pos);

ObjData *get_obj_in_list(char *name, ObjData *list);
ObjData *get_obj_in_list_num(int num, ObjData *list);
ObjData *get_obj(char *name);
ObjData *get_obj_num(int nr);

void    obj_to_room(ObjData * object, room_rnum room);
void	obj_from_room(ObjData *object);
void	obj_to_obj(ObjData *obj, ObjData *obj_to);
void	obj_from_obj(ObjData *obj);
void	object_list_new_owner(ObjData *list, CharData *ch);

void	extract_obj(ObjData *obj);

/* ******* characters ********* */
CharData *get_char_room(char *name, int room);
CharData *get_char_num(int nr);
CharData *get_char(char *name);

void	 char_from_room(CharData *ch);
void	 char_to_room(CharData *ch, int room);
void	 extract_char(CharData *ch);
void     delete_extracted(void);

/* find if character can see */
CharData *get_char_room_vis(CharData *ch, char *name);
CharData *get_player_vis(CharData *ch, char *name);
CharData *get_char_vis(CharData *ch, char *name, int pc_only);
ObjData  *get_obj_in_list_vis(CharData *ch, char *name, ObjData *list);
ObjData  *get_obj_vis(CharData *ch, char *name);
ObjData  *get_object_in_equip_vis(CharData *ch, char *arg, ObjData *equipment[], int *j);

/* find all dots */
int	find_all_dots(char *arg);

#define FIND_INDIV	0
#define FIND_ALL	1
#define FIND_ALLDOT	2


/* Generic Find */
int	generic_find(char *arg, int bitvector, CharData *ch,
        CharData **tar_ch, ObjData **tar_obj);

#define FIND_CHAR_ROOM     1
#define FIND_CHAR_WORLD    2
#define FIND_OBJ_INV       4
#define FIND_OBJ_ROOM      8
#define FIND_OBJ_WORLD    16
#define FIND_OBJ_EQUIP    32


#endif /* __HANDLER_H__ */
