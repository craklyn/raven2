
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"


int
loadPlayer( char *playername, CharFileU *player )
{
  FILE * fl;

  if( !(fl = fopen("players.ARC", "r")))
  {
    printf( "\n***ERROR: Can't open players.archive\n\n" );
    exit(1);
  }

  for( ;; )
  {
    if( fread( player, sizeof(*player), 1, fl ) <= 0 )
    {
      fclose(fl);
      return(0);
    }

    if( strcmp( player->name, playername ) == 0 )
    {
      return(1);
    }
  }
}


void
replacePlayer( CharFileU *newplayer )
{
  FILE *fout = fopen("players.new", "w");
  FILE *fin  = fopen("players",     "r");
  CharFileU player;

  for( ;; )
  {
    if( fread( &player, sizeof(player), 1, fin ) <= 0 )
    {
      fclose(fin);
      fclose(fout);
      return;
    }

    if( strcmp( player.name, newplayer->name ) == 0 )
    {
      printf("Current: %-20s [%2d] [%10d] [%10d] [%20s] %s\n",
              player.name,
              player.level,
              player.points.gold,
              player.points.bank_gold,
              player.title,
              player.host );

      printf("Replace: %-20s [%2d] [%10d] [%10d] [%20s] %s\n",
              newplayer->name,
              newplayer->level,
              newplayer->points.gold,
              newplayer->points.bank_gold,
              newplayer->title,
              newplayer->host );

      newplayer->char_specials_saved.idnum = player.char_specials_saved.idnum;
      fwrite( newplayer, sizeof(*newplayer), 1, fout );
    }
    else
    {
      printf( "Duping: %s              \r", player.name );
      fwrite( &player, sizeof(player), 1, fout );
    }
  }
}


int main( int argc, char **argv )
{
  CharFileU player;

  if( argc != 2 )
  {
    printf("Usage: %s playername\n", argv[0]);
    exit(1);
  }

  if( loadPlayer( argv[1], &player ) )
  {
    replacePlayer( &player );
  }

  else
  {
    printf( "Cannot find %s in players.archive", argv[1] );
  }
  return 0;
}


