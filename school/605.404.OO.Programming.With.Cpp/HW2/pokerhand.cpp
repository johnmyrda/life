#include <stdio.h>
#include <string>
using std::string;
#include "card.h"
#include "pokerhand.h"
// Boost.Array constructor
PokerHand::PokerHand(boost::array<Card, 5>& array)
  : handRank(HandRank::HIGH_CARD)
{
  boost::array<Card, 5>::const_iterator array_iter = array.begin();
  uint8_t i = 0;
  while(array_iter != array.end())
  {
    getCard(i) = *array_iter;
    ++i;
    ++array_iter;
  }
  // Sort our hand so we can calculate its rank
  // Use binary predicate to just sort on CardValue and not Suit
  std::sort(this->cards, this->cards + 5);
  // Since we're sorted, the high card is at the end.
  // Calculate the rank of our hand
  calculateRank();
}

// Initialize a new poker hand with 5 cards
PokerHand::PokerHand(Card& card1, Card& card2, Card& card3, 
                     Card& card4, Card& card5)
{
  getCard(0) = card1;
  getCard(1) = card2;
  getCard(2) = card3;
  getCard(3) = card4;
  getCard(4) = card5;
  // Sort our hand so we can calculate its rank
  // Use binary predicate to just sort on CardValue and not Suit
  std::sort(this->cards, this->cards + 5);
  // Since we're sorted, the high card is at the end.
  // Calculate the rank of our hand
  calculateRank();
}

PokerHand::operator string()
{
  string s;
  // For all but the last card
  int i;
  for(i = 0; i < numCards - 1; i++)
  {
    s += static_cast<string>(getCard(i)) + " ";
  }
  // Now i == numCards - 1
  s += static_cast<string>(getCard(i));
  return s;
}

void PokerHand::checkFlush()
{
  bool failed = false;
  for(int i = 1; i < numCards; i++)
  {
    // Compare this card to the next.
    if(getCard(0).getSuit() != getCard(i).getSuit())
    {
      // We've failed to make a straight.
      failed = true;
      break;
    }
  }
  if(!failed)
  {
    // We have a flush.  I wonder if it's a straight flush!
    // Since we called checkStraight before this, we can tell.
    if(HandRank::STRAIGHT == getHandRank())
    {
      // Indeed, we have a straight flush.
      // Rank high card already set from straight.
      setHandRank(HandRank::STRAIGHT_FLUSH);
    }
    // Make sure we don't have four-of-a-kind or a full house.
    else if(HandRank::FOUR_OF_A_KIND != getHandRank() ||
            HandRank::FULL_HOUSE     != getHandRank()
           )
    {
      // Too bad we don't have those hands.  Set to flush.
      setHandRank(HandRank::FLUSH);
      // Must set rankHighCard to highCard
      setFirstRank(getHighCard());
    }
    
  }
}

void PokerHand::checkStraight()
{
  uint8_t inARow = 1; // Number of cards we've seen in a row

  // Check if the high card is an ace.  This is to handle the special case
  // where we have an ace low straight, like AD 2H 3D 4H 5H
  if(CardRank::ACE == getHighCard().getCardRank() &&
     CardRank::TWO == getCard(0).getCardRank()
    )
  {
    // Simply increment inARow to deal with the problem.
    // This makes up for the last precedes() call failing.
    ++inARow;
  }

  for(int i = 0; i < numCards - 1; i++)
  {
    // Compare this card to the next.
    if(getCard(i).precedes(getCard(i + 1)))
    {
      // We've found another one in order
      ++inARow;
    }
  }
  // If all 4 comparisons succeed, we have a straight
  if(5 == inARow)
  {
    setHandRank(HandRank::STRAIGHT);
    // Must set rankHighCard to the appropriate card.
    // This can be tricky because it might not be the last card
    // In the ace-low straight, we must set it to one less than the top.
    if(CardRank::ACE == getHighCard().getCardRank() &&
       CardRank::TWO == getCard(0).getCardRank()
      )
    {
      // Set to one before the high card.
      setFirstRank(getCard(numCards - 2));
    }
    else
    {
      // Normal case.
      // Set to the high card.
      setFirstRank(getHighCard());
    }
  }
}

