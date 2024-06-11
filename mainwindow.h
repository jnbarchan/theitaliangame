#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QJsonDocument>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QSet>

#include "carddeck.h"
#include "cardhand.h"
#include "cardgroup.h"

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
    bool havePlayedCard, haveDrawnCard;
    static constexpr int totalPlayers = 2;
    int activePlayer;
    enum HandLayout { HandLayoutHorizontal, HandLayoutHorizontalGapBetweenSuits, HandLayoutFan };
    HandLayout handLayout;
    BaizeScene *baizeScene;
    BaizeView *baizeView;
    QMenu *mainMenu;
    QAction *menuActionDrawCardEndTurn;
    QString _appRootPath;
    CardDeck cardDeck;
    CardHands hands;
    CardGroups cardGroups;
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
    void dealInitialFreeCards();
    void sortAndShow();
    int findCardInHandArea(const CardPixmapItem *item) const;
    bool isInitialCardGroup(const CardGroup &group) const;
    CardGroups badSetGroups() const;
    void tidyGroups();
    QJsonDocument serializeToJson() const;
    void deserializeFromJson(const QJsonDocument &doc);

private slots:
    void baizeViewCoordinatesChanged();
    void baizeSceneCardMoved(CardPixmapItem *item);
    void takeCardFromDrawPile();
    void actionHandLayout(HandLayout handLayout);
    void actionDeal();
    void actionRestartTurn();
    void updateDrawCardEndTurnAction();
    void actionDrawCardEndTurn();
};

#endif
