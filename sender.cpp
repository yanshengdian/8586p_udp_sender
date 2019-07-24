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
//    background-color: rgb(85, 255, 127);      绿色
//    background-color: rgb(145, 145, 145);     灰色
//    background-color: rgb(255, 0, 0);         红色
    ui->pushButton_find_client->setHidden(1);
    setMaximumHeight(250);
    setMinimumHeight(250);

    resend_cmd_timer = new QTimer(this);

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


    connect(resend_cmd_timer,SIGNAL(timeout()),this,SLOT(resend_timer_time_up()));
    ui->auto_enter_checkBox->setCheckState(Qt::Checked);
}

sender::~sender()
{
    delete ui;
}
void sender::resend_timer_time_up()
{
    send_cmd();
    resend_cmd_timer_timeout_counter++;
    if(resend_cmd_timer_timeout_counter==6){
        device_break();
        QMessageBox::information(0,"info","time out",QMessageBox::Ok | QMessageBox::Default ,0);

        resend_cmd_timer_timeout_counter=0;
        resend_cmd_timer->stop();
    }
}

void sender::on_sendButton_clicked()
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
    //send_to_add = ui->comboBox_device_address_list->currentText();

    if(QHostAddress(send_to_add).protocol()==QAbstractSocket::IPv4Protocol){
        _sender->bind(QHostAddress(add_to_use)/*,8265*/,QUdpSocket::ShareAddress);
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
    //if(! resend_cmd_timer->isActive()){
        resend_cmd_timer->stop();
        resend_cmd_timer_timeout_counter=0;
   // }
    while(_receiver->hasPendingDatagrams()){
        QByteArray datagram;
        datagram.resize(_receiver->pendingDatagramSize());
        _receiver->readDatagram(datagram.data(),datagram.size(),&senders_address,&senders_port);
//        ui->textBrowser->append(tr("address: %1").arg(senders_address.toString()));
//        ui->textBrowser->append(tr("port: %1").arg(senders_port));
//        ui->textBrowser->append(tr("size: %1").arg(datagram.size()));
//        ui->textBrowser->append(datagram.data());



        if(datagram == "ksd8586p on line"){
            for(int a=0;a<ui->comboBox_device_address_list->count();a++){
                if(senders_address.toString()==ui->comboBox_device_address_list->itemText(a)){
                    qDebug()<< "address already in list";
                    return;
                }
            }
            ui->textBrowser->append("get ksd8586p on line content");
            new_device_found();
            setMaximumHeight(520);
            setMinimumHeight(520);
            on_pushButton_read_all_clicked();
            ui->comboBox_device_address_list->insertItem(0,senders_address.toString());
            return;
        }


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
        if(cmd_part=="sf"){
            ui->textBrowser->append("sfOK!");
            ui->textBrowser->append(data_part.data());
            ui->lineEdit_fan_set_temp->setText(data_part);
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
        if(cmd_part=="ss"){
            ui->textBrowser->append("ssOK!");
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
            ui->lineEdit_fan_cel->setText(tr("%1").arg(data_part.toInt()));
        }

        if(cmd_part=="sy"){
            ui->textBrowser->append("syOK!");
            ui->textBrowser->append(data_part.data());
            ui->lineEdit_fan_cel->setText(tr("%1").arg(data_part.toInt()));
        }
        if(cmd_part=="sz"){
            ui->textBrowser->append("szOK!");
            ui->textBrowser->append(data_part.data());
            ui->lineEdit_solder_cal->setText(tr("%1").arg(data_part.toInt()));
        }

        if(cmd_part=="pu"){
            ui->textBrowser->append("puOK!");
            ui->textBrowser->append(data_part.data());
            //ui->lineEdit_fan_current_temp->setText(data_part);
            //ui->lineEdit_solder_on_off_state->setText(data_part);
            ui->lineEdit_fan_on_off_state->setText(data_part);
        }

        if(cmd_part=="as"){
            ui->textBrowser->append("asOK!");
            ui->textBrowser->append(data_part.data());
            //ui->lineEdit_fan_current_temp->setText(data_part);
            ui->lineEdit_solder_cal->setText(tr("%1").arg(data_part.toInt()));
        }


        if(cmd_part=="pk"){
            ui->textBrowser->append("pkOK!");
            ui->textBrowser->append(data_part.data());
            //ui->lineEdit_fan_current_temp->setText(data_part);
            ui->lineEdit_solder_on_off_state->setText(data_part);
        }

        if(cmd_part=="ra"){
            QByteArray all_data_part;
            all_data_part.resize(datagram.size()-2);
            ui->textBrowser->append("raOK!");

            for(int a=0;a<9;a++){
                all_data_part.insert((a*3),(datagram.mid((a*3+2),3)));
                ui->textBrowser->append(all_data_part.mid(a*3,3));
            }

            ui->lineEdit_fan_cel->setText(tr("%1").arg(all_data_part.mid(5*3,3).toInt()));
            ui->lineEdit_fan_current_temp->setText(all_data_part.mid(0*3,3));
            ui->lineEdit_fan_on_off_state->setText(all_data_part.mid(7*3,3));
            ui->lineEdit_fan_set_temp->setText(all_data_part.mid(1*3,3));
            ui->lineEdit_fan_speed->setText(all_data_part.mid(4*3,3));
            ui->lineEdit_solder_cal->setText(tr("%1").arg(all_data_part.mid(6*3,3).toInt()));
            ui->lineEdit_solder_current_temp->setText(all_data_part.mid(2*3,3));
            ui->lineEdit_solder_on_off_state->setText(all_data_part.mid(8*3,3));
            ui->lineEdit_solder_set_temp->setText(all_data_part.mid(3*3,3));
            new_device_found();

        }
        if(cmd_part=="wa"){
            QByteArray all_data_part;
            all_data_part.resize(datagram.size()-2);
            ui->textBrowser->append("waOK!");

            for(int a=0;a<7;a++){
                all_data_part.insert((a*3),(datagram.mid((a*3+2),3)));
                ui->textBrowser->append(all_data_part.mid(a*3,3));
            }
            ui->lineEdit_fan_set_temp->setText(all_data_part.mid(0*3,3));
            ui->lineEdit_solder_set_temp->setText(all_data_part.mid(1*3,3));
            ui->lineEdit_fan_speed->setText(all_data_part.mid(2*3,3));
            ui->lineEdit_fan_cel->setText(tr("%1").arg(all_data_part.mid(3*3,3).toInt()));
            ui->lineEdit_solder_cal->setText(tr("%1").arg(all_data_part.mid(4*3,3).toInt()));
            ui->lineEdit_fan_on_off_state->setText(all_data_part.mid(5*3,3));
            ui->lineEdit_solder_on_off_state->setText(all_data_part.mid(6*3,3));
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
    find_client_timer->start(1000);
    ui->pushButton_find_client->setEnabled(0);
    find_client_timer_counter=0;
    //find_client_timer_counter=36;
    timer_time_up();

}
void sender::timer_time_up()
{
    static QByteArray bc_ip;
    static QString add_to_use;
    static QByteArray unknow_client_address;
    find_client_timer_counter ++;
    //if(find_client_timer_counter==100){           //配合遍历方式
    if(find_client_timer_counter==10){              //配合广播方式
        find_client_timer->stop();
        find_client_timer_counter=0;
        ui->pushButton_find_client->setEnabled(1);
    }
    _sender = new QUdpSocket(this);
    add_to_use = ui->comboBox->currentText();
    bc_ip=add_to_use.toUtf8();
    unknow_client_address=bc_ip;
    unknow_client_address.resize(12);
    bc_ip.append(0x0a);

    _sender->bind(QHostAddress(add_to_use)/*,8265*/,QUdpSocket::ShareAddress);

    //广播方式找下位机
    _sender->writeDatagram(bc_ip.data(),bc_ip.size(),QHostAddress::Broadcast,8266);
    //或者遍历方式找下位机
//    for(find_client_timer_counter=0;find_client_timer_counter<100;find_client_timer_counter++){
//        unknow_client_address.append(tr("%1").arg(find_client_timer_counter));
//        unknow_client_address.append(0x0a);
//        if(unknow_client_address!=ui->comboBox_device_address_list->currentText().toUtf8()){
//            ui->textBrowser->append(tr("finding client: %1").arg(find_client_timer_counter));
//            ui->textBrowser->append(unknow_client_address);
//            _sender->writeDatagram(bc_ip.data(),bc_ip.size(),QHostAddress(unknow_client_address.data()),8266);
//            unknow_client_address.resize(12);
//        }
//    }



    delete _sender;
}

void sender::on_pushButton_read_fan_set_temp_clicked()
{
    cmd_datagram.resize(0);;
    cmd_datagram.append("rs___");
    cmd_datagram.append(0x0a);
    send_cmd();
    ui->lineEdit_fan_set_temp->setText("Waiting...");
}

void sender::on_pushButton_read_fan_current_temp_clicked()
{

    cmd_datagram.resize(0);;
    cmd_datagram.append("rr___");
    cmd_datagram.append(0x0a);
    send_cmd();
    ui->lineEdit_fan_current_temp->setText("Waiting...");
}
void sender::send_cmd()
{
    QString add_to_use;
    QString send_to_add;
    _sender = new QUdpSocket(this);
    add_to_use = ui->comboBox->currentText();
    //send_to_add = ui->lineEdit->text();
    send_to_add = ui->comboBox_device_address_list->currentText();
    _sender->bind(QHostAddress(add_to_use)/*,8265*/,QUdpSocket::ShareAddress);

    _sender->writeDatagram(cmd_datagram.data(),cmd_datagram.size(),QHostAddress(send_to_add),8266);
    delete _sender;
    if(! resend_cmd_timer->isActive()){
        resend_cmd_timer->start(1500);
        resend_cmd_timer_timeout_counter=0;
    }

}



void sender::new_device_found()
{
    ui->label_device_on_line->setText("_____Device Online!_____");
    ui->label_device_on_line->setStyleSheet("background-color: rgb(85, 255, 127)");

}
void sender::no_device()
{
    ui->label_device_on_line->setText(tr("_____No Device_____"));
    ui->label_device_on_line->setStyleSheet("background-color: rgb(145, 145, 145)");

}

void sender::device_break()
{
    ui->label_device_on_line->setStyleSheet("background-color: rgb(255, 0, 0)");
    ui->label_device_on_line->setText("_____Connection break!!!_____");
    quint16 a ;
    a = ui->comboBox_device_address_list->currentIndex();
    ui->comboBox_device_address_list->removeItem(a);

}


void sender::on_pushButton_read_fan_speed_clicked()
{
    cmd_datagram.resize(0);;
    cmd_datagram.append("rx___");
    cmd_datagram.append(0x0a);
    send_cmd();
    ui->lineEdit_fan_speed->setText("Waiting...");
}

void sender::on_pushButton_read_solder_set_temp_clicked()
{
    cmd_datagram.resize(0);;
    cmd_datagram.append("ru___");
    cmd_datagram.append(0x0a);
    send_cmd();
    ui->lineEdit_solder_set_temp->setText("Waiting...");
}

void sender::on_pushButton_read_solder_current_temp_clicked()
{
    cmd_datagram.resize(0);;
    cmd_datagram.append("rt___");
    cmd_datagram.append(0x0a);
    send_cmd();
    ui->lineEdit_solder_current_temp->setText("Waiting...");
}

void sender::on_pushButton_read_fan_cal_clicked()
{
    cmd_datagram.resize(0);;
    cmd_datagram.append("af___");
    cmd_datagram.append(0x0a);
    send_cmd();
    ui->lineEdit_fan_cel->setText("Waiting...");
}

void sender::on_pushButton_read_solder_cal_clicked()
{
    cmd_datagram.resize(0);;
    cmd_datagram.append("as___");
    cmd_datagram.append(0x0a);
    send_cmd();
    ui->lineEdit_solder_cal->setText("Waiting...");
}

void sender::on_pushButton_read_fan_on_off_state_clicked()
{
    cmd_datagram.resize(0);;
    cmd_datagram.append("pu___");
    cmd_datagram.append(0x0a);
    send_cmd();
    ui->lineEdit_fan_on_off_state->setText("Waiting...");
}

void sender::on_pushButton_read_solder_on_off_state_clicked()
{
    cmd_datagram.resize(0);;
    cmd_datagram.append("pk___");
    cmd_datagram.append(0x0a);
    send_cmd();
    ui->lineEdit_solder_on_off_state->setText("Waiting...");
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
        ui->lineEdit_fan_set_temp->setText("Waiting...");
    }

}

void sender::on_pushButton_write_solder_set_temp_clicked()
{
    cmd_datagram.resize(0);;
    cmd_datagram.append("ss");
    quint16 temp_value;
    temp_value=ui->lineEdit_solder_set_temp->text().toInt();
    //temp_value=ui->lineEdit_solder_cal->text().toInt();
    if((temp_value>500)|(temp_value<200)){
        QMessageBox::information(0,"info","between 200~500",QMessageBox::Ok | QMessageBox::Default ,0);
    }else{
        cmd_datagram.append(ui->lineEdit_solder_set_temp->text());
        cmd_datagram.append(0x0a);
        send_cmd();
        ui->lineEdit_solder_set_temp->setText("Waiting...");
    }

}

void sender::on_pushButton_write_fan_cal_clicked()
{///////
    cmd_datagram.resize(0);;
    cmd_datagram.append("sy");
    qint16 temp_value;
    //temp_value=ui->lineEdit_solder_cal->text().toInt();
    temp_value=ui->lineEdit_fan_cel->text().toInt();
    QByteArray all_text;
    QByteArray first_one;
    all_text.append(ui->lineEdit_fan_cel->text());
    first_one=all_text.left(1);
    if((first_one==" ")|(first_one=="w")|(first_one=="W")){
        QMessageBox::information(0,"info","between -99~ 99",QMessageBox::Ok | QMessageBox::Default ,0);
        return;
    }
    ui->textBrowser->append(tr("%1").arg(temp_value));
    if((temp_value>99)|(temp_value<-99)){
        QMessageBox::information(0,"info","between -99~ 99",QMessageBox::Ok | QMessageBox::Default ,0);
    }else{
        if(temp_value>=0){
            cmd_datagram.append(" ");
            if(temp_value<10){
                cmd_datagram.append("0");
            }
        }
        cmd_datagram.append(ui->lineEdit_fan_cel->text());

        if((temp_value> -10)&&(temp_value<0)){
            cmd_datagram.insert(3,"0");
        }

        if(ui->lineEdit_fan_cel->text().isEmpty()){
            cmd_datagram.append("0");
        }

        cmd_datagram.append(0x0a);
        send_cmd();
        ui->lineEdit_fan_cel->setText("Waiting...");
    }
}

void sender::on_pushButton_write_solder_cal_clicked()
{
    cmd_datagram.resize(0);;
    cmd_datagram.append("sz");
    qint16 temp_value;
    temp_value=ui->lineEdit_solder_cal->text().toInt();
    QByteArray all_text;
    QByteArray first_one;
    all_text.append(ui->lineEdit_solder_cal->text());

    first_one=all_text.left(1);
    if((first_one==" ")|(first_one=="w")|(first_one=="W")){
        QMessageBox::information(0,"info","between -99~ 99",QMessageBox::Ok | QMessageBox::Default ,0);
        return;
    }
    ui->textBrowser->append(tr("%1").arg(temp_value));
    if((temp_value>99)|(temp_value<-99)){
        QMessageBox::information(0,"info","between -99~ 99",QMessageBox::Ok | QMessageBox::Default ,0);
    }else{
        if(temp_value>=0){
            cmd_datagram.append(" ");
            if(temp_value<10){
                cmd_datagram.append("0");
            }
        }
        cmd_datagram.append(ui->lineEdit_solder_cal->text());

        if((temp_value> -10)&&(temp_value<0)){
            cmd_datagram.insert(3,"0");
        }

        if(ui->lineEdit_solder_cal->text().isEmpty()){
            cmd_datagram.append("0");
        }

        cmd_datagram.append(0x0a);
        send_cmd();
        ui->lineEdit_solder_cal->setText("Waiting...");
    }
}

void sender::on_pushButton_read_all_clicked()
{
    cmd_datagram.resize(0);;
    cmd_datagram.append("ra___");
    cmd_datagram.append(0x0a);
    send_cmd();
}

void sender::on_pushButton_clear_clicked()
{
    ui->lineEdit_fan_cel->clear();
    ui->lineEdit_fan_current_temp->clear();
    ui->lineEdit_fan_on_off_state->clear();
    ui->lineEdit_fan_set_temp->clear();
    ui->lineEdit_fan_speed->clear();
    ui->lineEdit_solder_cal->clear();
    ui->lineEdit_solder_current_temp->clear();
    ui->lineEdit_solder_on_off_state->clear();
    ui->lineEdit_solder_set_temp->clear();
}

void sender::on_pushButton_set_all_clicked()
{
    cmd_datagram.resize(0);;
    cmd_datagram.append("wa");      //1,2
    qint16 temp_value;
//    wa
//    /*风扇设定温度*/  aaa
//    /*烙铁设定温度*/  aaa
//    /*风扇速度*/    aaa
//    /*风扇矫正温度*/  aaa
//    /*烙铁矫正温度*/  aaa
//    /*风扇开关状态*/  aaa
//    /*烙铁开关状态*/  aaa
//    \n
///////////////////////////  风机设定温度     3,4,5

    temp_value=ui->lineEdit_fan_set_temp->text().toInt();
    if((temp_value>500)|(temp_value<100)){
        QMessageBox::information(0,"info","between 100~500",QMessageBox::Ok | QMessageBox::Default ,0);
    }else{
        cmd_datagram.append(ui->lineEdit_fan_set_temp->text());
        ui->lineEdit_fan_set_temp->setText("Waiting...");
    }


    /////////////////////////////////焊台温度设定 6,7,8

    temp_value=ui->lineEdit_solder_set_temp->text().toInt();
    if((temp_value>500)|(temp_value<200)){
        QMessageBox::information(0,"info","between 200~500",QMessageBox::Ok | QMessageBox::Default ,0);
    }else{
        cmd_datagram.append(ui->lineEdit_solder_set_temp->text());
        ui->lineEdit_solder_set_temp->setText("Waiting...");
    }
/////////////////////////////////风扇转速

        cmd_datagram.append("000");     //9,10,11

/////////////////////////////////风机校准   //12,13,14

    temp_value=ui->lineEdit_fan_cel->text().toInt();
    QByteArray all_text;
    QByteArray first_one;
    all_text.append(ui->lineEdit_fan_cel->text());
    first_one=all_text.left(1);
    if((first_one==" ")|(first_one=="w")|(first_one=="W")){
        QMessageBox::information(0,"info","between -99~ 99",QMessageBox::Ok | QMessageBox::Default ,0);
        return;
    }
    ui->textBrowser->append(tr("%1").arg(temp_value));
    if((temp_value>99)|(temp_value<-99)){
        QMessageBox::information(0,"info","between -99~ 99",QMessageBox::Ok | QMessageBox::Default ,0);
    }else{
        if(temp_value>=0){
            cmd_datagram.append(" ");
            if(temp_value<10){
                cmd_datagram.append("0");
            }
        }
        cmd_datagram.append(ui->lineEdit_fan_cel->text());

        if((temp_value> -10)&&(temp_value<0)){
            cmd_datagram.insert(12,"0");
        }

        if(ui->lineEdit_fan_cel->text().isEmpty()){
            cmd_datagram.append("0");
        }
        ui->lineEdit_fan_cel->setText("Waiting...");
    }





///////////////////////////////////焊台校准 15,16,17

    temp_value=ui->lineEdit_solder_cal->text().toInt();
    all_text.append(ui->lineEdit_solder_cal->text());
    first_one=all_text.left(1);
    if((first_one==" ")|(first_one=="w")|(first_one=="W")){
        QMessageBox::information(0,"info","between -99~ 99",QMessageBox::Ok | QMessageBox::Default ,0);
        return;
    }
    ui->textBrowser->append(tr("%1").arg(temp_value));
    if((temp_value>99)|(temp_value<-99)){
        QMessageBox::information(0,"info","between -99~ 99",QMessageBox::Ok | QMessageBox::Default ,0);
    }else{
        if(temp_value>=0){
            cmd_datagram.append(" ");
            if(temp_value<10){
                cmd_datagram.append("0");
            }
        }
        cmd_datagram.append(ui->lineEdit_solder_cal->text());

        if((temp_value> -10)&&(temp_value<0)){
            cmd_datagram.insert(15,"0");
        }

        if(ui->lineEdit_solder_cal->text().isEmpty()){
            cmd_datagram.append("0");
        }
        ui->lineEdit_solder_cal->setText("Waiting...");
    }

//    /*风扇开关状态*/  aaa           18,19,20
    cmd_datagram.append(" on");
 //    /*烙铁开关状态*/  aaa          21,22,23
    cmd_datagram.append(" on");

    cmd_datagram.append(0x0a);      //24
    send_cmd();

}

void sender::on_pushButton_write_fan_speed_clicked()
{
;;
}
