#ifndef __MISSILE_H__
#define __MISSILE_H__

#define ROOM_EXIT(room, door) (world[room].dir_option[door] && \
                               world[room].dir_option[door]->to_room != NOWHERE && \
                              !IS_SET(world[room].dir_option[door]->exit_info, EX_CLOSED))

#define TARG_LIGHT_OK(sub, obj) \
	(!IS_AFFECTED(sub, AFF_BLIND) && \
	(IS_LIGHT((obj)->in_room) || IS_AFFECTED((sub), AFF_INFRAVISION)))

#define CAN_SEE_TARG(ch, vict) \
	(SELF(ch, vict) || \
	 ((PRF_FLAGGED(ch, PRF_HOLYLIGHT) || \
	  		(INVIS_OK(ch, vict) &&  \
			 TARG_LIGHT_OK(ch, vict))) && \
      (GET_REAL_LEVEL(ch) >= GET_INVIS_LEV(vict))))

#define DEFAULT_SHORT_RANGE			1
#define DEFAULT_MEDIUM_RANGE		        2
#define DEFAULT_LONG_RANGE			4

#define MISSILE_TYPE_UNDEFINED                  0
#define MISSILE_TYPE_MISSILE		        1
#define MISSILE_TYPE_THROW			2
#define MISSILE_TYPE_SPELL			3
#define MISSILE_TYPE_GRENADE        4
#define NUM_MISSILE_TYPES                       5
/* Public constants */
char *missile_types[NUM_MISSILE_TYPES + 1];

/* Public functions */
extern CharData
*find_vict_dir( CharData *ch,
                char     *vict_name,
                int       range,
                int       dir );

extern CharData
*get_targ_room_vis( CharData *ch,
                    int       room,
                    char     *name );

extern bool
gen_missile( CharData *ch,
             CharData *vict,
             ObjData  *miss_obj,
             int       spell_no,
             int       dir,
             int       missile_type );

void gen_grenade(CHAR_DATA *ch, CHAR_DATA *vict, int dir, int missile_type);

extern int
missile_to_vict( CharData *ch,
                 CharData *vict,
                 char     *obj_name, 
                 int       dir );

extern bool
missile_hit( CharData *ch,
             CharData *vict,
             ObjData  *miss_obj,
             int       range,
             int       type );

#endif
