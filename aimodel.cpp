#include "aimodel.h"

AiModel::AiModel(QObject *parent)
    : QObject{parent}
{
}

CardHand &AiModel::aiHand() const
{
    CardHands &hands(this->hands());
    Q_ASSERT(hands.isAiPlayer(activePlayer()));
    return hands[activePlayer()];
}

QList<const Card *> AiModel::freeCardsInGroup(const CardGroup &group) const
{
    QList<const Card *> freeCards;
    if (logicalModel->isInitialCardGroup(group))
        freeCards.append(group);
    else
    {
        CardGroup::SetType setType;
        bool isGoodSet = group.isGoodSet(setType);
        Q_ASSERT(isGoodSet);
        if (group.count() > 3)
        {
            if (setType == CardGroup::RankSet)
            {
                freeCards.append(group);
            }
            else if (setType == CardGroup::RunSet)
            {
                freeCards.append(group.at(0));
                freeCards.append(group.at(group.count() - 1));
            }
        }
    }
    return freeCards;
}

QList<const Card *> AiModel::findAllFreeCardsOnBaize() const
{
    QList<const Card *> freeCards;
    for (const CardGroup &group : cardGroups())
        freeCards.append(freeCardsInGroup(group));
    return freeCards;
}


void AiModel::removeFirstCardRankSet(CardHand &hand, CardGroup &rankSet) const
{
    // remove the first card in hand
    // find any (longest) rank set using that card
    // because the set is longest, the further cards used cannot lie in any other rank set
    // so remove those cards too
    rankSet.clear();
    if (hand.isEmpty())
        return;
    rankSet.append(hand.takeAt(0));
    // check for same rank (different suits)
    const Card *card0 = rankSet.at(0);
    QSet<int> suits{card0->suit()};
    for (int i = 0; i < hand.count(); i++)
    {
        const Card *card = hand.at(i);
        if (card->rank() == card0->rank() && !suits.contains(card->suit()))
        {
            suits.insert(card->suit());
            rankSet.append(hand.takeAt(i--));
        }
    }
}

void AiModel::removeFirstCardRunSet(CardHand &hand, CardGroup &runSet) const
{
    // remove the first card in hand
    // find any (longest, sequential) run set using that card
    // because the set is longest, the further cards used cannot lie in any other run set
    // so remove those cards too
    runSet.clear();
    if (hand.isEmpty())
        return;
    runSet.append(hand.takeAt(0));
    // check for sequential cards in same suit
    const Card *card0 = runSet.at(0), *card1 = runSet.at(0);
    for (int i = 0; i < hand.count(); i++)
    {
        const Card *card = hand.at(i);
        if (card->suit() != card0->suit())
            continue;
        if (runSet.rankDifference(card->rank(), card0->rank()) == 1)
        {
            runSet.prepend(hand.takeAt(i));
            card0 = runSet.at(0);
            i = -1;
        }
        else if (runSet.rankDifference(card->rank(), card1->rank()) == -1)
        {
            runSet.append(hand.takeAt(i));
            card1 = runSet.at(runSet.count() - 1);
            i = -1;
        }
    }
}

void AiModel::removeFirstCardGenerateAll2CardPartialRunSets(CardHand &hand, CardGroups &runSets) const
{
    // remove the first card in hand
    // find any 2-card partial run set using that card
    // the cards may be sequential ("8-7", "2-A") or have a gap of 1 in rank ("8-6", "2-K")
    // because there can be further partial runs using the other cards ("8-6-4", "2-A-Q")
    // do not remove those other cards too
    runSets.clear();
    if (hand.isEmpty())
        return;
    const Card *card0(hand.takeAt(0));
    for (const Card *card1 : hand)
        if (card1->suit() == card0->suit())
        {
            CardGroup runSet;
            runSet.append(card0);
            int rankDifference = runSet.rankDifference(card0->rank(), card1->rank());
            if (rankDifference >= 0)
                runSet.append(card1);
            else
                runSet.prepend(card1);
            if (rankDifference != 0 && qAbs(rankDifference) <= 2)
                runSets.append(runSet);
        }
}


