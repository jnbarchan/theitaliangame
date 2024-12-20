#include <QActionGroup>
#include <QApplication>
#include <QBoxLayout>
#include <QComboBox>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QSaveFile>

#include "baizescene.h"
#include "baizeview.h"
#include "selectcardmenu.h"
#include "utils.h"

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setGeometry(100, 100, 1400, 900);  // default size

    // create the graphics scene & view
    this->baizeScene = new BaizeScene(this);
    this->baizeView = new BaizeView;

    baizeScene->setSceneRect(-1250, -1000, 2500, 2000);

    baizeView->setDragMode(QGraphicsView::RubberBandDrag);
    baizeView->setRubberBandSelectionMode(Qt::ContainsItemShape);

    setupUi();

    this->fixHandsToView = true;
    this->autoEndTurnOnDrawCard = false;
    this->aiContinuousPlay = false;
    this->_aiContinuousPlayFast = true;

    // signal connections
    connect(baizeView, &BaizeView::viewCoordinatesChanged, this, &MainWindow::baizeViewCoordinatesChanged);
    connect(baizeScene, &BaizeScene::singleCardMoved, this, &MainWindow::baizeSceneSingleCardMoved);
    connect(baizeScene, &BaizeScene::multipleCardsMoved, this, &MainWindow::baizeSceneMultipleCardsMoved);
    connect(baizeScene, &BaizeScene::drawPileDoubleClicked, this, &MainWindow::drawCardFromDrawPile);

    connect(this, &MainWindow::aiModelMakeTurn, &aiModel, &AiModel::makeTurn);
    connect(&aiModel, &AiModel::makeTurnPlay, this, &MainWindow::aiModelMakeTurnPlay);

    cardDeck.createCards();

    // hands.totalHands = 2;
    // hands.aiPlayers = { true, false };
    hands.totalHands = 1;
    hands.aiPlayers = { true };
    hands.initialHandCardCount = 13;

    this->aiModel.setDebugLevel(0);
    this->aiModel.logicalModel = &this->logicalModel;

    actionDeal();
    Q_ASSERT(!logicalModel.isDealOver(true));
}

MainWindow::~MainWindow()
{
    disconnect(baizeView, &BaizeView::viewCoordinatesChanged, this, &MainWindow::baizeViewCoordinatesChanged);
}

/*virtual*/ void MainWindow::closeEvent(QCloseEvent *event) /*override*/
{
    disconnect(baizeView, &BaizeView::viewCoordinatesChanged, this, &MainWindow::baizeViewCoordinatesChanged);
    QMainWindow::closeEvent(event);
}

const QString &MainWindow::appRootPath()
{
    // root directory for locating image files
    if (_appRootPath.isEmpty())
    {
        // look relative to where the *executable* directory is
        // if that is the right directory return that (i.e. application has been deployed)
        // if not assume it is the development "build" directory and return "../.." from there
        QDir dir(QCoreApplication::applicationDirPath());
        if (dir.exists("images"))
            _appRootPath = dir.absolutePath();
        else if (dir.cd("../.."))
            if (dir.exists("images"))
                _appRootPath = dir.absolutePath();
        Q_ASSERT(!_appRootPath.isEmpty());
    }
    return _appRootPath;
}

const QString MainWindow::appSavesPath()
{
    return appRootPath() + "/saves";
}

