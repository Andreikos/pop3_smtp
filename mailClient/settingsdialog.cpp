#include "settingsdialog.h"
#include "ui_SettingsDialog.h"
SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    connect(ui->buttonBox,SIGNAL(accepted()),this,SLOT(acceptChanges()));
}
SettingsDialog::~SettingsDialog()
{
    delete ui;
}
void SettingsDialog::setAccountsDataBase(QSqlDatabase accounts_base){
    this->accounts_base = accounts_base;
}
void SettingsDialog::setAddOption(bool add){
    this->add = add;
    if(!add){
        ui->email->setEnabled(false);
    }else{
        ui->email->setEnabled(true);
    }
}

void SettingsDialog::acceptChanges(){
    if(ui->pop3Host->text()==""){
        QMessageBox::warning(this,"Предупреждение","Введите имя сервера POP3");
    }else if(ui->smtpHost->text()==""){
        QMessageBox::warning(this,"Предупреждение","Введите имя сервера SMTP");
    }else if(ui->username->text()==""|ui->password->text()==""){
        QMessageBox::warning(this,"Предупреждение","Введите данные учётной записи для входа на сервер");
    }else if(ui->pop3Port->text()==""|ui->pop3Port->text().toInt()<=0){
        QMessageBox::warning(this,"Предупреждение","Введите порт сервера POP3 (должен быть больше нуля)");
    }else if(ui->smtpPort->text()==""|ui->smtpPort->text().toInt()<=0){
        QMessageBox::warning(this,"Предупреждение","Введите порт сервера SMTP (должен быть больше нуля)");
    }else{
        if(add){
            QString str = "INSERT INTO accounts(email, pop3_host, smtp_host, username, password, pop3_port, pop3_ssl, smtp_port, smtp_ssl) "
                        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";
            QSqlQuery *query = new QSqlQuery(accounts_base);
            query->prepare(str);
            query->bindValue(0,ui->email->text());
            query->bindValue(1,ui->pop3Host->text());
            query->bindValue(2,ui->smtpHost->text());
            query->bindValue(3,ui->username->text());
            query->bindValue(4,ui->password->text());
            query->bindValue(5,ui->pop3Port->text().toInt());
            query->bindValue(6,ui->pop3SSL->isChecked());
            query->bindValue(7,ui->smtpPort->text().toInt());
            query->bindValue(8,ui->smtpSSL->isChecked());
            query->exec();
            emit sendEmail(ui->email->text());
        }else{
            QString str = "UPDATE accounts SET pop3_host = ?, smtp_host = ?, username = ?, password = ?, pop3_port = ?, pop3_ssl = ?, smtp_port = ?, smtp_ssl = ? WHERE email=\'"+ui->email->text()+"\'";
            QSqlQuery *query = new QSqlQuery(accounts_base);
            query->prepare(str);
            query->bindValue(0,ui->pop3Host->text());
            query->bindValue(1,ui->smtpHost->text());
            query->bindValue(2,ui->username->text());
            query->bindValue(3,ui->password->text());
            query->bindValue(4,ui->pop3Port->text().toInt());
            query->bindValue(5,ui->pop3SSL->isChecked());
            query->bindValue(6,ui->smtpPort->text().toInt());
            query->bindValue(7,ui->smtpSSL->isChecked());
            query->exec();
        }
    }
}
void SettingsDialog::setFromDBase(QString email){
    QSqlQuery *query = new QSqlQuery(accounts_base);
    query->exec("SELECT * from accounts where email=\'"+email+"\'");
    bool res = query->first();
    if(res){
        QSqlRecord rec = query->record();
        ui->email->setText(query->value(rec.indexOf("email")).toString());
        ui->pop3Host->setText(query->value(rec.indexOf("pop3_host")).toString());
        ui->smtpHost->setText(query->value(rec.indexOf("smtp_host")).toString());
        ui->username->setText(query->value(rec.indexOf("username")).toString());
        ui->password->setText(query->value(rec.indexOf("password")).toString());
        ui->pop3Port->setText(query->value(rec.indexOf("pop3_port")).toString());
        ui->pop3SSL->setChecked(query->value(rec.indexOf("pop3_ssl")).toBool());
        ui->smtpPort->setText(query->value(rec.indexOf("smtp_port")).toString());
        ui->smtpSSL->setChecked(query->value(rec.indexOf("smtp_ssl")).toBool());
    }
}
void SettingsDialog::setForNew(){
    ui->email->setText("");
    ui->pop3Host->setText("");
    ui->smtpHost->setText("");
    ui->username->setText("");
    ui->password->setText("");
    ui->pop3Port->setText("");
    ui->pop3SSL->setChecked(false);
    ui->smtpPort->setText("");
    ui->smtpSSL->setChecked(false);
}