CardGroups AiModel::findAllCompleteRankSetsInHand() const
{
    CardGroups rankSets;
    CardHand hand(aiHand());
    while (!hand.isEmpty())
    {
        CardGroup rankSet;
        removeFirstCardRankSet(hand, rankSet);
        if (rankSet.count() >= 3)
        {
//            qDebug() << "findAllCompleteRankSetsInHand:" << "Rank set:" << rankSet.toString();
            rankSets.append(rankSet);
        }
    }
    return rankSets;
}

CardGroups AiModel::findAllCompleteRunSetsInHand() const
{
    CardGroups runSets;
    CardHand hand(aiHand());
    while (!hand.isEmpty())
    {
        CardGroup runSet;
        removeFirstCardRunSet(hand, runSet);
        if (runSet.count() >= 3)
        {
//            qDebug() << "findAllCompleteRunSetsInHand:" << "Run set:" << runSet.toString();
            runSets.append(runSet);
        }
    }
    return runSets;
}


CardGroups AiModel::findAllPartialRankSetsFrom2CardsInHand() const
{
    CardGroups rankSets;
    CardHand hand(aiHand());
    while (!hand.isEmpty())
    {
        CardGroup rankSet;
        removeFirstCardRankSet(hand, rankSet);
        if (rankSet.count() == 2)
        {
//            qDebug() << "findAllPartialRankSetsFrom2CardsHand:" << "Rank set:" << rankSet.toString();
            rankSets.append(rankSet);
        }
    }
    return rankSets;
}

CardGroups AiModel::findAllPartialRunSetsFrom2CardsInHand() const
{
    CardGroups runSets;
    CardHand hand(aiHand());
    while (!hand.isEmpty())
    {
        CardGroups firstCardRunSets;
        removeFirstCardGenerateAll2CardPartialRunSets(hand, firstCardRunSets);
        if (!firstCardRunSets.isEmpty())
        {
//            qDebug() << "findAllPartialRunSetsFrom2CardsHand:" << "Run sets:" << firstCardRunSets.toString();
            runSets.append(firstCardRunSets);
        }
    }
    return runSets;
}

CardGroups AiModel::completeAllPartialRankSetsFrom1CardOnBaize(const CardGroups &rankPartialSets) const
{
    CardGroups rankSets;
    if (rankPartialSets.isEmpty())
        return {};
    for (const CardGroup &cardGroup : rankPartialSets)
    {
        Q_ASSERT(cardGroup.count() == 2);
        Q_ASSERT(cardGroup.at(0)->rank() == cardGroup.at(1)->rank());
        Q_ASSERT(cardGroup.at(0)->suit() != cardGroup.at(1)->suit());
    }
    QList<const Card *> freeCards = findAllFreeCardsOnBaize();
    if (freeCards.isEmpty())
        return {};
    for (const CardGroup &partialSet : rankPartialSets)
        for (const Card *freeCard : freeCards)
            if (freeCard->rank() == partialSet.at(0)->rank())
            {
                CardGroup rankSet(partialSet);
                rankSet.append(freeCard);
                if (rankSet.isGoodSet())
                {
//                    qDebug() << "completeAllPartialRankSetsFrom1CardOnBaize:" << "Rank set:" << rankSet.toString();
                    rankSets.append(rankSet);
                }
            }
    return rankSets;
}

CardGroups AiModel::completeAllPartialRunSetsFrom1CardOnBaize(const CardGroups &runPartialSets) const
{
    CardGroups runSets;
    if (runPartialSets.isEmpty())
        return {};
    for (const CardGroup &cardGroup : runPartialSets)
    {
        Q_ASSERT(cardGroup.count() == 2);
        Q_ASSERT(cardGroup.at(0)->suit() == cardGroup.at(1)->suit());
        int rankDifference = qAbs(cardGroup.rankDifference(cardGroup.at(0)->rank(), cardGroup.at(1)->rank()));
        Q_ASSERT(rankDifference > 0 && rankDifference <= 2);
    }
    QList<const Card *> freeCards = findAllFreeCardsOnBaize();
    if (freeCards.isEmpty())
        return {};
    for (const CardGroup &partialSet : runPartialSets)
        for (const Card *freeCard : freeCards)
            if (freeCard->suit() == partialSet.at(0)->suit())
            {
                CardGroup runSet(partialSet);
                runSet.append(freeCard);
                runSet.rearrangeForSets();
                if (runSet.isGoodSet())
                {
//                    qDebug() << "completeAllPartialRunSetsFrom1CardOnBaize:" << "Run set:" << runSet.toString();
                    runSets.append(runSet);
                }
            }
    return runSets;
}

