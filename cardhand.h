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
    void removeCards(QList<const Card *> cards);
};



class CardHands : public QList<CardHand>
{
public:
    CardHands();
    int totalHands;
    QList<bool> aiPlayers;
    int initialHandCardCount;

    bool isAiPlayer(int player) const { return (player >= 0 && player < totalHands && player < aiPlayers.count()) ? aiPlayers.at(player) : false; }
    void clearHands();
    void dealHands(CardDeck &cardDeck);
    void sortHands();
    int findCardInHands(const Card *card) const;
    QJsonArray serializeToJson() const;
    void deserializeFromJson(const QJsonArray &arr, const CardDeck &cardDeck);
};

#endif // CARDHAND_H
