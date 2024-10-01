#include "mainclass.h"

mainClass::mainClass(QObject *parent) : QObject(parent)
{
    ff = false;
    return;
}

void mainClass::RunClass()
{
    Init();
    Config();
    MqttHostConfig();
    MqttWillConfig();

    mqtt_status = 0;
    mqtt_status_old = 0;

    MqttConnect();

    checker_mqtt_connecting->start();
    checkServerTimer->start();
    return;
}

void mainClass::Init()
{
    client = nullptr;

    checkServerTimer = nullptr;
    statusTimer = nullptr;
    checker_mqtt_connecting = nullptr;

    delay_Timer = nullptr;
    delay = false;

    quit_timer = nullptr;

    bSubIroTeamZeroResp = false;
    subIroTeamZeroResp = nullptr;

    bSubIroTeamZero_V2Resp = false;
    subIroTeamZero_V2Resp = nullptr;

    bSubServerResp = false;
    subServerResp = nullptr;


    hName.clear();
    hn = false;

    hPort = 0;
    hp = false;

    uName.clear();
    un = false;

    pWord.clear();
    pw = false;
    return;
}

void mainClass::Config()
{
    if(!hn){
        hName =  "emq.iroteam.com";
        hn = true;
    }
    if(!hp){
        hPort =  31536;
        hp = true;
    }
    if(!un){
        uName =  "AH_Zahedi";
        un = true;
    }
    if(!pw){
        pWord =  "6fad728c6f6b974";
        pw = true;
    }

    qDebug().noquote() << "Config: " << "host name: " << hName;
    qDebug().noquote() << "Config: " << "host port: " << hPort;
    qDebug().noquote() << "Config: " << "user name: " << uName;
    qDebug().noquote() << "Config: " << "pass word: " << pWord;


    client = new QMqttClient(this);

    checkServerTimer = new QTimer(this);
    checkServerTimer->setSingleShot(true);
    checkServerTimer->setInterval((10 * 1000) + 1500);
    checkServerTimer->stop();
    QObject::connect(checkServerTimer, &QTimer::timeout, this, &mainClass::on_checkServerTimerTimeout, Qt::QueuedConnection);

    quit_timer = new QTimer(this);
    quit_timer->setSingleShot(true);
    quit_timer->setInterval(10000);
    quit_timer->stop();
    QObject::connect(quit_timer, &QTimer::timeout, this, &mainClass::on_quit_timer_timeout, Qt::QueuedConnection);



    qRegisterMetaType<QMqttClient::ClientState>("ClientState");
    QObject::connect(client, &QMqttClient::stateChanged, this, &mainClass::on_stateChanged_client, Qt::QueuedConnection);

    //client->requestPing()
    QObject::connect(client, &QMqttClient::pingResponseReceived, this, &mainClass::on_pingResponseReceived, Qt::QueuedConnection);


    statusTimer = new QTimer(this);
    statusTimer->setSingleShot(true);
    statusTimer->setInterval(5 * 1000);
    statusTimer->stop();
    QObject::connect(statusTimer, &QTimer::timeout, this, &mainClass::on_statusTimerTimeout, Qt::QueuedConnection);


    checker_mqtt_connecting = new QTimer(this);
    checker_mqtt_connecting->setSingleShot(true);
    checker_mqtt_connecting->setInterval(3 * 1000);
    checker_mqtt_connecting->stop();
    QObject::connect(checker_mqtt_connecting, &QTimer::timeout, this, &mainClass::on_checker_mqtt_connecting, Qt::QueuedConnection);

    delay_Timer = new QTimer(this);
    delay_Timer->setSingleShot(true);
    delay_Timer->stop();
    QObject::connect(delay_Timer, &QTimer::timeout, this, &mainClass::on_Delay_Timeout, Qt::QueuedConnection);

    return;
}

void mainClass::MqttHostConfig()
{
    client->setProtocolVersion(QMqttClient::MQTT_3_1);

    client->setHostname(hName);
    client->setPort(hPort);
    client->setCleanSession(true);
    client->setUsername(uName);
    client->setPassword(pWord);
    client->setKeepAlive(120);

    return;
}

