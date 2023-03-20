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
    //task=new tasklist(); //创建进程信息子界面
    model=new QStandardItemModel();

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


void MainWindow::readData()
{
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
            usedIDs.insert(currentID);
            for(int i=0;i<12;i++) taskGroup<<new QStandardItem(QString(""));
            for(int i=0;i<13;++i) taskGroup[i]->setFlags(taskGroup[0]->flags()&~(Qt::ItemIsEditable));
            model->appendRow(taskGroup);
        }
        //通过名称获取在model中的索引
        QModelIndex groupIndex=model->indexFromItem(taskGroup[0]);
        //获取该任务集的指针
        QStandardItem *parentItem=model->itemFromIndex(groupIndex);
        QList<QStandardItem*> rowItems;
        rowItems<<new QStandardItem(QString("任务%1").arg(count++));
        //任务添加在对应任务集下
        for(int i=0;i<13;i++){
            if(i==1) continue;
            QString strValue=query.isNull(i)?"":query.value(i).toString(); //先判断是否为空
            if(i==2||i==3){
                QDateTime dateTime=QVariant::fromValue(strValue).toDateTime();
                strValue=dateTime.toString("yyyy-MM-dd HH:mm:ss");  //大写HH表示24小时制度 hh表示12小时制
            }

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
        /*结构
        *model  根
         **parentItem  父
           ***rowItems  子
        */
    }
#endif

    connect(ui->treeView,&QTreeView::clicked,this,[=](const QModelIndex &index){
        //获取所选中行的第i行的数据信息
        for(int i=1;i<13;++i){
            if(i==2||i==3){
                QString dateTimeEditName=QString("dateTimeEdit_%1").arg(i);
                QDateTimeEdit* dateTimeEdit=this->findChild<QDateTimeEdit*>(dateTimeEditName);
                if(dateTimeEdit){
                    QDateTime dateTime=index.sibling(index.row(),i).data().toDateTime();
                    //qDebug()<<dateTime;
                    if(dateTime.isValid()) dateTimeEdit->setDateTime(index.sibling(index.row(),i).data().toDateTime());
                    else dateTimeEdit->setDateTime(QDateTime::currentDateTime());
                }
            }
            else{
                QString lineEditName=QString("lineEdit_%1").arg(i);
                QLineEdit* lineEdit=this->findChild<QLineEdit*>(lineEditName);
                //选择任务集行 判空
                if(lineEdit) lineEdit->setText(index.sibling(index.row(),i).data().toString());
            }
        }
    });

#if 1
    connect(ui->dateTimeEdit_2,&QDateTimeEdit::dateTimeChanged,this,[=](const QDateTime &datetime){
        ui->dateTimeEdit_3->setMinimumDateTime(datetime); //设置最小时间
        if(ui->dateTimeEdit_3->dateTime()<datetime){
            ui->dateTimeEdit_3->setDateTime(datetime);
        }
    });
#endif
}

