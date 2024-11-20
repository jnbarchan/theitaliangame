#ifndef CARDGROUP_H
#define CARDGROUP_H

#include <QJsonArray>
#include <QList>

#include "card.h"
#include "carddeck.h"

class CardGroup : public QList<const Card *>
{
private:
    static long _nextUniqueId;
    long _uniqueId;
#ifdef QT_DEBUG
    QString _debugStr;
#endif

    void valueChanged();

public:
    CardGroup();
    CardGroup(std::initializer_list<const Card *> args);

    static void resetNextUniqueId();
    long uniqueId() const { return _uniqueId; }
    enum SetType { RankSet, RunSet };
    QString toString() const;
    int rankDifference(int rank0, int rank1) const;
    void rearrangeForSets();
    bool isGoodRankSet() const;
    bool isGoodRunSet() const;
    bool isGoodSetOfType(SetType setTypeWanted) const;
    bool isGoodSet(SetType &setType) const;
    bool isGoodSet() const;
    void removeCards(const QList<const Card *> &cards);

#ifdef QT_DEBUG
public:
    QList<const Card *> &operator=(QList<const Card *> &&other) { QList<const Card *> &res(QList<const Card *>::operator=(other)); valueChanged(); return res; }
    QList<const Card *> &operator=(const QList<const Card *> &other) { QList<const Card *> &res(QList<const Card *>::operator=(other)); valueChanged(); return res; }
    void append(const Card *value) { QList<const Card *>::append(value); valueChanged(); }
    void append(const QList<const Card *> &value) { QList<const Card *>::append(value); valueChanged(); }
    void clear() { QList<const Card *>::clear(); valueChanged(); }
    void insert(int i, const Card *value) { QList<const Card *>::insert(i, value); valueChanged(); }
    void move(int from, int to) { QList<const Card *>::move(from, to); valueChanged(); }
    void prepend(const Card *value) { QList<const Card *>::prepend(value); valueChanged(); }
    int	removeAll(const Card *value) { int res = QList<const Card *>::removeAll(value); valueChanged(); return res; }
    void removeAt(int i) { QList<const Card *>::removeAt(i); valueChanged(); }
    void removeFirst() { QList<const Card *>::removeFirst(); valueChanged(); }
    void removeLast() { QList<const Card *>::removeLast(); valueChanged(); }
    bool removeOne(const Card *card) { bool res = QList<const Card *>::removeOne(card); if (res) valueChanged(); return res; }
    void replace(int i, const Card *value) { QList<const Card *>::replace(i, value); valueChanged(); }
    void swapItemsAt(int i, int j) { QList<const Card *>::swapItemsAt(i, j); valueChanged(); }
    const Card *takeAt(int i) { const Card *res = QList<const Card *>::takeAt(i); valueChanged(); return res; }
    const Card *takeFirst() { const Card *res = QList<const Card *>::takeFirst(); valueChanged(); return res; }
    const Card *takeLast() { const Card *res = QList<const Card *>::takeLast(); valueChanged(); return res; }

private:
    using QList<const Card *>::operator[];
#endif
};



class CardGroups : public QList<CardGroup>
{
public:
    CardGroups();
    CardGroups(std::initializer_list<CardGroup> args);

    QString toString() const;
    void clearGroups();
    int findCardGroupByUniqueId(int uniqueId) const;
    int findCardInGroups(const Card *card) const;
    int removeCardFromGroups(const Card *card);
    void removeEmptyGroups();
    QList<const Card *> allCards() const;
    QJsonArray serializeToJson() const;
    void deserializeFromJson(const QJsonArray &arr, const CardDeck &cardDeck);
};

#endif // CARDGROUP_H
