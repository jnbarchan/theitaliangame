#include "card.h"

Card::Card(int id)
{
    this->id = id;
}

/*static*/ bool Card::compareForSortBySuit(const Card *cardA, const Card *cardB)
{
    // sort order: Diamonds, Clubs, Hearts, Spades
    //             then rank in descending order
    int suitA(cardA->suit()), suitB(cardB->suit());
    int rankA(cardA->rank()), rankB(cardB->rank());
    if (suitA == 1)
        suitA = -1;
    if (suitB == 1)
        suitB = -1;
    rankA = 12 - rankA;
    rankB = 12 - rankB;
    int valueA(suitA * 13 + rankA), valueB(suitB * 13 + rankB);
    return (valueA < valueB);
}
