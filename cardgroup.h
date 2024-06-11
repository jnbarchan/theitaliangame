#ifndef CARDGROUP_H
#define CARDGROUP_H

#include <QJsonArray>
#include <QList>

#include "card.h"
#include "carddeck.h"

class CardGroup : public QList<const Card *>
{
public:
    CardGroup();

    void rearrangeForSets();
    bool isGoodSet() const;

private:
    int rankDifference(int rank0, int rank1) const;
};



class CardGroups : public QList<CardGroup>
{
public:
    CardGroups();

    void clearGroups();
    int findCardInGroups(const Card *card) const;
    void removeEmptyGroups();
    QJsonArray serializeToJson() const;
    void deserializeFromJson(const QJsonArray &arr, const CardDeck &cardDeck);
};

#endif // CARDGROUP_H
