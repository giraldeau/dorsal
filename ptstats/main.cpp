#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QStringList>
#include <QString>
#include <QFile>
#include <QDataStream>
#include <QDebug>

#include "intel-pt.h"


int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("ptstats");

    QCommandLineParser parser;
    parser.process(app);

    QStringList dataPaths = parser.positionalArguments();

    qDebug() << dataPaths;

    for (const QString path: dataPaths) {
        QFile f(path);
        f.open(QFile::ReadOnly);
        QDataStream data(&f);

        qint32 header;
        data >> header;
        qDebug() << header;

        f.close();
    }

    return 0;
}

