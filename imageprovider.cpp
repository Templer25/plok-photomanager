#include "imageprovider.h"

#include <QFileInfo>
#include <QDebug>

#include <QElapsedTimer>

#include "imageloader_generic.h"
#include "imageloader_raw.h"

ImageProvider::ImageProvider(QObject *parent) :
    QDeclarativeImageProvider( QDeclarativeImageProvider::Image )
{
    m_imageLoader_generic = new ImageLoader_generic();
    m_imageLoader_raw = new ImageLoader_raw();
}

ImageProvider::~ImageProvider ()
{
    delete m_imageLoader_generic;
    delete m_imageLoader_raw;
}

QImage ImageProvider::requestImage ( const QString& id, QSize* size, const QSize& requestedSize )
{
    QString suffix = QFileInfo (id).suffix();

    QElapsedTimer stopwatch;
    stopwatch.start();

    QImage thumb;
    if (suffix == "RAW" || suffix == "raw" ||
            suffix == "ARW" || suffix == "arw" ||
            suffix == "CR2" || suffix == "cr2")
    {
        qDebug () << "Raw ImageProvider: " << id;

        m_imageLoader_raw->openImage( id );
        thumb = m_imageLoader_raw->loadThumbnail();

        qDebug() << "Raw ImageProvider load time: " << stopwatch.elapsed();
    } else
    {
        qDebug () << "Generic ImageProvider: " << id;

        m_imageLoader_generic->openImage( id );
        thumb = m_imageLoader_generic->loadThumbnail();

        qDebug() << "Generic ImageProvider load time: " << stopwatch.elapsed();
    }

    *size = thumb.size();

    stopwatch.start();

    QImage result_image;

    if (requestedSize.isValid())
    {
        if (requestedSize.width() == 0)
            result_image = thumb.scaledToHeight(requestedSize.height());
        if (requestedSize.height() == 0)
            result_image = thumb.scaledToWidth(requestedSize.width());

        if (requestedSize.width() && requestedSize.height())
            result_image = thumb.scaled( requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    else
        result_image = thumb;

    qDebug() << "ImageProvider scale time: " << stopwatch.elapsed();

    return result_image;
}

