
#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/class.h"
#include "util/utils.h"

typedef struct plr_save_file {
    char *name;
    RentInfo ri;
    int obj_count;
    ObjFileElem *objects;
} PlrSaveFile;

PlrSaveFile *read_plrobj(char *file)
{
    FILE *f = fopen(file, "rb");
    PlrSaveFile *plrobj;
    char *head, *tail;
    long size;
    int i;

    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    if (size < sizeof(RentInfo)) {
	fclose(f);
	return NULL;
    }
    size -= sizeof(RentInfo);
    if (size % sizeof(ObjFileElem) != 0) {
	fclose(f);
	return NULL;
    }
    if ((plrobj = malloc(sizeof(PlrSaveFile))) == NULL) {
	fclose(f);
	return NULL;
    }
    head = strrchr(file, '/');
    if (!head) head = file; else head++;
    tail = strrchr(file, '.');
    plrobj->name = strncpy(malloc(tail - head + 1), head, tail - head);
    plrobj->name[tail - head] = '\0';
    plrobj->name[0] = toupper(plrobj->name[0]);
    plrobj->obj_count = size / sizeof(ObjFileElem);
    plrobj->objects = calloc(plrobj->obj_count, sizeof(ObjFileElem));
    if (!plrobj->objects) {
	fclose(f);
	free(plrobj);
	return NULL;
    }
    fseek(f, 0, SEEK_SET);
    fread(&plrobj->ri, sizeof(RentInfo), 1, f);
    for (i = 0; i < plrobj->obj_count; i++)
	fread(&plrobj->objects[i], sizeof(ObjFileElem), 1, f);
    fclose(f);
    return plrobj;
}

void write_plrobj(char *file, PlrSaveFile *plrobj)
{
    FILE *f = fopen(file, "wb");
    long size;
    int i;

    if (!f) return;
    fwrite(&plrobj->ri, sizeof(RentInfo), 1, f);
    for (i = 0; i < plrobj->obj_count; i++)
	fwrite(&plrobj->objects[i], sizeof(ObjFileElem), 1, f);
    fclose(f);
}

void cprint(char *string)
{
    char *colortable[] = {
	"[0m", "[31m", "[32m", "[33m", "[34m", "[35m", "[36m",
        "[37m", "[1;31m", "[1;32m", "[1;33m", "[1;34m", "[1;35m",
	"[1;36m", "[1;37m", "[41m", "[42m", "[43m", "[44m",
	"[45m", "[46m", "[47m", "[41;1;33m", "[40m", "[30m",
	"[5m",
    };

    while (*string) {
	if (*string == '&') {
	    long l;
	    char *end;

	    l = -1;
	    l = strtol(string+1, &end, 10);
	    string = end;
	    if (l >= 0 && l < sizeof(colortable) / sizeof(char *)) {
		printf(colortable[l]);
	    }
	} else {
	    putchar(*string);
	    string++;
	}
    }
}

int VerifyObject(ObjFileElem *ect, ObjData *obj)
{
  int changed = 0;
  int i;

  /* these are the flags we know will change */
  int ignore_flags[] = {
    ITEM_CURSED, ITEM_MAGIC, ITEM_NOSELL, ITEM_EXPLODES, ITEM_TIMED, -1
  };

  if (!obj) return -1;

  /* first, check the object weight */
  //if (ect->weight != obj->obj_flags.weight)
    //changed = 1;

  /* remove the bits we're not interested in */
  for (i = 0; ignore_flags[i] != -1; i++) {
    REMOVE_BIT_AR(obj->obj_flags.extra_flags, ignore_flags[i]);
    if (IS_SET_AR(ect->extra_flags, ignore_flags[i]))
      SET_BIT_AR(obj->obj_flags.extra_flags, ignore_flags[i]);
  }

  /* see if every flag is now the same */
  for (i = 0; i < EF_ARRAY_MAX; i++)
    if (obj->obj_flags.extra_flags[i] != ect->extra_flags[i])
      changed = 1;

  return changed;
}

void CheckEqStats(PlrSaveFile *plr)
{
    int i;

    printf("Player %s\n", plr->name);
    for (i = 0; i < plr->obj_count; i++) {
        ObjData *obj = read_object( plr->objects[i].item_number, VIRTUAL );
	if (!obj) {
	    printf("  Unknown object #%d\n", plr->objects[i].item_number);
	} else if (VerifyObject(plr->objects + i, obj)) {
            printf("  Changed object #%d, ", plr->objects[i].item_number);
            cprint(obj->short_description);
            printf("\n");
	}
    }
}

