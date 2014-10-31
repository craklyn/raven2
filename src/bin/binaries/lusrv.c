/*
** This is a generic IP name server that runs as a standalone
** process and looks up addresses. This program was written to
** enable a non mulithreaded application to use gethostbyname
** without hanging up the entire process.
**
**                                        Digger 12/30/97
*/

#include "general/lulib.h"

cmdEntry cmdList[] = {
  { "+LU:", doLookup     },
  { "+DC:", doDisconnect },
  { "+EX:", doExit       },
  { "",     NULL         }
};


/*
** Read in the cmd line parameters and build the socket.
*/
int main( int argc, char **argv )
{
  int srv;

  srv = buildService( LUPORT );

  /*
  ** The service has been created, now start serving it.
  */
  while( serviceActive )
  {
    /*
    ** To keep things simple we will only service one connected
    ** client at a time. If this becomes a problem then we will
    ** need to work in the select mechanism to multiplex the
    ** the multiple clients.
    */
    int clt = getClient( srv );

    while( clt >= 0 )
    {
      char buf[2048] = "";
      memset( buf, 0, sizeof( buf ));

      if( read( clt, buf, sizeof(buf) ) <= 0 )
        clt = -1;
      else
      {
        clt = parseCommands( clt, buf, cmdList );
      }
    }
  }

  /*
  ** Shutdown the service and exit.
  */
  printf( "Server Shutting Down\n" );
  shutdown( srv, 2 );
  sleep(3);
  return 0;
}

