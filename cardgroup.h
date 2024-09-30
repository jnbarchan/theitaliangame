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
    CardGroup(std::initializer_list<const Card *> args) : QList<const Card *>(args) {}

    enum SetType { RankSet, RunSet };
    QString toString() const;
    int rankDifference(int rank0, int rank1) const;
    void rearrangeForSets();
    bool isGoodSet(SetType &setType) const;
    bool isGoodSet() const;
};



class CardGroups : public QList<CardGroup>
{
public:
    CardGroups();

    QString toString() const;
    void clearGroups();
    int findCardInGroups(const Card *card) const;
    void removeEmptyGroups();
    QJsonArray serializeToJson() const;
    void deserializeFromJson(const QJsonArray &arr, const CardDeck &cardDeck);
};

#endif // CARDGROUP_H
