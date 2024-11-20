#ifndef AIMODEL_H
#define AIMODEL_H

#include <QObject>

#include "logicalmodel.h"

class AiModelState
{
public:
    CardHand aiHand;
    CardGroups cardGroups;

    AiModelState();
    AiModelState(const CardHand &aiHand, const CardGroups &cardGroups);
    bool isNull() const { return (aiHand.isEmpty() && cardGroups.isEmpty()); }
};



class AiModelStates : public QList<AiModelState>
{

};



class AiModel : public QObject
{
    Q_OBJECT
public:
    explicit AiModel(QObject *parent = nullptr);

    int debugLevel() const { return _debugLevel; }
    void setDebugLevel(int level) { _debugLevel = level; }

    struct Statistics
    {
        long isGoodSetCalls;
        long aiModelStatesCreated;
    };
    static Statistics statistics;

    LogicalModel *logicalModel;

private:
    int _debugLevel;
    const CardDeck &cardDeck() const { return logicalModel->cardDeck; }
    int activePlayer() const { return logicalModel->activePlayer; }
    const CardGroups &cardGroups() const { return logicalModel->cardGroups; }
    const CardHands &hands() const { return logicalModel->hands; }
    const CardHand &aiHand() const;

    void resetStatistics();
    void showStatistics();
    QList<const Card *> freeCardsInGroup(const CardGroup &group) const;
    QList<const Card *> findAllFreeCardsInGroups(const CardGroups &groups) const;
    void removeFirstCardRankSet(CardHand &hand, CardGroup &rankSet) const;
    void removeFirstCardRunSet(CardHand &hand, CardGroup &runSet) const;
    void removeFirstCardGenerateAll2CardPartialRunSets(CardHand &hand, CardGroups &runSets) const;
    CardGroups pivotSets(CardGroups existingSets) const;
    void verifyChangedState(AiModelState initialState, AiModelState newState) const;
    void addNewSet(AiModelState &state, const CardGroup &newSet) const;
    void modifySet(AiModelState &state, const CardGroup &modifiedSet) const;
    void clearSet(AiModelState &state, const CardGroup &existingSet) const;
    void removeCardFromHand(AiModelState &state, const Card *card) const;
    void removeCardsFromHand(AiModelState &state, const QList<const Card *> &cards) const;
    void removeCardFromGroups(AiModelState &state, const Card *card) const;
    void removeCardsFromGroups(AiModelState &state, const QList<const Card *> &cards) const;
    void removeCardsFromOneGroup(AiModelState &state, const QList<const Card *> &cards) const;
    AiModelStates findAllCompleteRankSetsInHand(const AiModelState &initialState) const;
    AiModelStates findAllCompleteRunSetsInHand(const AiModelState &initialState) const;
    AiModelStates findAllPartialRankSetsFrom2CardsInHand(const AiModelState &initialState) const;
    AiModelStates findAllPartialRunSetsFrom2CardsInHand(const AiModelState &initialState) const;
    AiModelStates rearrangeBrokenSetOnBaizeToOtherSets(const AiModelState &initialState, const CardGroup &brokenSet) const;
    AiModelStates completePartialRankSetFrom1CardOnBaizeWithRearrangement(const AiModelState &initialState) const;
    AiModelStates completePartialRunSetFrom1CardOnBaizeWithRearrangement(const AiModelState &initialState) const;
    AiModelStates completePartialRankSetFrom1CardOnBaize(const AiModelState &initialState) const;
    AiModelStates completePartialRunSetFrom1CardOnBaize(const AiModelState &initialState) const;
    AiModelStates findAllCompleteRankSetsFrom2CardsInHand(const AiModelState &initialState) const;
    AiModelStates findAllCompleteRunSetsFrom2CardsInHand(const AiModelState &initialState) const;
    AiModelStates findAllAddToSetsFrom1CardInHand(const AiModelState &initialState) const;
    AiModelStates findAllCompleteSetsInHand(const AiModelState &initialState) const;
    AiModelStates findAllCompleteSetsFrom2CardsInHand(const AiModelState &initialState) const;
    AiModelStates findAllAddToCompleteSetsFrom1CardInHand(const AiModelState &initialState) const;
    AiModelStates findAllCompleteSetsFrom1CardInHand(const AiModelState &initialState) const;
    AiModelStates findAllMakeNewSetsFrom1CardInHand(const AiModelState &initialState) const;
    AiModelState findOneSimpleTurnPlay(const AiModelState &initialState, int depth) const;
    AiModelState searchEquivalent1FreeCardMoveStates(const AiModelState &initialState, int depth) const;
    AiModelState searchEquivalent1JoinSetsStates(const AiModelState &initialState, int depth) const;
    AiModelState searchEquivalentInitialStates(const AiModelState &initialState, int depth) const;
    AiModelState searchEquivalent1SplitSetsStates(const AiModelState &initialState, int depth) const;
    AiModelState searchEquivalent3RearrangeSetsStates(const AiModelState &initialState, int depth) const;
    AiModelState findOneComplexTurnPlay(const AiModelState &initialState, int depth) const;
    AiModelState findOneTurnPlay() const;

public slots:
    void makeTurn();

signals:
    void makeTurnPlay(AiModelState turnPlay);
};

#endif // AIMODEL_H
