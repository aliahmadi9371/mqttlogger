#ifndef MAINCLASS_H
#define MAINCLASS_H

#include <QObject>
#include <QApplication>
#include <QDebug>
#include <QDateTime>
#include <QTimer>

#include <QtMqtt/QtMqtt>

class mainClass : public QObject
{
    Q_OBJECT
public:
    explicit mainClass(QObject *parent = nullptr);
    void Init();
    void Config();
    void MqttConfig();
    void MqttHostConfig();
    void MqttWillConfig();
    void MqttConnect();
    void MqttDisconnect();

    void RunClass();

    QMqttClient* client{};

    int mqtt_status{};
    int mqtt_status_old{};

    QMqttClient::ClientState serverState;


    bool ping_proc;
    int ping_count;
    bool first;
    bool first_status;
    int connect_disconnect_counter;

    QTimer* checkServerTimer{};
    QTimer* statusTimer{};
    QTimer* checker_mqtt_connecting{};

    QTimer* delay_Timer{};
    bool delay;
    void Delay_ms(quint32 ms);

    QString hName;
    bool hn;

    quint16 hPort;
    bool hp;

    QString uName;
    bool un;

    QString pWord;
    bool pw;


    QTimer *quit_timer;

    bool ff;


    void mqtt_unsubscribe();
    void mqtt_subscribe();

    bool mqtt_check_subscribe();
    void mqtt_check_subscribe_status();

    bool mqtt_check_connectivity();

    bool MqttSubIroTeamZeroResp();
    bool MqttSubIroTeamZero_V2Resp();
    bool MqttSubServerResp();

    void MqttUnSubIroTeamZeroResp();
    void MqttUnSubIroTeamZero_V2Resp();
    void MqttUnSubServerResp();

    bool bSubIroTeamZeroResp;
    QMqttSubscription* subIroTeamZeroResp;

    bool bSubIroTeamZero_V2Resp;
    QMqttSubscription* subIroTeamZero_V2Resp;

    bool bSubServerResp;
    QMqttSubscription* subServerResp;

    void MqttPub(QString topic, QString msg, quint8 qos);
signals:


public slots:
    void on_checkServerTimerTimeout();
    void on_stateChanged_client(QMqttClient::ClientState state);
    void on_pingResponseReceived();
    void on_statusTimerTimeout();
    void on_checker_mqtt_connecting();
    void on_Delay_Timeout();

    void on_messageReceivedIroTeamZeroResp(QMqttMessage mqttMsg);
    void on_stateChangedIroTeamZeroResp(QMqttSubscription::SubscriptionState subState);
//    void on_qosChangedIroTeamZeroResp(quint8 qos);

    void on_messageReceivedIroTeamZero_V2Resp(QMqttMessage mqttMsg);
    void on_stateChangedIroTeamZero_V2Resp(QMqttSubscription::SubscriptionState subState);
//    void on_qosChangedIroTeamZero_V2Resp(quint8 qos);

    void on_messageReceivedServerResp(QMqttMessage mqttMsg);
    void on_stateChangedServerResp(QMqttSubscription::SubscriptionState subState);
//    void on_qosChangedServerResp(quint8 qos);


    void on_quit_timer_timeout();
};

#endif // MAINCLASS_H