void MainWindow::on_update_pushButton_clicked()
{
    QSqlQuery query;
    QString sql;

    QString column_1=ui->treeView->model()->headerData(1,Qt::Horizontal,Qt::DisplayRole).toString();
    QString column_2=ui->treeView->model()->headerData(2,Qt::Horizontal,Qt::DisplayRole).toString();
    QString column_3=ui->treeView->model()->headerData(3,Qt::Horizontal,Qt::DisplayRole).toString();
    QString column_4=ui->treeView->model()->headerData(4,Qt::Horizontal,Qt::DisplayRole).toString();
    QString column_5=ui->treeView->model()->headerData(5,Qt::Horizontal,Qt::DisplayRole).toString();
    QString column_6=ui->treeView->model()->headerData(6,Qt::Horizontal,Qt::DisplayRole).toString();
    QString column_7=ui->treeView->model()->headerData(7,Qt::Horizontal,Qt::DisplayRole).toString();
    QString column_8=ui->treeView->model()->headerData(8,Qt::Horizontal,Qt::DisplayRole).toString();
    QString column_9=ui->treeView->model()->headerData(9,Qt::Horizontal,Qt::DisplayRole).toString();
    QString column_10=ui->treeView->model()->headerData(10,Qt::Horizontal,Qt::DisplayRole).toString();
    QString column_11=ui->treeView->model()->headerData(11,Qt::Horizontal,Qt::DisplayRole).toString();

    QString planID=ui->lineEdit_1->text();
    QString start=ui->dateTimeEdit_2->dateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString end=ui->dateTimeEdit_3->dateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString timeType=ui->lineEdit_4->text().trimmed();
    QString directType=ui->lineEdit_8->text().trimmed();

    double direct_1=ui->lineEdit_5->text().toDouble();
    double direct_2=ui->lineEdit_6->text().toDouble();
    double exposure=ui->lineEdit_10->text().toDouble();
    double frameInterval=ui->lineEdit_11->text().toDouble();

    int frames=ui->lineEdit_9->text().toInt();
    int direct_3=ui->lineEdit_7->text().toInt();

    sql=QString("UPDATE tasklist SET %1='%2', %3='%4', %5='%6', %7=%8, %9=%10,%11=%12,%13='%14',%15=%16,%17=%18,%19=%20 WHERE %21='%22'")
            .arg(column_2).arg(start)
            .arg(column_3).arg(end)
            .arg(column_4).arg(timeType)
            .arg(column_5).arg(direct_1)
            .arg(column_6).arg(direct_2)
            .arg(column_7).arg(direct_3)
            .arg(column_8).arg(directType)
            .arg(column_9).arg(frames)
            .arg(column_10).arg(exposure)
            .arg(column_11).arg(frameInterval)
            .arg(column_1).arg(planID);

    if(!query.exec(sql)){
        qDebug()<<"fail to update: "<<query.lastError().text();
    }
    else{
        qDebug()<<"update succeeded";
    }
}

void MainWindow::on_addTaskGroup_pushButton_clicked()
{
    int currentID=1;
    while(usedIDs.contains(currentID)){ //找到最小未使用编号
        currentID++;
    }

    QList<QStandardItem*> taskGroup;
    taskGroup<<new QStandardItem(QString("任务集%1").arg(currentID));
    usedIDs.insert(currentID);
    for(int i=0;i<12;i++) taskGroup<<new QStandardItem(QString(""));
    for(int i=0;i<13;++i) taskGroup[i]->setFlags(taskGroup[0]->flags()&~(Qt::ItemIsEditable));

    //将任务集添加到模型中
    model->appendRow(taskGroup);

}

void MainWindow::on_del_pushButton_clicked()
{
    //获取当前选择行
    //index是指向特定单元格的QModelIndex对象  包含单元格的行号、列号等信息
    QModelIndex index=ui->treeView->currentIndex();
    if(index.isValid()){
        QStandardItem *item=model->itemFromIndex(index);
        if(item->parent()==nullptr){ //如果item是任务集
            if(item->rowCount()>0){  //任务集非空
                QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("删除任务集"), tr("该任务集下有任务，是否全部删除？"), QMessageBox::Yes | QMessageBox::No);
                if(ret==QMessageBox::Yes){
                    model->removeRow(index.row()); //删除该行
                    QString taskGroupName = index.sibling(index.row(), 0).data().toString(); // 获取任务集名称
                    QString num;
                    for(int i=4;i<taskGroupName.length();++i){
                        num+=taskGroupName[i];
                    }
                    usedIDs.remove(num.toInt());
                }
            }
            else{  //空任务集
                model->removeRow(index.row());
                QString taskGroupName = index.sibling(index.row(), 0).data().toString(); // 获取任务集名称
                QString num;
                for(int i=4;i<taskGroupName.length();++i){
                    num+=taskGroupName[i];
                }

                usedIDs.remove(num.toInt()); //从集合中移除
            }
        }
        else{  //如果item是任务     获取父节点索引
            model->removeRow(index.row(),index.parent());  //删除该任务
        }
    }
}

