void MainWindow::setupUi()
{
    // set centralwidget layout
    QWidget *widget = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(widget);
    setCentralWidget(widget);

    // create the menus
    this->mainMenu = new QMenu("File", this);
    menuBar()->addMenu(mainMenu);
    QList<QAction *> actions;

    QAction *menuActionAiContinuousPlay = mainMenu->addAction("AI Continuous Play", this, &MainWindow::actionAiContinuousPlay);
    menuActionAiContinuousPlay->setCheckable(true);
    mainMenu->addSeparator();

    QMenu *handLayoutSubmenu = new QMenu("Hand Layout", mainMenu);
    QActionGroup *handLayoutGroup = new QActionGroup(handLayoutSubmenu);
    handLayoutGroup->setExclusive(true);
    actions.clear();
    actions.append(handLayoutSubmenu->addAction("Horizontal", this, [this]() { actionHandLayout(HandLayoutHorizontal); } ));
    actions.append(handLayoutSubmenu->addAction("Horizontal with Gap between Suits", this, [this]() { actionHandLayout(HandLayoutHorizontalGapBetweenSuits); } ));
    actions.append(handLayoutSubmenu->addAction("Fan", this, [this]() { actionHandLayout(HandLayoutFan); } ));
    for (QAction *action : actions)
    {
        action->setCheckable(true);
        handLayoutGroup->addAction(action);
    }
    this->handLayout = HandLayoutHorizontal;
    actions[0]->setChecked(true);
    mainMenu->addMenu(handLayoutSubmenu);
    mainMenu->addSeparator();

    mainMenu->addAction("Open File...", this, &MainWindow::actionLoadFile);
    mainMenu->addAction("Save File...", this, &MainWindow::actionSaveFile);
    mainMenu->addSeparator();
    mainMenu->addAction("Deal", this, &MainWindow::actionDeal);
    mainMenu->addSeparator();
    mainMenu->addAction("Restart Turn", this, &MainWindow::actionRestartTurn);
    this->menuActionDrawCardEndTurn = mainMenu->addAction("Draw Card/End Turn", this, &MainWindow::actionDrawCardEndTurn);

    mainMenu->addSeparator();
    SelectCardMenu *selectCardMenu = new SelectCardMenu(&cardDeck, "Select Card from Draw Pile...", this);
    mainMenu->addMenu(selectCardMenu);
    connect(selectCardMenu, &SelectCardMenu::cardClicked, this, &MainWindow::extractCardFromDrawPile);

    mainMenu->addSeparator();
    mainMenu->addAction("Exit", qApp, &QApplication::quit);

    // load the cards
    const QString dirPath = appRootPath() + "/images/pack_1";
    baizeScene->loadCardImages(dirPath);

    // set the graphics view
    baizeView->setScene(baizeScene);
    layout->addWidget(baizeView);
}

void MainWindow::showHands()
{
    for (int player = 0; player < hands.count(); player++)
        if (player == activePlayer)
            showHand(player);
        else
            hideHand(player);
}

void MainWindow::handCardLayoutInfo(int player, int handCount, MainWindow::HandCardLayoutInfo &hcli) const
{
    Q_UNUSED(player);
    hcli.cardPixmap = baizeScene->cardPixmap(0);
    hcli.cardWidth = hcli.cardPixmap.width();
    hcli.cardHeight = hcli.cardPixmap.height();
    hcli.xBetweenCards = hcli.cardWidth / 3;
    hcli.yHandHeight = (handLayout == HandLayoutFan) ? hcli.cardHeight + 65 : hcli.cardHeight + 15;

    hcli.midHandIndex = (handCount > 0) ? (handCount - 1) / 2.0 : 0;

    hcli.sceneRectForHands = fixHandsToView ? baizeView->mapToScene(baizeView->viewport()->rect()).boundingRect() : baizeScene->sceneRect();
    hcli.xPos = hcli.sceneRectForHands.center().x();
    hcli.yPos = hcli.sceneRectForHands.bottomLeft().y();
    hcli.xPlayerNamePos = hcli.xPos - 125;
    hcli.yPlayerNamePos = hcli.yPos - 75;
    hcli.xPos -= 50 + hcli.midHandIndex * hcli.xBetweenCards;
    hcli.yPos -= 100 + hcli.cardHeight;
}

QRectF MainWindow::handAreaRect(int player) const
{
    const CardHand &hand(hands.at(player));
    int handCount = qMax(hand.count(), 20);
    HandCardLayoutInfo hcli;
    handCardLayoutInfo(player, handCount, hcli);

    QRectF rect;

    switch (handLayout)
    {
    case HandLayoutHorizontal:
        rect.setRect(hcli.xPos, hcli.yPos, (hcli.xBetweenCards * (handCount - 1)) + hcli.cardWidth, hcli.cardHeight);
        break;
    case HandLayoutHorizontalGapBetweenSuits:
        rect.setRect(hcli.xPos, hcli.yPos, (hcli.xBetweenCards * (handCount - 1)) + (3 * (hcli.xBetweenCards * 2)) + hcli.cardWidth, hcli.cardHeight);
        break;
    case HandLayoutFan:
        rect.setRect(hcli.xPos + (hcli.midHandIndex - 1) * hcli.xBetweenCards - hcli.cardWidth * 2, hcli.yPos, hcli.cardWidth * 6, hcli.cardHeight * 1.5);
        break;
    }

    rect.adjust(-25, -25, 25, 25);
    return rect;
}

