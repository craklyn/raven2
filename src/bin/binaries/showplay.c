/* ************************************************************************
*  file:  showplay.c                                  Part of CircleMud   *
*  Usage: list a diku playerfile                                          *
*  Copyright (C) 1990, 1991 - see 'license.doc' for complete information. *
*  All Rights Reserved                                                    *
************************************************************************* */

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"


void	show(char *filename)
{
   char	sexname;
   char	classname[10];
   FILE * fl;
   struct char_file_u player;
   int	num = 0;

   if (!(fl = fopen(filename, "r+"))) {
      perror("error opening playerfile");
      exit();
   }

   for (; ; ) {
      fread(&player, sizeof(struct char_file_u ), 1, fl);
      if (feof(fl)) {
	 fclose(fl);
	 exit();
      }

      switch (player.class) {
      case CLASS_THIEF	: 
	 strcpy(classname, "Th"); 
	 break;
      case CLASS_WARRIOR	: 
	 strcpy(classname, "Wa"); 
	 break;
      case CLASS_MAGIC_USER	: 
	 strcpy(classname, "Mu"); 
	 break;
      case CLASS_CLERIC	: 
	 strcpy(classname, "Cl"); 
	 break;
      default : 
	 strcpy(classname, "--"); 
	 break;
      }

      switch (player.sex) {
      case SEX_FEMALE	: 
	 sexname = 'F'; 
	 break;
      case SEX_MALE	: 
	 sexname = 'M'; 
	 break;
      case SEX_NEUTRAL: 
	 sexname = 'N'; 
	 break;
      default : 
	 sexname = '-'; 
	 break;
      }

      printf("%5d. ID: %5d (%c) [%2d %s] %-16s %9dg %9db\n", ++num,
          player.specials2.idnum, sexname, player.level,
          classname, player.name, player.points.gold,
          player.points.bank_gold);
   }
}


main(int argc, char **argv)
{
   if (argc != 2)
      printf("Usage: %s playerfile-name\n", argv[0]);
   else
      show(argv[1]);
}


