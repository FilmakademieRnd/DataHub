/*
-----------------------------------------------------------------------------
Copyright (c) 2024 Filmakademie Baden-Wuerttemberg, Animationsinstitut R&D Labs
https://research.animationsinstitut.de/datahub
https://github.com/FilmakademieRnd/DataHub

Datahub is a development by Filmakademie Baden-Wuerttemberg, Animationsinstitut
R&D Labs in the scope of the EU funded project MAX-R (101070072) and funding on
the own behalf of Filmakademie Baden-Wuerttemberg.  Former EU projects Dreamspace
(610005) and SAUCE (780470) have inspired the DataHub development.

The DataHub is intended for research and development purposes only.
Commercial use of any kind is not permitted.

There is no support by Filmakademie. Since the Data Hub is available for free,
Filmakademie shall only be liable for intent and gross negligence; warranty
is limited to malice. DataHub may under no circumstances be used for racist,
sexual or any illegal purposes. In all non-commercial productions, scientific
publications, prototypical non-commercial software tools, etc. using the DataHub
Filmakademie has to be named as follows: "DataHub by Filmakademie
Baden-Wuerttemberg, Animationsinstitut (http://research.animationsinstitut.de)".

In case a company or individual would like to use the Data Hub in a commercial
surrounding or for commercial purposes, software based on these components or
any part thereof, the company/individual will have to contact Filmakademie
(research<at>filmakademie.de) for an individual license agreement.
-----------------------------------------------------------------------------
*/

#include "CommandHandler.h"
#include "../SyncServer.h"
#include "sceneDataHandler.h"
#include <iostream>


using namespace DataHub;

CommandHandler::CommandHandler(Core* core, MessageSender* messageSender, MessageReceiver *messageReceiver, QString IPAdress, bool debug, zmq::context_t* context) 
	: ZeroMQHandler(core, IPAdress, debug, false, context), m_sender(messageSender), m_receiver(messageReceiver)
{
	connect(core, SIGNAL(tickSecond(int)), this, SLOT(tickTime(int)), Qt::DirectConnection);
	connect(core->getPlugin<SyncServer*>(), SIGNAL(broadcastSceneReceived(QString)), this, SLOT(broadcastSceneReceived(QString)), Qt::DirectConnection);
}

void CommandHandler::tickTime(int time)
{
	// increase local time for controlling client timeouts
	m_time++;
	std::cout << "\r" << "Time " << QDateTime::fromSecsSinceEpoch(m_time).toString("mm:ss").toStdString() << " ";

	checkPingTimeouts();
}

void CommandHandler::updatePingTimeouts(byte clientID, bool isServer)
{
	if (clientID > 250)
		return;

	//update ping timeout
	m_mutex.lock();
	auto client = m_pingMap.find(clientID);
	if (client != m_pingMap.end())
	{
		client.value() = m_time;
	}
	else
	{
		m_pingMap.insert(clientID, m_time);
		
		broadcastConnectionStatusUpdate(true, clientID, isServer);

		qInfo() << "New client registered:" << clientID;
	}
	m_mutex.unlock();
}

void CommandHandler::checkPingTimeouts()
{
	//check if ping timed out for any client
	m_mutex.lock();
	foreach(const unsigned int time, m_pingMap.values())
	{
		if (m_time - time > m_pingTimeout)
		{
			//connection to client lost
			byte clientID = m_pingMap.key(time);
			m_pingMap.remove(clientID);

			broadcastConnectionStatusUpdate(false, clientID, false);

			qInfo().nospace() << "Lost connection to client: " << clientID;

			SyncServer::removeClient(clientID);

			//check if client had lock
			m_receiver->CheckLocks(clientID);
		}
	}
	m_mutex.unlock();
}

void CommandHandler::broadcastConnectionStatusUpdate(bool newClient, byte clientID, bool isServer)
{
	char newMessage[7];

	newMessage[0] = m_targetHostID;
	newMessage[1] = m_core->m_time;
	newMessage[2] = ZeroMQHandler::MessageType::DATAHUB;
	newMessage[3] = CommandHandler::MessageType::CONNECTIONSTATUS;
	newMessage[4] = newClient; // client new or lost
	newMessage[5] = clientID; // new client ID
	newMessage[6] = isServer; // is the new client a server

	m_sender->QueBroadcastMessage(std::move(zmq::message_t(newMessage, 7)));
}

