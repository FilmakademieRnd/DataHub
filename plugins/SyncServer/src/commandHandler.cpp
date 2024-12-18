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

CommandHandler::CommandHandler(DataHub::Core* core, MessageSender* messageSender, MessageReceiver *messageReceiver, QString IPAdress, bool debug, zmq::context_t* context) 
	: ZeroMQHandler(core, IPAdress, debug, context), m_sender(messageSender), m_receiver(messageReceiver)
{
	connect(core, SIGNAL(tickSecond(int)), this, SLOT(tickTime(int)), Qt::DirectConnection);
}

void CommandHandler::tickTime(int time)
{
	// increase local time for controlling client timeouts
	m_time++;

	checkPingTimeouts();
}

void CommandHandler::updatePingTimeouts(byte clientID)
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

		char newMessage[6];

		newMessage[0] = m_targetHostID;
		newMessage[1] = m_core->m_time;
		newMessage[2] = MessageReceiver::MessageType::DATAHUB;
		newMessage[3] = 0; // data hub type 0 = connection status update
		newMessage[4] = 1; // new client registered
		newMessage[5] = clientID; // new client ID

		m_sender->QueBroadcastMessage(std::move(zmq::message_t(newMessage, 6)));

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

			char newMessage[6];

			newMessage[0] = m_targetHostID;
			newMessage[1] = m_core->m_time;
			newMessage[2] = MessageReceiver::MessageType::DATAHUB;
			newMessage[3] = 0; // data hub type 0 = connection status update
			newMessage[4] = 0; // client lost
			newMessage[5] = clientID; // new client ID

			m_sender->QueBroadcastMessage(std::move(zmq::message_t(newMessage, 6)));

			qInfo().nospace() << "Lost connection to client:" << clientID;

			//check if client had lock
			m_receiver->CheckLocks(clientID);
		}
	}
	m_mutex.unlock();
}

void CommandHandler::run()
{
	zmq::socket_t socket(*m_context, ZMQ_REP);
	//socket.setsockopt(ZMQ_CONNECT_TIMEOUT, 2000);
	//socket.setsockopt(ZMQ_SNDTIMEO, 2000);
	socket.bind(QString("tcp://" + m_IPadress + ":5558").toLatin1().data());

	qDebug() << "Starting " << metaObject()->className();

	while (true) {
		// checks if process should be aborted
		m_mutex.lock();
		bool stop = m_stop;
		m_mutex.unlock();

		zmq::message_t message;
		socket.recv(&message);

		if (message.size() > 0)
		{
			QByteArray msgArray = QByteArray((char*)message.data(), static_cast<int>(message.size()));

			byte clientID = msgArray[0];
			byte msgTime = msgArray[1];
			byte msgType = msgArray[2];

			switch (msgType)
			{
			case MessageReceiver::MessageType::PING:
			{
				char responseMsg[3];
				responseMsg[0] = m_targetHostID;
				responseMsg[1] = m_core->m_time;
				responseMsg[2] = MessageReceiver::MessageType::PING;

				QThread::msleep(10);
				socket.send(responseMsg, 3);

				updatePingTimeouts(clientID);

				break;
			}
			case MessageReceiver::MessageType::DATAHUB:
			{
				// ...
				break;
			}
			default:
				break;
			}
		}

		if (stop) {
			qDebug() << "Stopping CommandHandler";// in Thread "<<thread()->currentThreadId();
			break;
		}

		QThread::yieldCurrentThread();
	}

	// Set _working to false -> process cannot be aborted anymore
	m_mutex.lock();
	m_working = false;
	m_mutex.unlock();

	qDebug() << "CommandHandler process stopped";// in Thread "<<thread()->currentThreadId();

	emit stopped();
}