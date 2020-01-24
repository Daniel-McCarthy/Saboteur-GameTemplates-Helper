#ifndef LIST_H
#define LIST_H

#include <QTextStream>

class GameTemplate;
class List
{
public:
    List();
    static void listAllTemplates(QList<GameTemplate>* templates, bool includeSubTypes, QTextStream* standardOut);
    static void listAllSubTypes(QList<GameTemplate>* templates, QTextStream* standardOut);
};

#endif // LIST_H