void usage(void)
{
    printf("Usage: objsearch <action>\n\n");
    printf("Where <action> is one of:\n");
    printf("  o[bjects] <vnum>\n");
    printf("    Show players with a given object in rentfiles\n");
    printf("  e[q] <player>\n");
    printf("    Show equipment in a player's rentfile\n");
    printf("  c[hanged]\n");
    printf("    Show eq that has updated stats\n");
    printf("  p[ermaffects]\n");
    printf("    Show eq with perm affects\n");
    printf("  f[orgables]\n");
    printf("    Show a list of forgable weapons\n");
    printf("  s[houlin]\n");
    printf("    Show a list of shou-lin usable ARMOR\n");
    printf("  m[agely]\n");
    printf("    Show a list of Mage usable items\n");
    exit(3);
}

void blat_blat(void)
{
    glob_t files;
    int i;

    printf("Removing all non-zero rename slots from rentfiles...\n");

    if (glob("plrobjs/*/*.objs", 0, NULL, &files) != 0) {
	printf("Can't find rent files!\n");
        return;
    } else for (i = 0; i < files.gl_pathc; i++) {
	PlrSaveFile *plr = read_plrobj(files.gl_pathv[i]);

	if (!plr) {
	    printf("Can't read file %s\n", files.gl_pathv[i]);
	} else {
	    int j, k = 0;

	    for (j = 0; j < plr->obj_count; j++) {
		if (plr->objects[j].rename_slot != 0) k++;
                plr->objects[j].rename_slot = 0;
	    }
            printf("Player %s: %d changes\n", plr->name, k);
            if (k) write_plrobj(files.gl_pathv[i], plr);
	}
    }
    globfree(&files);

    printf("Removing all non-zero rename slots from lockers...\n");

    if (glob("lockers/*.objs", 0, NULL, &files) != 0) {
	printf("Can't find locker files!\n");
        return;
    } else for (i = 0; i < files.gl_pathc; i++) {
	FILE *f = fopen(files.gl_pathv[i], "r+b");
        ObjFileElem object;
        int k = 0;

        while (fread(&object, sizeof(ObjFileElem), 1, f) == 1) {
            if (object.rename_slot != 0) {
                object.rename_slot = 0; k++;
                fseek(f, -sizeof(ObjFileElem), SEEK_CUR);
                fwrite(&object, sizeof(ObjFileElem), 1, f);
            }
        }
        fclose(f);
        printf("Locker %s, %d changes\n", files.gl_pathv[i], k);
    }
    globfree(&files);
}

void object_search(int vnum)
{
    glob_t playerfiles;
    int i;

    if (glob("plrobjs/*/*.objs", 0, NULL, &playerfiles) != 0) {
	printf("Can't find rent files!\n");
	exit(3);
    }
    for (i = 0; i < playerfiles.gl_pathc; i++) {
	PlrSaveFile *plr = read_plrobj(playerfiles.gl_pathv[i]);

	if (!plr) {
	    printf("Can't read file %s\n", playerfiles.gl_pathv[i]);
	} else {
	    int j;

	    for (j = 0; j < plr->obj_count; j++) {
		if (plr->objects[j].item_number == vnum)
		    printf("Object #%d found in rentfile of player %s\n",
			    vnum, plr->name);
	    }
	}
    }
    globfree(&playerfiles);
}

void equip_list(char *player)
{
    glob_t playerfiles;
    char buf[200];
    int i;

    initialize_global_variables();
    printf("Reading object files\n");
    index_boot(2);      // hack!

    sprintf(buf, "plrobjs/*/%s*.objs", player);
    for (i = 0; buf[i]; i++) buf[i] = tolower(buf[i]);

    if (glob(buf, 0, NULL, &playerfiles) != 0) {
	printf("No players by that name found\n");
	exit(3);
    }
    for (i = 0; i < playerfiles.gl_pathc; i++) {
	PlrSaveFile *plr = read_plrobj(playerfiles.gl_pathv[i]);

	if (!plr) {
	    printf("Can't read file %s\n", playerfiles.gl_pathv[i]);
	} else {
	    int j;

	    printf("Player %s:\n", plr->name);
	    for (j = 0; j < plr->obj_count; j++) {
		ObjData *obj = read_object(plr->objects[j].item_number,
                    VIRTUAL);

		printf("  Object #%d, ", plr->objects[j].item_number);
		if (obj) cprint(obj->short_description);
		else printf("<unknown>");
		printf("\n");
	    }
	}
    }
    globfree(&playerfiles);
}

void changed_eq(void)
{
    glob_t playerfiles;
    int i;

    initialize_global_variables();
    printf("Reading object files\n");
    index_boot(2);      // hack!

    if (glob("plrobjs/*/*.objs", 0, NULL, &playerfiles) != 0) {
	printf("Can't find rent files!\n");
	exit(3);
    }
    for (i = 0; i < playerfiles.gl_pathc; i++) {
	PlrSaveFile *plr = read_plrobj(playerfiles.gl_pathv[i]);

	if (!plr) {
	    printf("Can't read file %s\n", playerfiles.gl_pathv[i]);
	} else {
            CheckEqStats(plr);
	}
    }
    globfree(&playerfiles);
}