CardGroups AiModel::findAllCompleteRankSetsFrom2CardsInHand() const
{
    CardGroups rankSets = findAllPartialRankSetsFrom2CardsInHand();
    rankSets = completeAllPartialRankSetsFrom1CardOnBaize(rankSets);
//    qDebug() << "findAllCompleteRankSetsFrom2CardsInHand:" << "Complete sets count:" << rankSets.count();
    return rankSets;
}

CardGroups AiModel::findAllCompleteRunSetsFrom2CardsInHand() const
{
    CardGroups runSets = findAllPartialRunSetsFrom2CardsInHand();
    runSets = completeAllPartialRunSetsFrom1CardOnBaize(runSets);
//    qDebug() << "findAllCompleteRunSetsFrom2CardsInHand:" << "Complete sets count:" << runSets.count();
    return runSets;
}


CardGroups AiModel::findAllAddToSetsFrom1CardInHand() const
{
    CardGroups addToSets;
    CardHand hand(aiHand());
    while (!hand.isEmpty())
    {
        const Card *card = hand.takeFirst();
        for (const CardGroup &existingSet : cardGroups())
        {
            if (existingSet.count() < 2)
                continue;
            if (existingSet.at(0)->rank() != card->rank() && existingSet.at(0)->suit() != card->suit())
                continue;
            CardGroup newSet(existingSet);
            newSet.append(card);
            newSet.rearrangeForSets();
            if (newSet.isGoodSet())
            {
                qDebug() << "findAllAddToSetsFrom1CardInHand:" << newSet.toString();
                addToSets.append(newSet);
            }
        }
    }
    return addToSets;
}


CardGroups AiModel::findAllMakeNewSetsFrom1CardInHand() const
{
    CardGroups newSets;
    CardHand hand(aiHand());
    while (!hand.isEmpty())
    {
        const Card *card0 = hand.takeFirst();
        for (const CardGroup &existingSet1 : cardGroups())
        {
            if (existingSet1.count() < 4)
                continue;
            CardGroup::SetType setType;
            bool isGoodSet = existingSet1.isGoodSet(setType);

            if (existingSet1.count() >= 5)
            {
                Q_ASSERT(isGoodSet);
                Q_ASSERT(setType == CardGroup::RunSet);
                if (existingSet1.at(0)->suit() == card0->suit())
                {
                    CardGroup newSet = {card0, existingSet1.at(0), existingSet1.at(1)};
                    newSet.rearrangeForSets();
                    if (newSet.isGoodSet())
                    {
                        qDebug() << "findAllMakeNewSetsFrom1CardInHand:" << newSet.toString();
                        newSets.append(newSet);
                    }
                    newSet = {card0, existingSet1.at(existingSet1.count() - 1), existingSet1.at(existingSet1.count() - 2)};
                    newSet.rearrangeForSets();
                    if (newSet.isGoodSet())
                    {
                        qDebug() << "findAllMakeNewSetsFrom1CardInHand:" << newSet.toString();
                        newSets.append(newSet);
                    }
                }
            }

            QList<const Card *> freeCards1(freeCardsInGroup(existingSet1));
            for (const Card *card1 : freeCards1)
            {
                if (card1->rank() != card0->rank() && card1->suit() != card0->suit())
                    continue;
                for (const CardGroup &existingSet2 : cardGroups())
                {
                    if (existingSet2 == existingSet1)
                        continue;
                    QList<const Card *> freeCards2(freeCardsInGroup(existingSet2));
                    for (const Card *card2 : freeCards2)
                    {
                        if (card2->rank() != card0->rank() && card2->suit() != card0->suit())
                            continue;
                        CardGroup newSet = {card0, card1, card2};
                        newSet.rearrangeForSets();
                        if (newSet.isGoodSet())
                        {
                            qDebug() << "findAllMakeNewSetsFrom1CardInHand:" << newSet.toString();
                            newSets.append(newSet);
                        }
                    }
                }
            }
        }
    }
    return newSets;
}


