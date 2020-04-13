#include <QDateTime>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QImage>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QShortcut>

#include <sail/sail.h>

//#include <sail/layouts/v2.h>
//#include <sail/layouts/v1.h>

#include "qtsail.h"
#include "ui_qtsail.h"

// PIMPL
//
class Q_DECL_HIDDEN QtSail::Private
{
public:
    QScopedPointer<Ui::QtSail> ui;

    QImage qimage;

    sail_context *context = nullptr;

    QFileInfoList files;
    int currentFile;
};

// Auto cleanup at scope exit
//
template<typename T>
class CleanUp
{
public:
    CleanUp(T func)
        : m_func(func)
    {}

    ~CleanUp()
    {
        m_func();
    }

private:
    T m_func;
};

////////////////////////////////////

QtSail::QtSail(QWidget *parent)
    : QWidget(parent)
    , d(new Private)
{
    d->ui.reset(new Ui::QtSail);
    d->ui->setupUi(this);
    QLabel *l = new QLabel;
    l->setAlignment(Qt::AlignCenter);
    d->ui->scrollArea->setWidget(l);

    connect(d->ui->pushOpen,     &QPushButton::clicked, this, &QtSail::onOpenFile);
    connect(d->ui->pushOpenDir,  &QPushButton::clicked, this, &QtSail::onOpenDir);
    connect(d->ui->pushProbe,    &QPushButton::clicked, this, &QtSail::onProbe);
    connect(d->ui->pushSave   ,  &QPushButton::clicked, this, &QtSail::onSave);
    connect(d->ui->pushPrevious, &QPushButton::clicked, this, &QtSail::onPrevious);
    connect(d->ui->pushNext,     &QPushButton::clicked, this, &QtSail::onNext);
    connect(d->ui->pushFirst,    &QPushButton::clicked, this, &QtSail::onFirst);
    connect(d->ui->pushLast,     &QPushButton::clicked, this, &QtSail::onLast);
    connect(d->ui->checkFit,     &QCheckBox::toggled,   this, &QtSail::onFit);

    d->ui->pushOpen->setShortcut(QKeySequence::Open);
    d->ui->pushOpen->setToolTip(d->ui->pushOpen->shortcut().toString());
    d->ui->pushSave->setShortcut(QKeySequence::Save);
    d->ui->pushSave->setToolTip(d->ui->pushSave->shortcut().toString());
    d->ui->pushPrevious->setShortcut(Qt::Key_Backspace);
    d->ui->pushPrevious->setToolTip(d->ui->pushPrevious->shortcut().toString());
    d->ui->pushNext->setShortcut(Qt::Key_Space);
    d->ui->pushNext->setToolTip(d->ui->pushNext->shortcut().toString());
    d->ui->pushFirst->setShortcut(Qt::Key_Home);
    d->ui->pushFirst->setToolTip(d->ui->pushFirst->shortcut().toString());
    d->ui->pushLast->setShortcut(Qt::Key_End);
    d->ui->pushLast->setToolTip(d->ui->pushLast->shortcut().toString());

    connect(new QShortcut(Qt::Key_F, this), &QShortcut::activated, this, [&]{
        if (isFullScreen()) {
            showNormal();
        } else {
            showFullScreen();
        }
    });

    SAIL_LOG_INFO("Init");
    sail_init(&d->context);

    if (d->context == nullptr || d->context->plugin_info_node == nullptr) {
        qCritical("Failed to load plugin info");
        return;
    }
}

QtSail::~QtSail()
{
    SAIL_LOG_INFO("Finish");
    sail_finish(d->context);
    d->context = nullptr;
}

int QtSail::pluginInfo(struct sail_plugin_info *plugin_info)
{
    SAIL_LOG_DEBUG("SAIL plugin layout version: %d", plugin_info->layout);
    SAIL_LOG_DEBUG("SAIL plugin version: %s", plugin_info->version);
    SAIL_LOG_DEBUG("SAIL plugin description: %s", plugin_info->description);
    SAIL_LOG_DEBUG("SAIL plugin path: %s", plugin_info->path);

    struct sail_string_node *node = plugin_info->extension_node;

    while (node != nullptr) {
        SAIL_LOG_DEBUG("SAIL extension '%s'", node->value);
        node = node->next;
    }

    node = plugin_info->mime_type_node;

    while (node != nullptr) {
        SAIL_LOG_DEBUG("SAIL mime type '%s'", node->value);
        node = node->next;
    }

    return 0;
}