void CommandHandler::broadcastSceneReceived(QString senderIP)
{
	m_mutex.lock();

	char newMessage[5];

	newMessage[0] = m_targetHostID;
	newMessage[1] = m_core->m_time;
	newMessage[2] = ZeroMQHandler::MessageType::DATAHUB;
	newMessage[3] = CommandHandler::MessageType::SCENERECEIVED;
	newMessage[4] = senderIP.section('.', 3, 3).toInt();

	m_sender->QueBroadcastMessage(std::move(zmq::message_t(newMessage, 5)));

	m_mutex.unlock();
}

void CommandHandler::handlePingMessage(QByteArray& commandMessage, char* responseMessage)
{
	byte msgServer = 0;

	if (commandMessage.size() > 3)
		byte msgServer = commandMessage[3];

	responseMessage[2] = CommandHandler::MessageType::PING;

	QThread::msleep(10);

	updatePingTimeouts(commandMessage[0], msgServer);
}

void CommandHandler::handleFileInfoMessage(QByteArray& commandMessage, char* responseMessage, zmq::multipart_t& multiResponseMessage)
{
	responseMessage[2] = CommandHandler::MessageType::FILEINFO;
	QList<QStringList> fileInfo = SceneDataHandler::infoFromDisk("./", m_IPadress.section('.', 0, 2) + "." + QString::number(commandMessage[0]));
	multiResponseMessage.add(zmq::message_t(responseMessage, 3));
	
	foreach(const QString & scenePartVersion, fileInfo[0])
		multiResponseMessage.addstr(scenePartVersion.toStdString());
}

void CommandHandler::handleIPMessage(QByteArray& commandMessage, char* responseMessage, zmq::multipart_t& multiResponseMessage)
{
	responseMessage[2] = CommandHandler::MessageType::ID;
	
	byte cID =  SyncServer::addClient(SyncServer::macToInt(commandMessage[3], commandMessage[4], commandMessage[5], commandMessage[6], commandMessage[7], commandMessage[8]));
	qDebug().nospace() << "Client: " << cID << " checked" << " with MAC: " << QByteArray(commandMessage.sliced(3), 6).toHex(':');
	
	multiResponseMessage.add(zmq::message_t(responseMessage, 3));
	multiResponseMessage.add(zmq::message_t(&cID, 1));
}

void CommandHandler::run()
{
	zmq::socket_t socket(*m_context, ZMQ_REP);
	//socket.setsockopt(ZMQ_CONNECT_TIMEOUT, 2000);
	socket.setsockopt(ZMQ_RCVTIMEO, 100);
	QString address = "tcp://" + m_IPadress + ":5558";
	socket.bind(address.toLatin1().data());

	SyncServer* syncServer = m_core->getPlugin<SyncServer*>();

	startInfo(address);

	while (true) {
		// checks if process should be aborted
		m_mutex.lock();
		bool stop = m_stop;
		m_mutex.unlock();

		zmq::message_t message;
		socket.recv(&message);

		if (message.size() > 0)
		{
			char responseMsg[3];
			responseMsg[0] = m_targetHostID;
			responseMsg[1] = m_core->m_time;
			responseMsg[2] = CommandHandler::MessageType::UNKNOWN;

			QByteArray msgArray = QByteArray((char*)message.data(), static_cast<int>(message.size()));

			byte clientID = msgArray[0];
			byte msgTime = msgArray[1];
			byte msgType = msgArray[2];

			switch (msgType)
			{
				case CommandHandler::MessageType::PING:
				{
					handlePingMessage(msgArray, responseMsg);
					socket.send(responseMsg, 3);

					break;
				}
				case CommandHandler::MessageType::REQUESTSCENE:
					syncServer->requestScene(clientID);
					socket.send(responseMsg, 3);
					break;
				case CommandHandler::MessageType::SENDSCENE:
					syncServer->sendScene(m_IPadress, clientID);
					socket.send(responseMsg, 3);
					break;
				case CommandHandler::MessageType::FILEINFO:
				{
					zmq::multipart_t fileInfoReply;
					handleFileInfoMessage(msgArray, responseMsg, fileInfoReply);
					zmq::send_multipart(socket, fileInfoReply);
					break;
				}
				case CommandHandler::MessageType::ID:
				{
					zmq::multipart_t ipReply;
					handleIPMessage(msgArray, responseMsg, ipReply);
					zmq::send_multipart(socket, ipReply);
					break;
				}
				default:
					break;
			}
		}

		if (stop) {
			break;
		}

		QThread::yieldCurrentThread();
	}

	m_mutex.lock();
	m_working = false;
	m_mutex.unlock();

	stopInfo(address);

	emit stopped(this);
}