void mainClass::MqttWillConfig()
{
    client->setWillMessage("IroTeam is disconnected");
    client->setWillQoS(2);
    client->setWillRetain(true);
    QString WillTopic = QString("IroTeam/local/STATUS");
    client->setWillTopic(WillTopic);

    return;
}

void mainClass::MqttConnect()
{
    QString cid = "server_logger" + QDateTime::currentDateTime().toString("|yyyy-MM-dd|hh:mm:ss");
    client->setClientId(cid);
    qDebug().noquote() << "MqttConnect --> " << "client id: " << cid;
    qDebug().noquote() << "MqttConnect --> " << "connect to " << hName << ":" << hPort;
    client->connectToHost();
    quit_timer->start();

    return;
}

void mainClass::MqttDisconnect()
{
    qDebug().noquote() << "MqttDisconnect --> " << "disconnect from" << hName << ":" << hPort;
    client->disconnectFromHost();

    return;
}

void mainClass::on_stateChanged_client(QMqttClient::ClientState state)
{
    switch(state){
        case QMqttClient::ClientState::Disconnected:{
            qDebug().noquote() << "on_stateChanged_client--> ClientState::Disconnected From " << hName << ":" << hPort;
            //statusTimer->stop();
            quit_timer->stop();
            Delay_ms(5000);
            MqttConnect();
            break;
        }
        case QMqttClient::ClientState::Connecting:{
            qDebug().noquote() << "on_stateChanged_client--> ClientState::Connecting to " << hName << ":" << hPort;
            break;
        }
        case QMqttClient::ClientState::Connected:{
            qDebug().noquote() << "on_stateChanged_client--> ClientState::Connected to " << hName << ":" << hPort;
            //on_statusTimerTimeout();
            quit_timer->stop();
            mqtt_subscribe();
            break;
        }
        default:{
            break;
        }
    }
    mqtt_check_subscribe_status();

    return;
}

void mainClass::on_checker_mqtt_connecting()
{
    checker_mqtt_connecting->stop();
    mqtt_check_subscribe_status();
    checker_mqtt_connecting->start();

    return;
}

void mainClass::mqtt_check_subscribe_status()
{
    if(client->state() == QMqttClient::ClientState::Connected){
        if(mqtt_check_subscribe()){
            mqtt_status = 2;
            if(mqtt_status != mqtt_status_old){
                ff = false;
                qDebug() << "mqtt_check_subscribe_status--> successfull subscription";
                mqtt_status_old = mqtt_status;
            }
        }
        else{
            mqtt_status = 1;
            if(mqtt_status != mqtt_status_old){
                qDebug() << "mqtt_check_subscribe_status--> unsuccessfull subscription";
                mqtt_status_old = mqtt_status;
            }
        }
    }
    else{
        mqtt_status = 0;
        if(mqtt_status != mqtt_status_old){
            qDebug() << "mqtt_check_subscribe_status--> not connected to server";
            mqtt_status_old = mqtt_status;
        }
    }

    return;
}

bool mainClass::mqtt_check_subscribe()
{
    if(client->state() == QMqttClient::ClientState::Connected){
        if(
            mqtt_check_connectivity()
          ){
            return true;
        }
        else{
            return false;
        }
    }
    else{
        return false;
    }
}

bool mainClass::mqtt_check_connectivity()
{
    if((bSubIroTeamZeroResp && subIroTeamZeroResp && (subIroTeamZeroResp->state() == QMqttSubscription::Subscribed)) &&
       (bSubIroTeamZero_V2Resp && subIroTeamZero_V2Resp && (subIroTeamZero_V2Resp->state() == QMqttSubscription::Subscribed)) &&
       (bSubServerResp && subServerResp && (subServerResp->state() == QMqttSubscription::Subscribed))){
//        qDebug() << "mqtt_check_connectivity--> trueeee";
        return true;
    }
    else{
//        qDebug() << "mqtt_check_connectivity--> falseee";
        return false;
    }

}

