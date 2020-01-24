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
