
//--------------------------------------------------------------------------
//
extern void save_aliases( CharData *ch );
extern void load_aliases( CharData *ch );

extern struct alias *find_alias( struct alias* alias_list, char *str );
extern void free_alias( struct alias* a );

extern ACMD(do_alias);

