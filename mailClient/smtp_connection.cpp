#include "smtp_connection.h"

SMTP_Connection::SMTP_Connection()
{
    ssl = false;
}

SMTP_Connection::~SMTP_Connection()
{

}

bool SMTP_Connection::Connect(QString host, quint16 port){
    p_host = host;
    p_port = port;
    if(ssl){
        ssl_sock = new QSslSocket(this);
        ssl_sock->connectToHostEncrypted(host,port);
        sock = ssl_sock;
    }else{
        sock = new QTcpSocket(this);
        sock->connectToHost(host,port);
    }
    if (!sock->waitForConnected(CONNECT_TIMEOUT))
    {
        emit error_code("Невозможно поключиться к серверу SMTP: ");
        return false;
    }
    connect(sock, SIGNAL(connected()),this, SLOT(on_connected()));
    connect(sock, SIGNAL(readyRead()), this, SLOT(on_read()));
    connect(sock, SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(slotError(QAbstractSocket::SocketError)));
    t = new QTextStream(sock);
    state = Init;
    return true;
}
void SMTP_Connection::setSSL(bool ssl){
    this->ssl = ssl;
}
void SMTP_Connection::slotError(QAbstractSocket::SocketError err)
{
    QString strError =
        "Error: " + (err == QAbstractSocket::HostNotFoundError ?
                     "The host was not found." :
                     err == QAbstractSocket::RemoteHostClosedError ?
                     "The remote host is closed." :
                     err == QAbstractSocket::ConnectionRefusedError ?
                     "The connection was refused." :
                     QString(sock->errorString())
                    );
    emit error_code(strError);
}
void SMTP_Connection::send_Mail()
{
    if (p_login.isEmpty() || p_password.isEmpty())
        {
            emit error_code("Логин или пароль не установлены");
            return;
        }
    if (p_mail_From.isEmpty() || p_mail_to.isEmpty())
        {
            emit error_code("Получатель или отправитель не задан");
            return;
        }
    if (p_body.isEmpty())
        {
            emit error_code("Тело сообщения не задано");
            return;
        }
    if (p_subject.isEmpty())
        {
            emit error_code("Тема сообщения не задана");
            return;
        }

}

void SMTP_Connection::on_connected()
{
    emit status("Подключение к SMTP серверу установлено");
}

void SMTP_Connection::on_read()
{
    if( !sock->canReadLine() )
        return;

    QString responseLine;

    do {
            responseLine = sock->readLine();
            response += responseLine;
        } while( sock->canReadLine() && responseLine[3] != ' ' );
    emit serverResponse(responseLine);
    responseLine.truncate( 3 );

    if ( state == Init && responseLine[0] == '2' )
        {

            *t << "HELO there\r\n";
            t->flush();
            state = Start;
        }
    else if (state == Start && responseLine[0] == '2')
        {
            *t << "AUTH LOGIN\r\n";
            t->flush();
            state = Login;
        }
    else if (state == Login && responseLine[0] == '3')
        {
            *t << encodeBase64(p_login) << "\r\n";
            t->flush();
            state = Pass;
        }
    else if (state == Pass && responseLine[0] == '3')
        {
            *t << encodeBase64(p_password) << "\r\n";
            t->flush();
            state = Mail;
        }
    else if ( state == Mail && responseLine[0] == '2' )
        {

            *t << "MAIL FROM: <" << p_mail_From << ">\r\n";
            t->flush();
            state = Rcpt;
        }
    else if ( state == Rcpt && responseLine[0] == '2' )
        {

            *t << "RCPT TO: <" << p_mail_to << ">\r\n";
            t->flush();
            state = Data;
        }
    else if ( state == Data && responseLine[0] == '2' )
        {
            *t << "DATA\r\n";
            t->flush();
            state = Body;
        }
    else if ( state == Body && responseLine[0] == '3' )
        {
            QString body;
            body = p_body;
            QString subject;
            subject = p_subject;
            message = QString::fromLatin1( "From: " ) + p_mail_From
                      +QString::fromLatin1( "\nTo: " ) + p_mail_to +QString::fromLatin1( "\nSubject: "
                                                                                                       ) + subject +QString::fromLatin1( "\n\n" ) + body +"\n";
            message.replace( QString::fromLatin1( "\n" ), QString::fromLatin1(
                    "\r\n" ) );
            message.replace( QString::fromLatin1( "\r\n.\r\n" ),
                             QString::fromLatin1( "\r\n..\r\n" ) );

            *t << message << "\n" << ".\r\n";
            t->flush();
            state = Quit;
        }
    else if ( state == Quit && responseLine[0] == '2' )
        {
            *t << "QUIT\r\n";
            state = Close;
            emit status( tr( "Message sent" ) );
        }
    else if ( state == Close )
        {

        }
    else
        {
            emit status(tr("Неизвестный ответ от сервера"));
            state = Close;
        }
    response = "";
}

void SMTP_Connection::set_Login(QString login)
{
    p_login = login;
}

void SMTP_Connection::set_Password_for_login(QString password)
{
    p_password = password;
}

void SMTP_Connection::set_Sender_mail(QString email)
{
    p_mail_From = email;
}

void SMTP_Connection::set_Recipient_mail(QString email)
{
    p_mail_to = email;
}

void SMTP_Connection::set_Body_text(QString text)
{
    p_body = text;
}

void SMTP_Connection::set_Subject(QString subject)
{
    p_subject = subject;
}

QString SMTP_Connection::encodeBase64( QString xml )
{
    QByteArray text;
    text.append(xml);
    return text.toBase64();
}

QString SMTP_Connection::decodeBase64( QString xml )
{
    QByteArray xcode("");;
    xcode.append(xml);
    QByteArray precode(QByteArray::fromBase64(xcode));
    QString notetxt = precode.data();
    return notetxt;
}