void QtSail::loadFileFromDir()
{
    if (d->currentFile < 0 || d->currentFile >= d->files.size()) {
        return;
    }

    d->ui->labelCounter->setText(tr("%1/%2").arg(d->currentFile+1).arg(d->files.size()));

    if (loadImage(d->files[d->currentFile].absoluteFilePath(), &d->qimage) == 0) {
        onFit(d->ui->checkFit->isChecked());
    }
}

int QtSail::loadImage(const QString &path, QImage *qimage)
{
    const struct sail_plugin_info *plugin_info;

    if (sail_plugin_info_by_extension(d->context, QFileInfo(path).suffix().toLocal8Bit(), &plugin_info) != 0) {
        QMessageBox::critical(this, tr("Error"), tr("Unsupported file extension: %1").arg(QFileInfo(path).suffix()));
        return 1;
    }

    // Load the specified codec
    //
    qint64 v = QDateTime::currentMSecsSinceEpoch();

    const struct sail_plugin *plugin;

    if (sail_load_plugin(d->context, plugin_info, &plugin) != 0) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to load plugin"));
        return 1;
    }

    sail_file *file = nullptr;
    sail_image *image = nullptr;
    sail_read_features *read_features = nullptr;
    sail_read_options *read_options = nullptr;
    uchar *image_bits = nullptr;

    auto cleanup_func = [&] {
        SAIL_LOG_DEBUG("Read clean up");

        delete [] image_bits;

        plugin->iface.v2->read_finish_v1(file);

        sail_destroy_read_features(read_features);
        read_features = nullptr;

        sail_destroy_read_options(read_options);
        read_options = nullptr;

        sail_destroy_image(image);
        image = nullptr;

        sail_destroy_file(file);
        file = nullptr;
    };

    CleanUp<decltype(cleanup_func)> cleanUp(cleanup_func);

    // Read the image file
    //

    // Determine the read features of the plugin: what the plugin can actually read?
    //
    SAIL_TRY_OR_CLEANUP(plugin->iface.v2->read_features_v1(&read_features),
                QMessageBox::critical(this, tr("Error"), tr("Failed to obtain plugin read features. Error: %1").arg(res)));

    SAIL_TRY_OR_CLEANUP(sail_alloc_file_for_reading(path.toLocal8Bit(), &file),
        QMessageBox::critical(this, tr("Error"), tr("Failed to open the specified file. Error: %1").arg(res)));

    // Allocate new read options and copy defaults from the read features
    // (preferred output pixel format etc.)
    //
    SAIL_TRY_OR_CLEANUP(sail_alloc_read_options_from_features(read_features, &read_options),
        QMessageBox::critical(this, tr("Error"), tr("Failed to allocate read options. Error: %1").arg(res)));

    // Start reading
    //
    SAIL_TRY_OR_CLEANUP(plugin->iface.v2->read_init_v1(file, read_options),
        QMessageBox::critical(this, tr("Error"), tr("Failed to start reading the specified file. Error: %1").arg(res)));

    // Seek to the next frame if any
    //
    SAIL_TRY_OR_CLEANUP(plugin->iface.v2->read_seek_next_frame_v1(file, &image),
        QMessageBox::critical(this, tr("Error"), tr("Failed to seek to the next frame. Error: %1").arg(res)));

    // Allocate image bits. Assume full-color images so divide by 8.
    //
    const int bytes_per_pixel = sail_bits_per_pixel(image->pixel_format) / 8;
    const QImage::Format qimage_format = QImage::Format_RGB888;
    const int scan_length = image->width * bytes_per_pixel;

    image_bits = new uchar[scan_length * image->height];

    if (image_bits == nullptr) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to allocate memory."));
        return 1;
    }

    // Actual read. Pass by pass, line by line.
    //
    for (int pass = 0; pass < image->passes; pass++) {
        SAIL_TRY_OR_CLEANUP(plugin->iface.v2->read_seek_next_pass_v1(file, image),
                    QMessageBox::critical(this, tr("Error"), tr("Failed to seek to the next pass. Error: %1").arg(res)));

        for (int j = 0; j < image->height; j++) {
            SAIL_TRY_OR_CLEANUP(plugin->iface.v2->read_scan_line_v1(file, image, image_bits + j * image->width * bytes_per_pixel),
                        QMessageBox::critical(this, tr("Error"), tr("Failed to read a scan line. Error: %1").arg(res)));
        }
    }

    SAIL_LOG_INFO("Loaded in %lld ms.", QDateTime::currentMSecsSinceEpoch() - v);

    *qimage = QImage(image_bits, image->width, image->height, scan_length, qimage_format).copy();

    QString meta;
    struct sail_meta_entry_node *node = image->meta_entry_node;

    if (node != nullptr) {
        meta = tr("%1: %2").arg(node->key).arg(node->value);
    }

    d->ui->labelStatus->setText(tr("%1  [%2x%3]  [%4 -> %5]  %6")
                                .arg(QFileInfo(path).fileName())
                                .arg(image->width)
                                .arg(image->height)
                                .arg(sail_pixel_format_to_string(image->source_pixel_format))
                                .arg(sail_pixel_format_to_string(image->pixel_format))
                                .arg(meta)
                                );

    return 0;
}

