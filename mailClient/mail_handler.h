#ifndef MAIL_HANDLER_H
#define MAIL_HANDLER_H
#include <QMap>
#include <QStringList>
#include <QTextCodec>
namespace ReadMail{
    typedef QMap<QString, QString> Mail_Field_Format;
    typedef QMap<QString, QStringList> Mail_BoundaryKey;

    QString decodeWordSequence(const QByteArray& str);
    QByteArray translateQuotedPrintableToBin(const QByteArray &input);
    QString decodeByteArray(const QByteArray &encoded, const QString &charset);
    static inline int hexValueOfChar(const char input) ;
}


#endif // MAIL_HANDLER_H
