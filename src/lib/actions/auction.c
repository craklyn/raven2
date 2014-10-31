
#include "general/conf.h"
#include "general/sysdep.h"
 
#include "general/db.h"
#include "general/structs.h"
#include "util/utils.h"
#include "general/comm.h"
#include "actions/interpreter.h"
#include "general/handler.h"
#include "general/color.h"
#include "actions/auction.h"
#include "general/class.h"
#include "olc/oedit.h"            /* For oPrintValues */
#include "specials/combspec.h"    /* For combSpecName */
#include "actions/act.h"



/* Local function */
static struct char_data *get_ch_by_id(long idnum);
static void   auction_output(char *black);

/*local global variables */
static int find_seller_mob(int vnum);

/*
 * auction_output : takes two strings and dispenses them to everyone connected
 *             based on if they have color on or not.  Note that the buf's are
 *             commonly used *color and *black so I allocate my own buffer.
 */
static void auction_output(char *black)
{
  struct descriptor_data *d;

  for( d = descriptor_list; d; d = d->next )
  {
    if( !d->connected && d->character &&
        !PLR_FLAGGED(d->character, PLR_WRITING) &&
        !PRF_FLAGGED(d->character, PRF_NOAUCT) &&
        !ROOM_FLAGGED(d->character->in_room, ROOM_SOUNDPROOF))
    {
      sendChar( d->character, "[&05 AUCTION:&00 %s ]\r\n", black );
    }
  }
}


void
auction_update()
{
  char auction_buf[255];

  if( auction.ticks == AUC_NONE ) return;  

  // Seller left
  //
  if( !get_ch_by_id( auction.seller ))
  {
    if( auction.obj ) extract_obj(auction.obj);
    auction_reset();
    return;
  }

  // If there is an auction but it's not sold yet
  //
  if( auction.ticks >= AUC_BID && auction.ticks <= AUC_SOLD )
  {
    // If there is a bidder and it's not sold yet
    //
    if( get_ch_by_id( auction.bidder ) && ( auction.ticks < AUC_SOLD ))
    {
      sprintf( auction_buf, "%s is going %s%s%s for %ld coin%s",
               auction.obj->short_description,
               auction.ticks == AUC_BID ? "once" : "",
               auction.ticks == AUC_ONCE ? "twice" : "",
               auction.ticks == AUC_TWICE ? "for the last call" : "",
               auction.bid, auction.bid != 1 ? "s" : " ");
      auction_output(auction_buf);
      auction.ticks++;
      return;
    }

    // If there is no bidder and we ARE in the sold state
    //
    if( !get_ch_by_id( auction.bidder ) && ( auction.ticks == AUC_SOLD ))
    {
      sprintf( auction_buf, "%s has failed to be sold for %ld coin%s",
               auction.obj->short_description,
               auction.bid,
               auction.bid != 1 ? "s" : " ");
      auction_output(auction_buf);
      /* Give the poor fellow his unsold goods back */
      obj_to_char(auction.obj, get_ch_by_id(auction.seller));
      /* Reset the auction for next time */
      auction_reset();
      return;
    }

    // If there is no bidder and we are not in the sold state
    //
    if (!get_ch_by_id(auction.bidder) && (auction.ticks < AUC_SOLD))
    {
      sprintf(auction_buf, "%s is going %s%s%s",
              auction.obj->short_description,
              auction.ticks == AUC_BID ? "once" : "",
              auction.ticks == AUC_ONCE ? "twice" : "",
              auction.ticks == AUC_TWICE ? "for the last call" : "");
      auction_output(auction_buf);
      auction.ticks++;
      return;
    }

    // Sold
    //
    if( get_ch_by_id( auction.bidder ) && ( auction.ticks >= AUC_SOLD ))
    {
      /* Get pointers to the bidder and seller */
      CharData* seller = get_ch_by_id(auction.seller);
      CharData* bidder = get_ch_by_id(auction.bidder);

      sprintf( auction_buf, "%s is SOLD for %ld coin%s",
               auction.obj->short_description ?
                 auction.obj->short_description : "something",
               auction.bid, auction.bid != 1 ? "s" : "");
      auction_output(auction_buf);
  
      // If the seller is still around we give him the money
      //
      if( seller )
      {
        GET_GOLD(seller) += (auction.bid);
        act( "Congrats! You have sold $p!", FALSE,
                                            seller, auction.obj, 0, TO_CHAR);
      }
      // If the bidder is here he gets the object
      //
      if( bidder )
      {
        obj_to_char(auction.obj, bidder);
        act( "Congrats! You now have $p!", FALSE,
                                           bidder, auction.obj, 0, TO_CHAR);
      }

      auction_reset();
      return;
    }
  }
  return;
}