int QtSail::saveImage(const QString &path)
{
    const struct sail_plugin_info *plugin_info;

    if (sail_plugin_info_by_extension(d->context, QFileInfo(path).suffix().toLocal8Bit(), &plugin_info) != 0) {
        QMessageBox::critical(this, tr("Error"), tr("Unsupported file extension: %1").arg(QFileInfo(path).suffix()));
        return 1;
    }

    // Load the specified codec
    //
    qint64 v = QDateTime::currentMSecsSinceEpoch();

    const struct sail_plugin *plugin;

    if (sail_load_plugin(d->context, plugin_info, &plugin) != 0) {
        SAIL_LOG_ERROR("Failed to alloc plugin");
        return 1;
    }

    sail_file *file = nullptr;
    sail_image *image = nullptr;
    sail_write_features *write_features = nullptr;
    sail_write_options *write_options = nullptr;

    auto cleanup_func = [&] {
        SAIL_LOG_DEBUG("Write clean up");

        plugin->iface.v2->write_finish_v1(file);

        sail_destroy_write_features(write_features);
        write_features = nullptr;

        sail_destroy_write_options(write_options);
        write_options = nullptr;

        sail_destroy_image(image);
        image = NULL;

        sail_destroy_file(file);
        file = NULL;
    };

    CleanUp<decltype(cleanup_func)> cleanUp(cleanup_func);

    // write the image file
    //
    SAIL_TRY_OR_CLEANUP(plugin->iface.v2->write_features_v1(&write_features),
                QMessageBox::critical(this, tr("Error"), tr("Failed to obtain plugin write features. Error: %1").arg(res)));

    SAIL_TRY_OR_CLEANUP(sail_alloc_file_for_writing(path.toLocal8Bit(), &file),
        QMessageBox::critical(this, tr("Error"), tr("Failed to open the specified file. Error: %1").arg(res)));

    SAIL_TRY_OR_CLEANUP(sail_alloc_write_options(&write_options),
        QMessageBox::critical(this, tr("Error"), tr("Failed to allocate write options. Error: %1").arg(res)));

    write_options->pixel_format = write_features->preferred_output_pixel_format;
    write_options->io_options = SAIL_IO_OPTION_META_INFO;
    write_options->compression = -10;

    SAIL_TRY_OR_CLEANUP(plugin->iface.v2->write_init_v1(file, write_options),
        QMessageBox::critical(this, tr("Error"), tr("Failed to start writeing the specified file. Error: %1").arg(res)));

    SAIL_TRY(sail_alloc_image(&image));

    image->width = d->qimage.width();
    image->height = d->qimage.height();
    image->pixel_format = SAIL_PIXEL_FORMAT_RGB;
    image->passes = 1;

    // Assume pixel formats aligned to 8 bits.
    const int bytes_per_line = image->width * (sail_bits_per_pixel(image->pixel_format) / 8);

    struct sail_meta_entry_node *meta_entry_node;

    SAIL_TRY(sail_alloc_meta_entry_node(&meta_entry_node));
    SAIL_TRY_OR_CLEANUP(sail_strdup("Comment", &meta_entry_node->key),
                        /* cleanup */ sail_destroy_meta_entry_node(meta_entry_node));
    SAIL_TRY_OR_CLEANUP(sail_strdup("JPEG KOOL COMMENT", &meta_entry_node->value),
                        /* cleanup */ sail_destroy_meta_entry_node(meta_entry_node));
    image->meta_entry_node = meta_entry_node;

    SAIL_LOG_DEBUG("Image size: %dx%d", image->width, image->height);
    SAIL_LOG_DEBUG("Output pixel format: %s", sail_pixel_format_to_string(write_options->pixel_format));

    SAIL_TRY_OR_CLEANUP(plugin->iface.v2->write_seek_next_frame_v1(file, image),
        QMessageBox::critical(this, tr("Error"), tr("Failed to seek to the next frame. Error: %1").arg(res)));

    // Actual write
    //
    for (int pass = 0; pass < image->passes; pass++) {
        SAIL_TRY_OR_CLEANUP(plugin->iface.v2->write_seek_next_pass_v1(file, image),
                    QMessageBox::critical(this, tr("Error"), tr("Failed to seek to the next pass. Error: %1").arg(res)));

        for (int j = 0; j < image->height; j++) {
            SAIL_TRY_OR_CLEANUP(plugin->iface.v2->write_scan_line_v1(file, image, d->qimage.bits() + j * bytes_per_line),
                        QMessageBox::critical(this, tr("Error"), tr("Failed to write a scan line. Error: %1").arg(res)));
        }
    }

    SAIL_LOG_INFO("Saved in %lld ms.", QDateTime::currentMSecsSinceEpoch() - v);

    return 0;
}

