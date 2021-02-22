#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::CheckTimeout()
{
    if (receivedUdp) {
        mutexTimeout.lock();
        receivedUdp = 0;
        mutexTimeout.unlock();
        return;
    }
    else {
        QMessageBox::warning(this, "Disconnected!", "Disconnected by timeout");
        ui->textEdit->appendPlainText("Disconnected by timeout\n");
        emit DisconnectUDP();
    }
}

UDPClass::UDPClass()
{
    // UDP socket
    udpsocket = new QUdpSocket(this);
    udpsocket->setSocketOption(QAbstractSocket::LowDelayOption, 1);

    udpsocket_client = new QUdpSocket(this);
    udpsocket_client->setSocketOption(QAbstractSocket::LowDelayOption, 1);

    keepaliveTimer = new QTimer(this);
    keepaliveTimer->setTimerType(Qt::VeryCoarseTimer);
    connect(keepaliveTimer, SIGNAL(timeout()), this, SLOT(SendKA()));
}

void UDPClass::IniateUDP(QString _serverName, int _port, bool local)
{
    qDebug() << "Entered in IniateUDP";

    serverName = _serverName;
    serverPort = _port;

    //  1: Initate connection
    char packet[2];
    packet[0] = 1;
    packet[1] = 0;

    connect(udpsocket_client, SIGNAL(readyRead()),this, SLOT(LeggiDato()));

    if (local){
        udpsocket_client->bind(QHostAddress::LocalHost, 9777);
        qDebug() << "Entered in local bind";
    }
    else{
        udpsocket_client->bind(QHostAddress::AnyIPv4, 9777);
        qDebug() << "Entered in not-local bind";
    }

    // Flush possible remaining packets
    while (udpsocket_client->hasPendingDatagrams())
        udpsocket_client->receiveDatagram(0);

    udpsocket->writeDatagram(packet, sizeof(packet), QHostAddress(serverName), serverPort);
    qDebug() << "Packet 01 sent...";

    keepaliveTimer->start(1000);
}

void UDPClass::SendKA()
{
    //qDebug() << "Entered in SendKA";

    char init[2];
    init[0] = 3;
    init[1] = 9;
    udpsocket->writeDatagram(init, sizeof(init), QHostAddress(serverName), serverPort);
}

void UDPClass::LeggiDato()
{
    //qDebug() << "Entered in LeggiDato";

    if (!lettaConf) {

        //qDebug() << "Entered in LeggiDato and lettaConf = 0";

        qint64 dataRead = -1;
        // Protocol:
        //  02: RIC. CONF Ok
        //  05: SEND CONF again
        char packet[2];
        packet[1] = 0;
        packet[0] = 2;

        dataRead = udpsocket_client->readDatagram(buffer, sizeof(struct ConfigStimWhole));

        if (dataRead != sizeof(struct ConfigStimWhole)) {
            emit appendTextEdit("Problem reading the CONF packet. Packet read: ");
            emit appendTextEdit(QString::number(dataRead));
            emit appendTextEdit("Asking for a new CONF.\n");

            // BUG ATTENZIONE!!
            // WARNING BISOGNA CAMBIARE IL SERVER, DICENDO CHE 5 E' PER AVERE UNA NUOVA CONF. ORA E' CHE 0X03 VUOLE MANDARE una nuova conf!!
            packet[0] = 5;
            udpsocket->writeDatagram(packet, sizeof(packet), QHostAddress(serverName), serverPort);
            qDebug() << "Packet 05 sent";
            return;
        }

        // Anche di questo pacchetto andrebbe chiesto l'ACK
        udpsocket->writeDatagram(packet, sizeof(packet), QHostAddress(serverName), serverPort);
        qDebug() << "Packet 02 sent";
        svrConnected = 1;
        emit appendTextEdit("Connected");

        // occhio, perche' per come e' scritto, CONF puntera' a buffer. Se buffer dovesse cambiare,
        // anche CONF cambia. Pero' per scrivere nella GUI bisogna chiamare di nuovo writeconfwin()
        // per evitare questa cosa si potrebbe allocare CONF e poi fare una memcpy
        CONF = reinterpret_cast<ConfigStimWhole *>(buffer);
        // copio in una variabile dove mantengo la prima configurazione letta dal server. Nel caso facessi modifiche,
        // e volessi tornare a questa conf. basta premere default dalla gui
        memcpy(&CONF_SVR, CONF, sizeof(CONF_SVR));
        lettaConf = 1;

        emit startTimer(TIMER, 1000);
        emit startTimer(TIMERSCROLL, SCROLLRATE);
        emit startTimer(TIMERTIMEOUT, TIMEOUTUDP);

        qDebug() << "lettaConf = " << lettaConf;

        return;
    }

    //qDebug() << "Entered in LeggiDato and lettaConf = 1";

    udpsocket_client->readDatagram(buffer3, sizeof(messageShort) + sizeof(cnt) + sizeof(kneeYaw) + sizeof(shankYaw) + sizeof(thighYaw));

    unsigned short tsh[7];
    memcpy(tsh, buffer3, sizeof(tsh));
    memcpy(&cnt, buffer3 + sizeof(messageShort), sizeof(cnt));
    memcpy(&kneeYaw, buffer3 + sizeof(messageShort) + sizeof(cnt), sizeof(kneeYaw));
    memcpy(&shankYaw, buffer3 + sizeof(messageShort) + sizeof(cnt) + sizeof(kneeYaw), sizeof(shankYaw));
    memcpy(&thighYaw, buffer3 + sizeof(messageShort) + sizeof(cnt) + sizeof(kneeYaw) + sizeof(shankYaw), sizeof(thighYaw));

    //qDebug() << "size of messageShort = " << sizeof(messageShort) << "\n";
    //qDebug() << "cnt = " << cnt << "\n";
    //qDebug() << "kneeYaw = " << kneeYaw*180/3.14159265 << "\n";
    //qDebug() << "shankYaw = " << shankYaw*180/3.14159265 << "\n";
    //qDebug() << "thighYaw = " << thighYaw*180/3.14159265 << "\n";

    messageShort[0] = tsh[0];
    messageShort[1] = tsh[1];
    messageShort[2] = tsh[2];
    messageShort[3] = tsh[3];
    messageShort[4] = tsh[4];
    messageShort[5] = tsh[5];
    messageShort[6] = tsh[6];

    pwShort[0] = MapLinPwm(messageShort[RED_RAW_S],0);
    pwShort[1] = MapLinPwm(messageShort[BLUE_RAW_S],1);
    pwShort[2] = MapLinPwm(messageShort[BLACK_RAW_S],2);
    pwShort[3] = MapLinPwm(messageShort[WHITE_RAW_S],3);

    if (mutexTimeout.tryLock()) {
        receivedUdp = 1;
        mutexTimeout.unlock();
    }

    if (mutex.tryLock()) {
        // trying to keeping only the first append, this does not slow down

        dataListRaw[0].append(QPointF((cnt)*PERIOD, messageShort[0]));
        dataListRaw[1].append(QPointF((cnt)*PERIOD, messageShort[1]));
        dataListRaw[2].append(QPointF((cnt)*PERIOD, messageShort[2]));
        dataListRaw[3].append(QPointF((cnt)*PERIOD, messageShort[3]));
        dataListRaw[4].append(QPointF((cnt)*PERIOD, messageShort[4]));
        dataListRaw[5].append(QPointF((cnt)*PERIOD, messageShort[5]));
        dataListRaw[6].append(QPointF((cnt)*PERIOD, messageShort[6]));

        mutex.unlock();
    }

    // If we got another WHILE we were in this function, it means we are late!!
    // So we clean the buffer to keep data up to date.
    while (udpsocket_client->hasPendingDatagrams())
        udpsocket_client->receiveDatagram(0);
}