void mainClass::on_checkServerTimerTimeout()
{
    checkServerTimer->stop();

    QApplication::processEvents(QEventLoop::AllEvents);
    if(client->state() == QMqttClient::ClientState::Connected){
        if(ping_proc){
            ping_count++;
            qDebug().noquote() << "on_checkServerTimerTimeout--> Failed Ping From" << QString("[%1:%2]").arg(hName).arg(QString::number(hPort));
            if(ping_count > 0){
                ping_count = 0;
                ping_proc = false;
                MqttDisconnect();
            }
        }
        else{
            ping_count = 0;
            ping_proc = true;
            client->requestPing();
        }
    }


    checkServerTimer->start();

    return;
}

void mainClass::on_pingResponseReceived()
{
    ping_proc = false;

    return;
}

void mainClass::mqtt_unsubscribe()
{
    qDebug().noquote() << "mqtt_unsubscribe--> ";
    MqttUnSubIroTeamZeroResp();
    MqttUnSubIroTeamZero_V2Resp();
    MqttUnSubServerResp();

    return;
}


void mainClass::mqtt_subscribe()
{
    qDebug().noquote() << "mqtt_subscribe--> ";
    //Table thread to Mqtt thread and Mqtt thread to Table thread
//    if(!subIroTeamZeroResp)
        MqttSubIroTeamZeroResp();
//    else if(!bSubIroTeamZeroResp && (subIroTeamZeroResp->state() == QMqttSubscription::Unsubscribed))
//        MqttSubIroTeamZeroResp();

//    if(!subIroTeamZero_V2Resp)
        MqttSubIroTeamZero_V2Resp();
//    else if(!bSubIroTeamZero_V2Resp && (subIroTeamZero_V2Resp->state() == QMqttSubscription::Unsubscribed))
//        MqttSubIroTeamZero_V2Resp();

//    if(!subServerResp)
        MqttSubServerResp();
//    else if(!bSubServerResp && (subServerResp->state() == QMqttSubscription::Unsubscribed))
//        MqttSubServerResp();

    return;
}

void mainClass::on_statusTimerTimeout()
{
    statusTimer->stop();
    //qDebug() << "on_statusTimerTimeout";
    if(mqtt_check_subscribe()){
//        qDebug() << MQTTCLASS << "on_statusTimerTimeout--> publish IroTeam is Connected";
        QString msg = "";
        if(!first_status){
            first_status = true;
            msg = "IroTeam is first connected";
        }
        else{
            msg = "IroTeam is connected";
        }
            //qDebug() << "msg = " << msg;
            QString topic = QString("IroTeam/%1/STATUS").arg("local");
            MqttPub(topic, msg, 2);
    }

    statusTimer->start();

    return;
}

void mainClass::MqttPub(QString topic, QString msg, quint8 qos)
{
    if(client->state() == QMqttClient::ClientState::Connected){
        QMqttTopicName topicName;
        topicName.setName(topic);
        client->publish(topicName, msg.toLocal8Bit(), qos , false);
    }
    else{
        qDebug() << "MqttPub--> client is not connected to server";
    }
}


bool mainClass::MqttSubIroTeamZeroResp()
{
    bool ret = false;
    qDebug() << "MqttSubIroTeamZeroResp--> ";
    QString topicString = QString("IroTeamZero/#");
    quint8 qos = 1;
    QMqttTopicFilter topicFilter;
    topicFilter.setFilter(topicString);
    subIroTeamZeroResp = client->subscribe(topicFilter,qos);

    if(subIroTeamZeroResp){
        ret = true;
        qRegisterMetaType<QMqttMessage>("QMqttMessage");
        QObject::connect(subIroTeamZeroResp, &QMqttSubscription::messageReceived, this, &mainClass::on_messageReceivedIroTeamZeroResp, Qt::QueuedConnection);

        qRegisterMetaType<QMqttSubscription::SubscriptionState>("SubscriptionState");
        QObject::connect(subIroTeamZeroResp, &QMqttSubscription::stateChanged, this, &mainClass::on_stateChangedIroTeamZeroResp ,Qt::QueuedConnection);

//        QObject::connect(subIroTeamZeroResp, &QMqttSubscription::qosChanged, this, &MqttClass::on_qosChangedIroTeamZeroResp, Qt::QueuedConnection);
    }
    else{
        ret = false;
    }

    return ret;
}

