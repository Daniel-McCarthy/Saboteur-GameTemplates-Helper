#include "list.h"
#include "gametemplate.h"

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
