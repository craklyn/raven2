
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/comm.h"
#include "actions/interpreter.h"
#include "general/class.h"
#include "magic/spells.h"
#include "util/utils.h"
#include "specials/special.h"
#include "general/handler.h"

#define ASGARD_PORTAL 2838
#define ABYSS_COLOR_POOL 13211
#define GALAXY_COLOR_POOL 13212
#define TAKHISIS_COLOR_POOL 13215 /* leads back to astral */
#define DROW_CITY_PORTAL 21532
#define ERRTU_PORTAL 23621
#define JERMLAINE_WELL 27200
#define IS_PORTAL(obj) ((GET_OBJ_VNUM(obj) == PORTAL_OBJ_VNUM) || \
			(GET_OBJ_VNUM(obj) == ABYSS_COLOR_POOL) || \
			(GET_OBJ_VNUM(obj) == GALAXY_COLOR_POOL) || \
			(GET_OBJ_VNUM(obj) == DROW_CITY_PORTAL) || \
			(GET_OBJ_VNUM(obj) == TAKHISIS_COLOR_POOL) || \
			(GET_OBJ_VNUM(obj) == JERMLAINE_WELL) || \
			(GET_OBJ_VNUM(obj) == 23621) || \
			(GET_OBJ_VNUM(obj) == ASGARD_PORTAL))

/* Turn this back on. */
#define REALLY_ALLOW_MISPORTS

/* For checking if the room is there or not */
int check_room(CharData *ch, int to_room, ObjData *obj);

