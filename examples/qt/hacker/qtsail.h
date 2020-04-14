#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QWidget>
#include <QScopedPointer>

#include <sail/error.h>

class QImage;

struct sail_plugin_info;

class QtSail : public QWidget
{
    Q_OBJECT

public:
    QtSail(QWidget *parent = nullptr);
    ~QtSail();

private:
    sail_error_t loadImage(const QString &path, QImage *qimage);
    sail_error_t saveImage(const QString &path, QImage *qimage);
    sail_error_t pluginInfo(const sail_plugin_info *plugin_info) const;
    void loadFileFromDir();
    QStringList filters() const;

private: // slots
    void onOpenFile();
    void onOpenDir();
    void onProbe();
    void onSave();
    void onPrevious();
    void onNext();
    void onFirst();
    void onLast();
    void onFit(bool fit);

private:
    class Private;
    const QScopedPointer<Private> d;
};

#endif
