/* ************************************************************************
*   File: graph.c                                       Part of CircleMUD *
*  Usage: various graph algorithms                                        *
*                                                                         *
*  All rights reserved.  See license.doc for complete information.        *
*                                                                         *
*  Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University *
*  CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.               *
************************************************************************ */


#define TRACK_THROUGH_DOORS

/* You can define or not define TRACK_THOUGH_DOORS, above, depending on
   whether or not you want track to find paths which lead through closed
   or hidden doors.
*/

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "util/utils.h"
#include "general/class.h"
#include "general/comm.h"
#include "actions/interpreter.h"
#include "general/handler.h"
#include "magic/spells.h"
#include "actions/fight.h"
#include "actions/movement.h"


struct bfs_queue_struct {
  sh_int room;
  char dir;
  struct bfs_queue_struct *next;
};

static struct bfs_queue_struct *queue_head = 0, *queue_tail = 0;

/* Utility macros */
#define MARK(room) (SET_BIT_AR(ROOM_FLAGS(room), ROOM_BFS_MARK))
#define UNMARK(room) (REMOVE_BIT_AR(ROOM_FLAGS(room), ROOM_BFS_MARK))
#define IS_MARKED(room) (IS_SET_AR(ROOM_FLAGS(room), ROOM_BFS_MARK))
#define TOROOM(x, y) (world[(x)].dir_option[(y)]->to_room)
#define IS_CLOSED(x, y) (IS_SET(world[(x)].dir_option[(y)]->exit_info, EX_CLOSED))

#ifdef TRACK_THROUGH_DOORS
#define VALID_EDGE(x, y) (world[(x)].dir_option[(y)] && \
			  (TOROOM(x, y) != NOWHERE) &&	\
			  (!IS_MARKED(TOROOM(x, y))) && \
                          (!ROOM_FLAGGED(x, ROOM_NOTRACK)))
#else
#define VALID_EDGE(x, y) (world[(x)].dir_option[(y)] && \
			  (TOROOM(x, y) != NOWHERE) &&	\
			  (!IS_CLOSED(x, y)) &&		\
			  (!IS_MARKED(TOROOM(x, y))))
#endif

void bfs_enqueue(sh_int room, char dir)
{
  struct bfs_queue_struct *curr;

  CREATE(curr, struct bfs_queue_struct, 1);
  curr->room = room;
  curr->dir = dir;
  curr->next = 0;

  if (queue_tail) {
    queue_tail->next = curr;
    queue_tail = curr;
  } else
    queue_head = queue_tail = curr;
}


void bfs_dequeue(void)
{
  struct bfs_queue_struct *curr;

  curr = queue_head;

  if (!(queue_head = queue_head->next))
    queue_tail = 0;
  free(curr);
}


void bfs_clear_queue(void)
{
  while (queue_head)
    bfs_dequeue();
}


/* find_first_step: given a source room and a target room, find the first
   step on the shortest path from the source to the target.

   Intended usage: in mobile_activity, give a mob a dir to go if they're
   tracking another mob or a PC.  Or, a 'track' skill for PCs.
*/

int find_first_step(sh_int src, sh_int target)
{
  int curr_dir;
  sh_int curr_room;

  if (src < 0 || src > top_of_world || target < 0 || target > top_of_world) {
    mlog("Illegal value passed to find_first_step (graph.c)");
    return BFS_ERROR;
  }
  if (src == target)
    return BFS_ALREADY_THERE;

  /* clear marks first */
  for (curr_room = 0; curr_room <= top_of_world; curr_room++)
    UNMARK(curr_room);

  MARK(src);

  /* first, enqueue the first steps, saving which direction we're going. */
  for (curr_dir = 0; curr_dir < NUM_OF_DIRS; curr_dir++)
    if (VALID_EDGE(src, curr_dir)) {
      MARK(TOROOM(src, curr_dir));
      bfs_enqueue(TOROOM(src, curr_dir), curr_dir);
    }
  /* now, do the classic BFS. */
  while (queue_head) {
    if (queue_head->room == target) {
      curr_dir = queue_head->dir;
      bfs_clear_queue();
      return curr_dir;
    } else {
      for (curr_dir = 0; curr_dir < NUM_OF_DIRS; curr_dir++)
	if (VALID_EDGE(queue_head->room, curr_dir)) {
	  MARK(TOROOM(queue_head->room, curr_dir));
	  bfs_enqueue(TOROOM(queue_head->room, curr_dir), queue_head->dir);
	}
      bfs_dequeue();
    }
  }

  return BFS_NO_PATH;
}


