#include "accountsmanager.h"
#include "ui_accountsManager.h"
AccountsManager::AccountsManager(QWidget *parent) : QDialog(parent),
    ui(new Ui::AccountsManager)
{
    ui->setupUi(this);
   // accounts_base = QSqlDatabase::addDatabase("QSQLITE");
   // accounts_base.setDatabaseName("mail_accounts.sqlite");
//    if (!accounts_base.open()) {
//        QMessageBox::warning(this,"Ошибка","Не удалось открыть базу данных");
//    }
    connect(ui->addAccount,SIGNAL(clicked()),this,SLOT(addAcc()));
    connect(ui->deleteAccount,SIGNAL(clicked()),this,SLOT(delAcc()));
    connect(ui->changeAccount,SIGNAL(clicked()),this,SLOT(chanAcc()));
    connect(ui->closeButton,SIGNAL(clicked()),this,SLOT(close()));
}
AccountsManager::~AccountsManager()
{
    delete ui;
}
void AccountsManager::setAccountsDataBase(QSqlDatabase accounts_base){
    this->accounts_base = accounts_base;
    QSqlQuery *query = new QSqlQuery(accounts_base);
    query->exec("SELECT email FROM accounts;");

        while(query->next()){
            QSqlRecord rec = query->record();
            QString tmp = query->value(rec.indexOf("email")).toString();
            if(ui->listWidget->findItems(tmp,Qt::MatchExactly| Qt::MatchCaseSensitive).size()==0){
                ui->listWidget->addItem(tmp);
            }

        }

}
void AccountsManager::addItem(QString email){
    ui->listWidget->addItem(email);
}

void AccountsManager::addAcc(){
    emit needAddAccount();
}
void AccountsManager::delAcc(){
    if(ui->listWidget->currentRow()!=-1){
        emit needDeleteAccount(ui->listWidget->currentItem()->text());
        QList<QListWidgetItem*> list = ui->listWidget->selectedItems();
        for(int i=0;i<list.size();i++){
            ui->listWidget->takeItem(ui->listWidget->row(list.at(i)));
        }
    }
}
void AccountsManager::chanAcc(){
    if(ui->listWidget->currentRow()!=-1){
        emit needChangeAccount(ui->listWidget->currentItem()->text());
    }
}