bool mainClass::MqttSubIroTeamZero_V2Resp()
{
    bool ret = false;
    qDebug() << "MqttSubIroTeamZero_V2Resp--> ";
    QString topicString = QString("IroTeamZero_V2/#");
    quint8 qos = 1;
    QMqttTopicFilter topicFilter;
    topicFilter.setFilter(topicString);
    subIroTeamZero_V2Resp = client->subscribe(topicFilter,qos);

    if(subIroTeamZero_V2Resp){
        ret = true;
        qRegisterMetaType<QMqttMessage>("QMqttMessage");
        QObject::connect(subIroTeamZero_V2Resp, &QMqttSubscription::messageReceived, this, &mainClass::on_messageReceivedIroTeamZero_V2Resp, Qt::QueuedConnection);

        qRegisterMetaType<QMqttSubscription::SubscriptionState>("SubscriptionState");
        QObject::connect(subIroTeamZero_V2Resp, &QMqttSubscription::stateChanged, this, &mainClass::on_stateChangedIroTeamZero_V2Resp ,Qt::QueuedConnection);

//        QObject::connect(subIroTeamZero_V2Resp, &QMqttSubscription::qosChanged, this, &MqttClass::on_qosChangedIroTeamZero_V2Resp, Qt::QueuedConnection);
    }
    else{
        ret = false;
    }

    return ret;
}

bool mainClass::MqttSubServerResp()
{
    bool ret = false;
    qDebug() << "MqttSubServerResp--> ";
    QString topicString = QString("Server/#");
    quint8 qos = 1;
    QMqttTopicFilter topicFilter;
    topicFilter.setFilter(topicString);
    subServerResp = client->subscribe(topicFilter,qos);

    if(subServerResp){
        ret = true;
        qRegisterMetaType<QMqttMessage>("QMqttMessage");
        QObject::connect(subServerResp, &QMqttSubscription::messageReceived, this, &mainClass::on_messageReceivedServerResp, Qt::QueuedConnection);

        qRegisterMetaType<QMqttSubscription::SubscriptionState>("SubscriptionState");
        QObject::connect(subServerResp, &QMqttSubscription::stateChanged, this, &mainClass::on_stateChangedServerResp ,Qt::QueuedConnection);

//        QObject::connect(subServerResp, &QMqttSubscription::qosChanged, this, &MqttClass::on_qosChangedServerResp, Qt::QueuedConnection);
    }
    else{
        ret = false;
    }

    return ret;
}

void mainClass::MqttUnSubIroTeamZeroResp()
{
    if(subIroTeamZeroResp && bSubIroTeamZeroResp){
        QString topicString = QString("IroTeamZero/#");
        QMqttTopicFilter topicFilter;
        topicFilter.setFilter(topicString);
        client->unsubscribe(topicFilter);

//        while(subIroTeamZeroResp->state() == QMqttSubscription::Subscribed){
//            QApplication::processEvents(QEventLoop::AllEvents);
//        }

//        QObject::disconnect(subIroTeamZeroResp, &QMqttSubscription::messageReceived, this, &MqttClass::on_messageReceivedIroTeamZeroResp);
//        QObject::disconnect(subIroTeamZeroResp, &QMqttSubscription::stateChanged, this, &MqttClass::on_stateChangedIroTeamZeroResp);

        subIroTeamZeroResp->deleteLater();
        subIroTeamZeroResp = nullptr;
        bSubIroTeamZeroResp = false;
        qDebug() << "MqttUnSubIroTeamZeroResp--> ";
    }

    return;
}

