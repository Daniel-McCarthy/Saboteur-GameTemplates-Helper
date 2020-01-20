#include <QApplication>
#include <QDebug>
#include <QTextStream>
#include <QFile>

QList<GameTemplate>* openGameTemplatesFile(QString directory) {
    QFile templatesFile(directory);

    if (templatesFile.open(QIODevice::ReadOnly) == false) {
        qCritical("Output command failed. The supplied file path can not be opened.");
        return nullptr;
    }

    QByteArray templatesFileBytes = templatesFile.readAll();

    int offsetOfFileIdentifier = templatesFileBytes.indexOf("AULB");
    if (offsetOfFileIdentifier == -1) {
        qCritical("Output command failed. The supplied file does not contain the \"AULB\" file identifier.");
        return nullptr;
    }

    if (offsetOfFileIdentifier != 0) {
        // Remove any data prior to the file identifer.
        templatesFileBytes.remove(0, offsetOfFileIdentifier);
    }

    return parseGameTemplatesFile(templatesFileBytes);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    return a.exec();
}
