#include "mainwindow.h"
#include "ui_mainwindow.h"

#include<QDebug>
#include<QtSql/QSqlError>
#include<QtSql/QSqlQuery>
#include<QtSql/QSqlRecord>
#include<QMessageBox>
#include<QDateTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("tasklist");
    //ui->taskInfo->hide();

    task=new tasklist(); //创建进程信息子界面

    if(QSqlDatabase::contains("qt_sql_default_connection")){
        db=QSqlDatabase::database("qt_sql_default_connection");
    }
    else db=QSqlDatabase::addDatabase("QMYSQL");

    db.setHostName("127.0.0.1");
    db.setPort(3306);
    db.setUserName("root");
    db.setPassword("123456");
    db.setDatabaseName("task");

    if(!db.open()){
        qDebug()<<db.lastError();
        qDebug()<<"数据库连接失败";
        return;
    }else{
        qDebug()<<"数据库连接成功";
    }


    QSqlQuery query;
    if(!query.exec("Select * from tasklist")){
        qDebug()<<"Query error: "<<query.lastError().text();
        return;
    }

#if 0
    ui->taskInfo->setRowCount(query.size());   //行
    ui->taskInfo->setColumnCount(query.record().count()); //列

    //给表格控件添加水平表头
    QStringList headerLabels;
    headerLabels<<"计划编号"<<"起始时刻"<<"终止时刻"<<"时间码类型"<<"指向1"<<"指向2"<<"指向3"<<"指向类型"<<"拍摄帧数"<<"曝光时间"<<"帧间间隔"<<"完成状态";
    ui->taskInfo->setHorizontalHeaderLabels(headerLabels);
    //qDebug()<<ui->taskInfo->horizontalHeaderItem(0)->text();

    int row=0;
    while(query.next()){
        for(int i=0;i<query.record().count();i++){
            QTableWidgetItem *item=new QTableWidgetItem(query.value(i).toString());
            //QTableWidgetItem item(query.value(i).toString());
            ui->taskInfo->setItem(row,i,item);
        }
        row++;
    }

    /*检测单元格内容是否被修改
    cellChanged: 用户交互的修改才会触发  多次修改只会发出一次信号
    itemChanged: 无论用户交互的修改还是代码修改都会触发  多次修改就会发出多个信号
    */

    connect(ui->taskInfo,&QTableWidget::cellChanged,this,[=](int row,int col){
        //if(ui->taskInfo!=nullptr) qDebug()<<"指针不为空!";
        QString keyName=ui->taskInfo->horizontalHeaderItem(0)->text();

        QString KeyValue=ui->taskInfo->item(row,0)->text();

        QString columnName=ui->taskInfo->horizontalHeaderItem(col)->text(); //获得字段名称

        QSqlQuery query;
        QString sql;

        if(col==0||col==3||col==7){ //varchar类型 QString类型数据可以直接插入
            QString newValue=ui->taskInfo->item(row,col)->text();    //获取单元格修改后的值
            sql=QString("update tasklist set %1 = '%2' where %3 = %4").arg(columnName,0).arg(newValue,1).arg(keyName,2).arg(KeyValue,3);
        }
        else if(col==1||col==2){
            //QString newValue=ui->taskInfo->item(row,col)->text();    //获取单元格修改后的值
            QDateTime newValue=QDateTime::fromString(ui->taskInfo->item(row,col)->text(),"yyyy-MM-dd hh:mm:ss");
            sql=QString("update tasklist set %1 = '%2' where %3 = %4").arg(columnName,0).arg(newValue.toString(),1).arg(keyName,2).arg(KeyValue,3);
        }
        else if(col==6||col==8||col==11){
            int newValue=ui->taskInfo->item(row,col)->text().toInt();    //获取单元格修改后的值
            sql=QString("update tasklist set %1 = %2 where %3 = %4").arg(columnName,0).arg(newValue,1).arg(keyName,2).arg(KeyValue,3);
        }
        else{
            double newValue=ui->taskInfo->item(row,col)->text().toDouble();
            sql=QString("update tasklist set %1 = %2 where %3 = %4").arg(columnName,0).arg(newValue,1).arg(keyName,2).arg(KeyValue,3);
        }


        if(!query.exec(sql)){
            qDebug()<<"fail to update: "<<query.lastError().text();
        }
        else{
            qDebug()<<"修改成功";
        }
    });
    //可以把编辑框完成编辑信号绑定到槽函数 判断编辑内容是否合法
    //connect(ui->lineEdit, &QLineEdit::editingFinished, this, &MainWindow::onLineEditFocusOut);
#endif
    //treeView模块
    //创建数据模型指针变量  并关联视图控件以同步模型的数据变化到视图
    m_model=new QStandardItemModel(ui->treeView);
    ui->treeView->setModel(m_model);

    //设置水平表头
    //ui->treeView->header()->setSectionResizeMode(QHeaderView::Fixed);
    m_model->setHorizontalHeaderLabels(QStringList()<<"计划编号"<<"起始时刻"<<"终止时刻"<<"时间码类型"<<"指向1"<<"指向2"<<"指向3"<<"指向类型"<<"拍摄帧数"<<"曝光时间"<<"帧间间隔"<<"完成状态");



}

MainWindow::~MainWindow()
{
    if(task!=nullptr){
        delete task;
        task=nullptr;
    }
    int rowCount=ui->taskInfo->rowCount();
    int columnCount=ui->taskInfo->columnCount();
    for(int row=0;row<rowCount;++row){
        for(int col=0;col<columnCount;++col){
            QTableWidgetItem *item=ui->taskInfo->takeItem(row,col);
            if(item!=nullptr) delete item;
        }
    }

    if(db.isOpen()) db.close();
    delete ui;
}


/*
void MainWindow::focusOutEvent(QFocusEvent *event)
{
    QMainWindow::focusOutEvent(event);

    if(needSave){  //判断是否需要保存更改
        //弹出提示框
        QMessageBox::StandardButton reply=QMessageBox::question(this,"Tip","Do you want to save the changes?",
                                                                QMessageBox::Yes|QMessageBox::No);
        if(reply==QMessageBox::Yes){

        }
    }
}
*/

void MainWindow::on_processInfo_pushButton_clicked()
{
    //task->exec();
}

void MainWindow::on_addTask_pushButton_clicked()
{

}


void MainWindow::on_addTaskGroup_pushButton_clicked()
{

}

