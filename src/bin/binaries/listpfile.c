
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
    printf("Size of 'player' data struct: [%d]", sizeof(player));
    getchar();

    time_t tt;
    printf("Size of time_t data: [%d]", sizeof(tt));
    getchar();

    int32_t i32t;
    printf("Size of int32_t data: [%d]", sizeof(i32t));
    getchar();

    u_char uc;
    printf("Size of u_char data: [%d]", sizeof(uc));
    getchar();

    long l;
    printf("Size of long data: [%d]", sizeof(l));
    getchar();

    long cp;
    printf("Size of char* data: [%d]", sizeof(cp));
    getchar();

    int i;
    printf("Size of int data: [%d]", sizeof(i));
    getchar();

    u_int ui;
    printf("Size of u_int data: [%d]", sizeof(ui));
    getchar();

    sh_int si;
    printf("Size of sh_int data: [%d]", sizeof(si));
    getchar();

    short s;
    printf("Size of short data: [%d]", sizeof(s));
    getchar();

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


