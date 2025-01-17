#include <QApplication>
#include <QDebug>
#include <QTextStream>
#include <QFile>
#include <QFileDialog>
#include <iostream>

#include <gametemplate.h>
#include <list.h>
#include <search.h>

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

    bool exit = false;
    while (exit == false) {
        standardOut << "\nEnter your command (Type \"help\" for options):\n";
        standardOut.flush();
        QString command = "";

        std::string inputRaw;
        std::getline(std::cin, inputRaw);
        QString input = QString::fromStdString(inputRaw);

        if (input.toLower() == "search") {
            standardOut << "\nSearch: Please enter an command for search. (Enter \"search help\" for command choices. Or enter \"exit\" to enter a new command.)\n";
            standardOut.flush();

            std::string searchRawInput;
            std::getline(std::cin, searchRawInput);
            QString searchArgument = QString::fromStdString(searchRawInput);

            if (searchArgument.toLower() == "search help") {
                standardOut << "\nSearch command:\n\tAvailable options:\n"
                            << "\t\ttemplatesWithKeyword: List any game template that have names containing the user entered keyword.\n"
                            << "\t\ttemplatesOfSubtypeWithKeyword: List any game template of a user entered subtype that contain a user entered keyword.\n";
            }

            if (searchArgument.toLower() == "templateswithkeyword") {
                standardOut << "\nPlease enter the text you wish to search template names for (or \"exit\" to back out.):\n";
                standardOut.flush();
                std::string keywordRaw;
                std::getline(std::cin, keywordRaw);
                QString keywordEntered = QString::fromStdString(keywordRaw);

                Search::searchTemplatesForKeyword(templates, keywordEntered, &standardOut);
            }

            if (searchArgument.toLower() == "templatesofsubtypewithkeyword") {
                standardOut << "\nPlease enter the name of the subtype you wish to list templates for (or \"exit\" to back out.):\n";
                standardOut.flush();
                std::string subtypeRawInput;
                std::getline(std::cin, subtypeRawInput);
                QString subtypeEntered = QString::fromStdString(subtypeRawInput);

                standardOut << "\nPlease enter the text you wish to search template names for:\n";
                standardOut.flush();
                std::string keywordRaw;
                std::getline(std::cin, keywordRaw);
                QString keywordEntered = QString::fromStdString(keywordRaw);

                if (subtypeEntered != "exit") {
                    Search::searchTemplatesofSubtypeForKeyword(templates, subtypeEntered, keywordEntered, &standardOut);
                }
            }
        }
        }

        if (input.toLower() == "list") {
            standardOut << "\nList: Please enter an argument to list. (Enter \"list help\" for command choices. Or enter \"exit\" to enter a new command.)\n";
            standardOut.flush();

            std::string listRawInput;
            std::getline(std::cin, listRawInput);
            QString listArgument = QString::fromStdString(listRawInput);

            if (listArgument.toLower() == "list help") {
                standardOut << "\nList command:\n\tAvailable options:\n"
                            << "\t\ttemplates: List the name of every game template\n"
                            << "\t\ttemplatesAndSubtypes: List the name and subtype of each game template.\n"
                            << "\t\ttemplatesOfSubtype: List all of the templates with a given user entered subtype.\n"
                            << "\t\ttemplatesWithHash: List all templates that have a given user entered hexadecimal 4 byte hash.\n"
                            << "\t\ttemplatesWithHashAndDataPair: List all templates that have a given user hexadecimal hash that is paired with a user entered set of byte data.\n"
                            << "\t\ttemplatesByModel: List templates associated with a model hash value organized by each model hash.\n"
                            << "\t\tsubtypes: List all of the unique subtypes found in alphabetical order.\n"
                            << "\t\thashesOfTemplate: List all of the hash identifiers of a user entered template.\n"
                            << "\t\thashesOfTemplateWithDataSize: List all hashes of a specific template that has a data size matching a user entered number.\n"
                            << "\t\thashesAndValuesOfTemplate: List all of the hash identifiers and associated values of a user entered template.\n"
                            << "\t\tinstancesOfHashInTemplate: List all instances of a user entered hash in a user entered template.\n";
            }

            if (listArgument.toLower() == "templates") {
                List::listAllTemplates(templates, false, &standardOut);
            }

            if (listArgument.toLower() == "templatesandsubtypes") {
                List::listAllTemplates(templates, true, &standardOut);
            }

            // List all existing subtypes (no duplicates, sort alphabetically)
            if (listArgument.toLower() == "subtypes") {
                List::listAllSubTypes(templates, &standardOut);
            }

            if (listArgument.toLower() == "hashesoftemplate" || listArgument.toLower() == "hashesandvaluesoftemplate") {
                bool valuesToo = listArgument.toLower() == "hashesandvaluesoftemplate";

                // Take input for what template to find
                // or allow to back out.
                bool exitList = false;
                while (exitList == false) {
                    standardOut << "\nPlease enter the name of the subtype you wish to list templates for (or \"exit\" to back out.):\n";
                    standardOut.flush();
                    std::string listRawInput;
                    std::getline(std::cin, listRawInput);
                    QString templateEntered = QString::fromStdString(listRawInput);

                    if (templateEntered.toLower() == "exit") {
                        exitList = true;
                    } else {
                        bool resultsFound = List::listHashesOfTemplate(templates, templateEntered, valuesToo, &standardOut);
                        if (resultsFound == true) {
                            exitList = true;
                        }
                        standardOut << "Please try again by entering another subtype or \"exit\" to back out.";
                    }
                }
            }

            // List all hashes of a specific user entered template that has paired data that matches a numerical size entered by the user.
            if (listArgument.toLower() == "hashesoftemplatewithdatasize") {
                // Take input for what template to find
                // or allow to back out.
                bool exitList = false;
                while (exitList == false) {
                    standardOut << "\nPlease enter the name of the template you wish to list hashes for (or \"exit\" to back out.):\n";
                    standardOut.flush();
                    std::string listRawInput;
                    std::getline(std::cin, listRawInput);
                    QString templateEntered = QString::fromStdString(listRawInput);

                    standardOut << "\nPlease enter the size of the data you wish to find hashes associated with.:\n";
                    standardOut.flush();
                    std::string dataSizeRawInput;
                    std::getline(std::cin, dataSizeRawInput);
                    uint sizeEntered = QString::fromStdString(dataSizeRawInput).toUInt(nullptr);

                    if (templateEntered.toLower() == "exit") {
                        exitList = true;
                    } else {
                        List::listHashesOfTemplatesWithDataOfSize(templates, templateEntered, sizeEntered, &standardOut);
                        exitList = true;
                    }
                }
            }

            // List all templates that have a specific subtype (alphabetical order)
            if (listArgument.toLower() == "templatesofsubtype") {
                //Take input for what kind of subtype
                // or allow to back out.
                bool exitList = false;
                while (exitList == false) {
                    standardOut << "\nPlease enter the name of the subtype you wish to list templates for (or \"exit\" to back out.):\n";
                    standardOut.flush();
                    std::string listRawInput;
                    std::getline(std::cin, listRawInput);
                    QString subtypeEntered = QString::fromStdString(listRawInput);

                    if (subtypeEntered.toLower() == "exit") {
                        exitList = true;
                    } else {
                        bool resultsFound = List::listAllTemplatesofSubType(templates, subtypeEntered, &standardOut);
                        if (resultsFound == true) {
                            exitList = true;
                        }
                        standardOut << "Please try again by entering another subtype or \"exit\" to back out.";
                    }
                }
            }

            // List all game templates that have a particular user entered hash value (and the data associated with that hash).
            if (listArgument.toLower() == "templateswithhash") {
                bool exitList = false;
                while (exitList == false) {
                    standardOut << "\nPlease enter the hexadecimal of the hash you wish to search and list templates for (for example: 049A02F0) (or \"exit\" to back out.):\n";
                    standardOut.flush();
                    std::string hashInput;
                    std::getline(std::cin, hashInput);
                    QString hashEntered = QString::fromStdString(hashInput);

                    if (hashEntered.toLower() == "exit") {
                        exitList = true;
                    } else {
                        bool hashConvertedSuccess = false;
                        uint32_t hash = hashEntered.toUInt(&hashConvertedSuccess, 16);

                        if (hashConvertedSuccess) {
                            List::listAllTemplatesWithHash(templates, hash, &standardOut);
                            exitList = true;
                        } else {
                            standardOut << "\tListing failed: Hash could not be handled. Please enter a hexadecimal hash such as 049A02F0.\n\n";
                        }
                    }
                }
            }

            // List all game templates that have a particular user entered hash value and also have an associated user entered data value.
            if (listArgument.toLower() == "templateswithhashanddatapair") {
                bool exitList = false;
                while (exitList == false) {
                    standardOut << "\nPlease enter the hexadecimal of the hash you wish to search and list templates for (for example: 049A02F0) (or \"exit\" to back out.):\n";
                    standardOut.flush();
                    std::string hashInput;
                    std::getline(std::cin, hashInput);
                    QString hashEntered = QString::fromStdString(hashInput);

                    standardOut << "\nPlease enter the hexadecimal of the data you wish to find being paired with the previous entered hash (for example: 049A02F0) (or \"exit\" to back out.):\n";
                    standardOut.flush();
                    std::string dataInput;
                    std::getline(std::cin, dataInput);
                    QString dataEntered = QString::fromStdString(dataInput);
                    QByteArray dataArray = QByteArray();//::fromStdString(dataInput);

                    while (dataEntered.length() >= 2) {
                        QString byteString = dataEntered.left(2);
                        dataEntered = dataEntered.right(dataEntered.length() - 2);
                        dataArray.push_back(byteString.toUInt(nullptr, 16) & 0xFF);
                    }

                    if (hashEntered.toLower() == "exit") {
                        exitList = true;
                    } else {
                        bool hashConvertedSuccess = false;
                        uint32_t hash = hashEntered.toUInt(&hashConvertedSuccess, 16);

                        if (hashConvertedSuccess) {
                            List::listAllTemplatesWithHashAndValuePair(templates, hash, dataArray,  &standardOut);
                            exitList = true;
                        } else {
                            standardOut << "\tListing failed: Hash could not be handled. Please enter a hexadecimal hash such as 049A02F0.\n\n";
                        }
                    }
                }
            }

            // List all templates associated with a model hash organized by each model hash.
            if (listArgument.toLower() == "templatesbymodel") {
                List::listTemplatesByModel(templates, &standardOut);
            }

            // List all instances of a specifc user entered hash in a user entered template (if any).
            if (listArgument.toLower() == "instancesofhashintemplate") {
                standardOut << "\nPlease enter the name of the template you wish to list hashes for (or \"exit\" to back out.):\n";
                standardOut.flush();
                std::string listRawInput;
                std::getline(std::cin, listRawInput);
                QString templateEntered = QString::fromStdString(listRawInput);


                standardOut << "\nPlease enter the hexadecimal of the hash you wish to search and list templates for (for example: 049A02F0):\n";
                standardOut.flush();
                std::string hashInput;
                std::getline(std::cin, hashInput);
                QString hashEntered = QString::fromStdString(hashInput);

                if (templateEntered != "exit") {
                    bool hashConvertedSuccess = false;
                    uint32_t hash = hashEntered.toUInt(&hashConvertedSuccess, 16);

                    if (hashConvertedSuccess) {
                        List::listInstancesOfHashInTemplate(templates, templateEntered, hash, &standardOut);
                    } else {
                        standardOut << "\tListing failed: Hash could not be handled. Please enter a hexadecimal hash such as 049A02F0.\n\n";
                    }
                }
            }
        }
    }

    return a.exec();
}
