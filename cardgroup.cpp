#include <QDebug>

#include "utils.h"
#include "cardgroup.h"

/*static*/ long CardGroup::_nextUniqueId = 1L;

CardGroup::CardGroup()
{
    this->_uniqueId = _nextUniqueId++;
}

CardGroup::CardGroup(std::initializer_list<const Card *> args) :
    QList<const Card *>(args)
{
    this->_uniqueId = _nextUniqueId++;
}

/*static*/ void CardGroup::resetNextUniqueId()
{
    CardGroup::_nextUniqueId = 1L;
}

QString CardGroup::toString() const
{
    QString str;
    for (int i = 0; i < count(); i++)
    {
        if (i > 0)
            str += ", ";
        str += at(i)->toString();
    }
    return str;
}

int CardGroup::rankDifference(int rank0, int rank1) const
{
    // return the difference in rank *from* rank1 *to* rank0, i.e. rank0 - rank1 (a la strcmp())
    // this can be positive (rank0 > rank1) or negative (rank0 < rank1) or zero (rank0 == rank1)
    // this deals with the "wraparound" which occurs at an Ace, which can be high or low (like 2-A-K)
    // we meassure the distance in both directions and return the number (positive or negative) whose *absolute* value is smallest
    int diff0(rank0 - rank1);
    int diff1((diff0 > 0) ? diff0 - 13 : diff0 + 13);
    return (qAbs(diff0) < qAbs(diff1)) ? diff0 : diff1;
}

void CardGroup::rearrangeForSets()
{
    // see if we wish to rearrange the cards in a group suitably to make sets
    // we try to maintain the order of what we have already, based on the first/left-hand card
    if (count() < 2)
        return;
    const Card *card0(at(0)), *card1(at(1));
    if (count() == 2)
    {
        if (card0->suit() == card1->suit())
            if (rankDifference(card0->rank(), card1->rank()) == -1)
                swapItemsAt(0, 1);
        return;
    }
    if (card0->rank() == card1->rank())
    {
        // same ranks
        int nextArrangedIndex = 2;
        for (int i = nextArrangedIndex; i < count(); i++)
            if (at(i)->rank() == card0->rank())
                move(i, nextArrangedIndex++);
    }
    else if (card0->suit() == card1->suit())
    {
        // same suits
        int nextArrangedIndex = 1;
        while (nextArrangedIndex < count() &&
               at(nextArrangedIndex)->suit() == at(nextArrangedIndex - 1)->suit() && rankDifference(at(nextArrangedIndex)->rank(), at(nextArrangedIndex - 1)->rank()) == -1)
            nextArrangedIndex++;
        for (int i = nextArrangedIndex; i < count(); i++)
        {
            if (at(i)->suit() == at(nextArrangedIndex - 1)->suit() && rankDifference(at(i)->rank(), at(nextArrangedIndex - 1)->rank()) == -1)
            {
                move(i, nextArrangedIndex);
                i = ++nextArrangedIndex - 1;
            }
            else if (at(i)->suit() == at(0)->suit() && rankDifference(at(i)->rank(), at(0)->rank()) == 1)
            {
                move(i, 0);
                i = ++nextArrangedIndex - 1;
            }
        }
    }
}

bool CardGroup::isGoodRankSet() const
{
    if (count() < 3)
        return false;
    // check for same rank (different suits)
    QSet<int> suits;
    for (const Card* card : *this)
        if (card->rank() != at(0)->rank() || suits.contains(card->suit()))
            return false;
        else
            suits.insert(card->suit());
    return true;
}

bool CardGroup::isGoodRunSet() const
{
    if (count() < 3)
        return false;
    // check for sequential cards in same suit
    // (assumes ordered as per `rearrangeForSets`)
    for (int i = 1; i < count(); i++)
        if (at(i)->suit() != at(i - 1)->suit() || rankDifference(at(i)->rank(), at(i - 1)->rank()) != -1)
            return false;
    return true;
}

bool CardGroup::isGoodSetOfType(SetType setTypeWanted) const
{
    return (setTypeWanted == SetType::RankSet) ? isGoodRankSet() : (setTypeWanted == SetType::RunSet) ? isGoodRunSet() : false;
}

bool CardGroup::isGoodSet(SetType &setType) const
{
    if (isGoodRankSet())
    {
        setType = SetType::RankSet;
        return true;
    }
    else if (isGoodRunSet())
    {
        setType = SetType::RunSet;
        return true;
    }
    return false;
}

bool CardGroup::isGoodSet() const
{
    SetType setType;
    return isGoodSet(setType);
}

void CardGroup::removeCards(const QList<const Card *> &cards)
{
    for (const Card *card : cards)
        removeOne(card);
}



CardGroups::CardGroups()
{

}

CardGroups::CardGroups(std::initializer_list<CardGroup> args) :
    QList<CardGroup>(args)
{
}

QString CardGroups::toString() const
{
    QString str;
    for (int i = 0; i < count(); i++)
    {
        if (i > 0)
            str += ", ";
        str += QString("[%1]").arg(at(i).toString());
    }
    return str;
}

void CardGroups::clearGroups()
{
    clear();
    CardGroup::resetNextUniqueId();
}

int CardGroups::findCardGroupByUniqueId(int uniqueId) const
{
    for (int i = 0; i < count(); i++)
        if (at(i).uniqueId() == uniqueId)
            return i;
    return -1;
}

int CardGroups::findCardInGroups(const Card *card) const
{
    for (int i = 0; i < count(); i++)
        if (at(i).indexOf(card) >= 0)
            return i;
    return -1;
}

int CardGroups::removeCardFromGroups(const Card *card)
{
    for (int i = 0; i < count(); i++)
        if ((*this)[i].removeOne(card))
            return i;
    return -1;
}

void CardGroups::removeEmptyGroups()
{
    for (int i = count() - 1; i >= 0; i--)
        if (at(i).isEmpty())
            removeAt(i);
}

QList<const Card *> CardGroups::allCards() const
{
    QList<const Card *> cards;
    for (const CardGroup &cardGroup : *this)
        cards.append(cardGroup);
    return cards;
}

QJsonArray CardGroups::serializeToJson() const
{
    QJsonArray arr;
    for (const CardGroup &group : *this)
    {
        QJsonArray arr0;
        for (const Card *card : group)
            arr0.append(card->id);
        arr.append(arr0);
    }
    return arr;
}

void CardGroups::deserializeFromJson(const QJsonArray &arr, const CardDeck &cardDeck)
{
    clearGroups();
    for (int i = 0; i < arr.count(); i++)
    {
        CardGroup group;
        const QJsonArray &arr0(arr.at(i).toArray());
        for (const auto &val : arr0)
            group.append(cardDeck.findCard(val.toInt()));
        append(group);
    }
}
