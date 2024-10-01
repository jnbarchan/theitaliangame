#include "logicalmodel.h"

LogicalModel::LogicalModel()
{

}

void LogicalModel::shuffleAndDeal()
{
    cardGroups.clearGroups();

    cardDeck.shuffle();

    hands.dealHands(cardDeck);
    dealInitialFreeCards();
}

void LogicalModel::dealInitialFreeCards()
{
    for (int i = 0; i < 4; i++)
    {
        const Card *card = cardDeck.dealNextCard();
        cardDeck.addInitialFreeCard(card);
        CardGroup group;
        group.append(card);
        cardGroups.append(group);
        Q_ASSERT(!group.isGoodSet());
        Q_ASSERT(isInitialCardGroup(group));
    }
}

bool LogicalModel::isInitialCardGroup(const CardGroup &group) const
{
    return (group.count() == 1 && cardDeck.isInitialFreeCard(group.at(0)));
}

CardGroup &LogicalModel::initialFreeCardGroup(const Card *card)
{
    int groupNum = cardGroups.findCardInGroups(card);
    Q_ASSERT(groupNum >= 0);
    CardGroup &group(cardGroups[groupNum]);
    Q_ASSERT(!group.isGoodSet());
    Q_ASSERT(isInitialCardGroup(group));
    return group;
}

CardGroups LogicalModel::badSetGroups() const
{
    CardGroups badSets;
    for (const CardGroup &group : cardGroups)
        if (!group.isGoodSet() && !isInitialCardGroup(group))
            badSets.append(group);
    return badSets;
}

const Card *LogicalModel::drawCardFromDrawPile()
{
    const Card *card = cardDeck.dealNextCard();
    if (card == nullptr)
        return nullptr;
    CardHand &hand(hands[activePlayer]);
    hand.append(card);
    hand.sortHand();
    return card;
}

const Card *LogicalModel::extractCardFromDrawPile(int index)
{
    const Card *card = cardDeck.extractCardFromDrawPile(index);
    if (card == nullptr)
        return nullptr;
    CardHand &hand(hands[activePlayer]);
    hand.append(card);
    hand.sortHand();
    return card;
}

void LogicalModel::updateInitialFreeCards()
{
    for (const Card *card : cardDeck.initialFreeCards())
    {
        int inGroup = cardGroups.findCardInGroups(card);
        if (inGroup >= 0 && cardGroups.at(inGroup).count() > 1)
            cardDeck.removeFromInitialFreeCards(card);
    }
}

void LogicalModel::startOfTurn()
{
    _startOfTurnHand = hands.at(activePlayer);
}

void LogicalModel::endOfTurn()
{
    updateInitialFreeCards();

    if (++activePlayer >= hands.totalHands)
        activePlayer = 0;
}
