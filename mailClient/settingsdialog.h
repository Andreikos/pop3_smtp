#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H
#include <QDialog>
#include <QDebug>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>

namespace Ui{
class SettingsDialog;
}
class SettingsDialog : public QDialog   
{
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget *parent = 0);
    void setFromDBase(QString email);
    void setForNew();
    void setAccountsDataBase(QSqlDatabase accounts_base);
    void setAddOption(bool add);
    ~SettingsDialog();
private:
    Ui::SettingsDialog *ui;
    bool add;
//    QString pop3Host,smtpHost,user,pass;
//    int pop3Port,smtpPort;
//    bool pop3SSL,smtpSSL;
    QSqlDatabase accounts_base;

public slots:
    void acceptChanges();
signals:
    void sendEmail(QString);
  //  void sendPop3Settings(QString,int,QString,QString,bool);
  //  void sendSmtpSettings(QString,int,bool);
};

#endif // SETTINGSDIALOG_H