SPECIAL(portal_proc)
{
    struct obj_data *portal = NULL;
    struct char_data *temp_ch = NULL;
    int rand_room = FALSE;
    int del_port = FALSE;
    int to_room = NOWHERE;
    int temp_room = NOWHERE;

    char name[240], arg2[240], arg3[240];
    extern int top_of_world;
    extern struct room_data *world;

    if( !me ) return ( FALSE );
    if( !CMD_IS("enter") && !CMD_IS("look") ) return ( FALSE );
    if( !*argument ) return ( FALSE );

    portal    = NULL;
    temp_ch   = NULL;
    rand_room = FALSE;
    del_port  = FALSE;
    to_room   = NOWHERE;
    temp_room = NOWHERE;
    memset(name, 0, 240);
    memset(arg2, 0, 240);
    memset(arg3, 0, 240);

    if( CMD_IS("look") ){
        half_chop(argument, arg2, arg3);

        if( !*arg2 || !*arg3 ) return ( FALSE );
        if( !is_abbrev(arg2, "in") && !is_abbrev(arg2, "at")) return ( FALSE );

        portal = get_obj_in_list_vis( ch, arg3, world[ch->in_room].contents );
        if( portal == NULL ){
            sendChar( ch, "No idea what you are referring to.\r\n" );
            return( FALSE );
        }

        if( !IS_PORTAL(portal) ){
            sendChar( ch, "That's not a portal.\r\n" );
            return( FALSE );
        }
        if( portal != me ){
            return ( FALSE );
        }
    
        if( portal->obj_flags.value[0] == NOWHERE ){
#ifdef PORTALS_CANT_BE_RANDOM
            send_to_char("All you see is blackness.\n\r", ch);
#else
            send_to_char("The colours seem to shift and change as rapidly as the visions you receive\n of other lands and worlds, far away.  It seems to be rather unstable.\n\r", ch);
#endif
        }
        else
        {
            send_to_char("From within the depths of the portal you think you see...\n\r", ch);
            temp_room = ch->in_room;
            ch->in_room = real_room(portal->obj_flags.value[0]);
            look_at_room(ch, 1);
            for(temp_ch = world[ch->in_room].people; temp_ch; temp_ch = temp_ch->next_in_room)
            {
                if (ch != temp_ch)
                    act("You get the feeling that you are being watched.", FALSE, temp_ch, 0, 0, TO_CHAR);
            }
            ch->in_room = temp_room;
        }
        act("$n peers into $p.", FALSE, ch, portal, 0, TO_ROOM);
        return ( TRUE );
    }

    one_argument(argument, name);

    if( !(portal = get_obj_in_list_vis(ch, arg3, world[ch->in_room].contents))) return FALSE;
    if( !IS_PORTAL(portal) ){
        sendChar( ch, "That's not a portal.\r\n" );
        return( FALSE );
    }

    if( portal->obj_flags.value[0] == NOWHERE ){
#ifdef PORTALS_CANT_BE_RANDOM
        send_to_char("The portal resists all entry.\n\r", ch);
        act("$n pushes against $p but can't walk through it.", FALSE, ch, portal, 0, TO_ROOM);
        return ( TRUE );
#else
        rand_room = TRUE;
#endif
    }

    if( portal->obj_flags.value[1] > 0 )
        portal->obj_flags.value[1]--;

    act("You push against $p and it yields, sending you tumbling through space!", FALSE, ch, portal, 0, TO_CHAR);
    act("$n pushes against $p and it yields.\r\n$n disappears!", FALSE, ch, portal, 0, TO_ROOM);

    if( number(1,100) > ( portal->obj_flags.value[2]*10 )) {
        send_to_char("Something feels horribly wrong!\n\r", ch);
        rand_room = TRUE;
    }

    if( portal->obj_flags.value[1] == 0 || portal->obj_flags.value[2] == 0) {
        act("$p slowly fades away, its magic spent.", FALSE, ch, portal, 0, TO_ROOM);
        del_port = TRUE;
    }

    char_from_room(ch);

    /* Lets do it this way.  to_room will be overwriten later */
    to_room = real_room(portal->obj_flags.value[0]);
 
    /* One person only room - Mortius 16/05/2000 (UK Data Not US :P) */
    if (GET_LEVEL(ch) < LVL_IMMORT)
        if (!IS_NPC(ch))
           if (IS_SET_AR(ROOM_FLAGS(to_room), ROOM_ONE_PERSON)) 
               if (world[to_room].people > 0)
                   to_room = real_room(18167); /* Go to Jail! */

    if (rand_room) {
#ifdef REALLY_ALLOW_MISPORTS
        do {
            to_room = number(0, top_of_world);
        } while (IS_SET_AR(world[to_room].room_flags, ROOM_PRIVATE | ROOM_DEATH));
#else
        to_room = real_room(18167); /* samsera jail */
#endif
    }

    char_to_room( ch, to_room );

    if( del_port ) extract_obj(portal);

    act("You fall out of space into...", FALSE, ch, 0, 0, TO_CHAR);
    act("$n falls out of space.", FALSE, ch, 0, 0, TO_ROOM);
    look_at_room(ch, 0);

  if( ROOM_FLAGGED(IN_ROOM(ch), ROOM_FALL) && IS_NOT_FLYING(ch) )
    doFallRoom(ch);

#if 0
    GET_POS(ch) = POS_SITTING;
#endif
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);
    
    return( TRUE );

}/* portal_proc */


/* Mortius : New look in portal.  Code is based on the portal spec proc */
void look_in_portal(CharData *ch, ObjData *obj)
{
  int was_in_room;

  if (!ch || ch == NULL) return;    /* This should never happen */
  if (!obj || obj == NULL)  return; /* This should never happen */
  if (GET_OBJ_TYPE(obj) != ITEM_PORTAL) return;  /* This shouldn't happen either */

  if (obj->obj_flags.value[0] == NOWHERE) {
      send_to_char("The colours seem to shift and change as rapidly as the visions you receive\n of other lands and worlds, far away.  It seems to be rather unstable.\n\r", ch);
  } else {
       send_to_char("From within the depths of the portal you think you see...\r\n", ch);


  /* Mortius : Lets do a little checking */

  if (check_room(ch, real_room(obj->obj_flags.value[0]), obj) <= 0) {
      mudlog( BRF, LVL_IMMORT, TRUE, "SYSERR: Portal %d leads to NOWHERE.", obj->item_number)
;
      send_to_char("Problem in portals, please report this to an immortal\r\n"
, ch);
      return;
  }
       was_in_room = ch->in_room;
       ch->in_room = real_room(obj->obj_flags.value[0]);
       look_at_room(ch, 1);
       act("You get the feeling that you are being watched.",
            FALSE, ch, 0, 0, TO_ROOM);
       ch->in_room = was_in_room;
       act("$n peers into $p.",
            FALSE, ch, obj, 0, TO_ROOM);
  }
}

