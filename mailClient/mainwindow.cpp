#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->tableWidget->setColumnCount(5);
    ui->tableWidget->setShowGrid(false);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList()<<""<<""<<"Отправитель"<<"Тема письма"<<"Дата/Время");
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);

    ui->tableWidget->horizontalHeader()->resizeSection(0,50);
    ui->tableWidget->setColumnHidden(0,true);
    ui->tableWidget->horizontalHeader()->resizeSection(1,50);
    ui->tableWidget->horizontalHeader()->resizeSection(2,100);
    ui->tableWidget->horizontalHeader()->resizeSection(3,100);
    ui->tableWidget->horizontalHeader()->resizeSection(4,100);
    settings_dial = new SettingsDialog;
    sendm = new SendMessage;
    connect(sendm,SIGNAL(getSettings()),this,SLOT(getSMTPSettings()));
    connect(this,SIGNAL(sendSMTPSettings(QString,QString,int,QString,QString,bool)),sendm,SLOT(setSMTPSettings(QString,QString,int,QString,QString,bool)));

    acc_manager = new AccountsManager();

    connect(settings_dial,SIGNAL(sendEmail(QString)),this,SLOT(addSwitcherItem(QString)));
    connect(acc_manager,SIGNAL(needAddAccount()),this,SLOT(addAccount()));
    connect(acc_manager,SIGNAL(needDeleteAccount(QString)),this,SLOT(deleteAccount(QString)));
    connect(acc_manager,SIGNAL(needChangeAccount(QString)),this,SLOT(changeAccount(QString)));


    connect(ui->settingsAction,SIGNAL(triggered()),this,SLOT(openAccountsManager()));
    accounts_base = QSqlDatabase::addDatabase("QSQLITE","accounts_connection");
    messages = QSqlDatabase::addDatabase("QSQLITE","messages_connection");
    accounts_base.setDatabaseName("mail_accounts.sqlite");
    if (!accounts_base.open()) {
        QMessageBox::warning(this,"Ошибка","Не удалось открыть базу данных");
    }
    QSqlQuery *a_query = new QSqlQuery(accounts_base);
    QString str = "CREATE TABLE IF NOT EXISTS accounts ("
                "id integer PRIMARY KEY NOT NULL, "
                "email VARCHAR(255), "
                "pop3_host VARCHAR(255), smtp_host VARCHAR(255), username VARCHAR(255), password VARCHAR(255),"
                "pop3_port integer, pop3_ssl boolean, smtp_port integer, smtp_ssl boolean"
                ");";
    a_query->exec(str);
    pop3 = new POP3_Connection(false);
    connect(pop3,SIGNAL(sendError(QString)),this,SLOT(showError(QString)));
    connect(pop3,SIGNAL(sendResponse(QString)),this,SLOT(showResponse(QString)));
    connect(this,SIGNAL(nextMess()),pop3,SLOT(nextMessage()));
    connect(ui->connectTo,SIGNAL(clicked()),this,SLOT(connectServer()));
    connect(pop3,SIGNAL(newMessage(QString,QString,QString,QString,QString)),this,SLOT(addNewMessage(QString,QString,QString,QString,QString)));
    connect(pop3,SIGNAL(finishedGettingMessages(bool)),this,SLOT(gettingMessagesNotify(bool)));
    connect(this,SIGNAL(stopGettingMessages()),pop3,SLOT(setStopGet()));
    connect(ui->stopGetMess,SIGNAL(clicked()),this,SLOT(stopGet()));
    connect(ui->sendEmailAction,SIGNAL(triggered()),sendm,SLOT(show()));
    initAccSwitcher();
    ui->progressBar->setVisible(false);
    ui->stopGetMess->setVisible(false);
    ui->label_5->setVisible(false);
    ui->subject->setReadOnly(true);
    ui->dateTime->setReadOnly(true);
    ui->fromWho->setReadOnly(true);
    ui->messField->setReadOnly(true);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    connect(ui->tableWidget,SIGNAL(cellClicked(int,int)),this,SLOT(readMessage(int,int)));
     connect(this,SIGNAL(progress(int)),ui->progressBar,SLOT(setValue(int)));
    mess_count = 0;
}

