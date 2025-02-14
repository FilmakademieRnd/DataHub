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

CommandHandler::CommandHandler(DataHub::Core* core, MessageSender* messageSender, MessageReceiver *messageReceiver, QString IPAdress, bool debug, zmq::context_t* context) 
	: ZeroMQHandler(core, IPAdress, debug, false, context), m_sender(messageSender), m_receiver(messageReceiver)
{
	connect(core, SIGNAL(tickSecond(int)), this, SLOT(tickTime(int)), Qt::DirectConnection);
	connect(core->getPlugin<DataHub::SyncServer*>(), SIGNAL(sceneReceived()), this, SLOT(test()), Qt::DirectConnection);
}

void CommandHandler::tickTime(int time)
{
	// increase local time for controlling client timeouts
	m_time++;

	checkPingTimeouts();
}

void CommandHandler::updatePingTimeouts(byte clientID, bool isServer)
{
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

		char newMessage[7];

		newMessage[0] = m_targetHostID;
		newMessage[1] = m_core->m_time;
		newMessage[2] = MessageReceiver::MessageType::DATAHUB;
		newMessage[3] = CommandHandler::MessageType::CONNECTIONSTATUS;
		newMessage[4] = 1; // new client registered
		newMessage[5] = clientID; // new client ID
		newMessage[6] = isServer; // is the new client a server

		m_sender->QueBroadcastMessage(std::move(zmq::message_t(newMessage, 7)));

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

			char newMessage[7];

			newMessage[0] = m_targetHostID;
			newMessage[1] = m_core->m_time;
			newMessage[2] = MessageReceiver::MessageType::DATAHUB;
			newMessage[3] = CommandHandler::MessageType::CONNECTIONSTATUS;
			newMessage[4] = 0; // client lost
			newMessage[5] = clientID; // new client ID
			newMessage[6] = false; // always false

			m_sender->QueBroadcastMessage(std::move(zmq::message_t(newMessage, 7)));

			qInfo().nospace() << "Lost connection to client:" << clientID;

			//check if client had lock
			m_receiver->CheckLocks(clientID);
		}
	}
	m_mutex.unlock();
}

void CommandHandler::test()
{
	//[SEIM] continue here !!!

	int i = 1;
}

void CommandHandler::run()
{
	zmq::socket_t socket(*m_context, ZMQ_REP);
	//socket.setsockopt(ZMQ_CONNECT_TIMEOUT, 2000);
	socket.setsockopt(ZMQ_RCVTIMEO, 100);
	QString address = "tcp://" + m_IPadress + ":5558";
	socket.bind(address.toLatin1().data());

	DataHub::SyncServer* syncServer = m_core->getPlugin<DataHub::SyncServer*>();

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
			responseMsg[2] = MessageReceiver::MessageType::EMPTY;
			
			QByteArray msgArray = QByteArray((char*)message.data(), static_cast<int>(message.size()));

			byte clientID = msgArray[0];
			byte msgTime = msgArray[1];
			byte msgType = msgArray[2];
			
			switch (msgType)
			{
			case MessageReceiver::MessageType::PING:
			{
				byte msgServer = msgArray[3];
				responseMsg[2] = MessageReceiver::MessageType::PING;

				QThread::msleep(10);
				socket.send(responseMsg, 3);

				updatePingTimeouts(clientID, msgServer);

				break;
			}
			case MessageReceiver::MessageType::DATAHUB:
			{
				responseMsg[2] = MessageReceiver::MessageType::DATAHUB;

				switch (msgArray[3])
				{
				case CommandHandler::MessageType::REQUESTSCENE:
					syncServer->requestScene(m_IPadress.section('.', 0, 2) + "." + QString::number(clientID));
					socket.send(responseMsg, 3);
					break;
				case CommandHandler::MessageType::SENDSCENE:
					//m_core->sceneSend(m_IPadress.section('.', 0, 2) + "." + QString::number(clientID));
					socket.send(responseMsg, 3);
					break;
				}
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

	// Set _working to false -> process cannot be aborted anymore
	m_mutex.lock();
	m_working = false;
	m_mutex.unlock();

	stopInfo(address);

	emit stopped(this);
}