#include "card.h"
#include "selectcardmenu.h"

SelectCardMenu::SelectCardMenu(CardDeck *cardDeck, const QString &title, QWidget *parent/* = nullptr*/)
    : QMenu(title, parent)
{
    this->cardDeck = cardDeck;
    connect(this, &QMenu::aboutToShow, this, [this]() { populateSuitCards(); });
}

void SelectCardMenu::populateSuitCards()
{
    if (suitMenus.isEmpty())
    {
        const QStringList suits{ "Clubs", "Diamonds", "Hearts", "Spades" };
        for (int suit = 0; suit < suits.count(); suit++)
        {
            QMenu *suitMenu = addMenu(suits.at(suit));
            this->suitMenus.append(suitMenu);
            for (int i = 0; i < 13; i++)
            {
                int id = suit + i * 4;
                Card card(id);
                suitMenu->addAction(card.toString());
            }
            connect(suitMenu, &QMenu::triggered, this, [this](QAction *action) { emit cardClicked(action->data().toInt()); });
        }
    }
    for (int suit = 0; suit < suitMenus.count(); suit++)
    {
        QMenu *suitMenu = suitMenus.at(suit);
        for (int i = 0; i < 13; i++)
        {
            QAction *action = static_cast<QAction *>(suitMenu->children().at(i + 1));
            action->setEnabled(false);
            action->setData(QVariant());
        }
    }
    for (int i = cardDeck->nextCardToBeDealt; i < cardDeck->count(); i++)
    {
        Card card(cardDeck->at(i)->id);
        QMenu *suitMenu = suitMenus.at(card.suit());
        QAction *action = static_cast<QAction *>(suitMenu->children().at(card.rank() + 1));
        action->setEnabled(true);
        action->setData(i);
   }
}