void mainClass::MqttUnSubIroTeamZero_V2Resp()
{
    if(subIroTeamZero_V2Resp && bSubIroTeamZero_V2Resp){
        QString topicString = QString("IroTeamZero_V2/#");
        QMqttTopicFilter topicFilter;
        topicFilter.setFilter(topicString);
        client->unsubscribe(topicFilter);

//        while(subIroTeamZero_V2Resp->state() == QMqttSubscription::Subscribed){
//            QApplication::processEvents(QEventLoop::AllEvents);
//        }

//        QObject::disconnect(subResp, &QMqttSubscription::messageReceived, this, &MqttClass::on_messageReceivedIroTeamZero_V2Resp);
//        QObject::disconnect(subResp, &QMqttSubscription::stateChanged, this, &MqttClass::on_stateChangedIroTeamZero_V2Resp);

        subIroTeamZero_V2Resp->deleteLater();
        subIroTeamZero_V2Resp = nullptr;
        bSubIroTeamZero_V2Resp = false;
        qDebug() << "MqttUnSubIroTeamZero_V2Resp--> ";
    }

    return;
}

void mainClass::MqttUnSubServerResp()
{
    if(subServerResp && bSubServerResp){
        QString topicString = QString("Server/#");
        QMqttTopicFilter topicFilter;
        topicFilter.setFilter(topicString);
        client->unsubscribe(topicFilter);

//        while(subServerResp->state() == QMqttSubscription::Subscribed){
//            QApplication::processEvents(QEventLoop::AllEvents);
//        }

//        QObject::disconnect(subResp, &QMqttSubscription::messageReceived, this, &MqttClass::on_messageReceivedServerResp);
//        QObject::disconnect(subResp, &QMqttSubscription::stateChanged, this, &MqttClass::on_stateChangedServerResp);

        subServerResp->deleteLater();
        subServerResp = nullptr;
        bSubServerResp = false;
        qDebug() << "MqttUnSubResp--> ";
    }

    return;
}


void mainClass::on_messageReceivedIroTeamZeroResp(QMqttMessage mqttMsg)
{
    QString log;
    log = QString("{\"dt\":\"%1\",\"topic\":\"%2\",\"message\":%3},").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz")).arg(mqttMsg.topic().name()).arg(QString::fromLocal8Bit(mqttMsg.payload()));
    qInfo().noquote().nospace() <<  log;

    return;
}

void mainClass::on_messageReceivedIroTeamZero_V2Resp(QMqttMessage mqttMsg)
{
    QString log;
    log = QString("{\"dt\":\"%1\",\"topic\":\"%2\",\"message\":%3},").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz")).arg(mqttMsg.topic().name()).arg(QString::fromLocal8Bit(mqttMsg.payload()));
    qInfo().noquote() <<  log;

    return;
}

void mainClass::on_messageReceivedServerResp(QMqttMessage mqttMsg)
{
    QString m = QString(mqttMsg.payload());
    if(mqttMsg.topic().name().contains("/M/Stat") && m.count("tsf") == 2 && m.count("tsr") == 2){
        int firstIndex = m.indexOf("tsf");
        int secondIndex = m.indexOf("tsf", firstIndex + 1);
        m = m.insert(secondIndex + 2, 'u');

        firstIndex = m.indexOf("tsr");
        secondIndex = m.indexOf("tsr", firstIndex + 1);
        m = m.insert(secondIndex + 2, 'u');

        QString log;
        log = QString("{\"dt\":\"%1\",\"topic\":\"%2\",\"message\":%3},").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz")).arg(mqttMsg.topic().name()).arg(m);
        qInfo().noquote().nospace() <<  log;
    }
    else{
        QString log;
        log = QString("{\"dt\":\"%1\",\"topic\":\"%2\",\"message\":%3},").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss:zzz")).arg(mqttMsg.topic().name()).arg(QString(mqttMsg.payload()));
        qInfo().noquote().nospace() <<  log;
    }


    return;
}

