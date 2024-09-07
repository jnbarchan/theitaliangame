#include <QApplication>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QJsonArray>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QtMath>

#include "cardimages.h"
#include "baizescene.h"


void CardPixmapItem::setRotatationAboutPoint(const QPointF &point, qreal angle)
{
    setTransformOriginPoint(point);
    setRotation(angle);
}

void CardPixmapItem::resetRotatation()
{
    QPointF origScenePos(scenePos());
    setTransformOriginPoint(QPointF());
    setRotation(0.0);
    setPos(origScenePos);
}

QRectF CardPixmapItem::boundingRect() const /*override*/
{
    QRectF rect(QGraphicsPixmapItem::boundingRect());
    rect.adjust(-2.0, -2.0, 0, 0);
    return rect;
}

void CardPixmapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) /*override*/
{
    painter->save();

    QGraphicsPixmapItem::paint(painter, option, widget);

    if (isSelected())
    {
        QRectF boundrect(boundingRect());
        painter->setPen(QPen(Qt::black, 4.0, Qt::DashLine));
        painter->drawRect(boundrect);
    }

    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen(QPen(Qt::gray, 2.0));
    QRectF boundrect(boundingRect());
    boundrect.adjust(1.0, 1.0, 1.0, 1.0);
    QRectF cliprect(boundrect);
    cliprect.adjust(0, 0, -1.0, -1.0);
    painter->setClipRect(cliprect);
    painter->drawRoundedRect(boundrect, 5.0, 5.0);

    painter->restore();
}



CardGroupBoxItem::CardGroupBoxItem(ShowAs showAs)
{
    setShowAs(showAs);
}

void CardGroupBoxItem::setShowAs(ShowAs showAs)
{
    _showAs = showAs;
    QPen pen;
    switch (_showAs)
    {
    case ShowAsBadSet: pen.setColor(Qt::red); break;
    case ShowAsGoodSet: pen.setColor(/*Qt::transparent*/Qt::cyan/*TEMPORARY*/); break;
    case ShowAsInitialCardGroup: pen.setColor(Qt::yellow); break;
    }
    pen.setWidth(4);
    setPen(pen);
}



CardBlinker::CardBlinker(BaizeScene *baizeScene) :
    QObject(baizeScene)
{
    this->_baizeScene = baizeScene;
    this->card = nullptr;
}

void CardBlinker::start(const Card *card)
{
    stop();
    CardPixmapItem *item = _baizeScene->findItemForCard(card);
    if (!item)
        return;
    this->wasVisible = item->isVisible();
    this->card = card;
    this->countdown = 10;
    this->timer.setInterval(200);
    connect(&this->timer, &QTimer::timeout, this, &CardBlinker::onBlinkingTimeout, Qt::UniqueConnection);
    this->timer.start();
}

void CardBlinker::onBlinkingTimeout()
{
    CardPixmapItem *item;
    if (!card || !(item = _baizeScene->findItemForCard(card)) || --countdown <= 0)
    {
        stop();
        return;
    }
    item->setVisible((countdown & 1) == 0);
}

void CardBlinker::stop()
{
    timer.stop();
    countdown = 0;
    if (card)
    {
        CardPixmapItem *item = _baizeScene->findItemForCard(card);
        if (item)
            item->setVisible(wasVisible);
        card = nullptr;
    }
}



BaizeScene::BaizeScene(QObject *parent) :
    QGraphicsScene(parent)
{
    this->_cardImages = nullptr;
    this->_drawPileItem = nullptr;
    this->_blinkingCard = new CardBlinker(this);
    this->_preventMovingCards = false;
}

BaizeScene::~BaizeScene()
{
    if (_blinkingCard)
        delete _blinkingCard;
    if (_cardImages)
        delete _cardImages;
}

void BaizeScene::loadCardImages(const QString dirPath)
{
    // physically load all the card images into `cardsImages[]` array
    CardImages *newCardImages = new CardImages(dirPath);
    // if we have existing card images (not first time) and could not find images in the new card set directory
    // do not wipe out existing card images
    if (_cardImages && !newCardImages->foundImages())
        return;
    delete _cardImages;
    _cardImages = newCardImages;
    invalidate();
}

const QPixmap BaizeScene::cardPixmap(int id) const
{
    return _cardImages->cardPixmap(id).scaledToWidth(100);
}

const QPixmap BaizeScene::cardBackPixmap() const
{
    return _cardImages->cardBackPixmap().scaledToWidth(100);
}

void BaizeScene::createDrawPile()
{
    QPixmap pixmap(cardBackPixmap());
    this->_drawPileItem = addPixmap(pixmap);
    _drawPileItem->setPos(-pixmap.width() / 2, -pixmap.height() / 2);
}

