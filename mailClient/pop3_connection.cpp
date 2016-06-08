#include "pop3_connection.h"

#include <iostream>
#include <QtDebug>

POP3_Connection::POP3_Connection(bool readOnly,bool ssl)
{
    this->ssl = ssl;
    this->readOnly = readOnly;
    state = NotConnected;
    num = 0;
}

void POP3_Connection::SetReadOnly(bool readOnly){
    this->readOnly = readOnly;
}

void POP3_Connection::SetSSL(bool ssl){
    this->ssl = ssl;
}

bool POP3_Connection::Connect(QString host,short unsigned int port)
{
    qDebug()<<host<<endl;
    qDebug()<<port<<endl;
    if(ssl){
        ssl_sock = new QSslSocket(this);
        ssl_sock->connectToHostEncrypted(host,port);
        m_sock = ssl_sock;
    }else{
        m_sock = new QTcpSocket(this);
        m_sock->connectToHost(host,port);
    }


    QString response;
    ReadResponse(false,response);

    if (response.startsWith("+OK")){
        emit sendResponse(response);
        state = Authorization;
    }
    else{
        return false;
    }
    return true;
}

QString POP3_Connection::doCommand(QString command,bool isMultiline)
{
    QString response;
    qint64 writeResult;
    if(!ssl)
        writeResult = m_sock->write(command.toLatin1());
    else
        writeResult = ssl_sock->write(command.toLatin1());
    if (writeResult != command.size()) emit sendError("Невозможно отправить все данные: ");
    if(!ssl)
        if (writeResult > 0 && !m_sock->waitForBytesWritten(WRITE_TIMEOUT)) emit sendError("Невозможно записать данные в буфер. Достигнут таймаут.");
    else
        if (writeResult > 0 && !ssl_sock->waitForBytesWritten(WRITE_TIMEOUT)) emit sendError("Невозможно записать данные в буфер. Достигнут таймаут.");
    if (!ReadResponse(isMultiline,response))
        return "";

    return response;
}

bool POP3_Connection::ReadResponse(bool isMultiline,QString& response)
{
    char buff[1512+1];
    bool couldRead;
    if(!ssl)
        couldRead = m_sock->waitForReadyRead( READ_TIMEOUT ) ;
    else
        couldRead = ssl_sock->waitForReadyRead( READ_TIMEOUT ) ;
    if (!couldRead) emit sendError("Невозможно получить данные. Достигнут таймаут.");
    bool complete=false;
    bool completeLine=false;
    unsigned int offset=0;
    while (couldRead && !complete)
    {
        if (offset >= sizeof(buff))
        {
            qDebug() << "Переполнение буфера \n";
            return false;
        }
        qint64 bytesRead;
        if(!ssl)
            bytesRead = m_sock->readLine(buff + offset,sizeof(buff)-offset);
        else
            bytesRead = ssl_sock->readLine(buff + offset,sizeof(buff)-offset);
        if (bytesRead == -1)
            return false;
        couldRead = bytesRead > 0;
        completeLine = buff[offset+bytesRead-1]=='\n';
        if (couldRead)
        {
            if (completeLine)
            {
                offset = 0;
                if (response.size() == 0)
                {
                    response.append(buff);
                    if (!response.startsWith("+OK"))
                        complete = true;
                    else
                        complete = !isMultiline;
                }
                else
                {
                    complete = ( strcmp(buff,".\r\n") == 0 );
                    if (!complete)
                    {
                        if (buff[0] == '.' && buff[1]== '.')
                            response.append(buff+1);
                        else
                            response.append(buff);
                    }
                }
            }
            else
            {
                offset += bytesRead;
            }
        }
        if (couldRead && !complete)
        {
            if (m_sock->bytesAvailable() <= 0)
            {
                couldRead = m_sock->waitForReadyRead( READ_TIMEOUT ) ;
            }
        }
    }
    return couldRead && complete;
}

