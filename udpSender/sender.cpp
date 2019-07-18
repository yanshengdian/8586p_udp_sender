#include "sender.h"
#include "ui_sender.h"
#include <QtNetwork>
#include <QMessageBox>

//ksd8586p on line  启动时  第一条

sender::sender(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::sender)
{
    ui->setupUi(this);
   // setWindowTitle(tr("卡萨帝 wifi 控制台"));
    ui->label_device_on_line->setText(tr("No Device"));
    setMaximumHeight(280);
    setMinimumHeight(280);

    /*
background-color_ng: rgb(116, 116, 116);
background-color_ok: rgb(0, 255, 127);*/


    find_client_timer = new QTimer(this);
    is_client_online = 0;
    ////////////////////////
    QString localhostname = QHostInfo::localHostName();
    QHostInfo localhostinfo = QHostInfo::fromName(localhostname);
    foreach(QHostAddress address, localhostinfo.addresses()){
        if(address.protocol() == QAbstractSocket::IPv4Protocol){
            ui->comboBox->insertItem(0,address.toString());
        }
    }
    ///////////////////////
    _receiver = new QUdpSocket(this);
    int port = 8266;
    _receiver->bind(port,QUdpSocket::ShareAddress|QUdpSocket::ReuseAddressHint);
    connect(_receiver,SIGNAL(readyRead()),this,SLOT(process_pending_datagram()));
    connect(find_client_timer,SIGNAL(timeout()),this,SLOT(timer_time_up()));
    ui->auto_enter_checkBox->setCheckState(Qt::Checked);
}

sender::~sender()
{
    delete ui;
}

void sender::on_sendButton_released()
{
    QByteArray datagram;
    QString temp_string;
    if(ui->hex_checkBox->isChecked()){
        temp_string=ui->sendTextEdit->toPlainText().toAscii();
        datagram = temp_string.toAscii();
        datagram = datagram.toHex();
        QMessageBox::information(0,"info","sengding Hex",QMessageBox::Ok | QMessageBox::Default ,0);
    }
    else{
        temp_string=ui->sendTextEdit->toPlainText();
        datagram=temp_string.toUtf8();
    }

    if(ui->auto_enter_checkBox->isChecked()){
        datagram.append(0x0a);
    }

    QString add_to_use;
    QString send_to_add;
    _sender = new QUdpSocket(this);
    add_to_use = ui->comboBox->currentText();
    send_to_add = ui->lineEdit->text();
    if(QHostAddress(send_to_add).protocol()==QAbstractSocket::IPv4Protocol){
        _sender->bind(QHostAddress(add_to_use),8265,QUdpSocket::ShareAddress);
        //_sender->writeDatagram(datagram.data(),datagram.size(),QHostAddress::Broadcast,8266);
        _sender->writeDatagram(datagram.data(),datagram.size(),QHostAddress(send_to_add),8266);
    }
    else{
       QMessageBox::critical(0 ,"error" , "Wrong ipaddress",QMessageBox::Ok | QMessageBox::Default ,0 );
    }
        delete _sender;
}