void MainWindow::hideHand(int player)
{
    if (aiContinuousPlayFast())
        return;
    const CardHand &hand(hands.at(player));
    for (int x = 0; x < hand.count(); x++)
    {
        CardPixmapItem *item = baizeScene->findItemForCard(hand.at(x));
        if (item)
            item->hide();
    }
}

void MainWindow::showHand(int player, bool enforceCorrectStacking /*= false*/)
{
    if (aiContinuousPlayFast())
        return;
    const CardHand &hand(hands.at(player));
    HandCardLayoutInfo hcli;
    handCardLayoutInfo(player, hand.count(), hcli);

    int xPos(hcli.xPos), yPos(hcli.yPos);
    for (int x = 0; x < hand.count(); x++)
    {
        const Card *card = hand.at(x);
        CardPixmapItem *item = baizeScene->findItemForCard(card);
        if (item)
        {
            if (enforceCorrectStacking)
            {
                delete item;
                item = nullptr;
            }
            else
            {
                item->show();
                item->resetRotatation();
            }
        }
        switch (handLayout)
        {
        case HandLayoutHorizontal:
            if (item)
                item->setPos(xPos, yPos);
            else
                baizeScene->addCard(card, xPos, yPos);
            xPos += hcli.xBetweenCards;
            break;
        case HandLayoutHorizontalGapBetweenSuits:
            if (x > 0 && card->suit() != hand.at(x - 1)->suit())
                xPos += hcli.xBetweenCards * 2;
            if (item)
                item->setPos(xPos, yPos);
            else
                baizeScene->addCard(card, xPos, yPos);
            xPos += hcli.xBetweenCards;
            break;
        case HandLayoutFan: {
            int xPos2 = xPos + hcli.midHandIndex * hcli.xBetweenCards;
            if (item)
                item->setPos(xPos2, yPos);
            else
                item = baizeScene->addCard(card, xPos2, yPos);
            QPointF rotationPoint(item->mapFromScene(item->pos().x() + hcli.cardWidth / 2, item->pos().y() + hcli.yHandHeight + 200));
            item->setRotatationAboutPoint(rotationPoint, (x - hcli.midHandIndex) * 5);
            break;
        }
        }
    }

    QRectF handArea(handAreaRect(activePlayer));
    baizeScene->handAreaRectItem()->setRect(handArea);
    baizeScene->playerNameItem()->setPlainText(QString("%1 #%2").arg(hands.isAiPlayer(activePlayer) ? "AI" : "Human").arg(activePlayer));
    baizeScene->playerNameItem()->setPos(hcli.xPlayerNamePos, hcli.yPlayerNamePos);
    QToolButton *button = qobject_cast<QToolButton *>(baizeScene->drawCardEndTurnButton()->widget());
    button->setDefaultAction(menuActionDrawCardEndTurn);
    baizeScene->drawCardEndTurnButton()->setPos(hcli.xPlayerNamePos + 150, hcli.yPlayerNamePos + 5);
}

void MainWindow::shuffleAndDeal()
{
    if (!aiContinuousPlayFast())
        baizeScene->reset();
    logicalModel.shuffleAndDeal();
}

void MainWindow::showInitialFreeCards()
{
    if (aiContinuousPlayFast())
        return;
    for (int i = 0; i < cardDeck.initialFreeCards().count(); i++)
    {
        const Card *card = cardDeck.initialFreeCards().at(i);
        baizeScene->addCard(card, -275 + i * 150, -300);
        const CardGroup &group(logicalModel.initialFreeCardGroup(card));
        baizeScene->layoutCardsAsGroup(group, true, true);
    }
}

void MainWindow::sortAndShow()
{
    hands.sortHands();
    showHands();
}

int MainWindow::findCardInHandArea(const CardPixmapItem *item) const
{
    QRectF rect(handAreaRect(activePlayer));
    if (rect.contains(item->scenePos()))
        return activePlayer;
    return -1;
}

