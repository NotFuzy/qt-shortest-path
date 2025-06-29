#include <QApplication>
#include <QFile>
#include <QMessageBox>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Загрузка стиля из ресурсов
    QFile styleFile(":/light.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = QLatin1String(styleFile.readAll());
        a.setStyleSheet(style);
    } else {
        QMessageBox::warning(nullptr, QObject::tr("Ошибка"), QObject::tr("Не удалось загрузить файл стилей!"));
    }

    MainWindow w;
    w.show();
    return a.exec();
}
