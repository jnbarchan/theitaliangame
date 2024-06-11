#ifndef CARDIMAGES_H
#define CARDIMAGES_H

#include <QImage>
#include <QPixmap>
#include <QString>
#include <QVector>

#include "card.h"


class CardImages
{
public:
    CardImages(const QString &dirPath);
    ~CardImages();

    bool foundImages() const { return !cardPixmap(51).isNull(); }
    const QPixmap &cardPixmap(int id) const;
    const QPixmap &cardBackPixmap() const;

private:
    QVector<QPixmap> images;
    QPixmap backImage;
};

#endif // CARDIMAGES_H
