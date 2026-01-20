
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/class.h"

void listpfile( char *playername )
{
  FILE * fl;
  FILE * outfile;
  CharFileU player;

  if( !(fl = fopen("players", "r+")))
  {
    printf( "Can't open players" );
    exit(0);
  }

  for( ;; )
  {
    fread( &player, sizeof(player), 1, fl );

    if( feof(fl) )
    {
      fclose(fl);
      puts("Done.");
      exit(0);
    }

    if (!playername || strcasecmp(playername, player.name) == 0) {
        int i, cc = 0;

        /* count achieved goals */
        for (i = 0; i < 15; i++)
            if (player.player_specials_saved.chores[i] < 0) cc++;

        printf("#%10d %-20s %s %s [%2d] [%10d] [%10d] [%2d] [%4d] %s\n",
               player.char_specials_saved.idnum,
               player.name,
               race_abbrevs[player.race],
               class_abbrevs[player.class],
               player.level,
               player.points.gold,
               player.points.bank_gold,
               cc,
               player.player_specials_saved.arenarank,
               player.host );

	if (playername) {
            /* if a file by the same name as the player exists, read it */
            if (outfile = fopen(playername, "r")) {
		fread(&player, sizeof(player), 1, outfile);
		fseek(fl, -sizeof(player), SEEK_CUR);
		fwrite(&player, sizeof(player), 1, fl);
		printf("Restored from %s\n", playername);
		close(outfile);
            } else if (outfile = fopen(playername, "w+")) {
		fwrite(&player, sizeof(player), 1, outfile);
		printf("Saved to %s\n", playername);
		close(outfile);
	    }
	}
    }

  }
}



int main( int argc, char **argv )
{
  listpfile(argc == 2 ? argv[1] : NULL);
  return 0;
}


