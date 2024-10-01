#ifndef LOGICALMODEL_H
#define LOGICALMODEL_H

#include "carddeck.h"
#include "cardhand.h"
#include "cardgroup.h"

class LogicalModel
{
public:
    LogicalModel();

    int activePlayer;
    CardDeck cardDeck;
    CardHands hands;
    CardGroups cardGroups;

    const CardHand &startOfTurnHand() const { return _startOfTurnHand; }
    void shuffleAndDeal();
    void dealInitialFreeCards();
    bool isInitialCardGroup(const CardGroup &group) const;
    CardGroup &initialFreeCardGroup(const Card *card);
    CardGroups badSetGroups() const;
    const Card *drawCardFromDrawPile();
    const Card *extractCardFromDrawPile(int index);
    void updateInitialFreeCards();
    void startOfTurn();
    void endOfTurn();

private:
    CardHand _startOfTurnHand;
};

#endif // LOGICALMODEL_H