/*
 * do_bid : user interface to place a bid.
 */
ACMD(do_bid)
{
  char auction_buf[255];
  long bid;

  /* NPC's can not bid or auction if charmed */
  if (ch->master && AFF_FLAGGED(ch, AFF_CHARM))
    return;

  if (PRF_FLAGGED(ch, PRF_NOAUCT)) {
      send_to_char("You are not even on the channel!\r\n", ch);
      return;
  }

  if (PLR_FLAGGED(ch, PLR_HUNTED) || PLR_FLAGGED(ch, PLR_THIEF)) {
      send_to_char("You are to busy hiding to bid on anything.\r\n", ch);
      return;
  }

  /* There isn't an auction */
  if (auction.ticks == AUC_NONE) {
    send_to_char("Nothing is up for sale.\r\n", ch);
    return;
  }

  one_argument(argument, buf);
  bid = atoi(buf);

  /* They didn't type anything else */
  if (!*buf) {
    sprintf(buf2, "Current bid: %ld coin%s\r\n", auction.bid,
        auction.bid != 1 ? "s." : ".");
    send_to_char(buf2, ch);
  /* The current bidder is this person */
  } else if (ch == get_ch_by_id(auction.bidder)) {
    send_to_char("You're trying to outbid yourself.\r\n", ch);
  /* The seller is the person who tried to bid if so stop auction*/
  } else if (ch == get_ch_by_id(auction.seller))
    send_to_char("You are unable to bid on your own item.\r\n", ch);
  /* Tried to auction below the minimum */
  else if ((bid < auction.bid) && !get_ch_by_id(auction.bidder)) {
    sprintf(buf2, "The minimum is currently %ld coins.\r\n", auction.bid);
    send_to_char(buf2, ch);
  /* Tried to bid below the minimum where there is a bid, 10% increases */
  } else if ((bid < (auction.bid * 1.10) && get_ch_by_id(auction.bidder)) || bid == 0) {
    sprintf(buf2, "Try bidding at least 10%% over the current bid of %ld. (%.0f coins).\r\n",
        auction.bid, auction.bid * 1.10 + 1);
    send_to_char(buf2, ch);
  /* Not enough gold on hand! */
  } else if (GET_GOLD(ch) < bid) {
    sprintf(buf2, "You have only %d coins on hand.\r\n", GET_GOLD(ch));
    send_to_char(buf2, ch);
  /* it's an ok bid */
  } else {
    /* Give last bidder money back if he's around! */
    if (get_ch_by_id(auction.bidder))
      GET_GOLD(get_ch_by_id(auction.bidder)) += auction.bid;
    /* This is the bid value */
    auction.bid = bid;
    /* The bidder is this guy */
    auction.bidder = GET_IDNUM(ch);
    /* This resets the auction to first chance bid */
    auction.ticks = AUC_BID;
    /* Get money from new bidder. */
    GET_GOLD(get_ch_by_id(auction.bidder)) -= auction.bid;
    sprintf(auction_buf, "A bid of %ld coin%s is heard on %s",
       auction.bid,
       auction.bid!=1 ? "s" :"",
       auction.obj->short_description);
    mudlog(CMP, LVL_ANGEL, TRUE, "AUCTION: %s has bid %ld coin(s) for %s",
            GET_NAME(ch), auction.bid, auction.obj->short_description);
    auction_output(auction_buf);
  }
}

/*
 * do_auction : user interface for placing an item up for sale
 */
