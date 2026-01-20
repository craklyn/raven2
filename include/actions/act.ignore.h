#ifndef __ACT_IGNORE_H__
#define __ACT_IGNORE_H__

//--------------------------------------------------------------------------
//
extern void save_ignores( CharData *ch );
extern void load_ignores( CharData *ch );

extern struct ignore *find_ignore(struct ignore* ignore_list, char *str);
extern void free_ignore(struct ignore* a);

extern ACMD(do_ignore);

#endif
