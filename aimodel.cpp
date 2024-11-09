#include "utils.h"
#include "aimodel.h"

AiModel::AiModel(QObject *parent)
    : QObject{parent}
{
}

const CardHand &AiModel::aiHand() const
{
    const CardHands &hands(this->hands());
    Q_ASSERT(hands.isAiPlayer(activePlayer()));
    return hands.at(activePlayer());
}


void AiModel::verifyChangedState(AiModelState initialState, AiModelState newState) const
{
    QList<const Card *> initialCards(initialState.aiHand);
    QList<const Card *> newStateCards(newState.aiHand);
    initialCards.append(initialState.cardGroups.allCards());
    newStateCards.append(newState.cardGroups.allCards());
    while (!initialCards.isEmpty())
    {
        if (!newStateCards.removeOne(initialCards.first()))
            Q_ASSERT(false);
        initialCards.removeFirst();
    }
    Q_ASSERT(initialCards.isEmpty());
    Q_ASSERT(newStateCards.isEmpty());
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
        if (isGoodSet)
            if (group.count() > 3)
            {
                if (setType == CardGroup::RankSet)
                    freeCards.append(group);
                else if (setType == CardGroup::RunSet)
                    freeCards.append({group.at(0), group.at(group.count() - 1)});
            }
    }
    return freeCards;
}

QList<const Card *> AiModel::findAllFreeCardsInGroups(const CardGroups &groups) const
{
    QList<const Card *> freeCards;
    for (const CardGroup &group : groups)
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


AiModelStates AiModel::findAllCompleteRankSetsInHand(const AiModelState &initialState) const
{
    AiModelStates newStates;
    CardHand hand(initialState.aiHand);
    while (!hand.isEmpty())
    {
        CardGroup rankSet;
        removeFirstCardRankSet(hand, rankSet);
        if (rankSet.count() >= 3)
        {
            AiModelState newState(initialState);
            newState.aiHand.removeCards(rankSet);
            newState.cardGroups.append(rankSet);
            verifyChangedState(initialState, newState);
            newStates.append(newState);
        }
    }
    return newStates;
}

AiModelStates AiModel::findAllCompleteRunSetsInHand(const AiModelState &initialState) const
{
    AiModelStates newStates;
    CardHand hand(initialState.aiHand);
    while (!hand.isEmpty())
    {
        CardGroup runSet;
        removeFirstCardRunSet(hand, runSet);
        if (runSet.count() >= 3)
        {
            AiModelState newState(initialState);
            newState.aiHand.removeCards(runSet);
            newState.cardGroups.append(runSet);
            verifyChangedState(initialState, newState);
            newStates.append(newState);
        }
    }
    return newStates;
}


AiModelStates AiModel::findAllPartialRankSetsFrom2CardsInHand(const AiModelState &initialState) const
{
    AiModelStates newStates;
    CardHand hand(initialState.aiHand);
    while (!hand.isEmpty())
    {
        CardGroup rankSet;
        removeFirstCardRankSet(hand, rankSet);
        if (rankSet.count() == 2)
        {
            AiModelState newState(initialState);
            newState.aiHand.removeCards(rankSet);
            newState.cardGroups.append(rankSet);
            verifyChangedState(initialState, newState);
            newStates.append(newState);
        }
    }
    return newStates;
}

AiModelStates AiModel::findAllPartialRunSetsFrom2CardsInHand(const AiModelState &initialState) const
{
    AiModelStates newStates;
    CardHand hand(initialState.aiHand);
    while (!hand.isEmpty())
    {
        CardGroups firstCardRunSets;
        removeFirstCardGenerateAll2CardPartialRunSets(hand, firstCardRunSets);
        for (const CardGroup &runSet : firstCardRunSets)
        {
            Q_ASSERT(runSet.count() == 2);
            AiModelState newState(initialState);
            newState.aiHand.removeCards(runSet);
            newState.cardGroups.append(runSet);
            verifyChangedState(initialState, newState);
            newStates.append(newState);
        }
    }
    return newStates;
}

AiModelStates AiModel::completePartialRankSetFrom1CardOnBaize(const AiModelState &initialState) const
{
    AiModelStates newStates;
    const CardGroup &partialSet(initialState.cardGroups.last());
    Q_ASSERT(partialSet.count() == 2);
    Q_ASSERT(partialSet.at(0)->rank() == partialSet.at(1)->rank());
    Q_ASSERT(partialSet.at(0)->suit() != partialSet.at(1)->suit());
    QList<const Card *> freeCards = findAllFreeCardsInGroups(initialState.cardGroups);
    for (const Card *freeCard : freeCards)
    {
        if (partialSet.contains(freeCard))
            continue;
        if (freeCard->rank() == partialSet.at(0)->rank())
        {
            CardGroup rankSet(partialSet);
            rankSet.append(freeCard);
            if (rankSet.isGoodSet())
            {
                AiModelState newState(initialState);
                Q_ASSERT(newState.cardGroups.last() == partialSet);
                newState.cardGroups.removeCardFromGroups(freeCard);
                newState.cardGroups.replace(newState.cardGroups.count() - 1, rankSet);
                verifyChangedState(initialState, newState);
                newStates.append(newState);
            }
        }
    }
    return newStates;
}

AiModelStates AiModel::completePartialRunSetFrom1CardOnBaize(const AiModelState &initialState) const
{
    AiModelStates newStates;
    const CardGroup &partialSet(initialState.cardGroups.last());
    Q_ASSERT(partialSet.count() == 2);
    Q_ASSERT(partialSet.at(0)->suit() == partialSet.at(1)->suit());
    int rankDifference = qAbs(partialSet.rankDifference(partialSet.at(0)->rank(), partialSet.at(1)->rank()));
    Q_ASSERT(rankDifference > 0 && rankDifference <= 2);
    QList<const Card *> freeCards = findAllFreeCardsInGroups(initialState.cardGroups);
    for (const Card *freeCard : freeCards)
    {
        if (partialSet.contains(freeCard))
            continue;
        if (freeCard->suit() == partialSet.at(0)->suit())
        {
            CardGroup runSet(partialSet);
            runSet.append(freeCard);
            runSet.rearrangeForSets();
            if (runSet.isGoodSet())
            {
                AiModelState newState(initialState);
                Q_ASSERT(newState.cardGroups.last() == partialSet);
                newState.cardGroups.removeCardFromGroups(freeCard);
                newState.cardGroups.replace(newState.cardGroups.count() - 1, runSet);
                verifyChangedState(initialState, newState);
                newStates.append(newState);
            }
        }
    }
    return newStates;
}

AiModelStates AiModel::findAllCompleteRankSetsFrom2CardsInHand(const AiModelState &initialState) const
{
    AiModelStates newStates;
    AiModelStates rankPartialSetStates = findAllPartialRankSetsFrom2CardsInHand(initialState);
    for (const AiModelState &rankPartialSetState : rankPartialSetStates)
    {
        AiModelStates completeSets = completePartialRankSetFrom1CardOnBaize(rankPartialSetState);
        newStates.append(completeSets);
    }
    return newStates;
}

AiModelStates AiModel::findAllCompleteRunSetsFrom2CardsInHand(const AiModelState &initialState) const
{
    AiModelStates newStates;
    AiModelStates runPartialSetStates = findAllPartialRunSetsFrom2CardsInHand(initialState);
    for (const AiModelState &runPartialSetState : runPartialSetStates)
    {
        AiModelStates completeSets = completePartialRunSetFrom1CardOnBaize(runPartialSetState);
        newStates.append(completeSets);
    }
    return newStates;
}


AiModelStates AiModel::findAllAddToSetsFrom1CardInHand(const AiModelState &initialState) const
{
    AiModelStates newStates;
    CardHand hand(initialState.aiHand);
    while (!hand.isEmpty())
    {
        const Card *card = hand.takeFirst();
        for (const CardGroup &existingSet : initialState.cardGroups)
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
                AiModelState newState(initialState);
                newState.aiHand.removeOne(card);
                int newCardGroupIndex = newState.cardGroups.indexOf(existingSet);
                Q_ASSERT(newCardGroupIndex >= 0);
                newState.cardGroups.replace(newCardGroupIndex, newSet);
                verifyChangedState(initialState, newState);
                newStates.append(newState);
            }
        }
    }
    return newStates;
}