void MainWindow::tidyGroups(bool verifyNoBadBads /*= false*/)
{
    cardGroups.removeEmptyGroups();
    if (!aiContinuousPlayFast())
        baizeScene->removeAllCardGroupBoxes();
    for (CardGroup &group : cardGroups)
    {
        group.rearrangeForSets();
        bool isBadSetGroup = !group.isGoodSet();
        bool isInitialCardGroup = logicalModel.isInitialCardGroup(group);
        if (verifyNoBadBads)
            Q_ASSERT(!isBadSetGroup || isInitialCardGroup);
        if (!aiContinuousPlayFast())
            baizeScene->layoutCardsAsGroup(group, isBadSetGroup, isInitialCardGroup);
    }
}

bool MainWindow::havePlayedCard() const
{
   for (const Card *card : logicalModel.startOfTurnHand())
       if (!hands[activePlayer].contains(card))
               return true;
    return false;
}

void MainWindow::startTurn(bool restart /*= false*/)
{
    Q_ASSERT(!logicalModel.isDealOver(true));
    haveDrawnCard = false;
    if (!restart)
    {
        serializationDoc = serializeToJson();
        logicalModel.startOfTurn();
        autosave();
    }
    updateDrawCardEndTurnAction();

    if (hands.isAiPlayer(activePlayer))
        QTimer::singleShot(aiContinuousPlayFast() ? 0 : restart ? 2000 : 100, this, [this]() { emit aiModelMakeTurn(); });
}

void MainWindow::autosave()
{
    if (aiContinuousPlayFast())
        return;
    Q_ASSERT(!serializationDoc.isEmpty());
    const QString newPath = appSavesPath() + "/autosave.sav", backupPath = appSavesPath() + "/autosave.prev.sav";
    QFile::remove(backupPath);
    QFile::rename(newPath, backupPath);
    QSaveFile file(newPath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        file.write(serializationDoc.toJson());
        file.commit();
    }
}

QPointF MainWindow::findFreeAreaForCardGroup(const CardGroup &cardGroup) const
{
    // (try to) find free area for card group
    if (aiContinuousPlayFast())
        return QPointF();
    QList<QRectF> freeRectangles = baizeScene->findFreeRectanglesToPlaceCards(cardGroup.count(), baizeView->visibleSceneRect());
    if (freeRectangles.isEmpty())
        freeRectangles = baizeScene->findFreeRectanglesToPlaceCards(cardGroup.count());
    RandomNumber::random_shuffle(freeRectangles.begin(), freeRectangles.end());
    QPointF newGroupPoint = { -600, -300 };
    if (!freeRectangles.isEmpty())
    {
        const QRectF &freeRect(freeRectangles.first());
        QRectF cardsRect(QPointF(freeRect.topLeft()), baizeScene->cardsAsGroupSize(cardGroup.count()));
        Q_ASSERT(freeRect.contains((cardsRect)));
        cardsRect.moveCenter(freeRect.center());
        Q_ASSERT(freeRect.contains((cardsRect)));
        newGroupPoint = cardsRect.topLeft();
    }
    return newGroupPoint;
}

void MainWindow::reportDealIsOver(int winner)
{
    qDebug() << ((winner >= 0) ? "+++" : "---")
             << qUtf8Printable(QString("Deal #%1 over").arg(logicalModel.totalDeals))
             << ((winner >= 0) ? QString("Winner: %1").arg(winner) : QString("No winner"));
}

