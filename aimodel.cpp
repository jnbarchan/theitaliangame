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


QList<const Card *> AiModel::freeCardsInGroup(const CardGroup &group) const
{
    QList<const Card *> freeCards;
    if (logicalModel->isInitialCardGroup(group))
        freeCards.append(group);
    else
    {
        CardGroup::SetType setType;
        if (group.isGoodSet(setType))
            if (group.count() > 3)
            {
                if (setType == CardGroup::RankSet)
                    freeCards.append(group);
                else if (setType == CardGroup::RunSet)
                    freeCards.append({group.first(), group.last()});
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
    const Card *card0 = rankSet.first();
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
    const Card *card0 = runSet.first(), *card1 = runSet.last();
    for (int i = 0; i < hand.count(); i++)
    {
        const Card *card = hand.at(i);
        if (card->suit() != card0->suit())
            continue;
        if (runSet.rankDifference(card->rank(), card0->rank()) == 1)
        {
            runSet.prepend(hand.takeAt(i));
            card0 = runSet.first();
            i = -1;
        }
        else if (runSet.rankDifference(card->rank(), card1->rank()) == -1)
        {
            runSet.append(hand.takeAt(i));
            card1 = runSet.last();
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


CardGroups AiModel::pivotSets(CardGroups existingSets) const
{
    // "pivot" the card groups, so that we make new groups by combining each first card in each set
    // to make a new set, and same for the second, third... card in each set
    // this can be used to try making a group of run sets into ranks sets or vice versa

    CardGroups newSets;
    for (CardGroup &existingSet : existingSets)
    {
        Q_ASSERT(existingSet.count() == existingSets.first().count());
        // ensure consistently sorted so that we produce right "pairing"
        std::sort(existingSet.begin(), existingSet.end(), Card::compareForSortBySuit);
        for (int i = 0; i < existingSet.count(); i++)
        {
            if (newSets.count() < i + 1)
                newSets.resize(i + 1);
            newSets[i].append(existingSet.at(i));
        }
    }
    return newSets;
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


void AiModel::addNewSet(AiModelState &state, const CardGroup &newSet) const
{
    int index = state.cardGroups.findCardGroupByUniqueId(newSet.uniqueId());
    Q_ASSERT(index < 0);
    state.cardGroups.append(newSet);
}

void AiModel::modifySet(AiModelState &state, const CardGroup &modifiedSet) const
{
    int index = state.cardGroups.findCardGroupByUniqueId(modifiedSet.uniqueId());
    Q_ASSERT(index >= 0);
    state.cardGroups.replace(index, modifiedSet);
}

void AiModel::removeSet(AiModelState &state, const CardGroup &existingSet) const
{
    int index = state.cardGroups.findCardGroupByUniqueId(existingSet.uniqueId());
    Q_ASSERT(index >= 0);
    state.cardGroups.removeAt(index);
}


void AiModel::removeCardFromHand(AiModelState &state, const Card *card) const
{
    if (!state.aiHand.removeOne(card))
        Q_ASSERT(false);
}

void AiModel::removeCardsFromHand(AiModelState &state, const QList<const Card *> &cards) const
{
    for (const Card *card : cards)
        removeCardFromHand(state, card);
}

void AiModel::removeCardFromGroups(AiModelState &state, const Card *card) const
{
    int index = state.cardGroups.removeCardFromGroups(card);
    Q_ASSERT(index >= 0);
}

void AiModel::removeCardsFromGroups(AiModelState &state, const QList<const Card *> &cards) const
{
    for (const Card *card : cards)
        removeCardFromGroups(state, card);
}

void AiModel::removeCardsFromOneGroup(AiModelState &state, const QList<const Card *> &cards) const
{
    if (cards.isEmpty())
        return;
    int index = state.cardGroups.removeCardFromGroups(cards.at(0));
    Q_ASSERT(index >= 0);
    for (int i = 1; i < cards.count(); i++)
        if (!state.cardGroups[index].removeOne(cards.at(i)))
            Q_ASSERT(false);
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
            removeCardsFromHand(newState, rankSet);
            addNewSet(newState, rankSet);
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
            removeCardsFromHand(newState, runSet);
            addNewSet(newState, runSet);
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
            removeCardsFromHand(newState, rankSet);
            addNewSet(newState, rankSet);
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
            removeCardsFromHand(newState, runSet);
            addNewSet(newState, runSet);
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
            if (rankSet.isGoodRankSet())
            {
                AiModelState newState(initialState);
                removeCardFromGroups(newState, freeCard);
                modifySet(newState, rankSet);
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
            if (runSet.isGoodRunSet())
            {
                AiModelState newState(initialState);
                removeCardFromGroups(newState, freeCard);
                modifySet(newState, runSet);
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
            if (existingSet.first()->rank() != card->rank() && existingSet.first()->suit() != card->suit())
                continue;
            CardGroup newSet(existingSet);
            newSet.append(card);
            newSet.rearrangeForSets();
            if (newSet.isGoodSet())
            {
                AiModelState newState(initialState);
                removeCardFromHand(newState, card);
                modifySet(newState, newSet);
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
            if (existingSet1.count() > 1 && existingSet1.count() < 4)
                continue;
            CardGroup::SetType setType;
            bool isGoodSet1 = existingSet1.isGoodSet(setType);

            if (existingSet1.count() >= 5 && isGoodSet1)
            {
                Q_ASSERT(setType == CardGroup::RunSet);
                if (existingSet1.at(0)->suit() == card0->suit())
                    for (int pass = 0; pass < 2; pass++)
                    {
                        const Card *card1 = (pass == 0) ? existingSet1.at(0) : existingSet1.at(existingSet1.count() - 1);
                        const Card *card2 = (pass == 0) ? existingSet1.at(1) : existingSet1.at(existingSet1.count() - 2);
                        CardGroup newSet = {card0, card1, card2};
                        newSet.rearrangeForSets();
                        if (newSet.isGoodRunSet())
                        {
                            AiModelState newState(initialState);
                            removeCardFromHand(newState, card0);
                            removeCardsFromOneGroup(newState, {card1, card2});
                            addNewSet(newState, newSet);
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
                            removeCardFromHand(newState, card0);
                            removeCardFromGroups(newState, card1);
                            removeCardFromGroups(newState, card2);
                            addNewSet(newState, newSet);
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


AiModelState AiModel::searchEquivalent1FreeCardMoveStates(const AiModelState &initialState, int depth) const
{
    AiModelState turnPlay;

    // for each free card, move to each other (complete) set and search again
    QList<const Card *> freeCards = findAllFreeCardsInGroups(initialState.cardGroups);
    for (const Card *freeCard : freeCards)
        for (const CardGroup &existingSet : initialState.cardGroups)
        {
            if (existingSet.contains(freeCard))
                continue;
            if (existingSet.count() < 2)
                continue;
            if (existingSet.first()->rank() != freeCard->rank() && existingSet.first()->suit() != freeCard->suit())
                continue;
            CardGroup newSet(existingSet);
            newSet.append(freeCard);
            newSet.rearrangeForSets();
            if (newSet.isGoodSet())
            {
                AiModelState newState(initialState);
                removeCardFromGroups(newState, freeCard);
                modifySet(newState, newSet);
                verifyChangedState(initialState, newState);

                turnPlay = findOneSimpleTurnPlay(newState, depth + 1);
                if (!turnPlay.isNull())
                    return turnPlay;
            }
        }

    return {};
}

AiModelState AiModel::searchEquivalent1JoinSetsStates(const AiModelState &initialState, int depth) const
{
    AiModelState turnPlay;

    // for each complete run set, join onto each other complete run set and search again
    for (const CardGroup &existingSet1 : initialState.cardGroups)
    {
        if (existingSet1.count() < 2)
            continue;
        if (!existingSet1.isGoodRunSet())
            continue;
        for (const CardGroup &existingSet2 : initialState.cardGroups)
        {
            if (existingSet2 == existingSet1)
                continue;
            if (existingSet2.count() < 2)
                continue;
            if (!existingSet2.isGoodRunSet())
                continue;
            if (existingSet2.first()->suit() != existingSet1.first()->suit())
                continue;
            CardGroup newSet(existingSet1);
            newSet.append(existingSet2);
            newSet.rearrangeForSets();
            if (newSet.isGoodRunSet())
            {
                AiModelState newState(initialState);
                removeCardsFromOneGroup(newState, existingSet2);
                modifySet(newState, newSet);
                verifyChangedState(initialState, newState);

                turnPlay = findOneSimpleTurnPlay(newState, depth + 1);
                if (!turnPlay.isNull())
                    return turnPlay;
            }
        }
    }

    return {};
}

AiModelState AiModel::searchEquivalent1SplitSetsStates(const AiModelState &initialState, int depth) const
{
    AiModelState turnPlay;

    // for each long complete run set, split into each other short complete run set and search again
    for (const CardGroup &existingSet : initialState.cardGroups)
    {
        if (existingSet.count() < 6)
            continue;
        if (!existingSet.isGoodRunSet())
            continue;
        for (int splitIndex = 3; splitIndex <= existingSet.count() - 3; splitIndex++)
        {
            CardGroup existingSet1(existingSet);
            CardGroup newSet;
            while (splitIndex < existingSet1.count())
                newSet.append(existingSet1.takeAt(splitIndex));
            Q_ASSERT(existingSet1.isGoodRunSet());
            Q_ASSERT(newSet.isGoodRunSet());
            if (existingSet1.isGoodRunSet() && newSet.isGoodRunSet())
            {
                AiModelState newState(initialState);
                modifySet(newState, existingSet1);
                addNewSet(newState, newSet);
                verifyChangedState(initialState, newState);

                turnPlay = findOneSimpleTurnPlay(newState, depth + 1);
                if (!turnPlay.isNull())
                    return turnPlay;
            }
        }
    }

    return {};
}

AiModelState AiModel::searchEquivalent3RearrangeSetsStates(const AiModelState &initialState, int depth) const
{
    AiModelState turnPlay;

    // for each 3 complete sets which are all rank (of consecutive values) or run (of same ranks)
    // rearrange them to make 3 complete sets of runs (if was ranks) or ranks (if was runs)
    // and search again
    for (const CardGroup &existingSet1 : initialState.cardGroups)
    {
        if (existingSet1.count() != 3)
            continue;
        CardGroup::SetType setType1;
        if (!existingSet1.isGoodSet(setType1))
            continue;
        for (const CardGroup &existingSet2 : initialState.cardGroups)
        {
            if (existingSet2 == existingSet1)
                continue;
            if (existingSet2.count() != existingSet1.count())
                continue;
            if (!existingSet2.isGoodSetOfType(setType1))
                continue;
            for (const CardGroup &existingSet3 : initialState.cardGroups)
            {
                if (existingSet3 == existingSet1 || existingSet3 == existingSet2)
                    continue;
                if (existingSet3.count() != existingSet1.count())
                    continue;
                if (!existingSet3.isGoodSetOfType(setType1))
                    continue;
                CardGroups existingSets(CardGroups({existingSet1, existingSet2, existingSet3}));
                CardGroups newSets = pivotSets(existingSets);
                Q_ASSERT(newSets.count() == existingSets.count());
                bool badSet = false;
                for (CardGroup &newSet : newSets)
                {
                    newSet.rearrangeForSets();
                    if (!newSet.isGoodSet())
                        badSet = true;
                }
                if (!badSet)
                {
                    AiModelState newState(initialState);
                    for (const CardGroup &existingSet : existingSets)
                        removeSet(newState, existingSet);
                    for (const CardGroup &newSet : newSets)
                        addNewSet(newState, newSet);
                    verifyChangedState(initialState, newState);

                    turnPlay = findOneSimpleTurnPlay(newState, depth + 1);
                    if (!turnPlay.isNull())
                        return turnPlay;
                }
            }
        }
    }

    return {};
}


AiModelState AiModel::searchEquivalentInitialStates(const AiModelState &initialState, int depth) const
{
    AiModelState turnPlay;

    // for each free card, move to each other (complete) set and search again
    turnPlay = searchEquivalent1FreeCardMoveStates(initialState, depth);
    if (!turnPlay.isNull())
        return turnPlay;

    // for each complete run set, join onto each other complete run set and search again
    turnPlay = searchEquivalent1JoinSetsStates(initialState, depth);
    if (!turnPlay.isNull())
        return turnPlay;

    // for each long complete run set, split into each other short complete run set and search again
    turnPlay = searchEquivalent1SplitSetsStates(initialState, depth);
    if (!turnPlay.isNull())
        return turnPlay;

    // for each 3 complete sets which are all rank (of consecutive values) or run (of same ranks)
    // rearrange them to make 3 complete sets of runs (if was ranks) or ranks (if was runs)
    // and search again
    turnPlay = searchEquivalent3RearrangeSetsStates(initialState, depth);
    if (!turnPlay.isNull())
        return turnPlay;

    return {};
}


AiModelState AiModel::findOneComplexTurnPlay(const AiModelState &initialState, int depth) const
{
    AiModelState turnPlay;

    // rearrange initial state to equivalents and search in them
    turnPlay = searchEquivalentInitialStates(initialState, depth);
    if (!turnPlay.isNull())
        return turnPlay;

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
