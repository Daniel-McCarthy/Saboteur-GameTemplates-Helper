#ifndef SEARCH_H
#define SEARCH_H

#include <QString>
#include <QTextStream>

class GameTemplate;
class Search
{
public:
    Search();
    static void searchTemplatesForKeyword(QList<GameTemplate>* templates, QString keyword, QTextStream* standardOut);
    static void searchTemplatesofSubtypeForKeyword(QList<GameTemplate>* templates, QString subtype, QString keyword, QTextStream* standardOut);
};

#endif // SEARCH_H
