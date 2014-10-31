/* color.c */

#include "general/conf.h"
#include "general/sysdep.h"

#include "general/db.h"
#include "general/structs.h"
#include "general/class.h"
#include "util/utils.h"
#include "general/color.h"

// Vex - extended our list of color codes.
#define CNRM  "\x1B[0;0m"		// 00
#define CRED  "\x1B[31m"		// 01
#define CGRN  "\x1B[32m"		// 02
#define CYEL  "\x1B[33m"		// 03
#define CBLU  "\x1B[34m"		// 04
#define CMAG  "\x1B[35m"		// 05
#define CCYN  "\x1B[36m"		// 06
#define CNUL  ""

#define BBLK  "\x1B[1;30m"		// 07
#define BRED  "\x1B[1;31m"		// 08
#define BGRN  "\x1B[1;32m"		// 09
#define BYEL  "\x1B[1;33m"		// 10
#define BBLU  "\x1B[1;34m"		// 11
#define BMAG  "\x1B[1;35m"		// 12
#define BCYN  "\x1B[1;36m"		// 13
#define BWHT  "\x1B[1;37m"		// 14

#define BKRED  "\x1B[41m"		// 15
#define BKGRN  "\x1B[42m"		// 16
#define BKYEL  "\x1B[43m"		// 17
#define BKBLU  "\x1B[44m"		// 18
#define BKMAG  "\x1B[45m"		// 19
#define BKCYN  "\x1B[46m"		// 20
#define BKWHT  "\x1B[47m"		// 21

#define BKRBY  "\x1B[41m\x1B[1;33m"	// 22

#define BKBLK  "\x1B[40m" 		// 23
#define CBLK  "\x1B[30m"		// 24
#define CWHT  "\x1B[37m"
#define KBLN  "\x1B[5m"			// 25
#define KBLD  "\x1B[1m"
#define KCLR  "\x1B[2J"
#define KUND  "\x1B[4m"
#define KDAR  "\x1B[2m"
 

#define BEEP            "\07"
#define INVERSE         "\x1B[7m"
#define FLASH           "\x1B[5m"
#define RETURN          "\r\n" 
#define SPACE           " "
#define TAB             "\t"

#define INV_FLASH               "\x1B[7;5m"
#define INV_HILIGHT             "\x1B[1;7m"
#define FLASH_HILIGHT           "\x1B[1;5m"
#define INV_FLASH_HILIGHT       "\x1B[1;5;7m"
 
#define INITSEQ         "\x1B[1;24r"
#define CURSPOS         "\x1B[%d;%dH"
#define CURSRIG         "\x1B[%dC"
#define CURSLEF         "\x1B[%dD"
#define HOMECLR         "\x1B[2J\x1B[0;0H"
#define CTEOTCR         "\x1B[K"
#define CLENSEQ         "\x1B[r\x1B[2J"
#define INDUPSC         "\x1BM"
#define INDDOSC         "\x1BD"
#define SETSCRL         "\x1B[%d;24r"
#define INVERTT         "\x1B[0;1;7m"   
#define BOLDTEX         "\x1B[0;1m"
#define NORMALT         "\x1B[0m"
#define MARGSET         "\x1B[%d;%dr"
#define CURSAVE         "\x1B7"
#define CURREST         "\x1B8"

const char *COLORLIST[] = { CNRM, CRED, CGRN,   /* 00 - 02 */
                            CYEL, CBLU, CMAG,   /* 03 - 05 */
                            CCYN, BBLK, BRED,   /* 06 - 08 */
                            BGRN, BYEL, BBLU,   /* 09 - 11 */
                            BMAG, BCYN, BWHT,   /* 12 - 14 */
			    BKRED,BKGRN,BKYEL,  /* 15 - 17 */
                            BKBLU,BKMAG,BKCYN,  /* 18 - 20 */
                            BKWHT,BKRBY,BKBLK,  /* 21 - 23 */
                            CBLK, KBLN  	/* 24 - 26 */
			   };
#define MAX_COLORS 25
//					BKRED  BKGRN  BKYEL
const char *IMM_ONLY_COLOR_CODE[] = { 	"&15", "&16", "&17",
//					BKBLU  BKMAG  BKCYN
					"&18", "&19", "&20",
//					BKWHT  BKRBY  BKBLK
					"&21", "&22", "&23",
//					CBLK   KBLN
					"&24", "&25"
				     };
#define NUM_IMM_ONLY_COLOR_CODES 11

int mortal_color(char *inbuf, CharData *ch)
{
    int i;

    if (GET_LEVEL(ch) >= LVL_IMMORT)
	return 1;

    for (i = 0; i < NUM_IMM_ONLY_COLOR_CODES; i++)
	if (strstr(inbuf, IMM_ONLY_COLOR_CODE[i])) {
	    sendChar(ch, "You are not authorised to use %scolor%s!", IMM_ONLY_COLOR_CODE[i], CNRM);
	    return 0;
	}
    return 1;
}