bool POP3_Connection::Login(QString user, QString pwd)
{
    if (!SendUser(user))
        return false;
    if (!SendPasswd(pwd))
        return false;
    state = Transaction;
    return true;
}
bool POP3_Connection::SendUser(QString& user)
{
    QString res = doCommand("USER "+user+"\r\n",false);
    qDebug()<<res<<endl;
    if (res.startsWith("+OK"))
        return true;
    else{
        emit sendError(res);
        return false;
    }
}
bool POP3_Connection::SendPasswd(QString& pwd)
{
    QString res = doCommand("PASS "+pwd+"\r\n",false);
    qDebug()<<res<<endl;
    if (res.startsWith("+OK"))
        return true;
    else{
        emit sendError(res);
        return false;
    }
}
bool POP3_Connection::Quit()
{
    if (readOnly)
        if (!ResetDeleted())
            return false;
    QString res = doCommand("QUIT\r\n",false);
    if (res.startsWith("+OK"))
    {
        if (!m_sock->waitForDisconnected(DISCONNECT_TIMEOUT))
        {
            emit sendError("Соединение не было завершено сервером: ");
            return false;
        }
        else
            state = NotConnected;
        return true;
    }
    else
    {
        return false;
    }
}
bool POP3_Connection::GetMailboxStatus(int& nbMessages, int& totalSize)
{
    QString res = doCommand("STAT\r\n",false);
    if (res.startsWith("+OK"))
    {
        QStringList sl = res.split(' ',QString::SkipEmptyParts);
        if (sl.count() < 3)
            return false;
        else
        {
            nbMessages = sl[1].toInt();
            totalSize = sl[2].toInt();
        }
        return true;
    }
    else
        return false;
}
bool POP3_Connection::ResetDeleted()
{
    QString res = doCommand("RSET\r\n",false);
    if (res.startsWith("+OK"))
        return true;
    else
        return false;
}
bool POP3_Connection::NoOperation()
{
    QString res = doCommand("NOOP\r\n",false);
    if (res.startsWith("+OK"))
        return true;
    else
        return false;
}
QPair<QString,QString> POP3_Connection::parseUniqueIdListing(QString& line)
{
    QPair<QString,QString> p;
    QStringList sl = line.split(' ',QString::SkipEmptyParts);
    p.first = sl[0];
    p.second = sl[1];
    return p;
}
QPair<QString,int> POP3_Connection::parseMsgIdListing(QString& line)
{
    QPair<QString,int> p;
    QStringList sl = line.split(' ',QString::SkipEmptyParts);
    p.first = sl[0];
    p.second = sl[1].toInt();
    return p;
}
bool POP3_Connection::GetUniqueIdList(QVector< QPair<QString,QString> >& uIdList)
{
    QString res = doCommand("UIDL\r\n",true);
    if (res.startsWith("+OK"))
    {
        QStringList sl = res.split("\r\n",QString::SkipEmptyParts);
        int i;
        for (i=1;i<sl.count();i++)
            uIdList.append(parseUniqueIdListing(sl[i]));
        return true;
    }
    else
        return false;
}
bool POP3_Connection::GetUniqueIdList(QString msgId, QPair<QString,QString>& uIdList)
{
    QString res = doCommand("UIDL "+msgId+"\r\n",false);
    if (res.startsWith("+OK"))
    {
        QStringList sl = res.split(' ',QString::SkipEmptyParts);
        uIdList.first = sl[1];
        uIdList.second = sl[2];
        return true;
    }
    else
        return false;
}
bool POP3_Connection::Delete(QString msgId)
{
    if (readOnly)
        return false;
    QString res = doCommand("DELE "+msgId+"\r\n",false);
    if (res.startsWith("+OK"))
        return true;
    else
        return false;
}
bool POP3_Connection::GetMessageTop(QString msgId, int nbLines, QString& msgTop)
{
    QString res = doCommand("TOP "+msgId+" "+QString::number(nbLines)+"\r\n",true);
    if (res.startsWith("+OK"))
    {
        msgTop = res.section("\r\n",1);
        return true;
    }
    else
        return false;
}
bool POP3_Connection::GetMessage(QString msgId, QString& msg)
{
    QString res = doCommand("RETR "+msgId+"\r\n",true);
    if (res.size() == 0)
        return false;
    if (res.startsWith("+OK"))
    {
        msg = res.section("\r\n",1);
        return true;
    }
    else
    {
        return false;
    }
}
bool POP3_Connection::GetMsgList(QVector< QPair<QString,int> >& uIdList)
{
    QString res = doCommand("LIST\r\n",true);
    if (res.startsWith("+OK"))
    {
        QStringList sl = res.split("\r\n",QString::SkipEmptyParts);
        int i;
        for (i=1;i<sl.count();i++)
            uIdList.append(parseMsgIdListing(sl[i]));
        return true;
    }
    else
        return false;
}
bool POP3_Connection::GetMsgList(QString msgId, QPair<QString,int>& uIdList)
{
    QString res = doCommand("LIST "+msgId+"\r\n",false);
    if (res.startsWith("+OK"))
    {
        QStringList sl = res.split(' ',QString::SkipEmptyParts);
        uIdList.first = sl[1];
        uIdList.second = sl[2].toInt();
        return true;
    }
    else
        return false;
}

void POP3_Connection::setStopGet(){
    stopGet = true;
}

void POP3_Connection::getMessages(QVector<QString> list){

    GetUniqueIdList(uIdList);
    for(int i=0;i<uIdList.size();i++){
        if(list.indexOf(uIdList[i].second)!=-1){
            uIdList.removeAt(i);
            i--;
        }
    }
    stopGet = false;
    nextMessage();
}
void POP3_Connection::nextMessage(){
    if(num==uIdList.size()|uIdList.size()==0){
        emit finishedGettingMessages(true);
        num = 0;
    }else{
        QString top;
        GetMessageTop(QString(uIdList.at(num).first),0,top);
        QByteArray header = top.toUtf8();
        QString decode_header = ReadMail::decodeWordSequence(header);
        QString date,from,subject;
        QRegularExpression sub("Subject: +(.*)");
        QRegularExpressionMatch match = sub.match(decode_header);
        if (match.hasMatch()) {
            subject = match.captured(1);
        }
        QRegularExpression fr("From: +(.*)");
        QRegularExpressionMatch match2 = fr.match(decode_header);
        if (match2.hasMatch()) {
            from = match2.captured(1);
        }
        QRegularExpression d("Date: +(.*)");
        QRegularExpressionMatch match3 = d.match(decode_header);
        if (match3.hasMatch()) {
            date = match3.captured(1);
        }//*/
        QString uid = uIdList.at(num).second;
        QString encoded_mess;
        GetMessage(uIdList.at(num).first,encoded_mess);
        num++;
        if(!stopGet){
            emit newMessage(uid,from,subject,date,encoded_mess);
        }else{
            num = 0;
            stopGet = false;
            emit finishedGettingMessages(false);
        }
    }
}
Pop3ConnectionState POP3_Connection::getState(){
    return state;
}

#undef _ERROR

