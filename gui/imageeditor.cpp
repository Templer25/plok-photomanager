#include "imageeditor.h"
#include "ui_imageeditor.h"

#include "image/imageprovider.h"
#include "image/imageprocessing.h"

#include <QFileInfo>
#include <QDebug>

ImageEditor::ImageEditor(ImageProvider* imageProvider, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageEditor),
    m_imageProvider (imageProvider)
{
    ui->setupUi(this);

    updateLut();

    connect (ui->brightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(guiChanged()));
    connect (ui->contrastSlider, SIGNAL(valueChanged(int)), this, SLOT(guiChanged()));
    connect (ui->gammaSpinBox, SIGNAL(valueChanged(double)), this, SLOT(guiChanged()));

    connect (ui->redSpinBox, SIGNAL(valueChanged(double)), this, SLOT(guiChanged()));
    connect (ui->greenSpinBox, SIGNAL(valueChanged(double)), this, SLOT(guiChanged()));
    connect (ui->blueSpinBox, SIGNAL(valueChanged(double)), this, SLOT(guiChanged()));

    connect (ui->medianCheckBox, SIGNAL(toggled(bool)), this, SLOT(guiChanged()));
}

ImageEditor::~ImageEditor()
{
    delete ui;
}

void ImageEditor::blockSignals (bool block)
{
    QList<QWidget*> children = findChildren<QWidget*> ();

    for (int i=0; i<children.size(); i++)
    {
        children[i]->blockSignals (block);
    }
    QWidget::blockSignals(block);
}

void ImageEditor::setCapture(Capture capture)
{
    m_capture = capture;

    QStringList pictures = m_capture.photoList();

    if (!pictures.size())
        return;

    // TODO just select raw for now

    for (int i=0; i<pictures.size(); i++)
    {
        if (QFileInfo(pictures.at(i)).suffix() == "ARW" ||
            QFileInfo(pictures.at(i)).suffix() == "arw" ||
            QFileInfo(pictures.at(i)).suffix() == "CR2" ||
            QFileInfo(pictures.at(i)).suffix() == "cr2")
            m_currentPicture = pictures.at(i);
    }

    qDebug () << "setCapture:";

    Picture current_picture = currentPicture();

    m_workJpegMaster.clear();
    m_workRawMaster.clear();

    Image image;
    if (ui->imageDeveloper->currentIndex() == 0)
    {
        image = m_imageProvider->loadPreview (m_currentPicture);
        m_workJpegMaster = ImageProcessing::fastScale (image, ui->imageView->size());
    }
    if (ui->imageDeveloper->currentIndex() == 1)
    {
        image = m_imageProvider->loadMaster (m_currentPicture);
        m_workRawMaster = ImageProcessing::fastScale (image, ui->imageView->size());
    }

    current_picture.setImage( image );
    loadGUI_pictureProperties( current_picture.pictureProperties() );


    qDebug () << "assign/scale m_workImage:";


    updateLut( );
}

void ImageEditor::on_imageDeveloper_currentIndexChanged(const int &currentIndex)
{
    qDebug () << "currentIndexChanged:";

    Picture& current_picture = currentPicture();

    if (ui->imageDeveloper->currentIndex() == 0)
    {
        if (!current_picture.loaded())
            current_picture.setImage( m_imageProvider->loadPreview (m_currentPicture) );

        if (m_workJpegMaster.isNull())
            m_workJpegMaster = ImageProcessing::fastScale (current_picture.image(), ui->imageView->size());
    }
    if (ui->imageDeveloper->currentIndex() == 1)
    {
       if (!current_picture.loaded())
            current_picture.setImage( m_imageProvider->loadMaster (m_currentPicture) );

        if (m_workRawMaster.isNull())
            m_workRawMaster = ImageProcessing::fastScale (current_picture.image(), ui->imageView->size());
    }

    loadGUI_pictureProperties( current_picture.pictureProperties() );

    qDebug () << "assign/scale m_workImage:";

//    m_workImage = ImageProcessing::fastScale (current_picture.image(), ui->imageView->size());

    updateLut ();
}