void BaizeScene::createHandAreaRectItem()
{
    this->_handAreaRectItem = addRect(QRectF(), QPen(), QBrush(0xC4A484));
    _handAreaRectItem->setZValue(-1.0);
}

void BaizeScene::createPlayerNameItem()
{
    this->_playerNameItem = addText("Player");
    QFont font(_playerNameItem->font());
    font.setPixelSize(30);
    _playerNameItem->setFont(font);
    _playerNameItem->setDefaultTextColor(Qt::white);
}

void BaizeScene::createDrawCardEndTurnButton()
{
    QToolButton *button = new QToolButton;
    this->_drawCardEndTurnButton = addWidget(button);
    QFont font(_drawCardEndTurnButton->font());
    font.setPixelSize(20);
    _drawCardEndTurnButton->setFont(font);
}

void BaizeScene::reset()
{
    blinkingCard()->stop();
    preventMovingCards(false);
    clear();
    createDrawPile();
    createHandAreaRectItem();
    createPlayerNameItem();
    createDrawCardEndTurnButton();
}

CardPixmapItem *BaizeScene::addCard(const Card *card, int x, int y)
{
    Q_ASSERT(card);
    Q_ASSERT(!findItemForCard(card));
    // get correct pixmap image
    const QPixmap pixmap(cardPixmap(card->id));
    // create and add pixmap item to scene
    CardPixmapItem *item = new CardPixmapItem;
    item->setPixmap(pixmap);
    item->setFlag(QGraphicsItem::ItemIsMovable);
    item->setFlag(QGraphicsItem::ItemIsSelectable);
    addItem(item);
    // associate item with card passed in
    item->card = card;
    // place at scene position
    item->setPos(x, y);
    return item;
}

CardPixmapItem *BaizeScene::findItemForCard(const Card *card) const
{
    Q_ASSERT(card);
    // search all scene items for pixmap item associated with card passed in
    QList<QGraphicsItem *> items = this->items();
    for (QGraphicsItem *item : items)
    {
       CardPixmapItem *cardItem = dynamic_cast<CardPixmapItem *>(item);
       if (!cardItem)
           continue;
       if (cardItem->card == card)
           return cardItem;
    }
    return nullptr;
}

const Card *BaizeScene::findOtherCardForItemPosition(const CardPixmapItem *item) const
{
    Q_ASSERT(item);
    // search all scene items for *other* card at position of card item passed in
    // we try 2 passes:
    // first is for top-left of card item passed in, second is for bottom-left
    for (int pass = 0; pass < 2; pass++)
    {
        QPointF point((pass == 0) ? item->scenePos() : item->mapToScene(QPointF(0, item->pixmap().height())));
        QList<QGraphicsItem *> otherItems = this->items(point);
        for (QGraphicsItem *otherItem : otherItems)
        {
           CardPixmapItem *cardItem = dynamic_cast<CardPixmapItem *>(otherItem);
           if (!cardItem)
               continue;
           if (cardItem == item)
               continue;
           return cardItem->card;
        }
    }
    return nullptr;
}

void BaizeScene::removeAllCardGroupBoxes()
{
    QList<QGraphicsItem *> items = this->items();
    for (int i = items.count() - 1; i >= 0; i--)
    {
       CardGroupBoxItem *boxItem = dynamic_cast<CardGroupBoxItem *>(items[i]);
       if (boxItem)
       {
           removeItem(boxItem);
           delete boxItem;
       }
    }
}

QRectF BaizeScene::calcBoundingRect(const QList<CardPixmapItem *> &items)
{
    QRectF rect;
    for (const CardPixmapItem *item : items)
        rect = rect.united(item->mapToScene(item->boundingRect()).boundingRect());
    return rect;
}

QSize BaizeScene::cardsAsGroupSize(int cardCount)
{
    const QPixmap pixmap(cardPixmap(0));
    const int xBetweenItems = 25;
    int width = pixmap.width() + (cardCount - 1) * xBetweenItems;
    QSize cardsSize{width, pixmap.height()};
    return cardsSize;
}

