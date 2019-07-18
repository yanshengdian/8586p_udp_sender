#ifndef SENDER_H
#define SENDER_H

#include <QDialog>
#include <QTimer>

class QUdpSocket;

namespace Ui {
    class sender;
}

class sender : public QDialog
{
    Q_OBJECT

public:
    explicit sender(QWidget *parent = 0);
    ~sender();
    QByteArray cmd_datagram;


private slots:
    void on_sendButton_released();
    void process_pending_datagram();
    void on_clearButton_clicked();
    void on_pushButton_find_client_clicked();
    void timer_time_up();
    void resend_timer_time_up();                /////3秒内收不到反馈，重新发命令
    void on_pushButton_read_fan_set_temp_clicked();
    void on_pushButton_read_fan_current_temp_clicked();
    void on_pushButton_read_fan_speed_clicked();
    void on_pushButton_read_solder_set_temp_clicked();
    void on_pushButton_read_solder_current_temp_clicked();
    void on_pushButton_read_fan_cal_clicked();
    void on_pushButton_read_solder_cal_clicked();
    void on_pushButton_read_fan_on_off_state_clicked();
    void on_pushButton_read_solder_on_off_state_clicked();
    void on_pushButton_write_fan_set_temp_clicked();
    void on_pushButton_write_solder_set_temp_clicked();

private:
    Ui::sender *ui;
    QUdpSocket *_sender;
    QUdpSocket *_receiver;
    quint16   is_client_online;
    QTimer *find_client_timer;
    quint16 find_client_timer_counter;
    QTimer *resend_cmd_timer;
    quint16 resend_cmd_timer_timeout_counter;

    void send_cmd();
    void resend_cmd();
};

#endif // SENDER_H
