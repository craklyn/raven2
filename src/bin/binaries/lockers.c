
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"

void find_lockers(void)
{
  FILE * fl;
  FILE * outfile;
  CharFileU player;

  if( !(fl = fopen("players", "r+")))
  {
    printf( "Can't open players\n" );
    exit(1);
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
      exit(1);
    }

    if (player.player_specials_saved.locker_num)
        printf("%-20s [%2d]\n", player.name,
                player.player_specials_saved.locker_num);

  }
}



int main( int argc, char **argv )
{
    find_lockers();

    return (0);
}


