#ifndef SMTP_CONNECTION_H
#define SMTP_CONNECTION_H
#include <QTcpSocket>
#include <QTextStream>
#include <QSslSocket>
#define CONNECT_TIMEOUT 15*1000

class SMTP_Connection : public QObject
{
        Q_OBJECT

    public:
        SMTP_Connection();
        ~SMTP_Connection();
        bool Connect(QString host, quint16 port);
        void set_Login(QString login);
        void set_Password_for_login(QString password);
        void set_Sender_mail(QString email);
        void set_Recipient_mail(QString email);
        void set_Body_text(QString text);
        void set_Subject(QString subject);
        void send_Mail();
        void setSSL(bool ssl);
    private:

        QString p_host;
        quint32 p_port;
        QString p_login;
        QString p_password;
        QString p_mail_From;
        QString p_mail_to;
        QString p_body;
        QString p_subject;

        QString message;
        bool ssl;
        int state;
        QString response;

        QTcpSocket *sock;
        QSslSocket *ssl_sock;
        QTextStream *t;
        QString encodeBase64( QString xml );
        QString decodeBase64( QString xml );

        enum State {
                Init,
                Start,
                Login,
                Pass,
                Mail,
                Rcpt,
                Data,
                Body,
                Quit,
                Close
            };

    private slots:
        void on_connected();
        void on_read();
        void slotError(QAbstractSocket::SocketError);
    signals:
        void error_code(QString error_text);
        void status(QString);
        void serverResponse(QString);
};

#endif // SMTP_CONNECTION_H
