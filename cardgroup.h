#ifndef CARDGROUP_H
#define CARDGROUP_H

#include <QJsonArray>
#include <QList>

#include "card.h"
#include "carddeck.h"

class CardGroup : public QList<const Card *>
{
private:
    static long _nextUniqueId;
    long _uniqueId;

public:
    CardGroup();
    CardGroup(std::initializer_list<const Card *> args);

    static void resetNextUniqueId();
    long uniqueId() const { return _uniqueId; }
    enum SetType { RankSet, RunSet };
    QString toString() const;
    int rankDifference(int rank0, int rank1) const;
    void rearrangeForSets();
    bool isGoodRankSet() const;
    bool isGoodRunSet() const;
    bool isGoodSetOfType(SetType setTypeWanted) const;
    bool isGoodSet(SetType &setType) const;
    bool isGoodSet() const;
    void removeCards(const QList<const Card *> &cards);
};



class CardGroups : public QList<CardGroup>
{
public:
    CardGroups();
    CardGroups(std::initializer_list<CardGroup> args);

    QString toString() const;
    void clearGroups();
    int findCardGroupByUniqueId(int uniqueId) const;
    int findCardInGroups(const Card *card) const;
    int removeCardFromGroups(const Card *card);
    void removeEmptyGroups();
    QList<const Card *> allCards() const;
    QJsonArray serializeToJson() const;
    void deserializeFromJson(const QJsonArray &arr, const CardDeck &cardDeck);
};

#endif // CARDGROUP_H
