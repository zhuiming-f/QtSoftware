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

    readData();
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


void MainWindow::on_processInfo_pushButton_clicked()
{
    //task->exec();
}
/*
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
*/


void MainWindow::readData()
{
    QStandardItemModel *model=new QStandardItemModel();
    ui->treeView->setModel(model);
    model->setHorizontalHeaderLabels(QStringList()<<""<<"计划编号"<<"起始时刻"<<"终止时刻"<<"时间码类型"<<"指向1"<<"指向2"<<"指向3"<<"指向类型"<<"拍摄帧数"<<"曝光时间"<<"帧间间隔"<<"状态");
#if 1
    QSqlQuery query;
    query.exec("SELECT * FROM tasklist ORDER BY 所属任务集, 起始时刻");

    QList<QStandardItem*> taskGroup; //局部变量taskGroup会自动被销毁  QList类对象在销毁时会自动调用其中每个元素的析构函数 从而释放它们所占用的内存 无须手动释放
    int currentID=-1,count=1;
    while(query.next()){
        //QString taskName=query.value(0).toString();
        int groupID=query.value(1).toInt();

        if(groupID!=currentID){
            count=1;
            currentID=groupID;
            taskGroup.clear();
            taskGroup<<new QStandardItem(QString("任务集%1").arg(currentID));
            for(int i=0;i<12;i++) taskGroup<<new QStandardItem(QString(""));
            for(int i=0;i<13;++i) taskGroup[i]->setFlags(Qt::ItemIsEnabled);
            model->appendRow(taskGroup);
        }

        QModelIndex groupIndex=model->indexFromItem(taskGroup[0]);
        QStandardItem *parentItem=model->itemFromIndex(groupIndex);
        QList<QStandardItem*> rowItems;
        rowItems<<new QStandardItem(QString("任务%1").arg(count++));
        for(int i=0;i<13;i++){
            if(i==1) continue;

            QString strValue=query.isNull(i)?"":query.value(i).toString(); //先判断是否为空
            //if(i==7) qDebug()<<strValue<<Qt::endl;
            if(i==12){
                int value=query.value(i).toInt();

                if(value==1) strValue="已完成";
                else if(value==0) strValue="未完成";
                else if(value==2) strValue="处理中";
                else strValue="有冲突";
            }
            rowItems<<new QStandardItem(strValue);
        }
        parentItem->appendRow(rowItems);
    }
#endif
}

