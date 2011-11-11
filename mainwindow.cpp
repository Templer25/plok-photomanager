#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "imageview.h"

// Qt includes
#include <QFileDialog>
#include <QImageReader>
#include <QElapsedTimer>
#include <QDebug>
#include <QDeclarativeContext>
#include <QDeclarativeItem>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_thumbnailNavigator (0),
    m_thumbnailView (0),
    m_currentPath (".")
{
    ui->setupUi(this);

//    m_thumbnailView = ui->thumbnailView->rootObject()->findChild<QObject*> ("thumbnailView");
//    if (m_thumbnailView)
//        connect( thumbnailView, SIGNAL(loadNewImage(int)), this, SLOT(currentImageChanged(int)));

    m_thumbnailNavigator = ui->thumbnailNavigator->rootObject()->findChild<QObject*> ("thumbnailNavigator");
    if (m_thumbnailNavigator)
        connect( thumbnailNavigator, SIGNAL(loadNewImage(int)), this, SLOT(currentImageChanged(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_fileOpenButton_clicked()
{
    m_currentPath = QFileDialog::getExistingDirectory(this, "Select image directory");
    ui->fileEdit->setText( m_currentPath );

    loadDirectoryThumbnails( m_currentPath );
}

void MainWindow::on_actionUpload_images_triggered( bool checked )
{
    qDebug () << "Upload image";

    QList <QSharedPointer< ThumbnailModelItem > >::iterator it;
    for (it = m_thumbnailModel.begin(); it != m_thumbnailModel.end(); it++)
    {
        if ((*it)->selected())
        {
            qDebug () << "Uploading: " << (*it)->path();

            uploadImage( (*it)->path() );
        }
    }
}

void MainWindow::loadDirectoryThumbnails (QString dirName)
{
    QDir dir (dirName);
    dir.setFilter( QDir::Files );
    dir.setNameFilters(QStringList() << "*.jpg");

    QList <QFileInfo> file_info_list = dir.entryInfoList();
    QList <QFileInfo>::iterator it;

    QList<QObject*> modelList;
    m_thumbnailModel.clear();

    for (it = file_info_list.begin(); it != file_info_list.end(); it++)
    {
        QString file_name = it->fileName();
        QString file_path = QString ("file://") + it->filePath();

        QSharedPointer <ThumbnailModelItem> model_item (new ThumbnailModelItem( file_name, file_path ));

        m_thumbnailModel.push_back( model_item );
        modelList.append( &(*model_item) );
    }

    QDeclarativeContext *context = ui->thumbnailView->rootContext();
    context->setContextProperty("thumbnailViewModel", QVariant::fromValue<QList<QObject*> >(modelList));
    context->setContextProperty("mainWindow", this);

    QDeclarativeContext *nav_context = ui->thumbnailNavigator->rootContext();
    nav_context->setContextProperty("thumbnailViewModel", QVariant::fromValue<QList<QObject*> >(modelList));
    nav_context->setContextProperty("mainWindow", this);
}

void MainWindow::loadImage (QString fileName)
{
    QElapsedTimer timer;
    timer.start();

    QImageReader image_reader (fileName);
    if (!image_reader.canRead())
        return;

    image_reader.setQuality(25);

    m_currentImage = fileName;

    QSize image_size = image_reader.size();

    int scale = image_size.width() / 800;

    QSize scaled_size( image_size.width() / scale, image_size.height() / scale);

    image_reader.setScaledSize ( scaled_size);

    QImage image = image_reader.read();

    qDebug () << "image loading: " << timer.elapsed();

    ui->imageView->setImage (image);

    qDebug () << "drawing: " << timer.elapsed();
}

void MainWindow::uploadImage (QString fileName)
{
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart uuidPart;
    uuidPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"uuid\""));
    uuidPart.setBody("");

    QHttpPart filenamePart;
    filenamePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"filename\""));
    filenamePart.setBody( QFileInfo(fileName).fileName().toAscii() );

    qDebug() << QFileInfo(fileName).fileName();

    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/jpeg"));
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"datafile\""));

    QFile *file = new QFile( QUrl(fileName).toString(QUrl::RemoveScheme) );
    file->open(QIODevice::ReadOnly);
    imagePart.setBodyDevice(file);

    file->setParent(multiPart); // we cannot delete the file now, so delete it with the multiPart

    multiPart->append(uuidPart);
    multiPart->append(filenamePart);
    multiPart->append(imagePart);

    QUrl url("http://plok.org:8282/plokmc-1.0-SNAPSHOT/fileUpload");
    QNetworkRequest request(url);

    QNetworkReply *reply = m_networkAccessManager.post(request, multiPart);
    multiPart->setParent(reply); // delete the multiPart with the reply
    // here connect signals etc.

    connect( reply, SIGNAL( error(QNetworkReply::NetworkError)), this, SLOT(uploadError(QNetworkReply::NetworkError)));
    connect( reply, SIGNAL( uploadProgress (qint64, qint64)), this, SLOT(uploadProgress (qint64, qint64)));
    connect( reply, SIGNAL( finished()), this, SLOT(uploadFinished()));
}

void MainWindow::currentImageChanged (int currentIndex)
{
    m_currentPath = QUrl (m_thumbnailModel.at( currentIndex )->path()).toString(QUrl::RemoveScheme);
    m_currentImage = m_thumbnailModel.at( currentIndex )->name();

    qDebug() << m_currentPath;

    loadImage (m_currentPath);

    ui->mainTabWidget->setCurrentWidget(ui->imageViewPage);
}

void MainWindow::currentSelectionChanged (int currentIndex)
{
    m_thumbnailModel.at( currentIndex )->toggleSelected();
}

void MainWindow::doubleClickOnThumbnail( int currentIndex )
{
    m_currentPath = QUrl (m_thumbnailModel.at( currentIndex )->path()).toString(QUrl::RemoveScheme);
    m_currentImage = m_thumbnailModel.at( currentIndex )->name();

    qDebug() << m_currentPath;

    loadImage (m_currentPath);

    if (m_thumbnailView)
        m_thumbnailView->setProperty("currentIndex", currentIndex);

    if (m_thumbnailNavigator)
        m_thumbnailNavigator->setProperty("currentIndex", currentIndex);

    ui->mainTabWidget->setCurrentWidget(ui->imageViewPage);
}

void MainWindow::uploadError (QNetworkReply::NetworkError code)
{
    QString error_string;

    qDebug() << "Upload error: " << code;
}

void MainWindow::uploadProgress (qint64 bytesSent, qint64 bytesTotal )
{
    qDebug() << "Bytes send: " << bytesSent << "\t Bytes total: " << bytesTotal;
}

void MainWindow::uploadFinished (void)
{
    qDebug() << "Upload finished!";
}

