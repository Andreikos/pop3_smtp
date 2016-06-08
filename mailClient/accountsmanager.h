#ifndef ACCOUNTSMANAGER_H
#define ACCOUNTSMANAGER_H
#include <QDialog>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include <QMessageBox>
namespace Ui{
class AccountsManager;
}
class AccountsManager: public QDialog
{
    Q_OBJECT
public:
    explicit AccountsManager(QWidget *parent = 0);
    ~AccountsManager();
    void setAccountsDataBase(QSqlDatabase accounts_base);
private:
    Ui::AccountsManager *ui;
    QSqlDatabase accounts_base;
public slots:
    void addItem(QString email);
    void delAcc();
    void chanAcc();
    void addAcc();
signals:
    void needDeleteAccount(QString email);
    void needChangeAccount(QString email);
    void needAddAccount();
};

#endif // ACCOUNTSMANAGER_H