void UDPClass::DisconnectUDP()
{
    char packet[2];

    if (svrConnected) {
        udpsocket_client->close();

        // Disconnecting UDP means only notifying the server (which will start waiting for new connections)
        // It's important to implement in the server a check for the source of the data, (i.e.: the ip) in order
        // to be secure.

        // This message is sent only by disconnected() because if we are running locally we stop the program itself.

        // PROTOCOL: 0x09 0x09: stop
        packet[0] = 9;
        packet[1] = 9;

        while( udpsocket->writeDatagram(packet, sizeof(packet), QHostAddress(serverName), serverPort) != sizeof(packet)) {
            emit appendTextEdit("Something went wrong with the disconnection.. trying again.\n");
        }

        emit appendTextEdit("Disconnected");

        udpsocket->close();

        while (udpsocket_client->hasPendingDatagrams())
            udpsocket_client->receiveDatagram(0);

        // Important:
        // To be certain we can either send many times this packet, or implement a response
        emit ResetSettingsConnection();
    }

    keepaliveTimer->stop();
}

void UDPClass::udpSocketsClose()
{
    udpsocket->close();
    udpsocket_client->close();
}

void UDPClass::SendConf()
{
    char init[2];
    init[0] = 3;
    init[1] = 0;
    udpsocket->writeDatagram(init, sizeof(init), QHostAddress(serverName), serverPort);

    char packet[sizeof(ConfigStimWhole)];
    memcpy(packet, CONF, sizeof(ConfigStimWhole));
    udpsocket->writeDatagram(packet, sizeof(ConfigStimWhole), QHostAddress(serverName), serverPort);
}

void UDPClass::send88()
{
    qDebug() << "Entered in send88()";
    char packet[2];
    packet[1] = 8;
    packet[0] = 8;

    udpsocket->writeDatagram(packet, sizeof(packet), QHostAddress(serverName), serverPort);
    qDebug() << "Packet 88 sent";
}

void UDPClass::emergency()
{
    qDebug() << "Entered in emergency()";
    char packet[2];
    packet[1] = 9;
    packet[0] = 9;

    udpsocket->writeDatagram(packet, sizeof(packet), QHostAddress(serverName), serverPort);
    qDebug() << "Packet 99 sent";
}

void UDPClass::calibration() // calibration
{
    qDebug() << "Entered in calibration()";
    char packet[2];
    packet[1] = 7;
    packet[0] = 7;

    udpsocket->writeDatagram(packet, sizeof(packet), QHostAddress(serverName), serverPort);
    qDebug() << "Packet 77 sent";
}

void UDPClass::closeLoop() // calibration
{
    qDebug() << "Entered in closeLoop()";
    char packet[2];
    packet[1] = 6;
    packet[0] = 6;

    udpsocket->writeDatagram(packet, sizeof(packet), QHostAddress(serverName), serverPort);
    qDebug() << "Packet 66 sent";
}

void UDPClass::stopLoop() // calibration
{
    qDebug() << "Entered in stopLoop()";
    char packet[2];
    packet[1] = 5;
    packet[0] = 5;

    udpsocket->writeDatagram(packet, sizeof(packet), QHostAddress(serverName), serverPort);
    qDebug() << "Packet 55 sent";
}