void BaizeScene::layoutCardsAsGroup(QList<const Card *> cards, bool isBadSetGroup /*= false*/, bool isInitialCardGroup /*= false*/)
{
    if (cards.count() == 0)
        return;

    QList<CardPixmapItem *> items;
    for (const Card *card : cards)
    {
        CardPixmapItem *item = findItemForCard(card);
        Q_ASSERT(card);
        items.append(item);
    }

    const CardPixmapItem *leftmostItem = items.at(0);
    for (const CardPixmapItem *item : items)
        if (item->pos().x() < leftmostItem->pos().x())
            leftmostItem = item;

    int xPos(leftmostItem->pos().x()), yPos(leftmostItem->pos().y());
    const int xBetweenItems = 25;
    for (CardPixmapItem *item : items)
    {
        item->setPos(xPos, yPos);
        xPos += xBetweenItems;
    }
    for (int i = items.count() - 2; i >= 0; i--)
        items[i]->stackBefore(items[i + 1]);

    QRectF rect(calcBoundingRect(items));
    if (!rect.isNull())
    {
        CardGroupBoxItem::ShowAs showAs = CardGroupBoxItem::ShowAsGoodSet;
        if (isBadSetGroup)
            showAs = isInitialCardGroup ? CardGroupBoxItem::ShowAsInitialCardGroup : CardGroupBoxItem::ShowAsBadSet;
        CardGroupBoxItem *boxItem = new CardGroupBoxItem(showAs);
        rect.adjust(-2.0, -2.0, 4.0, 4.0);
        boxItem->setRect(rect);
        addItem(boxItem);
        boxItem->stackBefore(items[0]);
    }
}


QList<QGraphicsItem *> BaizeScene::findContainingItems(const QGraphicsItem *item, const QList<QGraphicsItem *> items) const
{
    // find all items in `items` which fully contain `item`
    const QRectF &boundingRect(item->mapToScene(item->boundingRect()).boundingRect());
    QList<QGraphicsItem *> containingItems;
    for (QGraphicsItem *containingItem : items)
    {
        const QRectF &containingBoundingRect(containingItem->mapToScene(containingItem->boundingRect()).boundingRect());
        if (containingItem != item && containingBoundingRect.contains(boundingRect))
            containingItems.append(containingItem);
    }
    return containingItems;
}

QList<QRectF> BaizeScene::calcMaximalFreeRectangles()
{
    // calculate the list of maximal free rectangular areas
    // used to decide where to place a new card group for the AI
    // algorithm taken from https://forum.qt.io/post/803192

    QList<QGraphicsItem *> items(this->items());

    // Remove items enclosed in other items (e.g. CardPixmapItems, allow it to use the enclosing group boxes/hand area)
    for (int i = items.count() - 1; i >= 0; i--)
        if (!findContainingItems(items.at(i), items).isEmpty())
            items.removeAt(i);

    // Generate vertical and horizontal sweep lines
    QVector<qreal> verticalLines = { sceneRect().left(), sceneRect().right() };
    QVector<qreal> horizontalLines = { sceneRect().top(), sceneRect().bottom() };
    for (const QGraphicsItem *item : items)
    {
        const QRectF &rect(item->mapToScene(item->boundingRect()).boundingRect());
        verticalLines.append(rect.left());
        verticalLines.append(rect.right());
        horizontalLines.append(rect.top());
        horizontalLines.append(rect.bottom());
    }

    // Sort lines lists and make unique
    std::sort(verticalLines.begin(), verticalLines.end());
    verticalLines.erase(std::unique(verticalLines.begin(), verticalLines.end()), verticalLines.end());
    std::sort(horizontalLines.begin(), horizontalLines.end());
    horizontalLines.erase(std::unique(horizontalLines.begin(), horizontalLines.end()), horizontalLines.end());

    // calculate list of all free rectangles
    QList<QRectF> freeRectangles;
    for (int i = 0; i < horizontalLines.count() - 1; i++)
        for (int j = 0; j < verticalLines.count() - 1; j++)
        {
            QRectF candidateRect = QRectF(QPointF(verticalLines.at(j), horizontalLines.at(i)), QPointF(verticalLines.at(j + 1), horizontalLines.at(i + 1)));
            bool intersects = false;
            for (const QGraphicsItem *item : items)
            {
                const QRectF &rect(item->mapToScene(item->boundingRect()).boundingRect());
                if (rect.intersects(candidateRect))
                    intersects = true;
            }
            if (!intersects)
                freeRectangles.append(candidateRect);
        }

    // amalgamate touching free rectangles horizontally
    std::sort(freeRectangles.begin(), freeRectangles.end(),
              [](const QRectF &r0, const QRectF &r1) { return qFuzzyCompare(r0.y(), r1.y()) ? r0.x() < r1.x() : r0.y() < r1.y(); } );
    for (int i = freeRectangles.count() - 1; i > 0; i--)
        if (freeRectangles.at(i - 1).topRight() == freeRectangles.at(i).topLeft() && freeRectangles.at(i - 1).bottomRight() == freeRectangles.at(i).bottomLeft())
        {
            freeRectangles[i - 1] = freeRectangles[i - 1].united(freeRectangles[i]);
            freeRectangles.removeAt(i);
        }

    return freeRectangles;
}

