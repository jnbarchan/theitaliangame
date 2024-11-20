#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QJsonDocument>
#include <QMainWindow>
#include <QPlainTextEdit>

#include "logicalmodel.h"
#include "aimodel.h"

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
    bool autoEndTurnOnDrawCard;
    bool haveDrawnCard;
    bool aiContinuousPlay, _aiContinuousPlayFast;
    bool aiContinuousPlayFast() const { return aiContinuousPlay && _aiContinuousPlayFast; }

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
    const QString appSavesPath();
    void setupUi();
    void showHands();
    void handCardLayoutInfo(int player, int handCount, MainWindow::HandCardLayoutInfo &hcli) const;
    QRectF handAreaRect(int player) const;
    void showHand(int player, bool enforceCorrectStacking = false);
    void hideHand(int player);
    void shuffleAndDeal();
    void showInitialFreeCards();
    void sortAndShow();
    int findCardInHandArea(const CardPixmapItem *item) const;
    void verifyInitialGroups();
    void tidyGroups(bool verifyNoBadBads = false);
    bool havePlayedCard() const;
    void startTurn(bool restart = false);
    void autosave();
    QPointF findFreeAreaForCardGroup(const CardGroup &cardGroup) const;
    void reportDealIsOver(int winner);
    void aiModelMakePlays(const AiModelState &turnPlay);
    QJsonDocument serializeToJson() const;
    void deserializeFromJson(const QJsonDocument &doc);

private slots:
    void baizeViewCoordinatesChanged();
    void baizeSceneSingleCardMoved(CardPixmapItem *item);
    void baizeSceneMultipleCardsMoved(QList<CardPixmapItem *> items);
    void drawCardFromDrawPile();
    void extractCardFromDrawPile(int id);
    void aiModelMakeTurnPlay(AiModelState turnPlay);
    void actionAiContinuousPlay(bool checked);
    void actionHandLayout(HandLayout handLayout);
    void actionLoadFile();
    void actionSaveFile();
    void actionDeal();
    void actionRestartTurn();
    void updateDrawCardEndTurnAction();
    void actionDrawCardEndTurn();

signals:
    void aiModelMakeTurn();
};

#endif
