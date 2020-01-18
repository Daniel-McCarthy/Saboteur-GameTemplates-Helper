#include "gametemplate.h"

GameTemplate::GameTemplate(QObject *parent) : QObject(parent)
{

}

GameTemplate::GameTemplate(QString name, QString templateType, QList<QList<uint8_t>> data) : QObject(nullptr), name(name), templateType(templateType), data(data)
{

}

GameTemplate::GameTemplate(const GameTemplate& otherTemplate) {

    this->name = otherTemplate.name;
    this->templateType = otherTemplate.templateType;
    this->data = otherTemplate.data;
}

void GameTemplate::operator=(const GameTemplate& otherTemplate) {
    this->name = otherTemplate.name;
    this->templateType = otherTemplate.templateType;
    this->data = otherTemplate.data;
}
