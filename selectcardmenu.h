#ifndef SELECTCARDMENU_H
#define SELECTCARDMENU_H

#include <QMenu>
#include <QObject>

#include "carddeck.h"

class SelectCardMenu : public QMenu
{
    Q_OBJECT

public:
    SelectCardMenu(CardDeck *cardDeck, const QString &title, QWidget *parent = nullptr);

private:
    CardDeck *cardDeck;
    QList<QMenu *> suitMenus;
    void populateSuitCards();

signals:
    void cardClicked(int id);
};

#endif // SELECTCARDMENU_H
