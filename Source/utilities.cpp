#include "utilities.h"

Utilities::Utilities()
{

}

QString Utilities::intToASCII(int32_t number, bool reverseEndian = false) {
    QString asciiText = "";

    for (int i = 0; i < 4; i++) {
        char currentCharacter = (reverseEndian == false) ? (number & 0xFF) : ((number & 0xFF000000) >> 24);

        if (currentCharacter > 0x1F && currentCharacter < 0x7F) {
            asciiText += currentCharacter;
        } else {
            asciiText += '.';
        }

        if (reverseEndian == false) {
            number >>= 8;
        } else {
            number <<= 8;
        }
    }

    return asciiText;
}

QString Utilities::uintToHex(uint32_t number) {
    return QString("%1").arg(number, 8, 16, QLatin1Char( '0' )).toUpper();
}
