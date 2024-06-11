#include <QDebug>

#include "cardgroup.h"

CardGroup::CardGroup()
{

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

bool CardGroup::isGoodSet() const
{
    if (count() < 3)
        return false;
    if (at(1)->rank() == at(0)->rank())
    {
        // check for same rank (different suits)
        QSet<int> suits;
        for (const Card* card : *this)
            if (card->rank() != at(0)->rank() || suits.contains(card->suit()))
                return false;
            else
                suits.insert(card->suit());
        return true;
    }
    else if (at(1)->suit() == at(0)->suit())
    {
        // check for sequential ranks in same suit
        // (assumes ordered as per `rearrangeForSets`)
        for (int i = 1; i < count(); i++)
            if (at(i)->suit() != at(i - 1)->suit() || rankDifference(at(i)->rank(), at(i - 1)->rank()) != -1)
                return false;
        return true;
    }
    return false;
}



CardGroups::CardGroups()
{

}

void CardGroups::clearGroups()
{
    clear();
}

int CardGroups::findCardInGroups(const Card *card) const
{
    for (int i = 0; i < count(); i++)
        if (at(i).indexOf(card) >= 0)
            return i;
    return -1;
//    for (auto &group : *this)
//        return &group;
    //    return nullptr;
}

void CardGroups::removeEmptyGroups()
{
    for (int i = count() - 1; i >= 0; i--)
        if (at(i).isEmpty())
            removeAt(i);;
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