void list_permitems(void)
{
  int i;

  initialize_global_variables();
  printf("Reading object files\n");
  index_boot(2);      // hack!

  for (i = 0; i < top_of_objt; i++) {
    ObjData *obj = obj_proto+i;
    if (obj->obj_flags.bitvector[0] || obj->obj_flags.bitvector[1]) {
      char t[300];
      sprintbitarray(obj->obj_flags.bitvector, affected_bits, AF_ARRAY_MAX, t);
      printf("#%d: %s: %s\n", obj_index[i].virtual, obj->short_description, t);
    }
  }
}

void list_forgable(void)
{
  int i;

  initialize_global_variables();
  printf("Reading object files\n");
  index_boot(2);      // hack!

  for (i = 0; i < top_of_objt; i++) {
    ObjData *obj = obj_proto+i;
    if (GET_OBJ_TYPE(obj) == ITEM_WEAPON && !IS_OBJ_STAT(obj, ITEM_MAGIC)) {
      int n,s;

      printf("#%d: %s, ", obj_index[i].virtual, obj->short_description);
      n = GET_OBJ_VAL(obj, 1); s = GET_OBJ_VAL(obj, 2);
      printf("%dd%d -> %dd%d, avg %.1f max %d\n", n, s, n+2, s+1,
          (n+2) * (s + 2) / 2.0, (n+2) * (s+1));
    }
  }
}

int find_damroll(ObjData *o)
{
  int i, dr = 0;

  for (i = 0; i < MAX_OBJ_AFFECT; i++)
    if (o->affected[i].location == APPLY_DAMROLL)
      dr += o->affected[i].modifier;
  return dr;
}

int find_hitroll(ObjData *o)
{
  int i, dr = 0;

  for (i = 0; i < MAX_OBJ_AFFECT; i++)
    if (o->affected[i].location == APPLY_HITROLL)
      dr += o->affected[i].modifier;
  return dr;
}

int find_hitpoints(ObjData *o)
{
  int i, dr = 0;

  for (i = 0; i < MAX_OBJ_AFFECT; i++)
    if (o->affected[i].location == APPLY_HIT)
      dr += o->affected[i].modifier;
  return dr;
}

void list_magely(void)
{
  int i, hp;
  char buf[200];

  initialize_global_variables();
  printf("Reading object files\n");
  index_boot(2);      // hack!

  for (i = 0; i < top_of_objt; i++) {
    ObjData *obj = obj_proto+i;
    if (IS_OBJ_STAT(obj, ITEM_ANTI_MAGIC_USER) && !IS_OBJ_STAT(obj, ITEM_ANTI_NECROMANCER)) {
      printf("#%d: %ddr %dhr %s\n", obj_index[i].virtual, find_damroll(obj),
          find_hitroll(obj), obj->short_description);
    }
  }
}

void list_shoulin(void)
{
  int i, hp;
  char buf[200];

  initialize_global_variables();
  printf("Reading object files\n");
  index_boot(2);      // hack!

  for (i = 0; i < top_of_objt; i++) {
    ObjData *obj = obj_proto+i;
    if ((!IS_OBJ_STAT(obj, ITEM_ANTI_SHOU_LIN) &&
         !IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL))
     && CAN_WEAR(obj, ITEM_WEAR_HEAD)) {
      printf("#%d: %ddr %dhr %s\n", obj_index[i].virtual, find_damroll(obj),
          find_hitroll(obj), obj->short_description);
    }
  }
}

void list_remortchange(void)
{
  int i, hp;
  char buf[200];

  initialize_global_variables();
  printf("Reading object files\n");
  index_boot(2);      // hack!

  for (i = 0; i < top_of_objt; i++) {
    ObjData *obj = obj_proto+i;
    if ((IS_OBJ_STAT(obj, ITEM_ANTI_GNOME) ||
         IS_OBJ_STAT(obj, ITEM_ANTI_DWARF))) {
      printf("%s (%d) should be !FAERIE\n", obj->short_description,
              obj_index[i].virtual);
    } else if (IS_OBJ_STAT(obj, ITEM_ANTI_TROLL)) {
      printf("%s (%d) should be !GIANT\n", obj->short_description,
              obj_index[i].virtual);
    }
  }
}

int main(int argc, char **argv)
{
    char *endptr;
    int vnum;

    if (argc < 2 || argc > 3) usage();
    switch (*argv[1]) {
        case 'b':
            blat_blat();
            break;
	case 'o':
	    if (argc != 3) usage();
	    vnum = strtol(argv[2], &endptr, 0);
	    if (*endptr != '\0') usage();
	    object_search(vnum);
	    break;
	case 'e':
	    if (argc != 3) usage();
	    equip_list(argv[2]);
	    break;
	case 'c':
	    changed_eq();
	    break;
        case 'p':
            list_permitems();
            break;
        case 'f':
            list_forgable();
            break;
        case 'l':
            list_shoulin();
            break;
        case 'm':
            list_magely();
            break;
        case 'r':
            list_remortchange();
            break;
	default:
	    usage();
    }
    return 0;
}

