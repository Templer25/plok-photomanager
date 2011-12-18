#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include <QDeclarativeImageProvider>
#include <QImage>

class ImageLoader;

class ImageProvider
{
public:
    explicit ImageProvider(QObject *parent = 0);
    virtual ~ImageProvider (void);

    QImage requestImage ( const QString& id, QSize* size, const QSize& requestedSize );

    ImageLoader* imageLoaderFromFormat (QString format);

    QStringList supportedSuffixes (void);
private:
    QList <ImageLoader*> m_imageLoaders;
};

#endif // IMAGEPROVIDER_H
