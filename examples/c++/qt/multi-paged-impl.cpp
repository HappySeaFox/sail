/*  This file is part of SAIL (https://github.com/smoked-herring/sail)

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

QtSail::QtSail(QWidget *parent)
    : QWidget(parent)
{
    m_ui.reset(new Ui::QtSail);
    m_ui->setupUi(this);

    QLabel *l = new QLabel;
    l->setAlignment(Qt::AlignCenter);
    m_ui->scrollArea->setWidget(l);

    m_animationTimer.reset(new QTimer);
    m_animationTimer->setSingleShot(true);
    connect(m_animationTimer.data(), &QTimer::timeout, this, [&]{
        onNext();
    });

    connect(m_ui->pushOpen,     &QPushButton::clicked, this, &QtSail::onOpenFile);
    connect(m_ui->pushProbe,    &QPushButton::clicked, this, &QtSail::onProbe);
    connect(m_ui->pushSave,     &QPushButton::clicked, this, &QtSail::onSave);
    connect(m_ui->checkFit,     &QCheckBox::toggled,   this, &QtSail::onFit);
    connect(m_ui->pushPrevious, &QPushButton::clicked, this, &QtSail::onPrevious);
    connect(m_ui->pushNext,     &QPushButton::clicked, this, &QtSail::onNext);
    connect(m_ui->pushStop,     &QPushButton::clicked, this, &QtSail::onStop);

    init();
}

void QtSail::onFit(bool fit)
{
    QPixmap pixmap;

    if (m_qimages.empty()) {
        return;
    }

    const QImage &qimage = m_qimages[m_currentIndex];

    if (fit) {
        if (qimage.width() > m_ui->scrollArea->viewport()->width() ||
                qimage.height() > m_ui->scrollArea->viewport()->height()) {
            pixmap = QPixmap::fromImage(qimage.scaled(m_ui->scrollArea->viewport()->size(),
                                                         Qt::KeepAspectRatio,
                                                         Qt::SmoothTransformation));
        } else {
            pixmap =  QPixmap::fromImage(qimage);
        }
    } else {
        pixmap =  QPixmap::fromImage(qimage);
    }

    qobject_cast<QLabel *>(m_ui->scrollArea->widget())->setPixmap(pixmap);
}

void QtSail::onPrevious()
{
    if (m_qimages.size() <= 1) {
        return;
    }

    if (m_currentIndex == 0) {
        m_currentIndex = m_qimages.size()-1;
    } else {
        m_currentIndex--;
    }

    SAIL_LOG_DEBUG("Image index: %d", m_currentIndex);
    onFit(m_ui->checkFit->isChecked());

    updateCurrentFrameLabel();
}

void QtSail::onNext()
{
    if (m_qimages.size() <= 1) {
        return;
    }

    if (m_currentIndex == m_qimages.size()-1) {
        m_currentIndex = 0;
    } else {
        m_currentIndex++;
    }

    SAIL_LOG_DEBUG("Image index: %d", m_currentIndex);
    onFit(m_ui->checkFit->isChecked());

    if (m_animated) {
        m_animationTimer->start(m_delays[m_currentIndex]);
    }

    updateCurrentFrameLabel();
}

void QtSail::onStop()
{
    m_animationTimer->stop();
    m_animated = false;
}

void QtSail::detectAnimated()
{
    m_animated = std::find_if(m_delays.begin(), m_delays.end(), [&](int v) {
        return v > 0;
    }) != m_delays.end();

    if (m_animated) {
        m_animationTimer->start(m_delays.first());
    }

    updateCurrentFrameLabel();
}

void QtSail::updateCurrentFrameLabel()
{
    m_ui->labelFrame->setText(QString("%1/%2").arg(m_currentIndex+1).arg(m_qimages.size()));
}
