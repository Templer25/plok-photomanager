#include "imageloader_raw.h"

#include <QDebug>
#include <QDateTime>

#include <libraw/libraw.h>

class ImageLoader_raw_p
{
public:
    void openImage (QString imagePath);

    QImage loadThumbnail (void);
    QImage loadImage (void);

    QMap <QString, QVariant> loadInfo (void);
private:
    LibRaw m_rawProcessor;
};


void ImageLoader_raw_p::openImage (QString imagePath)
{
    m_rawProcessor.open_file( imagePath.toAscii() );
}

QImage ImageLoader_raw_p::loadThumbnail ()
{
    m_rawProcessor.unpack_thumb();

    QSize size (m_rawProcessor.imgdata.thumbnail.twidth, m_rawProcessor.imgdata.thumbnail.theight);

    QImage image;
    image.loadFromData(
                reinterpret_cast <unsigned char*> (m_rawProcessor.imgdata.thumbnail.thumb),
                m_rawProcessor.imgdata.thumbnail.tlength,
                "JPG");

    int scale = size.width() / 480;
    if (scale < 1)
        scale = 1;

    QSize scaled_size( size.width() / scale,
                       size.height() / scale);

    return image.scaled (scaled_size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);;
}

QImage ImageLoader_raw_p::loadImage ()
{
    m_rawProcessor.unpack_thumb();

    // TODO load raw image
    QSize size (m_rawProcessor.imgdata.thumbnail.twidth, m_rawProcessor.imgdata.thumbnail.theight);

    QImage image;
    image.loadFromData(
                reinterpret_cast <unsigned char*> (m_rawProcessor.imgdata.thumbnail.thumb),
                m_rawProcessor.imgdata.thumbnail.tlength,
                "JPG");

    return image;
}


QMap <QString, QVariant> ImageLoader_raw_p::loadInfo (void)
{
    QMap <QString, QVariant> info;
    info["model"] = m_rawProcessor.imgdata.idata.model;
    info["make"] = m_rawProcessor.imgdata.idata.make;
    info["aperture"] = m_rawProcessor.imgdata.other.aperture;
    info["artist"] = m_rawProcessor.imgdata.other.artist;
    info["focal_len"] = m_rawProcessor.imgdata.other.focal_len;
    info["iso_speed"] = m_rawProcessor.imgdata.other.iso_speed;
    info["shot_order"] = m_rawProcessor.imgdata.other.shot_order;
    info["shutter"] = m_rawProcessor.imgdata.other.shutter;
    info["timestamp"] = QDateTime::fromTime_t( m_rawProcessor.imgdata.other.timestamp );
    info["black"] = m_rawProcessor.imgdata.color.black;
    info["cam_mul"] = QString::number(m_rawProcessor.imgdata.color.cam_mul[0]) + " " +
            QString::number(m_rawProcessor.imgdata.color.cam_mul[1]) + " " +
            QString::number(m_rawProcessor.imgdata.color.cam_mul[2]) + " " +
            QString::number(m_rawProcessor.imgdata.color.cam_mul[3]);

    return info;
}

ImageLoader_raw::ImageLoader_raw (QObject *parent)
{
    p = new ImageLoader_raw_p();
}

ImageLoader_raw::~ImageLoader_raw()
{
    delete p;
}

void ImageLoader_raw::openImage (QString imagePath)
{
    p->openImage(imagePath);
}

QImage ImageLoader_raw::loadThumbnail()
{
    return p->loadThumbnail();
}

QImage ImageLoader_raw::loadImage()
{
    return p->loadImage();
}

QMap <QString, QVariant> ImageLoader_raw::loadInfo (void)
{
    return p->loadInfo();
}

QStringList ImageLoader_raw::supportedFormats (void)
{
    QStringList filter;

    filter << "arw" << "cr2" << "tif";

    return filter;
}