#ifndef CARDDECK_H
#define CARDDECK_H

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
    int findCardIndex(int id) const;
};

#endif // CARDDECK_H