ACMD(do_auction)
{
  struct obj_data *obj;
  struct char_data *seller;
  char auction_buf[255];

  /* NPC's can not bid or auction if charmed */
  if (ch->master && AFF_FLAGGED(ch, AFF_CHARM))
    return;

  if(oracle_counter >= 1) {
      send_to_char("You shouldn't auction now.  A reboot approaches.\r\n", ch);
      return;
  }

  if (PRF_FLAGGED(ch, PRF_NOAUCT)) {
      send_to_char("You are not even on the channel!\r\n", ch);
      return;
  }

  if (PLR_FLAGGED(ch, PLR_HUNTED) || PLR_FLAGGED(ch, PLR_THIEF)) {
      send_to_char("You are to busy hiding to auction anything.\r\n", ch);
      return;
  }

  two_arguments(argument, buf1, buf2);

  seller = get_ch_by_id(auction.seller);

  /* There is nothing they typed */
  if (!*buf1)
    send_to_char("Auction what for what minimum?\r\n", ch);
  /* Hrm...logic error? */
  else if (strcmp(buf1, "-info") == 0) 
           do_auction_stat(ch, 0, 0, 0);
  else if (strcmp(buf1, "-kill") == 0) {
           if (auction.ticks == AUC_NONE) {
               send_to_char("Nothing is up for sale.\r\n", ch);
               return;
           }

           if (GET_LEVEL(ch) >= 54) {
              sprintf(auction_buf, "This auction has been halted by the gods");
              obj_to_char(auction.obj, get_ch_by_id(auction.seller));
              if (auction.bidder != -1)
	          GET_GOLD(get_ch_by_id(auction.bidder)) += auction.bid;
	      auction_reset();
	      auction_output(auction_buf);
	      mudlog(NRM, LVL_DEITY, TRUE, " %s stopped the auction", GET_NAME(ch));
           } else
               send_to_char("Huh?\r\n", ch);
  } else if (auction.ticks != AUC_NONE) {
    /* If seller is no longer present, auction continues with no seller */
    if (seller) {
      sprintf(buf2, "%s is currently up for auction at %ld coins.\r\n",
        auction.obj->short_description, auction.bid);
      send_to_char(buf2, ch);
    } else {
      sprintf(buf2, "No one is currently auctioning %s for %ld coins.\r\n",
         auction.obj->short_description, auction.bid);
      send_to_char(buf2, ch);
    }
  /* Person doesn't have that item */
  } else if ((obj = get_obj_in_list_vis(ch, buf1, ch->carrying)) == NULL)
    send_to_char("You don't seem to have that to sell.\r\n", ch);
  /* Can not auction corpses because they may decompose */
  else if ((GET_OBJ_TYPE(obj) == ITEM_CONTAINER) && (GET_OBJ_VAL(obj, 3)))
     send_to_char("You can not auction corpses.\n\r", ch);
  /* Can't auction ARTIFACTS */
  else if (IS_OBJ_STAT(obj, ITEM_ARTIFACT)){
     send_to_char("The Gods are angered with your greedy act.\r\n",ch);
     return;
  } else if (IS_OBJ_STAT(obj, ITEM_TIMED)) {
     send_to_char("The auctioneer gets confused by timed items.\r\n", ch);
     return;
  } else if (IS_OBJ_STAT(obj, ITEM_NOSELL)) {
     send_to_char("You seem unable to auction that item.\r\n", ch);
     return;
  } else if (IS_OBJ_STAT(obj, ITEM_SOULBOUND) || contains_soulbound(obj)) {
     sendChar(ch, "You cannot sell an item bound to you.\r\n");
     return;
  } else /* It's valid */
  {
    /* First bid attempt */
    auction.ticks = AUC_BID;

    /* This guy is selling it */
    if (IS_NPC(ch)) {
        auction.seller = find_seller_mob(GET_MOB_VNUM(ch));
    } else {
        auction.seller = GET_IDNUM(ch);
    }

    /* Can not make the minimum less than 5000 --KR */
    auction.bid = (atoi(buf2) > 5000 ? atoi(buf2) : 5000);

    /* Pointer to object */
    auction.obj = obj;

    /* Get the object from the character, so they cannot drop it! */
    obj_from_char(auction.obj);

    sprintf(auction_buf, "%s has put %s up for sale, minimum bid %ld coin%s",
       GET_NAME(ch),auction.obj->short_description,
       auction.bid, auction.bid != 1 ? "s." : ".");

    mudlog(CMP, LVL_ANGEL, TRUE, "AUCTION: %s is selling %s for %ld coin(s)",
            GET_NAME(ch), auction.obj->short_description, auction.bid);

    /* send out the messages */
    auction_output(auction_buf);
  }
}

/*
 * auction_reset : returns the auction structure to a non-bidding state
 */
void auction_reset(void)
{
  auction.bidder = -1;
  auction.seller = -1;
  auction.obj = NULL;
  auction.ticks = AUC_NONE;
  auction.bid = 0;
}

