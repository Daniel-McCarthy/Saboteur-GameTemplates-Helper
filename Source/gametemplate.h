#ifndef GAMETEMPLATE_H
#define GAMETEMPLATE_H

#include <QObject>

class GameTemplate : public QObject
{
    Q_OBJECT
public:
    explicit GameTemplate(QObject *parent = nullptr);
    explicit GameTemplate(QString name, QString templateType, QList<QPair<uint32_t, QList<uint8_t>>> data);
    explicit GameTemplate(const GameTemplate& otherTemplate);
    void operator=(const GameTemplate& otherTemplate);
    QString name = "";
    QString templateType = "";
    QList<QPair<uint32_t, QList<uint8_t>>> data;

};

#endif // GAMETEMPLATE_H
