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

    int currentTemplate = 0;
    while (dataStream.device()->pos() < bytes.length()) {
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
        QList<QPair<uint32_t, QByteArray>> dataRows = QList<QPair<uint32_t, QByteArray>>();
        while (currentByte < remainingData) {
            int32_t hash = 0;
            int32_t dataSize = 0;

            dataStream.setByteOrder(QDataStream::BigEndian); // Preserve original ordering for hash. (We do not need to use the number for any calculations, so keep it as it would appear in data)
            dataStream >> hash;
            dataStream.setByteOrder(QDataStream::LittleEndian);
            dataStream >> dataSize;
            currentByte += 8;

            char rawBytes[dataSize];
            dataStream.readRawData(rawBytes, dataSize);
            QByteArray bytes = QByteArray();
            for (int byteNum = 0; byteNum < dataSize; byteNum++) {
                bytes.push_back(rawBytes[byteNum]);
            }

            dataRows.push_back(QPair<uint32_t, QByteArray>(hash, bytes));
            currentByte += dataSize;

        }

        bool checkForTrailingDataFinished = false;
        while (checkForTrailingDataFinished == false) {
            QByteArray peekBytes = dataStream.device()->peek(12);
            char matchBytes[12] = { 8, 0, 0, 0,
                                    0, 0, 0, 0,
                                    0, 0, 0, 0 };
            QByteArray matchCase = QByteArray(matchBytes, 12);

            if (peekBytes == matchCase) {
                dataStream.skipRawData(12);
            } else {
                checkForTrailingDataFinished = true;
            }
        }

        // If there's not enough data for a template or the 8 - 0 - 0 data structure,
        // simply skip to the end of the file to terminate looping.
        int bytesLeft = bytes.length() - dataStream.device()->pos();
        if (bytesLeft < 12) {
            dataStream.skipRawData(bytesLeft);
        }

        templates->push_back(GameTemplate(name, templateType, dataRows));
        currentTemplate++;
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

void listAllTemplates(QList<GameTemplate>* templates, bool includeSubTypes, QTextStream* standardOut) {
    *standardOut << "Listing all Game Templates:\n";

    for (int i = 0; i < templates->count(); i++) {
        *standardOut << "\t" << templates->at(i).name << "\n";
        if (includeSubTypes) {
            *standardOut << "\t\t" << templates->at(i).templateType << "\n";
        }
        standardOut->flush();
    }
}

void listAllSubTypes(QList<GameTemplate>* templates, QTextStream* standardOut) {
    QList<QString> subtypes = QList<QString>();

    for (int i = 0; i < templates->length(); i++) {
        QString templateType = templates->at(i).templateType;
        if (subtypes.contains(templateType) == false) {
            subtypes.push_back(templateType);
        }
    }

    subtypes.sort(Qt::CaseSensitivity::CaseInsensitive);
    *standardOut << "\nListing all loaded template sub-types:\n";
    for (int i = 0; i < subtypes.length(); i++) {
        *standardOut << "\t" << subtypes.at(i) << "\n";
    }
}


bool templateNameExists(QList<GameTemplate>* templates, QString templateName) {
    bool templateFound = false;
    int i = 0;
    while (templateFound == false && i < templates->length()) {
        templateFound = templates->at(i).name.toLower() == templateName.toLower();
        i++;
    }

    return templateFound;
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
    QTextStream standardOut(stdout);

    // Prompt the user to select a valid templates file.
    QList<GameTemplate>* templates = nullptr;

    while (templates == nullptr) {
        templates = promptForTemplatesFile(&standardOut);
    }

    standardOut << "\nLoading the game templates has completed.\n";


    return a.exec();
}
