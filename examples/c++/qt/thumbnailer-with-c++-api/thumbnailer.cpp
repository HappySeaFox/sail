/*  This file is part of SAIL (https://github.com/HappySeaFox/sail)

    Copyright (c) 2026 Dmitry Baryshev

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

#include <string>
#include <vector>

#include <QDir>
#include <QEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontMetrics>
#include <QIcon>
#include <QImage>
#include <QListView>
#include <QMessageBox>
#include <QPixmap>
#include <QStandardItemModel>
#include <QToolButton>

#include <sail-c++/sail-c++.h>

#include "thumbnail_loader.h"
#include "thumbnailer.h"
#include "ui_thumbnailer.h"

namespace
{

const int THUMBNAIL_SIZE = 96;

/* File names longer than that many lines are elided by the view. */
const int NAME_LINES = 2;

/* Roughly that many characters of a file name fit into a single line of a grid cell. */
const int NAME_CHARS = 18;

/* Breathing space around the thumbnail and the file name inside a grid cell. */
const int CELL_MARGIN = 6;

/* Item data: the absolute path of the entry and whether it's a folder. */
const int PATH_ROLE   = Qt::UserRole + 1;
const int FOLDER_ROLE = Qt::UserRole + 2;

} // namespace

Thumbnailer::Thumbnailer(QWidget* parent)
    : QWidget(parent)
    , m_ui(new Ui::Thumbnailer)
    , m_model(new QStandardItemModel(this))
    , m_loader(new ThumbnailLoader(QSize(THUMBNAIL_SIZE, THUMBNAIL_SIZE)))
{
    m_ui->setupUi(this);

    // Collect the file extensions SAIL has codecs for. Asking SAIL about every single file
    // in a folder would be slower and would report unsupported files as errors.
    //
    for (const sail::codec_info& codec_info : sail::codec_info::list())
    {
        for (const std::string& extension : codec_info.extensions())
        {
            m_extensions.insert(QString::fromStdString(extension));
        }
    }

    m_ui->listView->setModel(m_model);
    m_ui->listView->setViewMode(QListView::IconMode);
    m_ui->listView->setIconSize(QSize(THUMBNAIL_SIZE, THUMBNAIL_SIZE));
    m_ui->listView->setResizeMode(QListView::Adjust);
    m_ui->listView->setMovement(QListView::Static);
    m_ui->listView->setUniformItemSizes(true);
    m_ui->listView->setWordWrap(true);
    m_ui->listView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Decode thumbnails in a worker thread to keep the UI responsive on large folders.
    //
    m_loader->moveToThread(&m_thread);

    connect(&m_thread, &QThread::finished, m_loader, &QObject::deleteLater);
    connect(this, &Thumbnailer::thumbnailRequested, m_loader, &ThumbnailLoader::load);
    connect(m_loader, &ThumbnailLoader::loaded, this, &Thumbnailer::onThumbnailLoaded);

    m_thread.start();

    updateGridSize();

    connect(m_ui->pushOpenFolder, &QToolButton::clicked, this, &Thumbnailer::onOpenFolder);
    connect(m_ui->listView, &QListView::activated, this, &Thumbnailer::onActivated);

    openFolder(QDir::currentPath());
}

Thumbnailer::~Thumbnailer()
{
    m_thread.quit();
    m_thread.wait();
}

void Thumbnailer::changeEvent(QEvent* event)
{
    QWidget::changeEvent(event);

    // The grid cells depend on the font, recompute them when it changes.
    //
    if (event->type() == QEvent::FontChange || event->type() == QEvent::StyleChange)
    {
        updateGridSize();
    }
}

void Thumbnailer::onOpenFolder()
{
    const QString path = QFileDialog::getExistingDirectory(this, tr("Select a folder"), m_path);

    if (path.isEmpty())
    {
        return;
    }

    openFolder(path);
}

void Thumbnailer::onActivated(const QModelIndex& index)
{
    if (index.data(FOLDER_ROLE).toBool())
    {
        openFolder(index.data(PATH_ROLE).toString());
    }
}

void Thumbnailer::onThumbnailLoaded(int row, int generation, const QImage& thumbnail)
{
    // The thumbnails of the previous folder may still be on their way.
    //
    if (generation != m_generation || row >= m_model->rowCount())
    {
        return;
    }

    m_model->item(row)->setIcon(QIcon(QPixmap::fromImage(thumbnail)));
}

void Thumbnailer::updateGridSize()
{
    // A full size thumbnail occupies the whole icon area, so the grid cells must reserve
    // the room for the file name on top of it, both in height and in width. Ask the font
    // how much room that is: hard coded cell sizes cut the names off with large fonts.
    //
    const QFontMetrics metrics = m_ui->listView->fontMetrics();

    const int width  = qMax(THUMBNAIL_SIZE, NAME_CHARS * metrics.averageCharWidth()) + 2 * CELL_MARGIN;
    const int height = THUMBNAIL_SIZE + NAME_LINES * metrics.height() + 2 * CELL_MARGIN;

    m_ui->listView->setGridSize(QSize(width, height));
}

void Thumbnailer::openFolder(const QString& path)
{
    const QDir dir(path);

    if (!dir.exists())
    {
        QMessageBox::critical(this, tr("Error"), tr("'%1' doesn't exist.").arg(QDir::toNativeSeparators(path)));
        return;
    }

    m_path = dir.absolutePath();

    // Tell the loader to skip the thumbnails scheduled for the previous folder.
    //
    m_generation++;
    m_loader->setGeneration(m_generation);

    m_model->clear();

    // The parent folder always goes first.
    //
    QDir parentDir(m_path);
    const bool hasParent = parentDir.cdUp();

    QStandardItem* parentItem = new QStandardItem(m_iconProvider.icon(QFileIconProvider::Folder), QStringLiteral(".."));
    parentItem->setData(hasParent ? parentDir.absolutePath() : m_path, PATH_ROLE);
    parentItem->setData(true, FOLDER_ROLE);
    m_model->appendRow(parentItem);

    const QFileInfoList entries =
        dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot, QDir::DirsFirst | QDir::Name);

    int images = 0;

    for (const QFileInfo& entry : entries)
    {
        QStandardItem* item = new QStandardItem(m_iconProvider.icon(entry), entry.fileName());
        item->setData(entry.absoluteFilePath(), PATH_ROLE);
        item->setData(entry.isDir(), FOLDER_ROLE);
        item->setToolTip(QDir::toNativeSeparators(entry.absoluteFilePath()));
        m_model->appendRow(item);

        if (entry.isDir())
        {
            continue;
        }

        // Files SAIL has no codec for keep their system icons.
        //
        if (!m_extensions.contains(entry.suffix().toLower()))
        {
            continue;
        }

        images++;

        // The decoded thumbnail arrives in onThumbnailLoaded().
        //
        emit thumbnailRequested(entry.absoluteFilePath(), m_model->rowCount() - 1, m_generation);
    }

    m_ui->labelStatus->setText(
        tr("%1  [%2 entries, %3 images]").arg(QDir::toNativeSeparators(m_path)).arg(entries.size()).arg(images));
}