void MainWindow::connectServer(){
    if(ui->accountSwitcher->count()>0){
        if(pop3->getState()==Transaction){
            emit stopGettingMessages();
        }
        while (ui->tableWidget->rowCount() > 0)
        {
            ui->tableWidget->removeRow(0);
        }
        ui->subject->setText("");
        ui->messField->setText("");
        ui->dateTime->setText("");
        ui->fromWho->setText("");
        qDebug()<<ui->accountSwitcher->currentText()<<endl;
        QString curr_acc = ui->accountSwitcher->currentText();
        QString tmp = curr_acc;
        QString db_name = curr_acc.replace('.','_')+".sqlite";
        messages.setDatabaseName(db_name);
        messages.open();
        QSqlQuery *query2 = new QSqlQuery(messages);
        query2->exec("SELECT COUNT(*) FROM messages");
        query2->first();
        int cnt = query2->value(0).toInt();
        if(cnt!=0){
                initTable();
        }
        QSqlQuery *query = new QSqlQuery(accounts_base);
        query->exec("SELECT * from accounts where email=\'"+ui->accountSwitcher->currentText()+"\'");
        bool res = query->first();
        if(res){
            QSqlRecord rec = query->record();
            getPOP3Settings(query->value(rec.indexOf("pop3_host")).toString(),query->value(rec.indexOf("pop3_port")).toInt(),
                            query->value(rec.indexOf("username")).toString(),query->value(rec.indexOf("password")).toString(),query->value(rec.indexOf("pop3_ssl")).toBool());
        }
    }
}


void MainWindow::addSwitcherItem(QString email){
    ui->accountSwitcher->addItem(email);
    ui->accountSwitcher->setCurrentIndex(ui->accountSwitcher->count()-1);
}
void MainWindow::readMessage(int row,int column){
    QString str = "SELECT * FROM messages WHERE id="+ui->tableWidget->item(row,0)->text();

    QSqlQuery *query = new QSqlQuery(messages);
    query->exec(str);
    query->next();
    QSqlRecord rec = query->record();
    ui->subject->setText(query->value(rec.indexOf("subject")).toString());
    ui->dateTime->setText(query->value(rec.indexOf("date")).toString());
    ui->fromWho->setText(query->value(rec.indexOf("fromwho")).toString());
    ui->messField->setText(query->value(rec.indexOf("message")).toString());
}

void MainWindow::initTable(){
    int i = 0;
        QSqlQuery *query = new QSqlQuery(messages);
        query->exec("SELECT * FROM messages");
        QString uid,from,subject,date;
            while(query->next()){
                QSqlRecord rec = query->record();
                uid = query->value(rec.indexOf("id")).toString();
                from = query->value(rec.indexOf("fromwho")).toString();
                subject = query->value(rec.indexOf("subject")).toString();
                date = query->value(rec.indexOf("date")).toString();
        ui->tableWidget->insertRow(i);
        QTableWidgetItem *item = new QTableWidgetItem(uid);
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);
        ui->tableWidget->setItem(i,0,item);
        QCheckBox *cb1 = new QCheckBox();
        QTableWidgetItem *check = new QTableWidgetItem();
        check->data(Qt::CheckStateRole);
        check->setCheckState(Qt::Unchecked);
        ui->tableWidget->setCellWidget(i,1,cb1);
        QTableWidgetItem *item2 = new QTableWidgetItem(from);
        item2->setFlags(item2->flags() ^ Qt::ItemIsEditable);
        ui->tableWidget->setItem(i,2,item2);
        QTableWidgetItem *item3 = new QTableWidgetItem(subject);
        item3->setFlags(item3->flags() ^ Qt::ItemIsEditable);
        ui->tableWidget->setItem(i,3,item3);
        QTableWidgetItem *item4 = new QTableWidgetItem(date);
        item4->setFlags(item4->flags() ^ Qt::ItemIsEditable);
        ui->tableWidget->setItem(i,4,item4);
        qDebug()<<uid<<endl;
        i++;
        qApp->processEvents();
        }

}

void MainWindow::openAccountsManager(){
    acc_manager->setAccountsDataBase(accounts_base);
    acc_manager->show();
    acc_manager->exec();
}
void MainWindow::initAccSwitcher(){
    QSqlQuery *query = new QSqlQuery(accounts_base);
    query->exec("SELECT email from accounts;");
        while(query->next()){
            QSqlRecord rec = query->record();
            ui->accountSwitcher->addItem(query->value(rec.indexOf("email")).toString());
        }
}

