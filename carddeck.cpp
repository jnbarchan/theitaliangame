#include <QJsonArray>

#include "utils.h"
#include "carddeck.h"

CardDeck::CardDeck()
{
    this->nextCardToBeDealt = 0;
}

CardDeck::~CardDeck()
{
    for (auto &elm : *this)
        delete elm;
    clear();
    _initialFreeCards.clear();
}

void CardDeck::resetForNewDeal()
{
    nextCardToBeDealt = 0;
    _initialFreeCards.clear();
}

void CardDeck::createCards()
{
    Q_ASSERT(isEmpty());
    for (int pack = 0; pack < 2; pack++)
        for (int j = 0; j < 52; j++)
            append(new Card(pack * 52 + j));
    resetForNewDeal();
}

void CardDeck::shuffle()
{
    RandomNumber::random_shuffle(begin(), end());
    resetForNewDeal();
}

bool CardDeck::canDealNextCard() const
{
    return (nextCardToBeDealt < count());
}

const Card *CardDeck::dealNextCard()
{
    if (!canDealNextCard())
        return nullptr;
    return at(nextCardToBeDealt++);
}

const Card *CardDeck::extractCardFromDrawPile(int index)
{
    Q_ASSERT(index >= nextCardToBeDealt && index < count());
    const Card *card(at(index));
    move(index, nextCardToBeDealt++);
    return card;
}

int CardDeck::findCardIndex(int id) const
{
    Q_ASSERT(id >= 0 && id < count());
    for (int i = 0; i < count(); i++)
        if (at(i)->id == id)
            return i;
    Q_ASSERT(false);
    return -1;
}

const Card *CardDeck::findCard(int id) const
{
    return at(findCardIndex(id));
}

bool CardDeck::isInitialFreeCard(const Card *card) const
{
    return _initialFreeCards.contains(card);
}

void CardDeck::addInitialFreeCard(const Card *card)
{
    Q_ASSERT(!_initialFreeCards.contains(card));
    _initialFreeCards.append(card);
}

void CardDeck::removeFromInitialFreeCards(const Card *card)
{
    Q_ASSERT(_initialFreeCards.contains(card));
    _initialFreeCards.removeOne(card);
}

QJsonObject CardDeck::serializeToJson() const
{
    QJsonObject obj;
    obj["nextCardToBeDealt"] = nextCardToBeDealt;
    QJsonArray arrCards;
    for (const Card *card : *this)
        arrCards.append(card->id);
    obj["cards"] = arrCards;
    QJsonArray arrFreeCards;
    for (const Card *card : _initialFreeCards)
        arrFreeCards.append(card->id);
    obj["initialFreeCards"] = arrFreeCards;
    return obj;
}

void CardDeck::deserializeFromJson(const QJsonObject &obj)
{
    resetForNewDeal();
    nextCardToBeDealt = obj["nextCardToBeDealt"].toInt();
    const QJsonArray &arrCards(obj["cards"].toArray());
    Q_ASSERT(arrCards.count() == count());
    for (int i = 0; i < arrCards.count(); i++)
    {
        int found = findCardIndex(arrCards.at(i).toInt());
        move(found, i);
    }
    _initialFreeCards.clear();
    const QJsonArray &arrFreeCards(obj["initialFreeCards"].toArray());
    for (const auto &val : arrFreeCards)
        _initialFreeCards.append(findCard(val.toInt()));
}
