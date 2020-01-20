#include <QApplication>
#include <QDebug>
#include <QTextStream>
#include <QFile>
#include <QFileDialog>
#include <iostream>

#include <gametemplate.h>

using namespace std;

QList<GameTemplate>* parseGameTemplatesFile(QByteArray bytes) {
    QDataStream dataStream(bytes);
    dataStream.setByteOrder(QDataStream::LittleEndian);

    dataStream.skipRawData(4); // Skip file identifier.
    int32_t totalTemplates = 0;
    dataStream >> totalTemplates;

    QList<GameTemplate>* templates = new QList<GameTemplate>();

    for (int i = 0; i < totalTemplates; i++) {
        int32_t templateSize = 0;
        dataStream >> templateSize;

        int32_t unknownValue1 = 0;
        int32_t unknownValue2 = 0;
        dataStream >> unknownValue1;
        dataStream >> unknownValue2;

        int32_t nameLength = 0;
        dataStream >> nameLength;

        QByteArray nameBuffer(nameLength - 1, Qt::Uninitialized);
        dataStream.readRawData(nameBuffer.data(), nameBuffer.length());
        QString name = QString(nameBuffer);
        dataStream.skipRawData(1); // Skip blank byte that terminates string.

        int32_t templateTypeLength = 0;
        dataStream >> templateTypeLength;

        QByteArray templateNameBuffer(templateTypeLength - 1, Qt::Uninitialized);
        dataStream.readRawData(templateNameBuffer.data(), templateNameBuffer.length());
        QString templateType = QString(templateNameBuffer);
        dataStream.skipRawData(1); // Skip blank byte that terminates string.

        // Check if this should be unknownValue3 or if this is accurate
        int32_t numberOfHashes = 0;
        dataStream >> numberOfHashes;

        // Template size is the total template byte count.
        // We do not subtract an immediate 4 as the size includes the int32 used to give the template size.
        // - 8 to account for the two unknown int32 values.
        // - 4 to account for the int32 that contains the name length, - name length to account for the string name
        // - 4 to account for the int32 that contains the template type name length, - templateTypeLength to account for the template name string.
        // - 4 to account for the int32 that contains the number of hashes/data pairs in the template.
        int32_t remainingData = templateSize - 8 - 4 - nameLength - 4 - templateTypeLength - 4;
        int currentByte = 0;
        QList<QList<uint8_t>> dataRows = QList<QList<uint8_t>>();
        while (currentByte < remainingData) {
            int32_t hash = 0;
            int32_t dataSize = 0;

            dataStream >> hash;
            dataStream >> dataSize;
            currentByte += 8;

            char rawBytes[dataSize];
            dataStream.readRawData(rawBytes, dataSize);
            QList<uint8_t> bytes = QList<uint8_t>();
            for (int byteNum = 0; byteNum < dataSize; byteNum++) {
                bytes.push_back(rawBytes[byteNum]);
            }

            dataRows.push_back(bytes);
            currentByte += dataSize;

        }
        int currentDSPosition = dataStream.device()->pos();
        templates->push_back(GameTemplate(name, templateType, dataRows));
    }
    return templates;
}

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


QList<GameTemplate>* promptForTemplatesFile(QTextStream* standardOut) {
    // Request the user to input a path to a file containing game templates.
    // This can be a .pack file that contains a GameTemplates.wsd file
    // or a direct GameTemplates.wsd file.
    // Note: If the file is inside a pack then it must contain the "GameTemplates.wsd"
    // string exactly. If this is not found, it will not be parsed.

    *standardOut << "Please select your loosefiles_BinPC.pack or GameTemplates.wsd file.";
    standardOut->flush();

    QString fileName = QFileDialog::getOpenFileName(nullptr, ("Open File"),
                                                      "/home",
                                                      ("Game Templates (*.wsd *.pack)"));

    return openGameTemplatesFile(fileName);
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    return a.exec();
}
