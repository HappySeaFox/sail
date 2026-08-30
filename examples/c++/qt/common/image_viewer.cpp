/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2020 Dmitry Baryshev

    The MIT License

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include <algorithm>

#include <QCheckBox>
#include <QDir>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QScrollArea>
#include <QTimer>
#include <QToolButton>

#include <sail-common/sail-common.h>

#include "image_viewer.h"
#include "ui_image_viewer.h"

ImageViewer::ImageViewer(Features features, QWidget* parent)
    : QWidget(parent)
    , m_ui(new Ui::ImageViewer)
    , m_animationTimer(new QTimer)
    , m_imageLabel(new QLabel)
    , m_features(features)
{
    m_ui->setupUi(this);

    m_imageLabel->setAlignment(Qt::AlignCenter);
    m_ui->scrollArea->setWidget(m_imageLabel);

    m_animationTimer->setSingleShot(true);

    // Hide the controls the example doesn't ask for.
    //
    m_ui->pushProbe->setVisible(m_features.testFlag(Probe));

    const bool multiFrame = m_features.testFlag(MultiFrame);

    m_ui->labelFrame->setVisible(multiFrame);
    m_ui->pushStop->setVisible(multiFrame);
    m_ui->pushPrevious->setVisible(multiFrame);
    m_ui->pushNext->setVisible(multiFrame);

    connect(m_animationTimer.data(), &QTimer::timeout, this, &ImageViewer::onNext);

    connect(m_ui->pushOpen, &QToolButton::clicked, this, &ImageViewer::onOpenFile);
    connect(m_ui->pushProbe, &QToolButton::clicked, this, &ImageViewer::onProbe);
    connect(m_ui->pushSave, &QToolButton::clicked, this, &ImageViewer::onSave);
    connect(m_ui->pushStop, &QToolButton::clicked, this, &ImageViewer::onStop);
    connect(m_ui->pushPrevious, &QToolButton::clicked, this, &ImageViewer::onPrevious);
    connect(m_ui->pushNext, &QToolButton::clicked, this, &ImageViewer::onNext);
    connect(m_ui->checkFit, &QCheckBox::toggled, this, &ImageViewer::onFit);
}

ImageViewer::~ImageViewer() = default;

sail_status_t ImageViewer::probeImage(const QString& /* path */, QString* /* info */)
{
    SAIL_LOG_AND_RETURN(SAIL_ERROR_NOT_IMPLEMENTED);
}

QStringList ImageViewer::filters() const
{
    return QStringList{QStringLiteral("All Files (*.*)")};
}

void ImageViewer::setStatus(const QString& text)
{
    m_ui->labelStatus->setText(text);
}

const QStringList& ImageViewer::cachedFilters() const
{
    // The list of the available codecs never changes at run time, so build the filters just once.
    //
    if (m_filters.isEmpty())
    {
        m_filters = filters();
    }

    return m_filters;
}

void ImageViewer::onOpenFile()
{
    const QString path =
        QFileDialog::getOpenFileName(this, tr("Select a file"), QString(), cachedFilters().join(QStringLiteral(";;")));

    if (path.isEmpty())
    {
        return;
    }

    openImage(path);
}

void ImageViewer::onProbe()
{
    const QString path = QFileDialog::getOpenFileName(this, tr("Select a file"));

    if (path.isEmpty())
    {
        return;
    }

    QElapsedTimer elapsedTimer;
    elapsedTimer.start();

    QString info;
    const sail_status_t res = probeImage(path, &info);

    if (res != SAIL_OK)
    {
        QMessageBox::critical(this, tr("Error"), tr("Failed to probe '%1'. Error: %2.").arg(path).arg(res));
        return;
    }

    QMessageBox::information(this, tr("File info"), tr("Probed in: %1 ms.\n%2").arg(elapsedTimer.elapsed()).arg(info));
}

void ImageViewer::onSave()
{
    if (m_qimages.isEmpty())
    {
        QMessageBox::warning(this, tr("Save image"), tr("Load an image first."));
        return;
    }

    const QString path =
        QFileDialog::getSaveFileName(this, tr("Select a file"), QString(), cachedFilters().join(QStringLiteral(";;")));

    if (path.isEmpty())
    {
        return;
    }

    const sail_status_t res = saveImage(path, m_qimages[m_currentIndex]);

    if (res != SAIL_OK)
    {
        QMessageBox::critical(this, tr("Error"), tr("Failed to save '%1'. Error: %2.").arg(path).arg(res));
        return;
    }

    const QMessageBox::StandardButton answer = QMessageBox::question(
        this, tr("Open file"),
        tr("%1 has been saved successfully. Open the saved file?").arg(QDir::toNativeSeparators(path)));

    if (answer == QMessageBox::Yes)
    {
        openImage(path);
    }
}

void ImageViewer::onFit()
{
    updateImage();
}

void ImageViewer::onPrevious()
{
    const int count = static_cast<int>(m_qimages.size());

    if (count <= 1)
    {
        return;
    }

    m_currentIndex = (m_currentIndex + count - 1) % count;

    SAIL_LOG_DEBUG("Image index: %d", m_currentIndex);

    updateImage();
}

void ImageViewer::onNext()
{
    const int count = static_cast<int>(m_qimages.size());

    if (count <= 1)
    {
        return;
    }

    m_currentIndex = (m_currentIndex + 1) % count;

    SAIL_LOG_DEBUG("Image index: %d", m_currentIndex);

    updateImage();

    if (m_animated)
    {
        m_animationTimer->start(m_delays[m_currentIndex]);
    }
}

void ImageViewer::onStop()
{
    m_animationTimer->stop();
    m_animated = false;
}

void ImageViewer::openImage(const QString& path)
{
    QVector<QImage> qimages;
    QVector<int> delays;

    const sail_status_t res = loadImage(path, &qimages, &delays);

    if (res != SAIL_OK)
    {
        QMessageBox::critical(this, tr("Error"), tr("Failed to load '%1'. Error: %2.").arg(path).arg(res));
        return;
    }

    if (qimages.isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("'%1' contains no images.").arg(path));
        return;
    }

    onStop();

    m_qimages      = qimages;
    m_delays       = delays;
    m_currentIndex = 0;

    // Frames without a reported delay are not animated.
    //
    m_delays.resize(m_qimages.size());

    updateImage();
    detectAnimated();
}

void ImageViewer::updateImage()
{
    if (m_qimages.isEmpty())
    {
        m_imageLabel->clear();
        return;
    }

    const QImage& qimage     = m_qimages[m_currentIndex];
    const QSize viewportSize = m_ui->scrollArea->viewport()->size();

    const bool scale = m_ui->checkFit->isChecked()
                       && (qimage.width() > viewportSize.width() || qimage.height() > viewportSize.height());

    m_imageLabel->setPixmap(QPixmap::fromImage(
        scale ? qimage.scaled(viewportSize, Qt::KeepAspectRatio, Qt::SmoothTransformation) : qimage));

    updateFrameLabel();
}

void ImageViewer::updateFrameLabel()
{
    m_ui->labelFrame->setText(QStringLiteral("%1/%2").arg(m_currentIndex + 1).arg(m_qimages.size()));
}

void ImageViewer::detectAnimated()
{
    m_animated = m_features.testFlag(MultiFrame)
                 && std::any_of(m_delays.cbegin(), m_delays.cend(), [](int delay) { return delay > 0; });

    if (m_animated)
    {
        m_animationTimer->start(m_delays.first());
    }
}
