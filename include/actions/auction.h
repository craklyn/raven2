/* ============================================================================
Header file for auction.c
Written by Xiuhtecuhtli of RavenMUD for RavenMUD.
============================================================================ */
#ifndef _AUCTION_H_
#define _AUCTION_H_

/* ============================================================================
 Structures
============================================================================ */

/* Make up our auction data type and the 'auction' variable */
struct auction_data {
  long seller;
  long bidder;
  struct obj_data *obj;
  long bid;
  int ticks;
} auction;

/* ============================================================================
Global functions.
============================================================================ */

void auction_reset(void);
void auction_update(void);

/* ============================================================================
Constants.
============================================================================ */
/* #define's for 'ticks' variable */
#define AUC_NONE       -1
#define AUC_NEW         0
#define AUC_BID         1
#define AUC_ONCE        2
#define AUC_TWICE       3
#define AUC_SOLD        4

#endif /* _AUCTION_H_*/