void ImageEditor::updateHistogram ( const Image& image )
{
    Histogram histogram (image.depth());

    ImageProcessing::createHistogram (image, histogram);

    ui->histogramView->setHistogram( histogram );
}

void ImageEditor::updateLut (void)
{
    qDebug () << "updateLut:";


    PictureProperties& picture_properties = currentPicture().pictureProperties();
    const Image& work_image = currentImage();

    Lut lut (work_image.depth());

    int* red = lut.red();
    int* green = lut.green();
    int* blue = lut.blue();

    if (!red || !green || !blue)
        return;

    float contrast = picture_properties.contrast;
    float brightness = picture_properties.brightness;
    float gamma = picture_properties.gamma;

    float wb_red = picture_properties.wb_red;
    float wb_green = picture_properties.wb_green;
    float wb_blue = picture_properties.wb_blue;

    ImageProcessing::generateLut(brightness, contrast, gamma, wb_red, wb_green, wb_blue, lut);

    ui->lutView->setLut( lut );

    Image dest_image (work_image.size(), work_image.channels(), work_image.size().width() * work_image.channels(), 8);

    if (work_image.depth() == 16 && ui->medianCheckBox->isChecked())
    {
#ifdef OPENCV
        Image tmp_image1 (work_image.size(), work_image.channels(), work_image.size().width() * work_image.channels() * 2, 16);

        ImageProcessing::medianFilter_16u(work_image, tmp_image1, 3);

        qDebug () << "applyLut:";

        ImageProcessing::applyLut (&tmp_image1, &dest_image, lut);
#else
        ImageProcessing::applyLut (&work_image, &dest_image, lut);
#endif
    } else
    {
        ImageProcessing::applyLut (&work_image, &dest_image, lut);
    }

    qDebug () << "updateLut -> imageView->setImage:";

    ui->imageView->setImage(dest_image.toQImage());

    qDebug () << "updateHistogram:";

    updateHistogram( dest_image );
}

void ImageEditor::guiChanged (void)
{
    PictureProperties& picture_adjustment = currentPicture().pictureProperties();

    picture_adjustment.brightness = float(ui->brightnessSlider->value()) / 100;
    picture_adjustment.contrast = 1.f + (float(ui->contrastSlider->value()) / 100);
    picture_adjustment.gamma = 1.f / float(ui->gammaSpinBox->value());

    picture_adjustment.wb_red = ui->redSpinBox->value();
    picture_adjustment.wb_green = ui->greenSpinBox->value();
    picture_adjustment.wb_blue = ui->blueSpinBox->value();

    updateLut();
}

void ImageEditor::loadGUI_pictureProperties (PictureProperties pictureAdjustments)
{
    blockSignals(true);

    float brightness = pictureAdjustments.brightness * 100;
    float contrast = (pictureAdjustments.contrast - 1.f) * 100;

    ui->brightnessSlider->setValue( brightness );
    ui->brightnessSpinBox->setValue( brightness );
    ui->contrastSlider->setValue( contrast );
    ui->contrastSpinBox->setValue( contrast );

    ui->gammaSpinBox->setValue( 1.f / float(pictureAdjustments.gamma) );

    ui->redSpinBox->setValue( pictureAdjustments.wb_red );
    ui->greenSpinBox->setValue( pictureAdjustments.wb_green );
    ui->blueSpinBox->setValue( pictureAdjustments.wb_blue );

    blockSignals (false);
}

Picture& ImageEditor::currentPicture (void)
{
    if (ui->imageDeveloper->currentIndex() == 0)
        return m_capture.preview();

    if (ui->imageDeveloper->currentIndex() == 1)
        return m_capture.rawMaster();
}

Image& ImageEditor::currentImage (void)
{
    if (ui->imageDeveloper->currentIndex() == 0)
        return m_workJpegMaster;

    if (ui->imageDeveloper->currentIndex() == 1)
        return m_workRawMaster;
}