void mainClass::on_stateChangedIroTeamZeroResp(QMqttSubscription::SubscriptionState subState)
{
    switch (subState){
    case QMqttSubscription::Unsubscribed:
        qDebug() << "on_stateChangedIroTeamZeroResp--> " << "QMqttSubscription::Unsubscribed";
        bSubIroTeamZeroResp = false;
        break;
    case QMqttSubscription::SubscriptionPending:
        qDebug() << "on_stateChangedIroTeamZeroResp--> " << "QMqttSubscription::SubscriptionPending";
        bSubIroTeamZeroResp = false;
        break;
    case QMqttSubscription::Subscribed:
        qDebug() << "on_stateChangedIroTeamZeroResp--> " << "QMqttSubscription::Subscribed";
        bSubIroTeamZeroResp = true;
        break;
    case QMqttSubscription::UnsubscriptionPending:
        qDebug() << "on_stateChangedIroTeamZeroResp--> " << "QMqttSubscription::UnsubscriptionPending";
        bSubIroTeamZeroResp = false;
        break;
    case QMqttSubscription::Error:
        qDebug() << "on_stateChangedIroTeamZeroResp--> " << "QMqttSubscription::Error";
        bSubIroTeamZeroResp = false;
        break;
    default:
        break;
    }
    mqtt_check_subscribe_status();

    if(
       (subIroTeamZeroResp->state() == QMqttSubscription::Unsubscribed || subIroTeamZeroResp->state() == QMqttSubscription::Error) && (subIroTeamZeroResp->state() != QMqttSubscription::SubscriptionPending || subIroTeamZeroResp->state() != QMqttSubscription::UnsubscriptionPending) &&
       (subIroTeamZero_V2Resp->state() == QMqttSubscription::Unsubscribed || subIroTeamZero_V2Resp->state() == QMqttSubscription::Error) && (subIroTeamZero_V2Resp->state() != QMqttSubscription::SubscriptionPending || subIroTeamZero_V2Resp->state() != QMqttSubscription::UnsubscriptionPending) &&
       (subServerResp->state() == QMqttSubscription::Unsubscribed || subServerResp->state() == QMqttSubscription::Error) && (subServerResp->state() != QMqttSubscription::SubscriptionPending || subServerResp->state() != QMqttSubscription::UnsubscriptionPending)
      ){
        QApplication::quit();
    }

    return;
}

void mainClass::on_stateChangedIroTeamZero_V2Resp(QMqttSubscription::SubscriptionState subState)
{
    switch (subState){
    case QMqttSubscription::Unsubscribed:
        qDebug() << "on_stateChangedIroTeamZero_V2Resp--> " << "QMqttSubscription::Unsubscribed";
        bSubIroTeamZero_V2Resp = false;
        break;
    case QMqttSubscription::SubscriptionPending:
        qDebug() << "on_stateChangedIroTeamZero_V2Resp--> " << "QMqttSubscription::SubscriptionPending";
        bSubIroTeamZero_V2Resp = false;
        break;
    case QMqttSubscription::Subscribed:
        qDebug() << "on_stateChangedIroTeamZero_V2Resp--> " << "QMqttSubscription::Subscribed";
        bSubIroTeamZero_V2Resp = true;
        break;
    case QMqttSubscription::UnsubscriptionPending:
        qDebug() << "on_stateChangedIroTeamZero_V2Resp--> " << "QMqttSubscription::UnsubscriptionPending";
        bSubIroTeamZero_V2Resp = false;
        break;
    case QMqttSubscription::Error:
        qDebug() << "on_stateChangedIroTeamZero_V2Resp--> " << "QMqttSubscription::Error";
        bSubIroTeamZero_V2Resp = false;
        break;
    default:
        break;
    }
    mqtt_check_subscribe_status();

    if(
       (subIroTeamZeroResp->state() == QMqttSubscription::Unsubscribed || subIroTeamZeroResp->state() == QMqttSubscription::Error) && (subIroTeamZeroResp->state() != QMqttSubscription::SubscriptionPending || subIroTeamZeroResp->state() != QMqttSubscription::UnsubscriptionPending) &&
       (subIroTeamZero_V2Resp->state() == QMqttSubscription::Unsubscribed || subIroTeamZero_V2Resp->state() == QMqttSubscription::Error) && (subIroTeamZero_V2Resp->state() != QMqttSubscription::SubscriptionPending || subIroTeamZero_V2Resp->state() != QMqttSubscription::UnsubscriptionPending) &&
       (subServerResp->state() == QMqttSubscription::Unsubscribed || subServerResp->state() == QMqttSubscription::Error) && (subServerResp->state() != QMqttSubscription::SubscriptionPending || subServerResp->state() != QMqttSubscription::UnsubscriptionPending)
      ){
        QApplication::quit();
    }

    return;
}