int immortal_color(char *inbuf, CharData *ch)
{
	int i;

	for (i = 0; i < NUM_IMM_ONLY_COLOR_CODES; i++)
		if (strstr(inbuf, IMM_ONLY_COLOR_CODE[i])) 
		{
			sendChar(ch, "%sColor%s is disabled to IMMs in this channel for the time being.", IMM_ONLY_COLOR_CODE[i], CNRM);
			return 0;
		}
		return 1;
}

/*
** Let's statically define the output buffer to give the heap a rest.
** This will be veeeeerrryyyyy broken if we multithread.
*/
static char outbuf[32768];

/*
** clr is a new function that was written to replace the 
** old clr macro. The macro didn't have any checks for null
** pointers in the character records. This new function will
** hopefully avoud the segmentation violations.
*/
int
clr( CharData *ch,
     int colorLevel )
{
    if( ch == NULL || ch->player_specials == NULL ){
        return  0;
    }
    return( _clrlevel(ch) >= colorLevel );
}

/*
** isColor will look at the first to characters and if both are
** digits the it will convert them to their integer value and
** return. A -1 will be returned upon failure.
*/
int isColor( char s[] )
{
    int cCode = -1;

    if( isdigit(s[0]) && isdigit(s[1]) ){
        cCode = (s[0]-'0')*10 + (s[1]-'0');
    }
    return( cCode );
}

/*
** The colorRatio is a canned function that will return an ANSI
** color string based on the resultant percentage of num/den. This
** is the function that was written to support hit/max, mana/max
** and move/max color prompt strings but may be used for anything
** needing this sort of relationship.
*/
char *
colorRatio( CharData *ch, int cookIt, int clvl, int num, int den )
{
    static char *colorRawANSI[] = {
        BKRBY,  /* 00 - 09 : BKRBY */
        BKRBY,  /* 10 - 19 : BKRBY */
        BRED,   /* 20 - 29 : BRED  */
        BYEL,   /* 30 - 39 : BYEL  */
        BYEL,   /* 40 - 49 : BYEL  */
        BYEL,   /* 50 - 59 : BYEL  */
        BCYN,   /* 60 - 69 : BCYN  */
        BCYN,   /* 70 - 79 : BCYN  */
        BGRN,   /* 80 - 89 : BGRN  */
        BGRN    /* 90 - 99 : BGRN  */
    };

    static char *colorCooked[] = {
        "&22",  /* 00 - 09 : BKRBY */
        "&22",  /* 10 - 19 : BKRBY */
        "&08",   /* 20 - 29 : BRED  */
        "&10",   /* 30 - 39 : BYEL  */
        "&10",   /* 40 - 49 : BYEL  */
        "&10",   /* 50 - 59 : BYEL  */
        "&13",   /* 60 - 69 : BCYN  */
        "&13",   /* 70 - 79 : BCYN  */
        "&09",   /* 80 - 89 : BGRN  */
        "&09"    /* 90 - 99 : BGRN  */
    };
#   define NUM_ENTRIES (sizeof(colorCooked) / sizeof(colorCooked[0]))
#   define LAST_ENTRY  (NUM_ENTRIES-1)
    /*
    ** Calculate the percentage and then the index
    ** based on the percentage value.
    */
    int pcnt = (int)(100.0 * (float)num/(float)den);
    int cidx = pcnt / NUM_ENTRIES;
    /*
    ** Bail out here if we haven't reached the user's
    ** current color level.
    */
    if( !clr( ch, clvl ) ){ return( KNRM ); }

    /*
    ** Avoid any underflow problems.
    */
    cidx = (cidx < 0 ? 0 : cidx);

    /*
    ** Avoid any overflow problems.
    */
    cidx = (cidx > LAST_ENTRY ? LAST_ENTRY : cidx );

    /*
    ** Finally, return a pointer to the static color string.
    */
    return((cookIt == COLOR_COOKED ? colorCooked[cidx] : colorRawANSI[cidx] ));
}

void procColor(char *inbuf, int color)
{
    register int j=0,p=0;
    int c,k;

    if(inbuf[0] == '\0') return;

    while( inbuf[j] != '\0' ){
        if(( inbuf[j] == '&' ) && (c = isColor( &inbuf[j+1] )) >= 0){

            if( c > MAX_COLORS ) c = 0;
            j += 3;
            if( color )
                for( k = 0; k < strlen( COLORLIST[c]); k++ ){
                    outbuf[p++] = COLORLIST[c][k];
                }
        }
        else {
            outbuf[p++] = inbuf[j++];
        }
    }

    outbuf[p] = '\0';
//mudlog(NRM, LVL_IMMORT, TRUE, "inbuf %d outbuf %d", strlen(inbuf), strlen(outbuf));
    strcpy(inbuf, outbuf);
}