void sender::process_pending_datagram()
{
    QHostAddress senders_address;
    quint16 senders_port;
    while(_receiver->hasPendingDatagrams()){
        QByteArray datagram;
        datagram.resize(_receiver->pendingDatagramSize());
        _receiver->readDatagram(datagram.data(),datagram.size(),&senders_address,&senders_port);
        ui->textBrowser->append(senders_address.toString());
        ui->textBrowser->append(tr("%1").arg(senders_port));
        ui->textBrowser->append(tr("size: %1").arg(datagram.size()));
        ui->textBrowser->append(datagram.data());
        if(! is_client_online){     //如果设备不在线
            if(datagram == "ksd8586p on line"){
                ui->textBrowser->append("get ksd8586p on line content");
                is_client_online =1;
                find_client_timer->stop();
                ui->lineEdit->setText(senders_address.toString());
                ui->pushButton_find_client->setEnabled(1);
                ui->label_device_on_line->setText("_____KSD8586P Online!_____");
                setMaximumHeight(550);
                setMinimumHeight(550);

            }else{
                ui->textBrowser->append("not ksd8586p on line content");
            }
        }
        else{                       //这就有设备在线
            //.
            QByteArray cmd_part;
            QByteArray data_part;
            //QByteArray ok_part;
            cmd_part = datagram.left(2);
            data_part=datagram.mid(2,3);
            if(cmd_part=="rr"){
                ui->textBrowser->append("rrOK!");
                ui->textBrowser->append(data_part.data());
                ui->lineEdit_fan_current_temp->setText(data_part);
            }

            if(cmd_part=="rs"){
                ui->textBrowser->append("rsOK!");
                ui->textBrowser->append(data_part.data());
                //ui->lineEdit_fan_current_temp->setText(data_part);
                ui->lineEdit_fan_set_temp->setText(data_part);
            }

            if(cmd_part=="rx"){
                ui->textBrowser->append("rxOK!");
                ui->textBrowser->append(data_part.data());
                //ui->lineEdit_fan_current_temp->setText(data_part);
                ui->lineEdit_fan_speed->setText(data_part);
            }

            if(cmd_part=="rt"){
                ui->textBrowser->append("rtOK!");
                ui->textBrowser->append(data_part.data());
                //ui->lineEdit_fan_current_temp->setText(data_part);
                ui->lineEdit_solder_current_temp->setText(data_part);
            }

            if(cmd_part=="ru"){
                ui->textBrowser->append("ruOK!");
                ui->textBrowser->append(data_part.data());
                //ui->lineEdit_fan_current_temp->setText(data_part);
                ui->lineEdit_solder_set_temp->setText(data_part);
            }

            if(cmd_part=="su"){
                ui->textBrowser->append("suOK!");
                ui->textBrowser->append(data_part.data());
                //ui->lineEdit_fan_current_temp->setText(data_part);
                ui->lineEdit_fan_on_off_state->setText(data_part);
            }

            if(cmd_part=="sk"){
                ui->textBrowser->append("skOK!");
                ui->textBrowser->append(data_part.data());
                //ui->lineEdit_fan_current_temp->setText(data_part);
                ui->lineEdit_solder_on_off_state->setText(data_part);
            }

            if(cmd_part=="sv"){
                ui->textBrowser->append("svOK!");
                ui->textBrowser->append(data_part.data());
                //ui->lineEdit_fan_current_temp->setText(data_part);
                ui->lineEdit_fan_on_off_state->setText(data_part);
            }

            if(cmd_part=="so"){
                ui->textBrowser->append("soOK!");
                ui->textBrowser->append(data_part.data());
                //ui->lineEdit_fan_current_temp->setText(data_part);
                ui->lineEdit_solder_on_off_state->setText(data_part);
            }

            if(cmd_part=="af"){
                ui->textBrowser->append("afOK!");
                ui->textBrowser->append(data_part.data());
                //ui->lineEdit_fan_current_temp->setText(data_part);
                //ui->lineEdit_solder_on_off_state->setText(data_part);
                ui->lineEdit_fan_cel->setText(data_part);
            }

            if(cmd_part=="pu"){
                ui->textBrowser->append("puOK!");
                ui->textBrowser->append(data_part.data());
                //ui->lineEdit_fan_current_temp->setText(data_part);
                //ui->lineEdit_solder_on_off_state->setText(data_part);
                ui->lineEdit_fan_on_off_state->setText(data_part);
            }
//as
            if(cmd_part=="as"){
                ui->textBrowser->append("asOK!");
                ui->textBrowser->append(data_part.data());
                //ui->lineEdit_fan_current_temp->setText(data_part);
                ui->lineEdit_solder_cal->setText(data_part);
            }


            if(cmd_part=="pk"){
                ui->textBrowser->append("pkOK!");
                ui->textBrowser->append(data_part.data());
                //ui->lineEdit_fan_current_temp->setText(data_part);
                ui->lineEdit_solder_on_off_state->setText(data_part);
            }
        }
    }
}

void sender::on_clearButton_clicked()
{
    ui->textBrowser->clear();
    ui->sendTextEdit->clear();
}

void sender::on_pushButton_find_client_clicked()
{
    if(!is_client_online){
        find_client_timer->start(5000);
    }
    ui->pushButton_find_client->setEnabled(0);
}
void sender::timer_time_up()
{
    QByteArray bc_ip;
    QString add_to_use;
    find_client_timer_counter ++;
    if(find_client_timer_counter==60){
        find_client_timer_counter=0;
        is_client_online =1;
    }
    _sender = new QUdpSocket(this);
    add_to_use = ui->comboBox->currentText();
    //send_to_add = ui->lineEdit->text();
    bc_ip=add_to_use.toUtf8();
    bc_ip.append(0x0a);

    _sender->bind(QHostAddress(add_to_use),8265,QUdpSocket::ShareAddress);
    //_sender->writeDatagram(bc_ip.data(),bc_ip.size(),QHostAddress(send_to_add),8266);
    _sender->writeDatagram(bc_ip.data(),bc_ip.size(),QHostAddress::Broadcast,8266);


     delete _sender;
}

