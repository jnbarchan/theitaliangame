#ifndef CARD_H
#define CARD_H


class Card
{
public:

public:
    Card(int id);

    inline int pack() const { return id / 52; }
    inline int suit() const { return id % 52 % 4; }
    inline int rank() const { return id % 52 / 4; }
    static bool compareForSortBySuit(const Card *cardA, const Card *cardB);

public:
    int id;
};

#endif // CARD_H