CardGroups AiModel::findAllCompleteSetsInHand() const
{
    CardGroups completeSets;
    CardGroups rankSets = findAllCompleteRankSetsInHand();
    CardGroups runSets = findAllCompleteRunSetsInHand();
    completeSets.append(rankSets);
    completeSets.append(runSets);
    qDebug() << "findAllCompleteSetsInHand:" << "Complete sets count:" << completeSets.count();
    return completeSets;
}

CardGroups AiModel::findAllCompleteSetsFrom2CardsInHand() const
{
    CardGroups completeSets;
    CardGroups rankSets = findAllCompleteRankSetsFrom2CardsInHand();
    CardGroups runSets = findAllCompleteRunSetsFrom2CardsInHand();
    completeSets.append(rankSets);
    completeSets.append(runSets);
    qDebug() << "findAllCompleteSetsFrom2CardsInHand:" << "Complete sets count:" << completeSets.count();
    return completeSets;
}

CardGroups AiModel::findAllAddToCompleteSetsFrom1CardInHand() const
{
    CardGroups completeSets;
    CardGroups addToSets = findAllAddToSetsFrom1CardInHand();
    completeSets.append(addToSets);
    qDebug() << "findAllAddToCompleteSetsFrom1CardInHand:" << "Complete sets count:" << completeSets.count();
    return completeSets;
}

CardGroups AiModel::findAllCompleteSetsFrom1CardInHand() const
{
    CardGroups completeSets;
    CardGroups newSets = findAllMakeNewSetsFrom1CardInHand();
    completeSets.append(newSets);
    qDebug() << "findAllCompleteSetsFrom1CardInHand:" << "Complete sets count:" << completeSets.count();
    return completeSets;
}


/*slot*/ void AiModel::makeTurn()
{
    Q_ASSERT(hands().isAiPlayer(activePlayer()));

    AiModelTurnMoves turnMoves;

    CardGroups completeSets;

    // find all 3+ complete sets in hand, nothing from baize
    completeSets = findAllCompleteSetsInHand();
    if (completeSets.count() > 0)
    {
        AiModelTurnMove turnMove;
        turnMove.moveType = AiModelTurnMove::PlayCompleteSetFromHand;
        turnMove.cardGroup = completeSets.at(cardDeck().random_int(completeSets.count() - 1));
        turnMoves.append(turnMove);
        emit madeTurn(turnMoves);
        return;
    }

    // find all 2+ partial sets in hand which can be completed from 1 free card on baize
    completeSets = findAllCompleteSetsFrom2CardsInHand();
    if (completeSets.count() > 0)
    {
        AiModelTurnMove turnMove;
        turnMove.moveType = AiModelTurnMove::PlayCompleteSetFromHandPlusBaize;
        turnMove.cardGroup = completeSets.at(cardDeck().random_int(completeSets.count() - 1));
        turnMoves.append(turnMove);
        emit madeTurn(turnMoves);
        return;
    }

    // find all 1 card in hand which can be added to existing sets on baize
    completeSets = findAllAddToCompleteSetsFrom1CardInHand();
    if (completeSets.count() > 0)
    {
        AiModelTurnMove turnMove;
        turnMove.moveType = AiModelTurnMove::PlayAddToCompleteSetOnBaizeFromHand;
        turnMove.cardGroup = completeSets.at(cardDeck().random_int(completeSets.count() - 1));
        turnMoves.append(turnMove);
        emit madeTurn(turnMoves);
        return;
    }

    // find all 1 card in hand which can be completed from 2 free cards on baize
    completeSets = findAllCompleteSetsFrom1CardInHand();
    if (completeSets.count() > 0)
    {
        AiModelTurnMove turnMove;
        turnMove.moveType = AiModelTurnMove::PlayCompleteSetFromHandPlusBaize;
        turnMove.cardGroup = completeSets.at(cardDeck().random_int(completeSets.count() - 1));
        turnMoves.append(turnMove);
        emit madeTurn(turnMoves);
        return;
    }

    emit madeTurn(turnMoves);
}
