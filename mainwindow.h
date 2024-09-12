#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QJsonDocument>
#include <QMainWindow>
#include <QPlainTextEdit>

#include "logicalmodel.h"
#include "aimodel.h"
// #include "baizescene.h"
// #include "baizeview.h"
class BaizeScene;
class BaizeView;
class CardPixmapItem;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    virtual void closeEvent(QCloseEvent *event) override;

private:
    bool fixHandsToView;
    bool autoEndTurn;
    bool haveDrawnCard;

    LogicalModel logicalModel;
    int &activePlayer = logicalModel.activePlayer;
    CardDeck &cardDeck = logicalModel.cardDeck;
    CardHands &hands = logicalModel.hands;
    CardGroups &cardGroups = logicalModel.cardGroups;
    AiModel aiModel;

    enum HandLayout { HandLayoutHorizontal, HandLayoutHorizontalGapBetweenSuits, HandLayoutFan };
    HandLayout handLayout;
    BaizeScene *baizeScene;
    BaizeView *baizeView;
    QMenu *mainMenu;
    QAction *menuActionDrawCardEndTurn;
    QString _appRootPath;
    QJsonDocument serializationDoc;

    struct HandCardLayoutInfo
    {
        QPixmap cardPixmap;
        int cardWidth, cardHeight;
        int xBetweenCards, yHandHeight;
        qreal midHandIndex;
        QRectF sceneRectForHands;
        int xPos, yPos;
        int xPlayerNamePos, yPlayerNamePos;
    };

    const QString &appRootPath();
    void setupUi();
    void clearHands();
    void sortHands();
    void sortHand(CardHand &hand);
    void showHands();
    void handCardLayoutInfo(int player, int handCount, MainWindow::HandCardLayoutInfo &hcli) const;
    QRectF handAreaRect(int player) const;
    void showHand(int player, bool enforceCorrectStacking = false);
    void hideHand(int player);
    void shuffleAndDeal();
    void showInitialFreeCards();
    void sortAndShow();
    int findCardInHandArea(const CardPixmapItem *item) const;
    void tidyGroups();
    bool havePlayedCard() const;
    void startTurn(bool restart = false);
    QPointF findFreeAreaForCardGroup(const CardGroup &cardGroup) const;
    void aiModelMakeMove(const AiModelTurnMove &turnMove);
    QJsonDocument serializeToJson() const;
    void deserializeFromJson(const QJsonDocument &doc);

private slots:
    void baizeViewCoordinatesChanged();
    void baizeSceneSingleCardMoved(CardPixmapItem *item);
    void baizeSceneMultipleCardsMoved(QList<CardPixmapItem *> items);
    void takeCardFromDrawPile();
    void aiModelMadeTurn(AiModelTurnMoves turnMoves);
    void actionHandLayout(HandLayout handLayout);
    void actionDeal();
    void actionRestartTurn();
    void updateDrawCardEndTurnAction();
    void actionDrawCardEndTurn();

signals:
    void aiModelMakeTurn();
};

#endif
