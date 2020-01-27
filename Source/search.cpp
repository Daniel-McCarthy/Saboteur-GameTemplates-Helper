#include "search.h"
#include "gametemplate.h"

Search::Search()
{

}

void Search::searchTemplatesForKeyword(QList<GameTemplate>* templates, QString keyword, QTextStream* standardOut) {
    QList<QString> matches = QList<QString>();
    *standardOut << "Searching for templates containing the keyword \"" << keyword << "\":\n";
    standardOut->flush();

    for (int i = 0; i < templates->length(); i++) {
        if (templates->at(i).name.contains(keyword, Qt::CaseInsensitive)) {
            matches.push_back(templates->at(i).name);
        }
    }

    // Sort alphabetically.
    matches.sort(Qt::CaseSensitive);

    if (matches.length() == 0) {
        *standardOut << "\tNo templates found with this keyword.\n\n";
        return;
    }

    for (int i = 0; i < matches.length(); i++) {
        *standardOut << "\t" << matches.at(i) << "\n";
    }
    standardOut->flush();
}
