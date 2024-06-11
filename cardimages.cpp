#include <QColor>
#include <QDebug>
#include <QDir>

#include "cardimages.h"

CardImages::CardImages(const QString &dirPath)
{
    images.clear();
    images.resize(52);
    for (int i = 0; i < 52; i++)
    {
        QString filePath = dirPath + "/" + QString("%1").arg(i, 2, 10, QChar('0')) + ".png";
        images[i].load(filePath);
    }
    QString filePath = dirPath + "/../" + "cardback" + ".png";
    backImage.load(filePath);
}

CardImages::~CardImages()
{
    images.clear();
}

const QPixmap &CardImages::cardPixmap(int id) const
{
    Q_ASSERT(id >= 0 && id < 52 * 2);
    return images.at(id % 52);
}

const QPixmap &CardImages::cardBackPixmap() const
{
    return backImage;
}
