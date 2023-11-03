/*
-----------------------------------------------------------------------------
This source file is part of VPET - Virtual Production Editing Tool
http://vpet.research.animationsinstitut.de/
http://github.com/FilmakademieRnd/VPET

Copyright (c) 2018 Filmakademie Baden-Wuerttemberg, Animationsinstitut R&D Lab

This project has been initiated in the scope of the EU funded project
Dreamspace under grant agreement no 610005 in the years 2014, 2015 and 2016.
http://dreamspaceproject.eu/
Post Dreamspace the project has been further developed on behalf of the
research and development activities of Animationsinstitut.

The VPET components Scene Distribution and Synchronization Server are intended
for research and development purposes only. Commercial use of any kind is not
permitted.

There is no support by Filmakademie. Since the Scene Distribution and
Synchronization Server are available for free, Filmakademie shall only be
liable for intent and gross negligence; warranty is limited to malice. Scene
Distribution and Synchronization Server may under no circumstances be used for
racist, sexual or any illegal purposes. In all non-commercial productions,
scientific publications, prototypical non-commercial software tools, etc.
using the Scene Distribution and/or Synchronization Server Filmakademie has
to be named as follows: "VPET-Virtual Production Editing Tool by Filmakademie
Baden-Wuerttemberg, Animationsinstitut (http://research.animationsinstitut.de)".

In case a company or individual would like to use the Scene Distribution and/or
Synchronization Server in a commercial surrounding or for commercial purposes,
software based on these components or any part thereof, the company/individual
will have to contact Filmakademie (research<at>filmakademie.de).
-----------------------------------------------------------------------------
*/

#include "CommandHandler.h"

CommandHandler::CommandHandler(DataHub::Core* core, BroadcastHandler* zmqHandler, QString IPAdress, bool debug, zmq::context_t* context) : ZeroMQHandler(core, IPAdress, debug, context), m_zmqHandler(zmqHandler)
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
	auto client = m_pingMap.find(clientID);
	if (client != m_pingMap.end())
	{
		client.value() = m_time;
	}
	else
	{
		m_pingMap.insert(clientID, m_time);

		QByteArray newMessage((qsizetype)6, Qt::Uninitialized);
		newMessage[0] = m_targetHostID;
		newMessage[1] = m_core->m_time;
		newMessage[2] = BroadcastHandler::MessageType::DATAHUB;
		newMessage[3] = 0; // data hub type 0 = connection status update
		newMessage[4] = 1; // new client registered
		newMessage[5] = clientID; // new client ID

		m_zmqHandler->BroadcastMessage(newMessage);

		//message = zmq::message_t(newMessage.constData(), static_cast<size_t>(newMessage.length()));
		//sender.send(message);

		qInfo() << "New client registered: " << clientID;
	}
}
void CommandHandler::checkPingTimeouts()
{
	//check if ping timed out for any client
	foreach(const unsigned int time, m_pingMap)
	{
		if (m_time - time > m_pingTimeout)
		{
			//connection to client lost
			byte clientID = m_pingMap.key(time);
			m_pingMap.remove(clientID);

			QByteArray newMessage((qsizetype)6, Qt::Uninitialized);
			newMessage[0] = m_targetHostID;
			newMessage[1] = m_core->m_time;
			newMessage[2] = BroadcastHandler::MessageType::DATAHUB;
			newMessage[3] = 0; // data hub type 0 = connection status update
			newMessage[4] = 0; // client lost
			newMessage[5] = clientID; // new client ID

			m_zmqHandler->BroadcastMessage(newMessage);

			//message = zmq::message_t(newMessage.constData(), static_cast<size_t>(newMessage.length()));
			//sender.send(message);

			qInfo().nospace() << "Lost connection to: " << (int)clientID;

			//check if client had lock
			m_mutex.lock();
			QList<QByteArray> values = m_zmqHandler->GetLockMap()->values(clientID);
			m_mutex.unlock();

			if (!values.isEmpty())
			{
				//release lock
				qInfo() << "Resetting lock!";
				char lockReleaseMsg[7];
				for (int i = 0; i < values.count(); i++)
				{
					const char* value = values[i].constData();
					lockReleaseMsg[0] = static_cast<char>(m_targetHostID);
					lockReleaseMsg[1] = static_cast<char>(m_core->m_time);  // time
					lockReleaseMsg[2] = static_cast<char>(BroadcastHandler::MessageType::LOCK);
					lockReleaseMsg[3] = value[0]; // sID
					//memcpy(lockReleaseMsg + 4, value + 1, 2);
					lockReleaseMsg[4] = value[1]; // oID part1
					lockReleaseMsg[5] = value[2]; // oID part2
					lockReleaseMsg[6] = static_cast<char>(false);

					m_zmqHandler->BroadcastMessage(lockReleaseMsg);
					//sender.send(lockReleaseMsg, 7);
					m_mutex.lock();
					m_zmqHandler->GetLockMap()->remove(clientID);
					m_mutex.unlock();
				}
			}
		}
	}
}

void CommandHandler::run()
{
	zmq::socket_t socket(*m_context, ZMQ_REP);
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
			case BroadcastHandler::MessageType::PING:
			{
				char responseMsg[3];
				responseMsg[0] = m_targetHostID;
				responseMsg[1] = m_core->m_time;
				responseMsg[2] = BroadcastHandler::MessageType::PING;

				Sleep(100);
				socket.send(responseMsg, 3);

				updatePingTimeouts(clientID);

				break;
			}
			case BroadcastHandler::MessageType::DATAHUB:
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