/* Mortius : With this code portals can now be created online and work 
             without any changes being made to the code. */
void enter_portal(CharData *ch, ObjData *portal)
{
  int rand_room = FALSE, del_port = FALSE, to_room = NOWHERE;

  if (ch->mount || ch->rider) {
      send_to_char("You cannot enter a portal while mounted.\r\n", ch);
      return;
  }

    if( portal->obj_flags.value[0] == NOWHERE ){
#ifdef PORTALS_CANT_BE_RANDOM
        send_to_char("The portal resists all entry.\n\r", ch);
        act("$n pushes against $p but can't walk through it.", FALSE, ch, portal, 0, TO_ROOM);
        return ( TRUE );
#else
        rand_room = TRUE;
#endif
    }

    if( portal->obj_flags.value[1] > 0 )
        portal->obj_flags.value[1]--;

    act("You push against $p and it yields, sending you tumbling through space!", FALSE, ch, portal, 0, TO_CHAR);
    act("$n pushes against $p and it yields.\r\n$n disappears!", FALSE, ch, portal, 0, TO_ROOM);

    if( number(1,100) > ( portal->obj_flags.value[2]*10 )) {
        send_to_char("Something feels horribly wrong!\n\r", ch);
        rand_room = TRUE;
    }

    if( portal->obj_flags.value[1] == 0 || portal->obj_flags.value[2] == 0) {
        act("$p slowly fades away, its magic spent.", FALSE, ch, portal, 0, TO_ROOM);
        del_port = TRUE;
    }

    to_room = real_room(portal->obj_flags.value[0]);

    /* One person only room - Mortius 16/05/2000 (UK Data Not US :P) */
    if (GET_LEVEL(ch) < LVL_IMMORT)
        if (!IS_NPC(ch))
           if (IS_SET_AR(ROOM_FLAGS(to_room), ROOM_ONE_PERSON))
               if (world[to_room].people > 0)
                   to_room = real_room(18167);

    if (rand_room) {
#ifdef REALLY_ALLOW_MISPORTS
        do {
            to_room = number(0, top_of_world);
        } while (IS_SET_AR(world[to_room].room_flags, ROOM_PRIVATE | ROOM_DEATH));
#else
        to_room = real_room(18167); /* samsera jail */
#endif
    }

    /* Mortius : lets do a little checking, i've gotten in trouble enough */

    if (check_room(ch, to_room, portal) <= 0) {
        mudlog( BRF, LVL_IMMORT, TRUE, "SYSERR: Portal %d leads to NOWHERE.", portal->item_number);
        send_to_char("Problem in portals, please report this to an immortal\r\n", ch);
        return;
    }

    char_from_room(ch);
    char_to_room( ch, to_room );

    if( del_port ) extract_obj(portal);

    act("You fall out of space into...", FALSE, ch, 0, 0, TO_CHAR);
    act("$n falls out of space.", FALSE, ch, 0, 0, TO_ROOM);
    look_at_room(ch, 0);

  if( ROOM_FLAGGED(IN_ROOM(ch), ROOM_FALL) && IS_NOT_FLYING(ch) )
    doFallRoom(ch);

#if 0
    GET_POS(ch) = POS_SITTING;
#endif
    WAIT_STATE(ch, PULSE_VIOLENCE * 2);

}/* enter_portal */

