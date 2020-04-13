#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QWidget>
#include <QScopedPointer>

class QImage;

struct sail_plugin_info;

class QtSail : public QWidget
{
    Q_OBJECT

public:
    QtSail(QWidget *parent = nullptr);
    ~QtSail();

private:
    int loadImage(const QString &path, QImage *qimage);
    int saveImage(const QString &path);
    int pluginInfo(struct sail_plugin_info *plugin_info);
    void loadFileFromDir();

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