AiModelStates AiModel::findAllMakeNewSetsFrom1CardInHand(const AiModelState &initialState) const
{
    AiModelStates newStates;
    CardHand hand(initialState.aiHand);
    while (!hand.isEmpty())
    {
        const Card *card0 = hand.takeFirst();
        for (const CardGroup &existingSet1 : initialState.cardGroups)
        {
            if (existingSet1.count() < 4)
                continue;
            CardGroup::SetType setType;
            bool isGoodSet = existingSet1.isGoodSet(setType);

            if (existingSet1.count() >= 5 && isGoodSet)
            {
                Q_ASSERT(setType == CardGroup::RunSet);
                if (existingSet1.at(0)->suit() == card0->suit())
                {
                    CardGroup newSet = {card0, existingSet1.at(0), existingSet1.at(1)};
                    newSet.rearrangeForSets();
                    if (newSet.isGoodSet())
                    {
                        AiModelState newState(initialState);
                        newState.aiHand.removeOne(card0);
                        newState.cardGroups.removeCardFromGroups(existingSet1.at(0));
                        newState.cardGroups.removeCardFromGroups(existingSet1.at(1));
                        newState.cardGroups.append(newSet);
                        verifyChangedState(initialState, newState);
                        newStates.append(newState);
                    }
                    newSet = {card0, existingSet1.at(existingSet1.count() - 1), existingSet1.at(existingSet1.count() - 2)};
                    newSet.rearrangeForSets();
                    if (newSet.isGoodSet())
                    {
                        AiModelState newState(initialState);
                        newState.aiHand.removeOne(card0);
                        newState.cardGroups.removeCardFromGroups(existingSet1.at(existingSet1.count() - 1));
                        newState.cardGroups.removeCardFromGroups(existingSet1.at(existingSet1.count() - 2));
                        newState.cardGroups.append(newSet);
                        verifyChangedState(initialState, newState);
                        newStates.append(newState);
                    }
                }
            }

            QList<const Card *> freeCards1(freeCardsInGroup(existingSet1));
            for (const Card *card1 : freeCards1)
            {
                if (card1->rank() != card0->rank() && card1->suit() != card0->suit())
                    continue;
                for (const CardGroup &existingSet2 : initialState.cardGroups)
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
                            AiModelState newState(initialState);
                            newState.aiHand.removeOne(card0);
                            newState.cardGroups.removeCardFromGroups(card1);
                            newState.cardGroups.removeCardFromGroups(card2);
                            newState.cardGroups.append(newSet);
                            verifyChangedState(initialState, newState);
                            newStates.append(newState);
                        }
                    }
                }
            }
        }
    }
    return newStates;
}


