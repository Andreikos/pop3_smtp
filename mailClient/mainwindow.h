#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <pop3_connection.h>
#include <settingsdialog.h>
#include <accountsmanager.h>
#include <parser.h>
#include <QMessageBox>
#include <QCheckBox>
#include <QSqlQuery>
#include <QDebug>
#include <QPair>
#include <QVector>
#include <QtConcurrent>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QErrorMessage>
#include <QTableWidgetItem>
#include "sendmessage.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    //void GetInbox();
private:
    Ui::MainWindow *ui;
    POP3_Connection *pop3;
    SettingsDialog *settings_dial;
    AccountsManager *acc_manager;
    QSqlDatabase accounts_base,messages;
    SendMessage *sendm;
    int mess_count;
    void initAccSwitcher();
    void initTable();
public slots:
    //void getInboxMessages();
    void openAccountsManager();
    void addAccount();
    void addNewMessage(QString uid,QString from,QString subject,QString date,QString mess);
    void changeAccount(QString email);
    void deleteAccount(QString email);
    void showError(QString error);
    void getPOP3Settings(QString host,int port,QString username,QString password,bool ssl);
    void addSwitcherItem(QString email);
    void showResponse(QString resp);
    void connectServer();
    void gettingMessagesNotify(bool finished);
    void stopGet();
    void readMessage(int,int);
    void getSMTPSettings();
  //  void deleteMessages();
signals:
    void nextMess();
    void stopGettingMessages();
    void progress(int);
    void sendSMTPSettings(QString,QString,int,QString,QString,bool);
  //
};

#endif // MAINWINDOW_H
