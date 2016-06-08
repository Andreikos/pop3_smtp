#ifndef POP3_CONNECTION_H
#define POP3_CONNECTION_H


#include <QObject>
#include <QTcpSocket>
#include <QSslSocket>
#include <QDebug>
#include <QDataStream>
#include <QStringList>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QTextCodec>
#include <QTime>
#include <QDate>
#include <QPair>
#include <QVector>
#include <QErrorMessage>
#include "pop3_commands.h"
#include "mail_handler.h"
#include "pop3connectionstate.h"
#define CONNECT_TIMEOUT 15*1000
#define DISCONNECT_TIMEOUT 5*1000
#define READ_TIMEOUT 15*1000
#define WRITE_TIMEOUT 15*1000

class POP3_Connection : public QObject
{
    Q_OBJECT
    private:

        QTcpSocket *m_sock;
        QSslSocket *ssl_sock;
        Pop3ConnectionState state;
        bool readOnly;
        bool ssl;
        bool stopGet;
        int num;
        QVector< QPair<QString,QString> > uIdList;
        QString doCommand(QString command,bool isMultiline);
        bool ReadResponse(bool isMultiline,QString& response);
        bool SendUser(QString& user);
        bool SendPasswd(QString& pwd);
        static QPair<QString,QString> parseUniqueIdListing(QString& line);
        static QPair<QString,int> parseMsgIdListing(QString& line);
    public:

        POP3_Connection(bool readOnly = true,bool ssl = true);
        void SetReadOnly(bool readOnly);
        void SetSSL(bool ssl);
        bool Connect(QString host="localhost",short unsigned  port=110);
        bool Login(QString user, QString pwd);
        bool Quit();
        bool GetMailboxStatus(int& nbMessages, int& totalSize);
        bool ResetDeleted();
        bool NoOperation();
        bool GetUniqueIdList(QVector< QPair<QString,QString> >& uIdList);
        bool GetUniqueIdList(QString msgId, QPair<QString,QString>& uIdList);
        bool GetMsgList(QVector< QPair<QString,int> >& uIdList);
        bool GetMsgList(QString msgId, QPair<QString,int>& uIdList);
        bool Delete(QString msgId);
        bool GetMessageTop(QString msgId, int nbLines, QString& msgTop);
        bool GetMessage(QString msgId, QString& msg);
        void getMessages(QVector<QString> list);
        Pop3ConnectionState getState();
    signals:
        void sendError(QString);
        void sendResponse(QString);
        void newMessage(QString,QString,QString,QString,QString);
        void finishedGettingMessages(bool);
public slots:
        void nextMessage();
        void setStopGet();

};

#endif // POP3_CONNECTION_H