AiModelStates AiModel::findAllCompleteSetsInHand(const AiModelState &initialState) const
{
    AiModelStates completeSets;
    AiModelStates rankSets = findAllCompleteRankSetsInHand(initialState);
    AiModelStates runSets = findAllCompleteRunSetsInHand(initialState);
    completeSets.append(rankSets);
    completeSets.append(runSets);
    qDebug() << __FUNCTION__ << "Complete sets count:" << completeSets.count();
    return completeSets;
}

AiModelStates AiModel::findAllCompleteSetsFrom2CardsInHand(const AiModelState &initialState) const
{
    AiModelStates completeSets;
    AiModelStates rankSets = findAllCompleteRankSetsFrom2CardsInHand(initialState);
    AiModelStates runSets = findAllCompleteRunSetsFrom2CardsInHand(initialState);
    completeSets.append(rankSets);
    completeSets.append(runSets);
    qDebug() << __FUNCTION__  << "Complete sets count:" << completeSets.count();
    return completeSets;
}

AiModelStates AiModel::findAllAddToCompleteSetsFrom1CardInHand(const AiModelState &initialState) const
{
    AiModelStates completeSets = findAllAddToSetsFrom1CardInHand(initialState);
    qDebug() << __FUNCTION__ << "Complete sets count:" << completeSets.count();
    return completeSets;
}

AiModelStates AiModel::findAllCompleteSetsFrom1CardInHand(const AiModelState &initialState) const
{
    AiModelStates completeSets = findAllMakeNewSetsFrom1CardInHand(initialState);
    qDebug() << __FUNCTION__ << "Complete sets count:" << completeSets.count();
    return completeSets;
}


AiModelState AiModel::findOneSimpleTurnPlay(const AiModelState &initialState, int depth) const
{
    AiModelStates turnPlays;
    AiModelState turnPlay;

    if (depth == 0)
    {
        // find all 3+ complete sets in hand, nothing from baize
        turnPlays = findAllCompleteSetsInHand(initialState);
        if (!turnPlays.isEmpty())
        {
            turnPlay = RandomNumber::random_element(turnPlays);
            Q_ASSERT(!turnPlay.isNull());
            return turnPlay;
        }
    }

    // find all 2+ partial sets in hand which can be completed from 1 free card on baize
    turnPlays = findAllCompleteSetsFrom2CardsInHand(initialState);
    if (!turnPlays.isEmpty())
    {
        turnPlay = RandomNumber::random_element(turnPlays);
        Q_ASSERT(!turnPlay.isNull());
        return turnPlay;
    }

    // find all 1 card in hand which can be added to existing sets on baize
    turnPlays = findAllAddToCompleteSetsFrom1CardInHand(initialState);
    if (!turnPlays.isEmpty())
    {
        turnPlay = RandomNumber::random_element(turnPlays);
        Q_ASSERT(!turnPlay.isNull());
        return turnPlay;
    }

    // find all 1 card in hand which can be completed from 2 free cards on baize
    turnPlays = findAllCompleteSetsFrom1CardInHand(initialState);
    if (!turnPlays.isEmpty())
    {
        turnPlay = RandomNumber::random_element(turnPlays);
        Q_ASSERT(!turnPlay.isNull());
        return turnPlay;
    }

    return {};
}

AiModelState AiModel::findOneComplexTurnPlay(const AiModelState &initialState, int depth) const
{
    AiModelState turnPlay;

    // // foreach rearrangement
    // turnPlay = findOneSimpleTurnPlay(initialState, depth + 1);
    // if (!turnPlay.isNull())
    //     return turnPlay;

    return {};
}

AiModelState AiModel::findOneTurnPlay() const
{
    const AiModelState initialState(aiHand(), cardGroups());
    AiModelState turnPlay;

    turnPlay = findOneSimpleTurnPlay(initialState, 0);
    if (!turnPlay.isNull())
        return turnPlay;

    turnPlay = findOneComplexTurnPlay(initialState, 0);
    if (!turnPlay.isNull())
        return turnPlay;

    return {};
}


/*slot*/ void AiModel::makeTurn()
{
    Q_ASSERT(hands().isAiPlayer(activePlayer()));

    AiModelState turnPlay = findOneTurnPlay();

    emit makeTurnPlay(turnPlay);
}