void QtSail::onOpenFile()
{
    static QString lastDir;

    const QString path = QFileDialog::getOpenFileName(this, tr("Select a file"), lastDir);

    if (path.isEmpty()) {
        return;
    }

    lastDir = QFileInfo(path).absolutePath();

    d->files.clear();

    if (loadImage(path, &d->qimage) == 0) {
        onFit(d->ui->checkFit->isChecked());
        d->ui->labelCounter->setText(QStringLiteral("1/1"));
    }
}

void QtSail::onOpenDir()
{
    static QString lastDir;

    const QString path = QFileDialog::getExistingDirectory(this, tr("Select a file"), lastDir);

    if (path.isEmpty()) {
        return;
    }

    lastDir = QFileInfo(path).absolutePath();

    d->files = QDir(path).entryInfoList(QStringList() << QStringLiteral("*.*"),
                                        QDir::Files,
                                        QDir::Name);
    d->currentFile = 0;
    loadFileFromDir();
}

void QtSail::onProbe()
{
    static QString lastDir;

    const QString path = QFileDialog::getOpenFileName(this, tr("Select a file"), lastDir);

    if (path.isEmpty()) {
        return;
    }

    lastDir = QFileInfo(path).absolutePath();

    const qint64 v = QDateTime::currentMSecsSinceEpoch();

    // Probe
    sail_image *image;
    const struct sail_plugin_info *plugin_info;
    int res;

    if ((res = sail_probe_image(path.toLocal8Bit(), d->context, &plugin_info, &image)) != 0) {
        QMessageBox::critical(this, tr("Error"), tr("Failed to probe the image. Error: %1").arg(res));
        return;
    }

    QMessageBox::information(this,
                             tr("File info"),
                             tr("Probed in: %1 ms.\nCodec: %2\nSize: %3x%4\nSource pixel format: %5\nOutput pixel format: %6")
                                .arg(QDateTime::currentMSecsSinceEpoch() - v)
                                .arg(plugin_info->description)
                                .arg(image->width)
                                .arg(image->height)
                                .arg(sail_pixel_format_to_string(image->source_pixel_format))
                                .arg(sail_pixel_format_to_string(image->pixel_format))
                             );

    sail_destroy_image(image);
}

void QtSail::onSave()
{
    static QString lastDir;

    const QString path = QFileDialog::getSaveFileName(this, tr("Select a file"), lastDir);

    if (path.isEmpty()) {
        return;
    }

    lastDir = QFileInfo(path).absolutePath();

    if (saveImage(path) != 0) {
        return;
    }

    if (QMessageBox::question(this, tr("Open file"), tr("%1 has been saved succesfully. Open the saved file?")
                              .arg(QDir::toNativeSeparators(path))) == QMessageBox::Yes) {
        if (!loadImage(path, &d->qimage)) {
            return;
        }
    }
}

void QtSail::onPrevious()
{
    d->currentFile--;
    loadFileFromDir();
}

void QtSail::onNext()
{
    d->currentFile++;
    loadFileFromDir();
}

void QtSail::onFirst()
{
    d->currentFile = 0;
    loadFileFromDir();
}

void QtSail::onLast()
{
    d->currentFile = d->files.size()-1;
    loadFileFromDir();
}

void QtSail::onFit(bool fit)
{
    QPixmap pixmap;

    if (fit) {
        if (d->qimage.width() > d->ui->scrollArea->viewport()->width() ||
                d->qimage.height() > d->ui->scrollArea->viewport()->height()) {
            pixmap = QPixmap::fromImage(d->qimage.scaled(d->ui->scrollArea->viewport()->size(),
                                                         Qt::KeepAspectRatio,
                                                         Qt::SmoothTransformation));
        } else {
            pixmap =  QPixmap::fromImage(d->qimage);
        }
    } else {
        pixmap =  QPixmap::fromImage(d->qimage);
    }

    qobject_cast<QLabel *>(d->ui->scrollArea->widget())->setPixmap(pixmap);
}
