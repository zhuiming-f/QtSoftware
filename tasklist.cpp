#include "tasklist.h"
#include "ui_tasklist.h"

#include<QTimer>
#include<QProcess>
#include<QSqlDatabase>

tasklist::tasklist(QDialog *parent) :
    QDialog(parent),
    ui(new Ui::tasklist)
{
    ui->setupUi(this);

    m_timer=new QTimer();
    connect(m_timer,&QTimer::timeout,this,&tasklist::on_refresh_pushButton_clicked);
    m_timer->start(3000);

}

tasklist::~tasklist()
{

    delete m_timer;
    delete ui;
}

void tasklist::slotProcessInfo(const QString &processName)
{
    // /FI指定筛选条件 IMAGENAME eq以进程名精准匹配  /FO指定输出格式 CSV以逗号分隔输出  /NH不输出列标题
    //QString command = "tasklist /FI \"IMAGENAME eq " + processName + "\" /FO CSV /NH"; //命令行指令
    QStringList arguments = {"/FI", "IMAGENAME eq " + processName, "/FO", "CSV", "/NH"};
    QString program = "tasklist";
    QProcess process;
    process.start(program,arguments); //启动命令行
    process.waitForFinished(-1); //等待进程完成

    QString output = process.readAll(); //获取输出信息
    QStringList lines = output.split('\n'); //将输出信息按行分割

    ui->listWidget_process->addItem( QString::fromUtf8("名称") + "\t\t" +
                                     "PID\t" +
                                     //QString::fromUtf8("状态") + "\t" +
                                     QString::fromUtf8("占用内存")+"\t"+
                                     QString::fromUtf8("处理文件夹"));

    for(const QString &line: lines){
        if(line.trimmed().isEmpty()) continue;

        QStringList values=line.split(',');
        QString name=values[0].remove('"');
        QString pid=values[1].remove('"');
        //QString status=values[4].remove('"');
        QString mem=values[5].remove('"').replace('\r',"");
        //QString path="";
        ui->listWidget_process->addItem(name+"\t"+pid+"\t"+mem/*+"\t"+path*/);
    }
}

void tasklist::on_refresh_pushButton_clicked()
{
    ui->listWidget_process->clear();  //清空ListWidget
    slotProcessInfo("chrome.exe");
}



