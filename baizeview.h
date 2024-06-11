#ifndef BAIZEVIEW_H
#define BAIZEVIEW_H

#include <QGraphicsView>

class BaizeView : public QGraphicsView
{
    Q_OBJECT

public:
    BaizeView();

protected:
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void scrollContentsBy(int dx, int dy) override;
    virtual void wheelEvent(QWheelEvent *event) override;

signals:
    void viewCoordinatesChanged();
};

#endif // BAIZEVIEW_H