QList<QRectF> BaizeScene::findFreeRectanglesToPlaceCards(int cardCount, QRectF placeInRect /*= QRectF()*/)
{
    if (placeInRect.isEmpty())
        placeInRect = sceneRect();

    QSize cardsSize(cardsAsGroupSize(cardCount));

    QList<QRectF> result;
    QList<QRectF> freeRectangles = calcMaximalFreeRectangles();
    for (const QRectF &rect : freeRectangles)
    {
        QRectF rect2 = rect.intersected(placeInRect);
        if (rect2.width() >= cardsSize.width() && rect2.height() >= cardsSize.height())
            result.append(rect2);
    }

    return result;
}

void BaizeScene::showFreeRectangles(const QList<QRectF> &freeRectangles)
{
    qDebug() << "showFreeRectangles count:" << freeRectangles.count();
    QGraphicsRectItem *background = this->addRect(this->sceneRect(), QPen(), QBrush(Qt::yellow));
    background->setZValue(-1000);
    for (const QRectF &rect : freeRectangles)
        this->addRect(rect, QPen(), QBrush(Qt::cyan))->setZValue(-500);
}


QJsonObject BaizeScene::serializeToJson() const
{
    QJsonObject obj;
    QJsonArray arrItems;
    for (const QGraphicsItem *item : items(Qt::AscendingOrder))
    {
        QJsonObject objItem;
        objItem["x"] = item->pos().x();
        objItem["y"] = item->pos().y();
        const CardPixmapItem *cardItem = dynamic_cast<const CardPixmapItem *>(item);
        const CardGroupBoxItem *groupBoxItem = dynamic_cast<const CardGroupBoxItem *>(item);
        bool omit = false;
        if (item == _drawPileItem || item  == _handAreaRectItem || item  == _playerNameItem || item == _drawCardEndTurnButton || groupBoxItem)
            omit = true;
        else if (cardItem)
            objItem["card"] = cardItem->card->id;
        else
            Q_ASSERT(false);
        if (!omit)
            arrItems.append(objItem);
    }
    obj["items"] = arrItems;
    return obj;
}

void BaizeScene::deserializeFromJson(const QJsonObject &obj, const CardDeck &cardDeck)
{
    reset();
    const QJsonArray &arrItems(obj["items"].toArray());
    for (const auto &val : arrItems)
    {
        const QJsonObject &objItem(val.toObject());
        QPointF pos(objItem["x"].toInt(), objItem["y"].toInt());
        if (objItem.contains("card"))
        {
            int id = objItem["card"].toInt();
            const Card *card = cardDeck.findCard(id);
            addCard(card, pos.x(), pos.y());
        }
        else
            Q_ASSERT(false);
    }
}


/*virtual*/ void BaizeScene::drawBackground(QPainter *painter, const QRectF &rect) /*override*/
{
    // call the base method
    QGraphicsScene::drawBackground(painter, rect);

    // clip to `rect`
    painter->save();
    painter->setClipRect(rect);

    // draw that part of the baize which lies in `rect`

    // fill a rect and draw a frame
    painter->fillRect(sceneRect(), 0x3ab503);
    painter->drawRect(sceneRect());
    painter->restore();
}

/*virtual*/ void BaizeScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) /*override*/
{
    QGraphicsScene::mouseDoubleClickEvent(event);

    QGraphicsItem *item = itemAt(event->scenePos(), QTransform());
    if (item == _drawPileItem)
        emit drawPileDoubleClicked();
}

/*virtual*/ void BaizeScene::mousePressEvent(QGraphicsSceneMouseEvent *event) /*override*/
{
    if (_preventMovingCards)
    {
        QGraphicsItem *item = itemAt(event->scenePos(), QTransform());
        CardPixmapItem *cardItem = dynamic_cast<CardPixmapItem *>(item);
        if (cardItem)
        {
            event->ignore();
            return;
        }
    }

    QGraphicsScene::mousePressEvent(event);
}

/*virtual*/ void BaizeScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) /*override*/
{
    QGraphicsItem *grabbedItem = mouseGrabberItem();

    QGraphicsScene::mouseReleaseEvent(event);

    if (grabbedItem && !mouseGrabberItem())
    {
        if (selectedItems().count() == 1)
        {
            CardPixmapItem *cardItem = dynamic_cast<CardPixmapItem *>(grabbedItem);
            clearSelection();
            if (cardItem)
                emit singleCardMoved(cardItem);
        }
        else
        {
            QList<CardPixmapItem *> cardItems;
            CardPixmapItem *cardItem;
            for (QGraphicsItem *item : selectedItems())
                if ((cardItem = dynamic_cast<CardPixmapItem *>(item)))
                    cardItems.append(cardItem);
            clearSelection();
            if (!cardItems.isEmpty())
                emit multipleCardsMoved(cardItems);
        }
    }

}