void PokerHand::checkPairs()
{
  int i = 0;
  const Card* card = &getCard(i);
  while(i < numCards - 1)
  {
    // Set j to index of next card
    int numAlike;
    card = &getCard(i);
    int originalCardIdx = i + 1;
    int j = originalCardIdx;
    // Find how many alike cards follow
    while(j < numCards && *card == getCard(j))
    {
      ++j;
    }
    // The distance from our original value of j is the number of alike
    // cards.
    numAlike = j - originalCardIdx;
    Card& originalCard = getCard(originalCardIdx);

    // Calculate Hand Rank
    switch(numAlike)
    {
      case 0: // None
        // Do nothing.
        break;
      case 1: // Pair
        if(HandRank::PAIR == getHandRank())
        {
          // We already have a pair!  2 pair now.
          setHandRank(HandRank::TWO_PAIR);
          // Check our original pair and see if this pair is higher.
          if(getFirstRank() < originalCard.getCardRank())
          {
            // Set the lower pair rank to the second pair rank
            setSecondRank(getFirstRank());
            // This is a higher pair.  Set it to the rank high card.
            setFirstRank(originalCard.getCardRank());
          }
          else
          {
            // The other pair is higher.  Just set the second pair.
            setSecondRank(originalCard.getCardRank());
          }
        }
        else if(HandRank::THREE_OF_A_KIND == getHandRank())
        {
          // We already have three of a kind!  Full House
          setHandRank(HandRank::FULL_HOUSE);

          // The first rank is always the three of a kind.
          setSecondRank(originalCard.getCardRank());
        }
        else
        {
          // Otherwise, just set one pair.
          setHandRank(HandRank::PAIR);
          // Set our first rank
          setFirstRank(originalCard.getCardRank());
        }
        break;
      case 2: // Three of a kind
        if(HandRank::PAIR == getHandRank())
        {
          // We already have a pair!  Full House
          setHandRank(HandRank::FULL_HOUSE);

          // The first rank is always the three of a kind.
          setSecondRank(getFirstRank());
          setFirstRank(originalCard.getCardRank());
        }
        else
        {
          // Otherwise, just set three of a kind.
          setHandRank(HandRank::THREE_OF_A_KIND);
          setFirstRank(originalCard.getCardRank());
        }
        break;
      case 3: // Four of a kind
        // Can't be combined with anything.
        setHandRank(HandRank::FOUR_OF_A_KIND);
        setFirstRank(originalCard.getCardRank());
        break;
      default:
        // Should never hit this state with a valid hand.
        break;
    }
    // Set i to j.
    i = j;
  }
}

// Compare
// Negative result means other hand is less than this.
// Zero means equal.
// Positive result means other hand is greater than this.
int8_t PokerHand::compare(const PokerHand& otherHand) const
{
  // Use a signed int to store the result.  This way, we can go negative.
  int8_t result = 0;
  // First, check that the hand rank matches
  if(getHandRank() != otherHand.getHandRank())
  {
    result = (otherHand.getHandRank() - getHandRank());
  }
  else
  {
    // Make some decisions based on the type of hand.
    if(getHandRank() == HandRank::TWO_PAIR ||
       getHandRank() == HandRank::FULL_HOUSE)
    {   
      // Special cases - have multiple ranks
      if(getFirstRank() == otherHand.getFirstRank())
      {
        // Our first ranks match!  Compare the second ranks.
        if(getSecondRank() == otherHand.getSecondRank())
        {
          // Check the high card
          return (otherHand.getHighCard() - getHighCard());
        }
        else
        {
          // Second ranks are different.
          result = (otherHand.getSecondRank() - getSecondRank());
        }
      }
      else
      {
        // First ranks are different.
        result = (otherHand.getFirstRank() - getFirstRank());
      }
    }
    else
    {
        // Applies to: HIGH_CARD, PAIR, THREE_OF_A_KIND, STRAIGHT, FLUSH,
        //             FOUR_OF_A_KIND, and STRAIGHT_FLUSH
        // Simple comparison of the first rank of the hand.
        result = (otherHand.getFirstRank() - getFirstRank());
    }
  }
  return result;
}

// Greater Than
bool PokerHand::operator > (const PokerHand& otherHand) const
{
  return compare(otherHand) < 0;
}

// Greater Than or Equal
bool PokerHand::operator >= (const PokerHand& otherHand) const
{
  return compare(otherHand) <= 0;
}

// Less Than
bool PokerHand::operator < (const PokerHand& otherHand) const
{
  return compare(otherHand) > 0;
}

// Less Than or Equal
bool PokerHand::operator <= (const PokerHand& otherHand) const
{
  return compare(otherHand) >= 0;
}

// Equal
bool PokerHand::operator == (const PokerHand& otherHand) const
{
  return compare(otherHand) == 0;
}

// Not Equal
bool PokerHand::operator != (const PokerHand& otherHand) const
{
  return compare(otherHand) != 0;
}

