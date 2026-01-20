
/* ************************************************************************
*  file: purgeplay.c                                    Part of CircleMUD * 
*  Usage: purge useless chars from playerfile                             *
*  All Rights Reserved                                                    *
*  Copyright (C) 1992, 1993 The Trustees of The Johns Hopkins University  *
************************************************************************* */

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/new_db.h"
#include "magic/spells.h"
#include "util/utils.h"
#include "general/class.h"

void	expand()
{
   FILE * fl;
   FILE * outfile;
   struct char_file_u     old_player;
   struct new_char_file_u new_player;

   if (!(fl = fopen("players", "r+"))) {
      printf("Can't open players\n");
      exit(0);
   }

   outfile = fopen("players.exp", "w");

   for (; ; ) {
      fread(&old_player, sizeof( old_player ), 1, fl);
      if (feof(fl)) {
	 fclose(fl);
	 fclose(outfile);
	 puts("Done.");
	 exit(0);
      }

      memset( (char *)&new_player, 0, sizeof( new_player ));

      strncpy( new_player.name,        old_player.name,        sizeof( new_player.name ));
      strncpy( new_player.description, old_player.description, sizeof( new_player.description ));
      strncpy( new_player.title,       old_player.title,       sizeof( new_player.title ));

      new_player.sex = old_player.sex;
      new_player.class = old_player.class;
	  new_player.race = old_player.race;
      new_player.level = old_player.level;
	  new_player.lostlevels = old_player.lostlevels;
	  new_player.orcs = old_player.orcs;
      new_player.birth = old_player.birth;
      new_player.played = old_player.played;
      new_player.weight = old_player.weight;
      new_player.height = old_player.height;

      bcopy( old_player.pwd, new_player.pwd, sizeof( new_player.pwd ));

      new_player.char_specials_saved = old_player.char_specials_saved;
      new_player.abilities = old_player.abilities;
      new_player.points = old_player.points;
	  new_player.player_specials_saved = old_player.player_specials_saved;

      bcopy( (char *)old_player.affected, (char *)new_player.affected, sizeof( old_player.affected ));
      
	  new_player.last_logon = old_player.last_logon;
      strncpy( new_player.host, old_player.host, sizeof( new_player.host ));

      
	  
	  {
          struct player_special_data_saved *to   = &new_player.player_specials_saved;
          struct player_special_data_saved *from = &old_player.player_specials_saved;

          /* NEW FIELDS */
		  memset( (char *)&to->skill_usage, 0, sizeof( to->skill_usage ));

          bcopy( from->skills,     to->skills,     sizeof( from->skills ));
          bcopy( from->conditions, to->conditions, sizeof( from->conditions ));
		  bcopy( from->pref,       to->pref,       sizeof( from->pref ));

      }/* End if actual expansion */

      fwrite(&new_player, sizeof( new_player ), 1, outfile);

    }
}



int main(int argc, char *argv[])
{
      expand();
	  return 0;
}

