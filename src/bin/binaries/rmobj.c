
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"


int
main( int argc, char **argv )
{
    FILE   *f_in;
    FILE   *f_out;
    struct obj_file_elem rm_obj;
    struct rent_info rent;
    char   new_name[80];
    int    lowobjnum, highobjnum;

    if( argc <= 3 ){
        printf( "\nUsage: rmobj <objfile> <lowobj> <highobj>\n\n" );
        exit( 1 );
    }

    lowobjnum  = atoi( argv[2] );
    highobjnum = atoi( argv[3] );

    if( !( f_in = fopen( argv[1], "r" ))){
        perror( "Cannot open input file" );
        exit( 1 );
    }

    strcpy( new_name, argv[1] );
    strcat( new_name, ".NEW"  );
    if( !( f_out = fopen( new_name, "w" ))){
        perror( "Cannot open output file" );
        exit( 1 );
    }
    fwrite( &rent, sizeof(rent), 1, f_out);

    fread(  &rent, sizeof(rent), 1, f_in);

    while( !feof(f_in) ){
        fread( &rm_obj, sizeof( rm_obj ), 1, f_in );
        if (ferror(f_in)) {
            perror("Reading crash file: Crash_load.");
            fclose(f_in);
            return 1;
        }

       if( !feof(f_in) ){
           /* All of the real work is done here !!! */
           if( rm_obj.item_number >= lowobjnum && rm_obj.item_number <= highobjnum ) {
               printf( "%s:Trashing item id %d.\n", argv[1], rm_obj.item_number );
           }
           else
               fwrite( &rm_obj, sizeof( rm_obj ), 1, f_out );
       }
    }

    fclose(f_out);
    return 0;

}
