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
    task=new tasklist(); //创建进程信息子界面
    model=new QStandardItemModel();
    dialog=new MyDialog();


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

    ui->treeView->setModel(model);
    model->setHorizontalHeaderLabels(QStringList()<<""<<"计划编号"<<"起始时刻"<<"终止时刻"<<"时间码类型"<<"指向1"<<"指向2"<<"指向3"<<"指向类型"<<"拍摄帧数"<<"曝光时间"<<"帧间间隔"<<"状态");

    readData();
    /*结构
    *model  根
     **parentItem  父
       ***rowItems  子
    */

    //treeView聚焦行信息显示在右侧文本框
    connect(ui->treeView,&QTreeView::clicked,this,[=](const QModelIndex &index){
        //获取所选中行的第i行的数据信息
        for(int i=1;i<13;++i){
            if(i==2||i==3){
                QString dateTimeEditName=QString("dateTimeEdit_%1").arg(i);
                QDateTimeEdit* dateTimeEdit=this->findChild<QDateTimeEdit*>(dateTimeEditName);
                if(dateTimeEdit){
                    QDateTime dateTime=index.sibling(index.row(),i).data().toDateTime();
                    //qDebug()<<dateTime;
                    dateTimeEdit->setDateTime(index.sibling(index.row(),i).data().toDateTime());
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

    //处理两个QDateTimeEdit编辑框的逻辑
    connect(ui->dateTimeEdit_2,&QDateTimeEdit::dateTimeChanged,this,[=](const QDateTime &datetime){
        ui->dateTimeEdit_3->setMinimumDateTime(datetime); //设置最小时间
        if(ui->dateTimeEdit_3->dateTime()<datetime){
            ui->dateTimeEdit_3->setDateTime(datetime);
        }
    });

    //处理计划编号lineEdit_1文本框内容随dateTimeEdit_2的修改而改变的逻辑
    connect(ui->dateTimeEdit_2,&QDateTimeEdit::dateTimeChanged,this,[=](const QDateTime &datetime){
        QDate date=datetime.date();
        int num=readLogFile("D:/ALLFiles/QtFiles/front/planID.log",date);
        QString planID=QString("%1_%2").arg(date.toString("yyyy-MM-dd")).arg(QString::number(num));
        ui->lineEdit_1->setText(planID);
    });

}

MainWindow::~MainWindow()
{
    if(db.isOpen()) db.close();
    QSqlDatabase::removeDatabase(db.connectionName());

    //task将当前窗口作为其父对象 在父对象ui被销毁时也会析构掉
    delete task;
    delete dialog;
    delete model;
    delete ui;
}


void MainWindow::refreshTaskGroup()
{

}

void MainWindow::refreshTaskID()
{
    vec.clear();
    QSqlQuery query;
    query.exec("SELECT 所属任务集,起始时刻 FROM tasklist ORDER BY 所属任务集, 起始时刻");

    int currentID=-1;
    //把每个任务集中开始最早的任务的时间作为任务集排序的依据
    while(query.next()){
        int groupID=query.value(0).toInt();

        if(groupID!=currentID){
            QDateTime dateTime=query.value(1).toDateTime();
            currentID=groupID;
            vec.emplace_back(dateTime,currentID);
        }
    }
    //任务集编号重新洗牌
    std::sort(vec.begin(),vec.end());

    QSqlQuery query1;

    for(int i=0;i<int(vec.size());++i){
        qDebug()<<vec[i].second;
        if(!query1.exec(QString("SELECT 计划编号 FROM tasklist WHERE 所属任务集 = %1").arg(vec[i].second))){
            qDebug()<<"select failed: "<<query.lastError().text();
        }
        else{
            qDebug()<<"select succeeded";
        }

        while(query1.next()){
            QString planID=query1.value(0).toString();
            query.exec(QString("UPDATE tasklist SET 所属任务集 = %1 WHERE 计划编号 = '%2'").arg(i+1).arg(planID));
        }
    }
}

void MainWindow::on_processInfo_pushButton_clicked()
{
    task->exec();
}

void MainWindow::readData()
{
    //更新之前的任务集个数
    int size1=model->rowCount();
    usedIDs.clear();
    //删除模型上添加的所有任务集
    model->removeRows(0,model->rowCount());
    QSqlQuery query;
    query.exec("SELECT * FROM tasklist ORDER BY 所属任务集, 起始时刻");

    QList<QStandardItem*> taskGroup; //局部变量taskGroup会自动被销毁  QList类对象在销毁时会自动调用其中每个元素的析构函数 从而释放它们所占用的内存 无须手动释放
    int currentID=-1,count=1;
    //数据库中任务集个数
    int size2=0;
    while(query.next()){
        int groupID=query.value(1).toInt();

        if(groupID!=currentID){
            count=1;
            currentID=groupID;
            size2++;
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
    }

    for(int j=size2+1;j<=size1;++j){
        taskGroup.clear();
        taskGroup<<new QStandardItem(QString("任务集%1").arg(j));
        usedIDs.insert(j);
        for(int i=0;i<12;i++) taskGroup<<new QStandardItem(QString(""));
        for(int i=0;i<13;++i) taskGroup[i]->setFlags(taskGroup[0]->flags()&~(Qt::ItemIsEditable));
        model->appendRow(taskGroup);
    }
}

void MainWindow::on_update_pushButton_clicked()
{
    QSqlQuery query;
    QString sql;

    //字段名
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


    //编辑框的值
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

    //更新后的任务可能会改变各任务集的次序

}

void MainWindow::on_addTaskGroup_pushButton_clicked()
{
    int currentID=1;
    while(usedIDs.contains(currentID)){ //找到最小未使用编号
        currentID++;
        //qDebug()<<currentID;
    }

    usedIDs.insert(currentID);
    QList<QStandardItem*> taskGroup;
    taskGroup<<new QStandardItem(QString("任务集%1").arg(currentID));
    for(int i=0;i<12;i++) taskGroup<<new QStandardItem(QString(""));
    for(int i=0;i<13;++i) taskGroup[i]->setFlags(taskGroup[0]->flags()&~(Qt::ItemIsEditable));

    //将任务集添加到模型中
    model->appendRow(taskGroup);

    QMessageBox::warning(this,tr("警告"),tr("如不给空任务集添加任务,程序退出后不会保存该任务集!"));
}

void MainWindow::on_addTask_pushButton_clicked()
{

    QModelIndex index=ui->treeView->currentIndex();
    QStandardItem *item=model->itemFromIndex(index);
    QSqlQuery query;
    if(item->parent()==nullptr){  //任务集才可以添加任务
        dialog->setStartTime();
        dialog->setEndTime();
        if(dialog->exec()==QDialog::Accepted){
            QString sql=QString("SELECT 计划编号,起始时刻,终止时刻 FROM tasklist ORDER BY 起始时刻 DESC");

            QDateTime start=dialog->getStartTime();
            QDateTime end=dialog->getEndTime();

            if(!query.exec(sql)){
                qDebug()<<"select failed: "<<query.lastError().text();
            }
            else{
                qDebug()<<"select succeeded";
            }

            bool flag=false;
            QString planID;
            while(query.next()){
                QDateTime startTime=query.value(1).toDateTime();
                QDateTime endTime=query.value(2).toDateTime();
                if(endTime<start){
                    flag=true;
                    break;  //无冲突
                }
                else{
                    if(end<startTime){ //与当前这个任务无交集 下一个
                        continue;
                    }
                    else{  //与当前任务有交集
                        planID=query.value(0).toString();
                        break;
                    }
                }
            }
            if(flag){
                //任务所要插入的任务集名
                QString taskGroupName=index.sibling(index.row(),0).data().toString();
                //待插入任务的日期
                QString date=start.date().toString("yyyy-MM-dd");
                //构造模糊查询模式
                QString pattern=QString("%1%").arg(date);
                QString sql=QString("SELECT 计划编号 FROM tasklist WHERE 计划编号 LIKE '%1' ORDER BY 计划编号 DESC").arg(pattern);
                if(!query.exec(sql)){
                    qDebug()<<"select failed: "<<query.lastError().text();
                }
                else{
                    qDebug()<<"select succeeded";
                }
                QString maxID=query.value(0).toString();
                QString insertID;
                for(int i=11;i<maxID.length();++i){
                    insertID+=maxID[i];
                }
                int num=insertID.toInt()+1;
                //待插入任务的数字
                insertID=QString::number(num);
                //待插入计划编号
                QString insertName=QString("%1_%2").arg(date).arg(insertID);

                //获取任务集数字
                QString taskGroupNum;
                for(int i=3;i<taskGroupName.length();++i){
                    taskGroupNum+=taskGroupName[i];
                }

                sql=QString("insert into tasklist(计划编号,所属任务集,起始时刻,终止时刻) values('%1','%2','%3')").arg(insertName).arg(taskGroupNum.toInt()).arg(start.toString("yyyy-MM-dd HH:mm:ss")).arg(end.toString("yyyy-MM-dd HH:mm:ss"));
                if(!query.exec(sql)){
                    qDebug()<<"insert failed: "<<query.lastError().text();
                }
                else{
                    qDebug()<<"insert succeeded";
                }
            }
            else{
                QString text=QString("与计划编号为: %1 的任务有冲突,添加失败").arg(planID);
                /*QMessageBox::StandardButton ret=*/
                QMessageBox::critical(this, tr("错误"), tr(text.toUtf8().constData()));
            }
        }
    }
    else{
        QMessageBox::warning(this, tr("警告"), tr("任务集下才能添加任务"));
    }
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
                    //删除放在后面 不然先删除该索引不在指向所要删除的任务集
                    QString taskGroupName = index.sibling(index.row(), 0).data().toString(); // 获取任务集名称
                    QString num;
                    for(int i=3;i<taskGroupName.length();++i){
                        num+=taskGroupName[i];
                    }
                    model->removeRow(index.row()); //删除该行
                    usedIDs.remove(num.toInt());
                }
                else{
                    return; //终止事件
                }
            }
            else{  //空任务集

                QString taskGroupName = index.sibling(index.row(), 0).data().toString(); // 获取任务集名称
                QString num;
                for(int i=4;i<taskGroupName.length();++i){
                    num+=taskGroupName[i];
                }
                model->removeRow(index.row());
                usedIDs.remove(num.toInt()); //从集合中移除
            }
        }
        else{  //如果item是任务     获取父节点索引
            QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("删除任务"), tr("确认删除该任务吗？"), QMessageBox::Yes | QMessageBox::No);
            if(ret==QMessageBox::Yes){
                model->removeRow(index.row(),index.parent());  //删除该任务
            }
            else{
                return;
            }
        }
        //refreshTaskID();
        //readData();
    }
}

























