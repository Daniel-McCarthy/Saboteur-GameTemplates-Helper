#ifndef UTILITIES_H
#define UTILITIES_H

#include <QString>

class Utilities
{
public:
    Utilities();
    static QString intToASCII(int32_t number, bool reverseEndian);
    static QString uintToHex(uint32_t number);
};

#endif // UTILITIES_H
