#ifndef CARD_H
#define CARD_H

#include <QString>


class Card
{
public:

public:
    Card(int id);

    inline int pack() const { return id / 52; }
    inline int suit() const { return id % 52 % 4; }
    inline int rank() const { return id % 52 / 4; }
    QString toString() const;
    static bool compareForSortBySuit(const Card *cardA, const Card *cardB);

public:
    int id;

private:
#ifdef QT_DEBUG
    QString _debugStr;
#endif
};

#endif // CARD_H
