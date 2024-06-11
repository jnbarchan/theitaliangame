#ifndef CARDHAND_H
#define CARDHAND_H

#include <QJsonArray>
#include <QList>

#include "card.h"
#include "carddeck.h"


class CardHand : public QList<const Card *>
{
public:
    CardHand();

    void sortHand();
};



class CardHands : public QList<CardHand>
{
public:
    CardHands();
    int totalHands;
    int initialHandCardCount;

    void clearHands();
    void dealHands(CardDeck &cardDeck);
    void sortHands();
    int findCardInHands(const Card *card) const;
    QJsonArray serializeToJson() const;
    void deserializeFromJson(const QJsonArray &arr, const CardDeck &cardDeck);
};

#endif // CARDHAND_H
