/* ************************************************************************
 *  file:  equpdate.c                                   Part of RavenMUD   *
 *  Usage: Updating equipment in lockers and in plrobj. The script has     *
 *  setup so that it will maintain the color code characters during and    *
 *  after the update.                                                      *
 *                                                                         *
 ************************************************************************* */

#include <glob.h>

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "util/utils.h"
#include "general/objsave.h"

/* external functions */
int Obj_to_store (ObjData * obj, FILE * fl);
void extract_obj (ObjData *obj);
void initialize_global_variables (void);
void index_boot (int mode);

void
cprint (char *string)
{
  char *colortable[] = {
    "[0m", "[31m", "[32m", "[33m", "[34m", "[35m", "[36m",
    "[37m", "[1;31m", "[1;32m", "[1;33m", "[1;34m", "[1;35m",
    "[1;36m", "[1;37m", "[41m", "[42m", "[43m", "[44m",
    "[45m", "[46m", "[47m", "[41;1;33m", "[40m", "[30m",
    "[5m",
  };

  while (*string)
  {
      if (*string == '&')
      {
          long l;
          char *end;

          l = -1;
          l = strtol (string + 1, &end, 10);
          string = end;
          if (l >= 0 && l < sizeof (colortable) / sizeof (char *))
          {
              printf (colortable[l]);
          }
      }
      else
      {
          putchar (*string);
          string++;
      }
  }
}

int
inlist (int vnum, int *list)
{
  int i;
  for (i = 0; list[i] != -1; i++) if (list[i] == vnum) return 1;
  return 0;
}

#define ITEMNUM item.item_number

void
update_file (char *file, int *vnum, long seek)
{
  FILE *f = fopen (file, "r+");
  ObjFileElem item;

  if (!f)
  {
      perror ("Can't open file");
      return;
  }
  fseek (f, seek, SEEK_SET);
  while (!feof (f))
  {
      long pos = ftell (f);

      fread (&item, sizeof (ObjFileElem), 1, f);
      if (ferror (f))
      {
          perror ("Reading item: update_file");
          fclose (f);
          return;
      }
      if (!feof (f) && ITEMNUM != -1 && (!vnum || inlist (ITEMNUM, vnum)))
      {
          ObjData *obj;

          obj = read_object(item.item_number, VIRTUAL);
          if (!obj)
          {
              printf ("Unknown object #%d in file %s!\n", item.item_number, file);
          }
          else
          {
              printf ("  Found item #%d, ", item.item_number);
              cprint (obj->short_description);
              printf (" in %s\n", file);

              // If an item is randomized, we cannot reset its stats:
              if (IS_SET_AR(item.extra_flags, ITEM_RANDOMIZED)) {
                  printf("Item #%d is ranomized, so not updated in %s!", item.item_number, file);
                  extract_obj(obj);
                  continue;
              }

              /* If a CURSED flag is found, it will be kept. */
              if (IS_SET_AR(item.extra_flags, ITEM_CURSED))
                  SET_BIT_AR(obj->obj_flags.extra_flags, ITEM_CURSED);
              else
                  REMOVE_BIT_AR (obj->obj_flags.extra_flags, ITEM_CURSED);

              // If an item has been identified, it stays identified:
              if (IS_SET_AR (item.extra_flags, ITEM_IDENTIFIED))
                  SET_BIT_AR (obj->obj_flags.extra_flags, ITEM_IDENTIFIED);

              // If an item is not rentable, it stays unrentable
              if (IS_SET_AR (item.extra_flags, ITEM_NORENT))
                  SET_BIT_AR (obj->obj_flags.extra_flags, ITEM_NORENT);

              // If an item was timed, it stays timed
              if (IS_SET_AR (item.extra_flags, ITEM_TIMED))
                  SET_BIT_AR (obj->obj_flags.extra_flags, ITEM_TIMED);

              /* Keep any renaming conventions that are found */
              obj->rename_slot = item.rename_slot;
              
              fseek (f, pos, SEEK_SET);
              Obj_to_store (obj, f);
              
              extract_obj (obj);
          }
      }
  }
  fclose (f);
}

void
update_stored_eq (int *vnum)
{
    glob_t files;
    int i;

    initialize_global_variables ();
    
    index_boot(2);

    if (glob ("plrobjs/*/*.objs", 0, NULL, &files) == 0)
    {
        for (i = 0; i < files.gl_pathc; i++)
        {
            update_file (files.gl_pathv[i], vnum, sizeof (RentInfo));
        }
        globfree (&files);
    }
    else
    {
        printf ("Can't find rent files!\n");
    }
    if (glob ("lockers/*.objs", 0, NULL, &files) == 0)
    {
        for (i = 0; i < files.gl_pathc; i++)
        {
            update_file (files.gl_pathv[i], vnum, 0);
        }
        globfree (&files);
    }
    else
    {
        printf ("Can't find locker files!\n");
    }
}

void
usage (void)
{
    printf ("Usage: equpdate <vnum> [<vnum> ...]\n\n");
    printf ("Where <vnum> is the vnum of the object to update, or -1 for all ");
    printf ("objects\n\nWARNING: The MUD must NOT be running when you execute");
    printf (" this command!\n\n");
    exit (0);
}

int
main (int argc, char **argv)
{
    char *endptr;
    int *vnum;
    int i;
    
    if (argc < 2) usage ();
    vnum = malloc (sizeof (int) * argc);
    vnum[argc - 1] = -1;
    for (i = 1; i < argc; i++)
    {
        vnum[i - 1] = strtol (argv[i], &endptr, 0);
        if (*endptr != '\0') usage ();
        printf ("Refreshing #%d\n", vnum[i - 1]);
    }
    update_stored_eq (vnum);
    return (0);
}