/************************************************************************
*  Functions and Commands which use the above fns		        *
************************************************************************/

ACMD(do_track)
{
  CharData *vict;
  int dir, num;
  int destination = 0;
  
  if( !GET_SKILL(ch, SKILL_TRACK) )
  {
    sendChar( ch, "You have no idea how.\r\n" );
    return;
  }

  one_argument(argument, arg);

  if( !*arg )
  {
    sendChar( ch, "Whom are you trying to track?\r\n" );
    return;
  }

  if( !(vict = get_char_vis(ch, arg, 0)) )
  {
    sendChar( ch, "No-one around by that name.\r\n" );
    return;
  }
  
  destination = vict->in_room;

  if (vict->false_trail) {
      if (ch->in_room == vict->false_trail) {
          sendChar(ch, "You discover you are on a false trail!\r\n");
          vict->false_trail = 0;
          return;
      } else {
        destination = vict->false_trail;
      }
  }
  
  if ((IS_SET_AR(ROOM_FLAGS(destination), ROOM_NOTRACK) || (IS_AFFECTED(vict, AFF_NOTRACK))))
  {
    sendChar (ch, "You can't sense a trail to %s from here.\r\n", HMHR(vict));
    return;
  }

  dir = find_first_step(ch->in_room, destination);

  switch( dir )
  {
  case BFS_ERROR:
    sendChar( ch, "Hmm.. something seems to be wrong.\r\n" );
    break;

  case BFS_ALREADY_THERE:
    sendChar(ch, "You're already in the same room!!\r\n" );
    break;

  case BFS_NO_PATH:
    sendChar( ch, "You can't sense a trail to %s from here.\r\n", HMHR(vict));
    break;

  default:
    num = number(0, 101);	/* 101% is a complete failure */
    if( GET_SKILL(ch, SKILL_TRACK) < num)
    {
      dir = number(0, NUM_OF_DIRS - 1);
      if( CAN_GO( ch, dir ) )
      {
        sendChar( ch, "You sense a trail %s from here!\r\n", dirs[dir] );
      }
      else
      {
        sendChar( ch, "You are having difficulty finding the trail." );
      }
    }
    else
    {
        sendChar( ch, "You sense a trail %s from here!\r\n", dirs[dir] );
    }
    break;
  }

}


void
hunt_victim( CharData* ch)
{
  ACMD(do_say);

  int dir, dest;
  byte found;
  CharData *tmp;

  if( !ch || !SEEKING(ch) ) return;

  /*
  ** make sure the char still exists
  */
  for( found = 0, tmp = character_list; tmp && !found; tmp = tmp->next )
  {
    if( SEEKING(ch) == tmp ) found = 1;
  }

  if( !found )
  {
    /*
    ** At this point the mob should head home.
    */
    do_say( ch, "Damn! The little dweeb managed to escape!!", 0, 0 );
    SEEKING(ch) = 0;
    return;
  }

  dest = SEEKING(ch)->in_room;
  if (SEEKING(ch)->false_trail) dest = SEEKING(ch)->false_trail;

  dir = find_first_step(ch->in_room, dest);

  if( dir < 0 )
  {
    sprintf(buf, "Damn!  Lost %s!", HMHR(SEEKING(ch)));
    do_say(ch, buf, 0, 0);
    SEEKING(ch) = 0;
    return;
  }

  else
  {
    perform_move(ch, dir, 0);
    if (ch->in_room == dest) {
      if (dest == SEEKING(ch)->in_room)
        hit(ch, SEEKING(ch), TYPE_UNDEFINED);
      else
        SEEKING(ch)->false_trail = 0;
    }
    return;
  }
}