/* -2, -3, for seller ids */
const int seller_mobs[] = {
    18073,
};

int find_seller_mob(int vnum)
{
    int i;

    for (i = 0; i < sizeof(seller_mobs) / sizeof(int); i++)
        if (seller_mobs[i] == vnum) return -i-2;

    return -1;
}

/*
 * get_ch_by_id : given an ID number, searches every descriptor for a
 *             character with that number and returns a pointer to it.
 */
static struct char_data *get_ch_by_id(long idnum)
{
  struct descriptor_data *d;

  for (d = descriptor_list; d; d = d->next)
    if (d && d->character && GET_IDNUM(d->character) == idnum)
      return (d->character);

  /* special hack: if idnum < -1, this is a mob selling */
  if (idnum < -1) {
      CharData *mob = character_list;

      while (mob && GET_MOB_VNUM(mob) != seller_mobs[-idnum-2])
          mob = mob->next;
      return mob;
  }

  return NULL;
}

ACMD(do_auction_stat)
{
  int i;

  /* NPC's can not bid or auction if charmed */
  if (ch->master && AFF_FLAGGED(ch, AFF_CHARM))
    return;

  /* There isn't an auction */
  if (auction.ticks == AUC_NONE) {
    send_to_char("Nothing is up for sale.\r\n", ch);
    return;
  }

  if (ch == get_ch_by_id(auction.seller)) {
    send_to_char("You should already know what you are selling.\r\n", ch);
    return;
  }

  if (auction.obj) {
    if( (IS_OBJ_STAT(auction.obj, ITEM_ARTIFACT))){
       send_to_char("You are unable to find out the stats of this item.\r\n",ch);
       return;
    } 
  }

/* Players no longer need to fear purchasing an item through auction
   that has generated poor stats. Editing out code for the -info cmd
   to work without the identified flag being set on the item.
  if (auction.obj) {
    if( !IS_OBJ_STAT(auction.obj, ITEM_IDENTIFIED) ){
       send_to_char("This item has not yet been identified.\r\n",ch);
       return;
    } 
  } */

  send_to_char("You feel informed:\r\n", ch);
  sprintf(buf, "Object '%s', Item type: ", auction.obj->short_description);
  sprinttype(GET_OBJ_TYPE(auction.obj), item_types, buf2, sizeof(buf2));
  strcat(buf, buf2);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  if (auction.obj->obj_flags.bitvector) {
      send_to_char("Item will give you following abilities:  ", ch);
      sprintbitarray(auction.obj->obj_flags.bitvector, affected_bits, AF_ARRAY_MAX, buf)
;
      strcat(buf, "\r\n");
      send_to_char(buf, ch);
  }

  send_to_char("Item is: ", ch);
  sprintbitarray(GET_OBJ_EXTRA(auction.obj), extra_bits, EF_ARRAY_MAX, buf);
  strcat(buf, "\r\n");
  send_to_char(buf, ch);

  sprintf(buf, "Weight: %d, Value: %d, Rent: %d\r\n",
          GET_OBJ_WEIGHT(auction.obj), GET_OBJ_COST(auction.obj), 
	  GET_OBJ_RENT(auction.obj));
  send_to_char(buf, ch);

  /* show them item values. */
  oPrintValues(auction.obj, buf);
  sendChar(ch, "%s\r\n", buf);

  /* And now applies. */
  send_to_char("Can affect you as :\r\n", ch);
  for (i = 0; i < MAX_OBJ_AFFECT; i++) {
       if ((auction.obj->affected[i].location != APPLY_NONE) &&
           (auction.obj->affected[i].modifier != 0)) {
            sprinttype(auction.obj->affected[i].location, apply_types, buf2, sizeof(buf2));
            sprintf(buf, "   Affects: %s By %d\r\n", 
	            buf2, auction.obj->affected[i].modifier);
            send_to_char(buf, ch);
       }
  }                              

  if (GET_CLASS(ch) == CLASS_MAGIC_USER) {
      if (auction.obj->item_number >= 0 &&
          obj_index[auction.obj->item_number].combatSpec &&
          GET_LEVEL(ch) >= 30) {
          combSpecName(obj_index[auction.obj->item_number].combatSpec, buf);
          sendChar(ch, "You sense this item has the following arcane power: %s\r\n", buf);
      }
  }
}