void MainWindow::aiModelMakePlays(const AiModelState &turnPlay)
{
    Q_ASSERT(hands.isAiPlayer(activePlayer));
    CardHand &aiHand(hands[activePlayer]);
    const CardGroups &cardGroupsChanged(turnPlay.cardGroups);
    Q_ASSERT(!cardGroupsChanged.isEmpty());
    const AiModelState originalState(aiHand, cardGroups);

    for (int pass = 0; pass < 2; pass++)
        for (const CardGroup &changedCardGroup : cardGroupsChanged)
        {
            enum ChangeType { Add, Delete, Modify } changeType;
            int oldCardGroupIndex = cardGroups.findCardGroupByUniqueId(changedCardGroup.uniqueId());
            if (oldCardGroupIndex < 0)
                changeType = Add;
            else if (changedCardGroup.isEmpty())
                changeType = Delete;
            else
                changeType = Modify;

            if (pass == 0 && changeType == Delete)
                continue;
            else if (pass == 1 && changeType != Delete)
                continue;

            if (changeType == Add && changedCardGroup.isEmpty())
                continue;
            else if (changeType == Delete && oldCardGroupIndex < 0)
                continue;
            else if (changeType == Modify && cardGroups.at(oldCardGroupIndex) == changedCardGroup)
                continue;

            if (changeType == Delete)
            {
                Q_ASSERT(cardGroups.at(oldCardGroupIndex).isEmpty());
                if (aiModel.debugLevel() >= 1)
                    qDebug() << "    " << __FUNCTION__ << "Deleted Group" << changedCardGroup.uniqueId() << originalState.cardGroups.at(oldCardGroupIndex).toString();
                continue;
            }

            Q_ASSERT(changedCardGroup.count() >= 3);
            Q_ASSERT(changedCardGroup.isGoodSet());
            for (const Card *card : changedCardGroup)
                Q_ASSERT(aiHand.contains(card) || cardGroups.findCardInGroups(card) >= 0);

            if (changeType == Add)
            {
                if (aiModel.debugLevel() >= 1)
                    qDebug() << "    " << __FUNCTION__ << "Added Group" << changedCardGroup.uniqueId() << changedCardGroup.toString();
                // (try to) find free area for new card group
                QPointF newGroupPoint = findFreeAreaForCardGroup(changedCardGroup);

                for (const Card *card : changedCardGroup)
                {
                    if (aiHand.contains(card))
                        aiHand.removeOne(card);
                    else
                    {
                        int wasInGroup = cardGroups.removeCardFromGroups(card);
                        Q_ASSERT(wasInGroup >= 0);
                    }
                    if (!aiContinuousPlayFast())
                    {
                        CardPixmapItem *item = baizeScene->findItemForCard(card);
                        Q_ASSERT(item);
                        item->setPos(newGroupPoint);
                    }
                }
                cardGroups.append(changedCardGroup);
            }
            else if (changeType == Modify)
            {
                if (aiModel.debugLevel() >= 1)
                    qDebug() << "    " << __FUNCTION__ << "Modified Group" << changedCardGroup.uniqueId() << originalState.cardGroups.at(oldCardGroupIndex).toString() << "-->" << changedCardGroup.toString();
                CardGroup &existingCardGroup(cardGroups[oldCardGroupIndex]);
                const Card *existingGroupCard = existingCardGroup.first();
                for (const Card *card : changedCardGroup)
                {
                    if (aiHand.contains(card))
                        aiHand.removeOne(card);
                    else
                    {
                        if (existingCardGroup.contains(card))
                            continue;
                        int wasInGroup = cardGroups.removeCardFromGroups(card);
                        Q_ASSERT(wasInGroup >= 0);
                    }
                    existingCardGroup.append(card);
                    if (!aiContinuousPlayFast())
                    {
                        CardPixmapItem *item = baizeScene->findItemForCard(card);
                        Q_ASSERT(item);
                        const CardPixmapItem *existingGroupItem = baizeScene->findItemForCard(existingGroupCard);
                        Q_ASSERT(existingGroupItem);
                        item->setPos(existingGroupItem->pos());
                    }
                }
            }
        }

    // tidy up
    logicalModel.updateInitialFreeCards();
    tidyGroups(true);
    showHand(activePlayer);
    updateDrawCardEndTurnAction();
}

QJsonDocument MainWindow::serializeToJson() const
{
    QJsonObject obj;
    obj["activePlayer"] = activePlayer;
    obj["cardDeck"] = cardDeck.serializeToJson();
    obj["hands"] = hands.serializeToJson();
    obj["cardGroups"] = cardGroups.serializeToJson();
    obj["scene"] = baizeScene->serializeToJson();

    QJsonDocument doc;
    doc.setObject(obj);
    return doc;
}

void MainWindow::deserializeFromJson(const QJsonDocument &doc)
{
    Q_ASSERT(doc.isObject());
    const QJsonObject &obj = doc.object();
    activePlayer = obj["activePlayer"].toInt();
    cardDeck.deserializeFromJson(obj["cardDeck"].toObject());
    hands.deserializeFromJson(obj["hands"].toArray(), cardDeck);
    cardGroups.deserializeFromJson(obj["cardGroups"].toArray(), cardDeck);
    baizeScene->deserializeFromJson(obj["scene"].toObject(), cardDeck);
}

/*slot*/ void MainWindow::baizeViewCoordinatesChanged()
{
    if (aiContinuousPlayFast())
        return;
    if (!fixHandsToView)
        return;
    showHands();
    // not sure why, but invalidating the background is needed to prevent "artifacts" being left when scrolling via the scrollbar button
    baizeScene->invalidate(baizeScene->sceneRect(), QGraphicsScene::BackgroundLayer);
}

