#include "sendmessage.h"
#include "ui_sendmessagedialog.h"
SendMessage::SendMessage(QWidget *parent) : QDialog(parent),
    ui(new Ui::SendMessage)
{
    ui->setupUi(this);
    smtp = new SMTP_Connection();
    connect(ui->sendButton,SIGNAL(clicked()),this,SLOT(send()));
    connect(smtp,SIGNAL(serverResponse(QString)),this,SLOT(showServerResponse(QString)));
    connect(smtp,SIGNAL(error_code(QString)),this,SLOT(showError(QString)));
    connect(smtp,SIGNAL(status(QString)),this,SLOT(showStatus(QString)));
}

SendMessage::~SendMessage(){
    delete ui;
}
void SendMessage::send(){
    emit getSettings();
}
void SendMessage::setSMTPSettings(QString email, QString host, int port, QString user, QString pass, bool ssl){
    qDebug()<<email<<endl;
    qDebug()<<host<<endl;
    qDebug()<<user<<endl;
    qDebug()<<pass<<endl;
    qDebug()<<port<<endl;
    smtp->set_Sender_mail(email);
    smtp->set_Subject(ui->subject->text());
    smtp->set_Body_text(ui->message->toPlainText());
    smtp->set_Recipient_mail(ui->to->text());
    smtp->set_Login(user);
    smtp->set_Password_for_login(pass);
    smtp->setSSL(ssl);
    bool res = smtp->Connect(host,port);
}
void SendMessage::showError(QString error){
    QMessageBox::warning(this,"Ошибка",error);
}
void SendMessage::showServerResponse(QString response){
    ui->serverMessages->setText(ui->serverMessages->toPlainText()+response);
}
void SendMessage::showStatus(QString status){
    QMessageBox::information(this,"Сообщение",status);
}
