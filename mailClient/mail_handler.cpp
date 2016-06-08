#include "mail_handler.h"

namespace ReadMail {

static QString decodeWord(const QByteArray &fullWord, const QByteArray &charset, const QByteArray &encoding, const QByteArray &encoded) {
        if (encoding == "Q") {
            return decodeByteArray(translateQuotedPrintableToBin(encoded), charset);
        } else if (encoding == "B") {
            return decodeByteArray(QByteArray::fromBase64(encoded), charset);
        } else {
            return fullWord;
        }
    }
QByteArray translateQuotedPrintableToBin(const QByteArray &input) {
        QByteArray res;
        for (int i = 0; i < input.size(); ++i) {
            if (input[i] == '_') {
                res += ' ';
            } else if (input[i] == '=' && i < input.size() - 2) {
                int hi = hexValueOfChar(input[++i]);
                int lo = hexValueOfChar(input[++i]);
                if (hi != -1 && lo != -1) {
                    res += static_cast<char> ((hi << 4) + lo);
                } else {
                    res += input.mid(i - 2, 3);
                }
            } else {
                res += input[i];
            }
        }
        return res;
    }
QString decodeByteArray(const QByteArray &encoded, const QString &charset) {
        if (QTextCodec * codec = QTextCodec::codecForName(charset.toLatin1())) {
            return codec->toUnicode(encoded);
        }
        return QString::fromUtf8(encoded, encoded.size());
    }

    static inline int hexValueOfChar(const char input) {
        if (input >= '0' && input <= '9') {
            return input - '0';
        } else if (input >= 'A' && input <= 'F') {
            return 0x0a + input - 'A';
        } else if (input >= 'a' && input <= 'f') {
            return 0x0a + input - 'a';
        } else {
            return -1;
        }
    }
    QString decodeWordSequence(const QByteArray& str) {
            QRegExp whitespace("^\\s+$");

            QString out;

            QRegExp encodedWord("\"?=\\?(\\S+)\\?(\\S+)\\?(.*)\\?=\"?");

            encodedWord.setMinimal(true);

            int pos = 0;
            int lastPos = 0;

            while (pos != -1) {
                pos = encodedWord.indexIn(str, pos);
                if (pos != -1) {
                    int endPos = pos + encodedWord.matchedLength();

                    QString preceding(str.mid(lastPos, (pos - lastPos)));
                    QString decoded = decodeWord(str.mid(pos, (endPos - pos)), encodedWord.cap(1).toLatin1(),
                            encodedWord.cap(2).toUpper().toLatin1(), encodedWord.cap(3).toLatin1());

                    if (!whitespace.exactMatch(preceding))
                        out.append(preceding);

                    out.append(decoded);

                    pos = endPos;
                    lastPos = pos;
                }
            }

            // Copy anything left
            out.append(QString::fromUtf8(str.mid(lastPos)));

            return out;
        }
}