/*slot*/ void MainWindow::baizeSceneSingleCardMoved(CardPixmapItem *item)
{
    if (aiContinuousPlayFast())
        return;
    Q_ASSERT(item);
    Q_ASSERT(item->card);

    int wasInHand = hands.findCardInHands(item->card);
    int wasInGroup = cardGroups.findCardInGroups(item->card);
    if (wasInHand >= 0)
        Q_ASSERT(wasInGroup < 0);

    int isInHandAreaNow = findCardInHandArea(item);
    const Card *onOtherCardNow = (isInHandAreaNow < 0) ? baizeScene->findOtherCardForItemPosition(item) : nullptr;
    if (onOtherCardNow)
        Q_ASSERT(onOtherCardNow != item->card);
    int onGroupNow = onOtherCardNow ? cardGroups.findCardInGroups(onOtherCardNow) : -1;

    item->resetRotatation();

    if (wasInHand >= 0)
    {
        // moving from Hand...
        if (isInHandAreaNow >= 0)
        {
            // ...moving from Hand to Hand
            Q_ASSERT(isInHandAreaNow == wasInHand);
            showHand(isInHandAreaNow);
        }
        else
        {
            // ...moving from Hand to baize
            hands[wasInHand].removeOne(item->card);
            showHand(wasInHand);
            // move to existing or new group
            if (onGroupNow < 0)
            {
                CardGroup group;
                if (onOtherCardNow)
                    group.append(onOtherCardNow);
                group.append(item->card);
                cardGroups.append(group);
            }
            else
                cardGroups[onGroupNow].append(item->card);
            if (wasInHand == activePlayer)
                updateDrawCardEndTurnAction();
        }
    }
    else
    {
        // moving from baize...
        if (isInHandAreaNow >= 0)
        {
            // ...moving from baize to Hand
            if (wasInGroup >= 0)
                cardGroups[wasInGroup].removeOne(item->card);
            hands[isInHandAreaNow].append(item->card);
            hands[isInHandAreaNow].sortHand();
            showHand(isInHandAreaNow, true);
        }
        else
        {
            // ...moving from baize to baize
            if (onGroupNow >= 0 && wasInGroup >= 0 && onGroupNow == wasInGroup)
            {
                // ...moving from group to same group
            }
            else
            {
                // ...moving to group
                Q_ASSERT(!(onGroupNow >= 0 && wasInGroup >= 0 && onGroupNow == wasInGroup));
                if (wasInGroup >= 0)
                    cardGroups[wasInGroup].removeOne(item->card);
                // move to existing or new group
                if (onGroupNow < 0)
                {
                    CardGroup group;
                    if (onOtherCardNow)
                        group.append(onOtherCardNow);
                    group.append(item->card);
                    cardGroups.append(group);
                }
                else
                    cardGroups[onGroupNow].append(item->card);
            }
        }
    }

    // tidy up
    tidyGroups();
    updateDrawCardEndTurnAction();
}

/*slot*/ void MainWindow::baizeSceneMultipleCardsMoved(QList<CardPixmapItem *> items)
{
    Q_UNUSED(items);
    // tidy up
    showHand(activePlayer, true);
    tidyGroups();
}

/*slot*/ void MainWindow::drawCardFromDrawPile()
{
    if (havePlayedCard() || haveDrawnCard)
        return;

    const Card *card = logicalModel.drawCardFromDrawPile();
    if (card == nullptr)
        return;
    if (!aiContinuousPlayFast())
    {
        showHand(activePlayer, true);
        baizeScene->blinkingCard()->start(card);
    }

    haveDrawnCard = true;
    updateDrawCardEndTurnAction();

    if (autoEndTurnOnDrawCard && !aiContinuousPlay)
        QTimer::singleShot(2000, this, &MainWindow::actionDrawCardEndTurn);
}

/*slot*/ void MainWindow::extractCardFromDrawPile(int id)
{
    const Card *card = logicalModel.extractCardFromDrawPile(id);
    if (card == nullptr)
        return;
    if (!aiContinuousPlayFast())
    {
        showHand(activePlayer, true);
        baizeScene->blinkingCard()->start(card);
    }
}

