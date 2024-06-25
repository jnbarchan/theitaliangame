#ifndef BAIZESCENE_H
#define BAIZESCENE_H

#include <QGraphicsPixmapItem>
#include <QGraphicsProxyWidget>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QJsonObject>
#include <QTimer>
#include <QToolButton>

#include "cardimages.h"
#include "carddeck.h"

class BaizeScene;

class CardPixmapItem : public QGraphicsPixmapItem
{
public:
    const Card *card = nullptr;

    void setRotatationAboutPoint(const QPointF &point, qreal angle);
    void resetRotatation();
    QRectF boundingRect() const override;

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
};


class CardGroupBoxItem : public QGraphicsRectItem
{
public:
    enum ShowAs { ShowAsGoodSet, ShowAsBadSet, ShowAsInitialCardGroup };

    CardGroupBoxItem(ShowAs showAs);

    ShowAs showAs() { return _showAs; }
    void setShowAs(ShowAs showAs);

private:
    ShowAs _showAs;
};


class CardBlinker : public QObject
{
public:
    CardBlinker(BaizeScene *baizeScene);

    void start(const Card *card);
    void stop();

private:
    BaizeScene *_baizeScene;
    QTimer timer;
    int countdown;
    const Card *card;
    bool wasVisible;

    void onBlinkingTimeout();
};



class BaizeScene : public QGraphicsScene
{
    Q_OBJECT

public:
    BaizeScene(QObject *parent = nullptr);
    ~BaizeScene();

    const CardImages *cardImages() const { return _cardImages; }
    void loadCardImages(const QString dirPath);
    const QPixmap cardPixmap(int id) const;
    QGraphicsPixmapItem *drawPileItem() const { return _drawPileItem; }
    QGraphicsRectItem *handAreaRectItem() const { return _handAreaRectItem; }
    QGraphicsTextItem *playerNameItem() const { return _playerNameItem; }
    QGraphicsProxyWidget *drawCardEndTurnButton() const { return _drawCardEndTurnButton; }
    void reset();
    void preventMovingCards(bool prevent) { _preventMovingCards = prevent; }
    CardBlinker *blinkingCard() { return _blinkingCard; }
    CardPixmapItem *addCard(const Card *card, int x, int y);
    CardPixmapItem *findItemForCard(const Card *card) const;
    const Card *findOtherCardForItemPosition(const CardPixmapItem *item) const;
    void removeAllCardGroupBoxes();
    QSize cardsAsGroupSize(int cardCount);
    void layoutCardsAsGroup(QList<const Card *> cards, bool isBadSetGroup = false, bool isInitialCardGroup = false);
    QList<QRectF> findFreeRectanglesToPlaceCards(int cardCount, QRectF placeInRect = QRectF());
    void showFreeRectangles(const QList<QRectF> &freeRectangles);
    QJsonObject serializeToJson() const;
    void deserializeFromJson(const QJsonObject &obj, const CardDeck &cardDeck);

private:
    CardImages *_cardImages;
    QGraphicsPixmapItem *_drawPileItem;
    QGraphicsRectItem *_handAreaRectItem;
    QGraphicsTextItem *_playerNameItem;
    QGraphicsProxyWidget *_drawCardEndTurnButton;
    CardBlinker *_blinkingCard;
    bool _preventMovingCards;
    const QPixmap cardBackPixmap() const;
    void createDrawPile();
    void createHandAreaRectItem();
    void createPlayerNameItem();
    void createDrawCardEndTurnButton();
    QRectF calcBoundingRect(const QList<CardPixmapItem *> &items);
    QList<QGraphicsItem *> findContainingItems(const QGraphicsItem *item, const QList<QGraphicsItem *> items) const;
    QList<QRectF> calcMaximalFreeRectangles();

protected:
    virtual void drawBackground(QPainter *painter, const QRectF &rect) override;
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

signals:
    void singleCardMoved(CardPixmapItem *cardPixmapItem);
    void multipleCardsMoved(QList<CardPixmapItem *> cardPixmapItems);
    void drawPileDoubleClicked();
};

#endif // BAIZESCENE_H