void MainWindow::addAccount(){

    settings_dial->setAccountsDataBase(accounts_base);
    settings_dial->setAddOption(true);
    settings_dial->setForNew();
    settings_dial->show();
    settings_dial->exec();
}
void MainWindow::changeAccount(QString email){

    settings_dial->setAccountsDataBase(accounts_base);
    settings_dial->setAddOption(false);
    settings_dial->setFromDBase(email);
    settings_dial->show();
    settings_dial->exec();
}
void MainWindow::deleteAccount(QString email){
    ui->accountSwitcher->removeItem(ui->accountSwitcher->findText(email));
    QSqlQuery *a_query = new QSqlQuery(accounts_base);
    QString str = "DELETE FROM accounts WHERE email=\'"+email+"\'";
    a_query->exec(str);
}
void MainWindow::getPOP3Settings(QString host,int port,QString username,QString password,bool ssl){
    pop3->SetSSL(ssl);
    bool res = pop3->Connect(host,port);
    if(res){
        pop3->Login(username,password);
        int mess,total_size;
        pop3->GetMailboxStatus(mess,total_size);
        ui->label_6->setText("Входящие ("+QString::number(mess)+")");
        QString name = ui->accountSwitcher->currentText().replace('.','_');
        messages.setDatabaseName(name+".sqlite");
        if (!messages.open()) {
            QMessageBox::warning(this,"Ошибка","Не удалось открыть базу данных");
        }
        QSqlQuery *a_query = new QSqlQuery(messages);
        QString str = "CREATE TABLE IF NOT EXISTS messages ("
                    "id TEXT, "
                    "fromwho VARCHAR(255), "
                    "subject VARCHAR(255), date TEXT, message TEXT"
                    ");";
        a_query->exec(str);
        ui->progressBar->setVisible(true);
        ui->label_5->setVisible(true);
        ui->stopGetMess->setVisible(true);
        mess_count = mess;
        QVector<QString> id_list(ui->tableWidget->rowCount());
        for(int i=0;i<id_list.size();i++){
            id_list[i] = ui->tableWidget->item(i,0)->text();
        }
        pop3->getMessages(id_list);
    }else{
        QMessageBox::warning(this,"Ошибка аутентификации","Не удалось подключиться к серверу POP3");
    }
}

void MainWindow::getSMTPSettings(){
    if(ui->accountSwitcher->currentText()!=""){
        accounts_base.setDatabaseName("mail_accounts.sqlite");
        if(!accounts_base.isOpen()){
            if (!accounts_base.open()) {
                QMessageBox::warning(this,"Ошибка","Не удалось открыть базу данных");
            }
        }
        QSqlQuery *query = new QSqlQuery(accounts_base);
        query->exec("SELECT * FROM accounts WHERE email=\'"+ui->accountSwitcher->currentText()+"\'");
        bool res = query->first();
        if(res){
            QSqlRecord rec = query->record();
            emit sendSMTPSettings(query->value(rec.indexOf("email")).toString(),query->value(rec.indexOf("smtp_host")).toString(),
                                  query->value(rec.indexOf("smtp_port")).toInt(),query->value(rec.indexOf("username")).toString(),
                                  query->value(rec.indexOf("password")).toString(),query->value(rec.indexOf("smtp_ssl")).toBool());
        }
    }else{
        emit sendSMTPSettings("","",0,"","",false);
    }
}

void MainWindow::addNewMessage(QString uid,QString from,QString subject,QString date,QString mess){
    qDebug()<<uid<<endl;
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
    QTableWidgetItem *item = new QTableWidgetItem(uid);
    item->setFlags(item->flags() ^ Qt::ItemIsEditable);
    ui->tableWidget->setItem(row,0,item);
    QCheckBox *cb1 = new QCheckBox();
    QTableWidgetItem *check = new QTableWidgetItem();
    check->data(Qt::CheckStateRole);
    check->setCheckState(Qt::Unchecked);
    ui->tableWidget->setCellWidget(row,1,cb1);
    QTableWidgetItem *item2 = new QTableWidgetItem(from);
    item2->setFlags(item2->flags() ^ Qt::ItemIsEditable);
    ui->tableWidget->setItem(row,2,item2);
    QTableWidgetItem *item3 = new QTableWidgetItem(subject);
    item3->setFlags(item3->flags() ^ Qt::ItemIsEditable);
    ui->tableWidget->setItem(row,3,item3);
    QTableWidgetItem *item4 = new QTableWidgetItem(date);
    item4->setFlags(item4->flags() ^ Qt::ItemIsEditable);
    ui->tableWidget->setItem(row,4,item4);
    qApp->processEvents();
    QSqlQuery *query = new QSqlQuery(messages);
    QString str = "INSERT INTO messages(id,fromwho,subject,date,message) VALUES(?,?,?,?,?);";
    query->prepare(str);
    query->bindValue(0,uid);
    query->bindValue(1,from);
    query->bindValue(2,subject);
    query->bindValue(3,date);
    query->bindValue(4,mess);
    query->exec();
    emit progress((int)((double)(row)/(double)(mess_count)*100));
    qApp->processEvents();
    emit nextMess();
}

void MainWindow::showError(QString error){
    QMessageBox::warning(this,"Ошибка",error);
}
void MainWindow::showResponse(QString resp){
    qDebug()<<"SIGNAL "+resp<<endl;
}
void MainWindow::gettingMessagesNotify(bool finished){
    emit progress(0);
    ui->progressBar->setVisible(false);
    ui->label_5->setVisible(false);
    ui->stopGetMess->setVisible(false);
    if(finished){
        QMessageBox::information(this,"Получение сообщений с сервера","Сообщения получены");
    }else{

        QMessageBox::warning(this,"Получение сообщений с сервера","Операция остановлена");
    }
}
void MainWindow::stopGet(){
    emit stopGettingMessages();
}


MainWindow::~MainWindow()
{
    delete ui;
}
