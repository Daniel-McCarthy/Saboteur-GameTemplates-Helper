#include "list.h"
#include "gametemplate.h"
#include "utilities.h"

List::List()
{

}

void List::listAllTemplates(QList<GameTemplate>* templates, bool includeSubTypes, QTextStream* standardOut) {
    *standardOut << "Listing all Game Templates:\n";

    for (int i = 0; i < templates->count(); i++) {
        *standardOut << "\t" << templates->at(i).name << "\n";
        if (includeSubTypes) {
            *standardOut << "\t\t" << templates->at(i).templateType << "\n";
        }
        standardOut->flush();
    }
}

void List::listAllSubTypes(QList<GameTemplate>* templates, QTextStream* standardOut) {
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

bool List::listAllTemplatesofSubType(QList<GameTemplate>* templates, QString subtype, QTextStream* standardOut) {
    *standardOut << "Listing all Game Templates of type \"" << subtype << "\":\n";

    if (templates->length() == 0) {
        *standardOut << "\tNo templates exist.\n";
        standardOut->flush();
        return false;
    }

    QList<QString> templatesFound = QList<QString>();

    for (int i = 0; i < templates->length(); i++) {
        if (templates->at(i).templateType.toLower() == subtype.toLower()) {
            templatesFound.push_back(templates->at(i).name);
        }
    }

    if (templatesFound.length() == 0) {
        *standardOut << "\tNo templates found with the entered subtype.\n";
        standardOut->flush();
        return false;
    }

    if (templatesFound.length() > 1) {
        templatesFound.sort(Qt::CaseSensitivity::CaseSensitive);
    }

    for (int i = 0; i < templatesFound.length(); i++) {
        *standardOut << "\t" << templatesFound.at(i) << "\n";
        standardOut->flush();
    }
    return true;
}

void List::listInstancesOfHashInTemplate(QList<GameTemplate>* templates, QString templateName, uint32_t hash, QTextStream* standardOut) {
    *standardOut << "Listing instances of the hash \"" << Utilities::uintToHex(hash).toUpper() << " of template \"" << templateName << "\":\n";
    bool hashFound = false;

    for (int i = 0; i < templates->length(); i++) {
        const GameTemplate* currentTemplate = &templates->at(i);
        if (currentTemplate->name == templateName) {

            for (int hashIndex = 0; hashIndex < currentTemplate->data.length(); hashIndex++) {
                uint32_t hashValue = currentTemplate->data.at(hashIndex).first;

                if (hashValue == hash) {
                    hashFound = true;
                    QString hashAsHex = Utilities::uintToHex(hashValue);
                    *standardOut << "\tHash: " << hashAsHex;
                    *standardOut << "\t" << Utilities::intToASCII(hashValue, true);

                    *standardOut << "\tData: " << currentTemplate->data.at(hashIndex).second.toHex().toUpper() << "\n";
                }
            }

            if (hashFound == false) {
                *standardOut << "\tNo instances of the hash were found.\n\n";
            }
            standardOut->flush();
            return;
        }
    }

    *standardOut << "\tNo such template name found.\n\n";
    standardOut->flush();
}

bool List::listHashesOfTemplate(QList<GameTemplate>* templates, QString templateName, bool printValuesToo, QTextStream* standardOut) {
    *standardOut << "Listing all Hashes of template \"" << templateName << "\":\n";

    for (int i = 0; i < templates->length(); i++) {
        const GameTemplate* currentTemplate = &templates->at(i);
        if (currentTemplate->name == templateName) {

            for (int hashIndex = 0; hashIndex < currentTemplate->data.length(); hashIndex++) {
                uint32_t hashValue = currentTemplate->data.at(hashIndex).first;
                QString hashAsHex = Utilities::uintToHex(hashValue);
                *standardOut << "\tHash: " << hashAsHex;
                *standardOut << "\t" << Utilities::intToASCII(hashValue, true);

                if (printValuesToo == true) {
                    *standardOut << "\tData: " << currentTemplate->data.at(hashIndex).second.toHex().toUpper() << "\n";
                } else {
                    *standardOut << "\n";
                }
            }
            return true;
        }
    }

    *standardOut << "\tNo such template name found.\n\n";
    standardOut->flush();
    return false;
}

void List::listAllTemplatesWithHash(QList<GameTemplate>* templates, uint32_t hash, QTextStream* standardOut) {
    *standardOut << "Listing all templates that contain the hash \"" << Utilities::uintToHex(hash) << "\":\n";
    bool anyHashesFound = false;

    for (int i = 0; i < templates->length(); i++) {
        const GameTemplate* currentTemplate = &(templates->at(i));
        QList<QPair<uint32_t, QByteArray>> hashesFound = QList<QPair<uint32_t, QByteArray>>();

        for (int hashIndex = 0; hashIndex < currentTemplate->data.length(); hashIndex++) {
            if (currentTemplate->data.at(hashIndex).first == hash) {
                anyHashesFound = true;
                hashesFound.push_back(QPair<uint32_t, QByteArray>(currentTemplate->data.at(hashIndex).first, currentTemplate->data.at(hashIndex).second));
            }
        }

        if (hashesFound.length() > 0) {
            *standardOut << "\t" << currentTemplate->name << ":\n";
            for (int j = 0; j < hashesFound.length(); j++) {
                uint32_t foundHash = hashesFound.at(j).first;
                QString hashAsHex = Utilities::uintToHex(foundHash);
                *standardOut << "\t\tHash: " << hashAsHex << " " << Utilities::intToASCII(foundHash, true) << " Data: " << hashesFound.at(j).second.toHex().toUpper() << "\n";
            }
        }
    }

    if (anyHashesFound == false) {
        *standardOut << "\tHash not found in any template.\n";
    }
    standardOut->flush();
}

void List::listAllTemplatesWithHashAndValuePair(QList<GameTemplate>* templates, uint32_t hash, QByteArray data, QTextStream* standardOut) {
    *standardOut << "Listing all templates that contain the hash \"" << Utilities::uintToHex(hash) << "\"" << " with the data \"" << data.toHex().toUpper() << "\":\n";
    bool anyHashesFound = false;

    for (int i = 0; i < templates->length(); i++) {
        const GameTemplate* currentTemplate = &(templates->at(i));
        QList<QPair<uint32_t, QByteArray>> hashesFound = QList<QPair<uint32_t, QByteArray>>();

        for (int hashIndex = 0; hashIndex < currentTemplate->data.length(); hashIndex++) {
            if (currentTemplate->data.at(hashIndex).first == hash && currentTemplate->data.at(hashIndex).second == data) {
                anyHashesFound = true;
                hashesFound.push_back(QPair<uint32_t, QByteArray>(currentTemplate->data.at(hashIndex).first, currentTemplate->data.at(hashIndex).second));
            }
        }

        if (hashesFound.length() > 0) {
            *standardOut << "\t" << currentTemplate->name << ":\n";
            for (int j = 0; j < hashesFound.length(); j++) {
                uint32_t foundHash = hashesFound.at(j).first;
                QString hashAsHex = Utilities::uintToHex(foundHash);
                *standardOut << "\t\tHash: " << hashAsHex << " " << Utilities::intToASCII(foundHash, true) << " Data: " << hashesFound.at(j).second.toHex().toUpper() << "\n";
            }
        }
    }

    if (anyHashesFound == false) {
        *standardOut << "\tHash not found in any template.\n";
    }
    standardOut->flush();
}

bool List::listHashesOfTemplatesWithDataOfSize(QList<GameTemplate>* templates, QString templateName, uint size, QTextStream* standardOut) {
    *standardOut << "Listing all Hashes of template \"" << templateName << "\" with data of size " << size << ":\n";
    bool hashPrinted = false;

    for (int i = 0; i < templates->length(); i++) {
        const GameTemplate* currentTemplate = &templates->at(i);
        if (currentTemplate->name == templateName) {

            for (int hashIndex = 0; hashIndex < currentTemplate->data.length(); hashIndex++) {
                if (currentTemplate->data.at(hashIndex).second.length() == size) {
                    uint32_t hashValue = currentTemplate->data.at(hashIndex).first;
                    QString hashAsHex = Utilities::uintToHex(hashValue);
                    *standardOut << "\tHash: " << hashAsHex;
                    *standardOut << "\t" << Utilities::intToASCII(hashValue, true);

                    *standardOut << "\tData: " << currentTemplate->data.at(hashIndex).second.toHex().toUpper() << "\n";
                    hashPrinted = true;
                }
            }

            if (hashPrinted == false) {
                *standardOut << "\tNo hashes of size " << size << " found.\n";
            }

            return true;
        }
    }

    *standardOut << "\tNo such template name found.\n\n";
    standardOut->flush();
    return false;
}

void List::listTemplatesByModel(QList<GameTemplate>* templates, QTextStream* standardOut) {
    struct ModelList {
        QString modelHash;
        QList<QString>* templateNames;
    };

    *standardOut << "Listing which templates include what models, organized by model hash value:" << endl;
    QList<ModelList> templatesByHash = QList<ModelList>();

    for (int templateIndex = 0; templateIndex < templates->length(); templateIndex++) {

        const GameTemplate* currentTemplate = &(templates->at(templateIndex));
        for (int hashIndex = 0; hashIndex < currentTemplate->data.length(); hashIndex++) {
            uint32_t currentHash = currentTemplate->data.at(hashIndex).first;
            // Check if the hash is handling a 3D model
            if (currentHash == 0x5042725B) {
                QByteArray modelHashBytes = currentTemplate->data.at(hashIndex).second;
                QString modelHash = modelHashBytes.toHex();

                bool modelAlreadyUsed = false;
                for (int i = 0; i < templatesByHash.length(); i++) {
                    if (templatesByHash.at(i).modelHash == modelHash) {
                        modelAlreadyUsed = true;
                        templatesByHash.at(i).templateNames->push_back(currentTemplate->name);
                    }
                }

                // Add this template to the list of templates
                // that use this particular model.
                if (modelAlreadyUsed == false) {
                    QList<QString>* newTemplateList = new QList<QString>();
                    newTemplateList->push_front(currentTemplate->name);

                    ModelList newModelList;
                    newModelList.modelHash = modelHash;
                    newModelList.templateNames = newTemplateList;
                    templatesByHash.push_front(newModelList);
                }
            }
        }
    }

    // Sort all template lists alphabetically
    for (int i = 0; i < templatesByHash.length(); i++) {
        QList<QString>* currentList = templatesByHash.at(i).templateNames;
        currentList->sort(Qt::CaseSensitive);
    }

    //Print all templates by model
    if (templatesByHash.length() > 0) {

        for (int i = 0; i < templatesByHash.length(); i++) {
            // Improve readability by adding a line of spacing between each model.
            if (i > 0) {
                *standardOut << endl;
            }

            *standardOut << "\tModel: " << templatesByHash.at(i).modelHash.toUpper() << endl;
            QList<QString>* templates = templatesByHash.at(i).templateNames;
            for (int j = 0; j < templates->length(); j++) {
                *standardOut << "\t\t" << templates->at(j) << endl;
            }
        }
    } else {
        *standardOut << "\tNo models found in the loaded templates." << endl;
    }

    // Clean up pointer lists.
    for (int i = 0; i < templatesByHash.length(); i++) {
        delete templatesByHash.at(i).templateNames;
    }
    standardOut->flush();
}
