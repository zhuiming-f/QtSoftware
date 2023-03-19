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

    //treeView模块
    //创建数据模型指针变量  并关联视图控件以同步模型的数据变化到视图
    m_model=new QStandardItemModel(ui->treeView);
    ui->treeView->setModel(m_model);

    //设置水平表头
    //ui->treeView->header()->setSectionResizeMode(QHeaderView::Fixed);
    m_model->setHorizontalHeaderLabels(QStringList()<<""<<"计划编号"<<"起始时刻"<<"终止时刻"<<"时间码类型"<<"指向1"<<"指向2"<<"指向3"<<"指向类型"<<"拍摄帧数"<<"曝光时间"<<"帧间间隔"<<"完成状态");



}

MainWindow::~MainWindow()
{
    if(task!=nullptr){
        delete task;
        task=nullptr;
    }
    /*
    int rowCount=ui->taskInfo->rowCount();
    int columnCount=ui->taskInfo->columnCount();
    for(int row=0;row<rowCount;++row){
        for(int col=0;col<columnCount;++col){
            QTableWidgetItem *item=ui->taskInfo->takeItem(row,col);
            if(item!=nullptr) delete item;
        }
    }
    */

    if(db.isOpen()) db.close();
    delete ui;
}


void MainWindow::on_processInfo_pushButton_clicked()
{
    //task->exec();
}

void MainWindow::on_addTaskGroup_pushButton_clicked()
{
    int idx=m_model->rowCount();  //获取模型的行数
    QList<QStandardItem*> taskGroup;
    taskGroup<<new QStandardItem(QString("任务集%1").arg(idx+1));

    taskGroup[0]->setData(-1,Qt::UserRole+1);
    m_model->appendRow(taskGroup);
}


void MainWindow::on_addTask_pushButton_clicked()
{
    QModelIndex cur=ui->treeView->currentIndex();
    int row=cur.row();
    int column=cur.column();
    int groupRow=cur.data(Qt::UserRole+1).toInt();

    if(row==-1||column==-1||groupRow!=-1){
        return;
    }

    QStandardItem *curItem=m_model->item(row);

    QList<QStandardItem*> task;
    task<<new QStandardItem(QString("任务%1").arg(curItem->rowCount()+1))
         << new QStandardItem(QString(" "))
         << new QStandardItem(QString(" "))
         << new QStandardItem(QString(" "))
         << new QStandardItem(QString(" "))
         << new QStandardItem(QString(" "))
         << new QStandardItem(QString(" "))
         << new QStandardItem(QString(" "))
         << new QStandardItem(QString(" "))
         << new QStandardItem(QString(" "))
         << new QStandardItem(QString(" "))
         << new QStandardItem(QString(" "))
         << new QStandardItem(QString(" "));


    for(int i=0;i<13;i++){
        task[i]->setData(row,Qt::UserRole+1);
    }

    curItem->appendRow(task);
}





void MainWindow::on_del_pushButton_clicked()
{
    QModelIndex curIndex = ui->treeView->currentIndex();
    int row = curIndex.row();
    int column = curIndex.column();

    //当前行列值包含-1值或当前节点非顶级节点时返回
    if( -1 == row || -1 == column)
    {
        return;
    }

    int parentRow = curIndex.data(Qt::UserRole + 1).toInt();
    //判断顶级节点值选择相应的移除操作
    if(-1 == parentRow)
    {
        m_model->removeRow(row);
    }
    else
    {
        //移除某个子节点需要找到其顶级节点
        QStandardItem *parentItem = m_model->item(parentRow);
        parentItem->removeRow(row);
    }
}


void MainWindow::readData()
{

}

