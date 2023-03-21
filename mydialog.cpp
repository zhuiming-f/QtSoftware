#include "mydialog.h"
#include "ui_mydialog.h"

MyDialog::MyDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MyDialog)
{
    ui->setupUi(this);

    //ui->dateTimeEdit_1->setDateTime(QDateTime::currentDateTime());
    //ui->dateTimeEdit_2->setDateTime(QDateTime::currentDateTime());

    connect(ui->dateTimeEdit_1,&QDateTimeEdit::dateTimeChanged,this,[=](const QDateTime &datetime){
        ui->dateTimeEdit_2->setMinimumDateTime(datetime); //设置最小时间
        if(ui->dateTimeEdit_2->dateTime()<datetime){
            ui->dateTimeEdit_2->setDateTime(datetime);
        }
    });
    connect(ui->confirm_pushButton,&QPushButton::clicked,this,&MyDialog::accept);
    connect(ui->cancel_pushButton,&QPushButton::clicked,this,&MyDialog::reject);
}

MyDialog::~MyDialog()
{
    delete ui;
}

QDateTime MyDialog::getStartTime()
{
    return ui->dateTimeEdit_1->dateTime();
}

QDateTime MyDialog::getEndTime()
{
    return ui->dateTimeEdit_2->dateTime();
}

void MyDialog::setStartTime()
{
    ui->dateTimeEdit_1->setDateTime(QDateTime::currentDateTime());
}

void MyDialog::setEndTime()
{
    ui->dateTimeEdit_2->setDateTime(QDateTime::currentDateTime());
}


