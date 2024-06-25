#ifndef AIMODEL_H
#define AIMODEL_H

#include <QObject>

#include "logicalmodel.h"

class AiModelTurnMove
{
public:
    enum MoveType { PlayCompleteSetFromHand, PlayCompleteSetFromHandPlusBaize, };
    MoveType moveType;
    CardGroup cardGroup;
};



class AiModelTurnMoves : public QList<AiModelTurnMove>
{

};



class AiModel : public QObject
{
    Q_OBJECT
public:
    explicit AiModel(QObject *parent = nullptr);

    LogicalModel *logicalModel;

    CardHand &aiHand() const;

private:
    int &activePlayer() const { return logicalModel->activePlayer; }
    CardDeck &cardDeck() const { return logicalModel->cardDeck; }
    CardHands &hands() const { return logicalModel->hands; }
    CardGroups &cardGroups() const { return logicalModel->cardGroups; }
    QList<const Card *> findAllFreeCardsOnBaize() const;
    void removeFirstCardRankSet(CardHand &hand, CardGroup &rankSet) const;
    void removeFirstCardRunSet(CardHand &hand, CardGroup &runSet) const;
    void removeFirstCardGenerateAll2CardPartialRunSets(CardHand &hand, CardGroups &runSets) const;
    CardGroups findAllCompleteRankSetsInHand() const;
    CardGroups findAllCompleteRunSetsInHand() const;
    CardGroups findAllPartialRankSetsFrom2CardsInHand() const;
    CardGroups findAllPartialRunSetsFrom2CardsInHand() const;
    CardGroups completeAllPartialRankSetsFrom1CardOnBaize(const CardGroups &rankPartialSets) const;
    CardGroups completeAllPartialRunSetsFrom1CardOnBaize(const CardGroups &runPartialSets) const;
    CardGroups findAllCompleteRankSetsFrom2CardsInHand() const;
    CardGroups findAllCompleteRunSetsFrom2CardsInHand() const;
    CardGroups findAllCompleteSetsInHand() const;
    CardGroups findAllCompleteSetsFrom2CardsInHand() const;

public slots:
    void makeTurn();

signals:
    void madeTurn(AiModelTurnMoves turnMoves);
};

#endif // AIMODEL_H