/*slot*/ void MainWindow::aiModelMakeTurnPlay(AiModelState turnPlay)
{
    if (turnPlay.isNull())
    {
        if (aiModel.debugLevel() >= 1)
            qDebug() << __FUNCTION__ << "AI draws card";
        drawCardFromDrawPile();
    }
    else
    {
        if (aiModel.debugLevel() >= 1)
            qDebug() << __FUNCTION__ << "AI makes play(s)";
        aiModelMakePlays(turnPlay);
    }

    if (aiContinuousPlay)
    {
        int winner;
        if (logicalModel.isDealOver(turnPlay.isNull(), winner))
        {
            reportDealIsOver(winner);
            QTimer::singleShot(aiContinuousPlayFast() ? 10 : 2000, this, &MainWindow::actionDeal);
            return;
        }
        actionDrawCardEndTurn();
    }
}

/*slot*/ void MainWindow::actionAiContinuousPlay(bool checked)
{
    if (checked == aiContinuousPlay)
        return;
    aiContinuousPlay = checked;
    if (aiContinuousPlay)
    {
        hands.aiPlayers.clear();
        for (int i = 0; i < hands.totalHands; i++)
            hands.aiPlayers.append(true);
        actionDeal();
    }
    else
    {
        logicalModel.updateInitialFreeCards();
        baizeScene->reset();
        showInitialFreeCards();
        for (const CardGroup &cardGroup : cardGroups)
        {
            if (logicalModel.isInitialCardGroup(cardGroup))
                continue;
            if (cardGroup.isEmpty())
                continue;
            QPointF newGroupPoint = findFreeAreaForCardGroup(cardGroup);
            for (const Card *card : cardGroup)
                baizeScene->addCard(card, newGroupPoint.x(), newGroupPoint.y());
        }
        tidyGroups(true);
        serializationDoc = serializeToJson();
        showHands();
    }
}

/*slot*/ void MainWindow::actionHandLayout(HandLayout handLayout)
{
    this->handLayout = handLayout;
    showHands();
}

/*slot*/ void MainWindow::actionLoadFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Open", appSavesPath(), "*.sav");
    if (filePath.isEmpty())
        return;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Error", QString("%1: %2").arg(filePath).arg(file.errorString()));
        return;
    }
    QByteArray jsonText = file.readAll();
    serializationDoc = QJsonDocument::fromJson(jsonText);
    actionRestartTurn();
}

/*slot*/ void MainWindow::actionSaveFile()
{
    Q_ASSERT(!serializationDoc.isEmpty());
    QString filePath = QFileDialog::getSaveFileName(this, "Save As", appSavesPath(), "*.sav");
    if (filePath.isEmpty())
        return;
    if (!filePath.endsWith(".sav"))
        filePath += ".sav";
    QSaveFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Error", QString("%1: %2").arg(filePath).arg(file.errorString()));
        return;
    }
    file.write(serializationDoc.toJson());
    file.commit();
}

/*slot*/ void MainWindow::actionDeal()
{
    shuffleAndDeal();
    showInitialFreeCards();
    this->activePlayer = 0;
    sortAndShow();

    startTurn();
}

/*slot*/ void MainWindow::actionRestartTurn()
{
    deserializeFromJson(serializationDoc);
    logicalModel.updateInitialFreeCards();
    tidyGroups(true);
    showHands();
    startTurn(true);
}

void MainWindow::updateDrawCardEndTurnAction()
{
    if (aiContinuousPlayFast())
        return;
    menuActionDrawCardEndTurn->setText((havePlayedCard() || haveDrawnCard) ? "End Turn" : "Draw Card");
    baizeScene->setPreventMovingCards(haveDrawnCard);
    menuActionDrawCardEndTurn->setEnabled(logicalModel.badSetGroups().isEmpty());
    if (logicalModel.isDealOver(true))
        menuActionDrawCardEndTurn->setEnabled(false);
}

/*slot*/ void MainWindow::actionDrawCardEndTurn()
{
    if (aiContinuousPlayFast())
        baizeScene->blinkingCard()->stop();

    if (!havePlayedCard() && !haveDrawnCard)
    {
        drawCardFromDrawPile();
        return;
    }

    int winner;
    if (logicalModel.isDealOver(true, winner))
    {
        reportDealIsOver(winner);
        showHands();
        menuActionDrawCardEndTurn->setEnabled(false);
        return;
    }

    logicalModel.endOfTurn();
    showHands();

    startTurn();
}
