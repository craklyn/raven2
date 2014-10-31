
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"

void listpfile( char *playername )
{
  FILE * fl;
  FILE * outfile;
  CharFileU player;
  char	*ptr;

  if( !(fl = fopen("players", "r+")))
  {
    printf( "Can't open %s.", filename );
    exit();
  }

  outfile = fopen("players.new", "w");

  for( ;; )
  {
    fread( &player, sizeof(player), 1, fl );

    if( feof(fl) )
    {
      fclose(fl);
      fclose(outfile);
      puts("Done.");
      exit();
    }

    for( ptr = player.name; *ptr; ptr++ )
    {
      printf("%-20s [%2d] [%10d] [%10d] [%20s] %s\n",
              player.name,
              player.level,
              player.points.gold,
              player.points.bank_gold,
              player.title,
              player.host );
    }

  }
}



void main( int argc, char **argv )
{
  if (argc != 2)
    printf("Usage: %s playername\n", argv[0]);
  else
    listpfile(argv[1]);
}


