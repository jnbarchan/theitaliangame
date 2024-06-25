#ifndef CARDDECK_H
#define CARDDECK_H

#include <random>

#include <QJsonObject>
#include <QList>

#include "card.h"

class CardDeck : public QList<const Card *>
{
public:
    CardDeck();
    ~CardDeck();

    void resetForNewDeal();
    void createCards();
    int random_int(int range);
    template<typename _RAIter>
      void
      random_shuffle(_RAIter itBegin, _RAIter itEnd) { std::shuffle(itBegin, itEnd, random_generator()); }
    void shuffle();
    const Card *dealNextCard();
    const Card *findCard(int id) const;
    const QList<const Card *> &initialFreeCards() const { return _initialFreeCards; }
    bool isInitialFreeCard(const Card *card) const;
    void addInitialFreeCard(const Card *card);
    void removeFromInitialFreeCards(const Card *card);
    QJsonObject serializeToJson() const;
    void deserializeFromJson(const QJsonObject &obj);

private:
    int nextCardToBeDealt;
    QList<const Card *> _initialFreeCards;

    static std::mt19937 &random_generator();
    int findCardIndex(int id) const;
};

#endif // CARDDECK_H