void sender::on_pushButton_read_fan_set_temp_clicked()
{
    cmd_datagram.resize(0);;
    cmd_datagram.append("rs___");
    cmd_datagram.append(0x0a);
    send_cmd();
}

void sender::on_pushButton_read_fan_current_temp_clicked()
{

    cmd_datagram.resize(0);;
    cmd_datagram.append("rr___");
    cmd_datagram.append(0x0a);
    send_cmd();
}
void sender::send_cmd()
{
    QString add_to_use;
    QString send_to_add;
    _sender = new QUdpSocket(this);
    add_to_use = ui->comboBox->currentText();
    send_to_add = ui->lineEdit->text();
    _sender->bind(QHostAddress(add_to_use),8265,QUdpSocket::ShareAddress);
    _sender->writeDatagram(cmd_datagram.data(),cmd_datagram.size(),QHostAddress(send_to_add),8266);
    delete _sender;
    resend_cmd_timer = new QTimer(this);
    resend_cmd_timer->start(3000);
    resend_cmd_timer_timeout_counter=0;
}


void sender::resend_cmd()
{
    QString add_to_use;
    QString send_to_add;
    _sender = new QUdpSocket(this);
    add_to_use = ui->comboBox->currentText();
    send_to_add = ui->lineEdit->text();
    _sender->bind(QHostAddress(add_to_use),8265,QUdpSocket::ShareAddress);
    _sender->writeDatagram(cmd_datagram.data(),cmd_datagram.size(),QHostAddress(send_to_add),8266);
    delete _sender;
    resend_cmd_timer = new QTimer(this);
    resend_cmd_timer->start(3000);
    resend_cmd_timer_timeout_counter=0;
}


void sender::on_pushButton_read_fan_speed_clicked()
{
    cmd_datagram.resize(0);;
    cmd_datagram.append("rx___");
    cmd_datagram.append(0x0a);
    send_cmd();
}

void sender::on_pushButton_read_solder_set_temp_clicked()
{
    cmd_datagram.resize(0);;
    cmd_datagram.append("ru___");
    cmd_datagram.append(0x0a);
    send_cmd();
}

void sender::on_pushButton_read_solder_current_temp_clicked()
{
    cmd_datagram.resize(0);;
    cmd_datagram.append("rt___");
    cmd_datagram.append(0x0a);
    send_cmd();
}

void sender::on_pushButton_read_fan_cal_clicked()
{
    cmd_datagram.resize(0);;
    cmd_datagram.append("af___");
    cmd_datagram.append(0x0a);
    send_cmd();
}

void sender::on_pushButton_read_solder_cal_clicked()
{
    cmd_datagram.resize(0);;
    cmd_datagram.append("as___");
    cmd_datagram.append(0x0a);
    send_cmd();
}

void sender::on_pushButton_read_fan_on_off_state_clicked()
{
    cmd_datagram.resize(0);;
    cmd_datagram.append("pu___");
    cmd_datagram.append(0x0a);
    send_cmd();
}

void sender::on_pushButton_read_solder_on_off_state_clicked()
{
    cmd_datagram.resize(0);;
    cmd_datagram.append("pk___");
    cmd_datagram.append(0x0a);
    send_cmd();
}

void sender::on_pushButton_write_fan_set_temp_clicked()
{
    cmd_datagram.resize(0);;
    cmd_datagram.append("sf");
    quint16 temp_value;
    temp_value=ui->lineEdit_fan_set_temp->text().toInt();
    if((temp_value>500)|(temp_value<100)){
        QMessageBox::information(0,"info","between 100~500",QMessageBox::Ok | QMessageBox::Default ,0);
    }else{
        cmd_datagram.append(ui->lineEdit_fan_set_temp->text());
        cmd_datagram.append(0x0a);
        send_cmd();
    }
}

void sender::on_pushButton_write_solder_set_temp_clicked()
{
    cmd_datagram.resize(0);;
    cmd_datagram.append("ss");
    quint16 temp_value;
    //temp_value=ui->lineEdit_fan_set_temp->text().toInt();
    temp_value=ui->lineEdit_solder_set_temp->text().toInt();
    if((temp_value>500)|(temp_value<200)){
        QMessageBox::information(0,"info","between 200~500",QMessageBox::Ok | QMessageBox::Default ,0);
    }else{
        cmd_datagram.append(ui->lineEdit_solder_set_temp->text());
        cmd_datagram.append(0x0a);
        send_cmd();
    }
}
