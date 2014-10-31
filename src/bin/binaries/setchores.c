
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/class.h"
#include "general/chores.h"

void update_chores(void)
{
  FILE * fl;
  CharFileU player;

  if( !(fl = fopen("players", "r+b")))
  {
    printf( "Can't open players" );
    exit();
  }

  for( ;; )
  {
    fread( &player, sizeof(player), 1, fl );

    if( feof(fl) )
    {
      fclose(fl);
      puts("Done.");
      exit();
    }

    chore_pfile_init(&player);
    fseek(fl, -sizeof(player), SEEK_CUR);
    fwrite(&player, sizeof(player), 1, fl);
  }
}



int main( int argc, char **argv )
{
  update_chores();
  return 0;
}


