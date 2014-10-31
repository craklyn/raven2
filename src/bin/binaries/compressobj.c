
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"

struct old_obj_file_elem {
   obj_vnum item_number;

   int	value[4];
   int	extra_flags;
   int	weight;
   int	timer;
   long bitvector;
   byte position;
   byte spare3;
   byte spare2;
   struct obj_affected_type affected[MAX_OBJ_AFFECT];
};


int
main( int argc, char **argv )
{
    FILE *f_in, *f_out;
    struct old_obj_file_elem obj_old;
    struct obj_file_elem obj_new;
    struct rent_info rent;

    char new_name[80];

    if( argc <= 1 ){
        printf( "\nUsage: exandobj <objfile>\n\n" );
        exit( 1 );
    }

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

    fread(  &rent, sizeof(rent), 1, f_in);
    fwrite( &rent, sizeof(rent), 1, f_out);

    while( !feof(f_in) ){
        fread( &obj_old, sizeof( obj_old ), 1, f_in );
        if (ferror(f_in)) {
            perror("Reading crash file: Crash_load.");
            fclose(f_in);
            return 1;
        }

       if( !feof(f_in) ){
           /* All of the real work is done here !!! */
#if 1
           printf( "Object %5d is being expanded.\n", obj_old.item_number );
#endif
           obj_new.item_number = obj_old.item_number;
           memcpy( &obj_new.value, &obj_old.value, sizeof( obj_old.value ));
           obj_new.extra_flags = obj_old.extra_flags;
           obj_new.weight      = obj_old.weight;
           obj_new.timer       = obj_old.timer;
           obj_new.position    = -1;
           obj_new.spare1      = -1;
           obj_new.spare2      = -1;
           obj_new.spare3      = -1;
           obj_new.bitvector   = obj_old.bitvector;
           memcpy( &obj_new.affected, &obj_old.affected, sizeof( obj_old.affected ));

           fwrite( &obj_new, sizeof( obj_new ), 1, f_out );
       }
    }

    memset( &obj_new, 0, sizeof( obj_new ));
    obj_new.item_number = 1298;
    obj_new.position    = -1;
    obj_new.spare1      = -1;
    obj_new.spare2      = -1;
    obj_new.spare3      = -1;
    fwrite( &obj_new, sizeof( obj_new ), 1, f_out );

    fclose(f_out);
    return 0;
}

