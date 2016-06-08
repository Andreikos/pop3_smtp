#ifndef SENDMESSAGE_H
#define SENDMESSAGE_H
#include <QDialog>
#include <smtp_connection.h>
#include <QMessageBox>
namespace Ui{
class SendMessage;
}
class SendMessage : public QDialog
{
    Q_OBJECT
public:
   explicit SendMessage(QWidget *parent = 0);
    ~SendMessage();
private:
    Ui::SendMessage *ui;
    SMTP_Connection *smtp;
    bool startConnection;
public slots:
    void send();
    void setSMTPSettings(QString email,QString host,int port,QString user,QString pass,bool ssl);
    void showError(QString error);
    void showServerResponse(QString reponse);
    void showStatus(QString status);
signals:
    void getSettings();
};

#endif // SENDMESSAGE_H
