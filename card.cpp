#include "card.h"

Card::Card(int id)
{
    this->id = id;
}

QString Card::toString() const
{
    QString str;
    switch (this->suit())
    {
    case 0: str += "C"; break;
    case 1: str += "D"; break;
    case 2: str += "H"; break;
    case 3: str += "S"; break;
    }
    int rank = this->rank() + 2;
    switch (rank)
    {
    case 14: str += "A"; break;
    case 13: str += "K"; break;
    case 12: str += "Q"; break;
    case 11: str += "J"; break;
    default: str += QString::number(rank); break;
    }
    return str;
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
