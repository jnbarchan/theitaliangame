#include <QDebug>

#include "cardhand.h"

CardHand::CardHand()
{

}

void CardHand::sortHand()
{
    std::sort(begin(), end(), Card::compareForSortBySuit);
}

void CardHand::removeCards(const QList<const Card *> &cards)
{
    for (const Card *card : cards)
        removeOne(card);
}



CardHands::CardHands()
{
    this->totalHands = 1;
    this->aiPlayers = { false };
    this->initialHandCardCount = 13;
}

void CardHands::clearHands()
{
    clear();
    for (int i = 0; i < totalHands; i++)
        append(CardHand());
}

void CardHands::dealHands(CardDeck &cardDeck)
{
    clearHands();
    cardDeck.resetForNewDeal();
    for (int i = 0; i < initialHandCardCount; i++)
        for (int j = 0; j < count(); j++)
        {
            const Card *card = cardDeck.dealNextCard();
            (*this)[j].append(card);
        }
}

void CardHands::sortHands()
{
    for (CardHand &hand : *this)
        hand.sortHand();
}

int CardHands::findCardInHands(const Card *card) const
{
    for (int i = 0; i < count(); i++)
        if (at(i).indexOf(card) >= 0)
            return i;
    return -1;
}

QJsonArray CardHands::serializeToJson() const
{
    QJsonArray arr;
    for (const CardHand &hand : *this)
    {
        QJsonArray arr0;
        for (const Card *card : hand)
            arr0.append(card->id);
        arr.append(arr0);
    }
    return arr;
}

void CardHands::deserializeFromJson(const QJsonArray &arr, const CardDeck &cardDeck)
{
    Q_ASSERT(arr.count() == totalHands);
    clearHands();
    for (int i = 0; i < arr.count(); i++)
    {
        CardHand &hand((*this)[i]);
        const QJsonArray &arr0(arr.at(i).toArray());
        for (const auto &val : arr0)
            hand.append(cardDeck.findCard(val.toInt()));
    }
}
