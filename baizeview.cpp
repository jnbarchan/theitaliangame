#include <QDebug>
#include <QWheelEvent>

#include "baizeview.h"

BaizeView::BaizeView()
{

}

/*virtual*/ void BaizeView::resizeEvent(QResizeEvent *event) /*override*/
{
    QGraphicsView::resizeEvent(event);
    emit viewCoordinatesChanged();
}

/*virtual*/ void BaizeView::scrollContentsBy(int dx, int dy) /*override*/
{
    QGraphicsView::scrollContentsBy(dx, dy);
    emit viewCoordinatesChanged();
}

/*virtual*/ void BaizeView::wheelEvent(QWheelEvent *event) /*override*/
{
    // Do a wheel-based zoom about the cursor position
    // (Adapted from https://stackoverflow.com/questions/19113532/qgraphicsview-zooming-in-and-out-under-mouse-position-using-mouse-wheel)
    event->accept();

    int zoomBy = event->angleDelta().y() / 120;
    qreal currentZoom = transform().m11();
    qreal factor;
    if (zoomBy < 0 && currentZoom < 3.0)
        factor = 1.1;
    else if (zoomBy > 0 && currentZoom > 0.333)
        factor = 1.0 / 1.1;
    else
        return;

    if (false)
    {
        // moves scroll position to odd place
        const ViewportAnchor anchor = transformationAnchor();
        setTransformationAnchor(AnchorUnderMouse);
        scale(factor, factor);
        setTransformationAnchor(anchor);
    }
    else
    {
        // seems to get it right
        auto targetViewportPos = event->position();
        auto targetScenePos = mapToScene(event->position().toPoint());
        scale(factor, factor);
        centerOn(targetScenePos);
        QPointF deltaViewportPos = targetViewportPos - QPointF(viewport()->width() / 2.0, viewport()->height() / 2.0);
        QPointF viewportCenter = mapFromScene(targetScenePos) - deltaViewportPos;
        centerOn(mapToScene(viewportCenter.toPoint()));
    }
    emit viewCoordinatesChanged();
}
