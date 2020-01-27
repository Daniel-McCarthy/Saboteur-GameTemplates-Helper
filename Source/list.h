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
    static bool listAllTemplatesofSubType(QList<GameTemplate>* templates, QString subtype, QTextStream* standardOut);
    static void listAllTemplatesWithHash(QList<GameTemplate>* templates, uint32_t hash, QTextStream* standardOut);
    static void listAllTemplatesWithHashAndValuePair(QList<GameTemplate>* templates, uint32_t hash, QByteArray data, QTextStream* standardOut);
    static bool listHashesOfTemplate(QList<GameTemplate>* templates, QString templateName, bool printValuesToo, QTextStream* standardOut);
    static bool listHashesOfTemplatesWithDataOfSize(QList<GameTemplate>* templates, QString templateName, uint size, QTextStream* standardOut);
};

#endif // LIST_H