void mainClass::on_stateChangedServerResp(QMqttSubscription::SubscriptionState subState)
{
    switch (subState){
    case QMqttSubscription::Unsubscribed:
        qDebug() << "on_stateChangedServerResp--> " << "QMqttSubscription::Unsubscribed";
        bSubServerResp = false;
        break;
    case QMqttSubscription::SubscriptionPending:
        qDebug() << "on_stateChangedServerResp--> " << "QMqttSubscription::SubscriptionPending";
        bSubServerResp = false;
        break;
    case QMqttSubscription::Subscribed:
        qDebug() << "on_stateChangedServerResp--> " << "QMqttSubscription::Subscribed";
        bSubServerResp = true;
        break;
    case QMqttSubscription::UnsubscriptionPending:
        qDebug() << "on_stateChangedServerResp--> " << "QMqttSubscription::UnsubscriptionPending";
        bSubServerResp = false;
        break;
    case QMqttSubscription::Error:
        qDebug() << "on_stateChangedServerResp--> " << "QMqttSubscription::Error";
        bSubServerResp = false;
        break;
    default:
        break;
    }
    mqtt_check_subscribe_status();

    if(
       (subIroTeamZeroResp->state() == QMqttSubscription::Unsubscribed || subIroTeamZeroResp->state() == QMqttSubscription::Error) && (subIroTeamZeroResp->state() != QMqttSubscription::SubscriptionPending || subIroTeamZeroResp->state() != QMqttSubscription::UnsubscriptionPending) &&
       (subIroTeamZero_V2Resp->state() == QMqttSubscription::Unsubscribed || subIroTeamZero_V2Resp->state() == QMqttSubscription::Error) && (subIroTeamZero_V2Resp->state() != QMqttSubscription::SubscriptionPending || subIroTeamZero_V2Resp->state() != QMqttSubscription::UnsubscriptionPending) &&
       (subServerResp->state() == QMqttSubscription::Unsubscribed || subServerResp->state() == QMqttSubscription::Error) && (subServerResp->state() != QMqttSubscription::SubscriptionPending || subServerResp->state() != QMqttSubscription::UnsubscriptionPending)
      ){
        QApplication::quit();
    }

    return;
}

//void mainClass::on_qosChangedIroTeamZeroResp(quint8 qos)
//{
//    qInfo() << "on_qosChangedIroTeamZeroResp--> " <<  qos;

//    return;
//}

//void mainClass::on_qosChangedIroTeamZero_V2Resp(quint8 qos)
//{
//    qInfo() << "on_qosChangedIroTeamZero_V2Resp--> " <<  qos;

//    return;
//}

//void mainClass::on_qosChangedServerResp(quint8 qos)
//{
//    qInfo() << "on_qosChangedServerResp--> " <<  qos;

//    return;
//}


void mainClass::on_Delay_Timeout()
{
    delay_Timer->stop();
    delay = false;

    return;
}

void mainClass::Delay_ms(quint32 ms)
{
    delay_Timer->start(ms);
    delay = true;
    while(delay){
        QApplication::processEvents(QEventLoop::AllEvents);
    }

    return;
}


void mainClass::on_quit_timer_timeout()
{
    QApplication::quit();
    return;
}
