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
to be named as follows: “VPET-Virtual Production Editing Tool by Filmakademie
Baden-Württemberg, Animationsinstitut (http://research.animationsinstitut.de)“.

In case a company or individual would like to use the Scene Distribution and/or
Synchronization Server in a commercial surrounding or for commercial purposes,
software based on these components or any part thereof, the company/individual
will have to contact Filmakademie (research<at>filmakademie.de).
-----------------------------------------------------------------------------
*/

#include "zeroMQHandler.h"

#include <QThread>
#include <QDebug>
#include <iostream>

ThreadBase::ThreadBase(DataHub::Core* core) : m_core(core)
{
	m_stop = false;
	m_working = false;
}

void ThreadBase::requestStart()
{
	m_mutex.lock();
	m_working = true;
	m_stop = false;
	qInfo() << "ZeroMQHandler requested to start"; // in Thread "<<thread()->currentThreadId();
	m_mutex.unlock();
}

void ThreadBase::requestStop()
{
	m_mutex.lock();
	if (m_working) {
		m_stop = true;
		qInfo() << "ZeroMQHandler stopping"; // in Thread "<<thread()->currentThreadId();
	}
	m_mutex.unlock();
}

ZeroMQPoller::ZeroMQPoller(DataHub::Core* core, zmq::pollitem_t* item, QWaitCondition* waitCondition) : ThreadBase(core), m_item(item), m_waitCondition(waitCondition)
{
}

void ZeroMQPoller::run()
{
	while (true) {
		// checks if process should be aborted
		m_mutex.lock();
		bool stop = m_stop;
		m_mutex.unlock();

		//try to receive a zeroMQ message
		zmq::poll(m_item, 1, -1);

		if (m_item->revents & ZMQ_POLLIN)
			m_waitCondition->wakeOne();

		if (stop) {
			qDebug() << "Stopping ZeroMQPoller";// in Thread "<<thread()->currentThreadId();
			break;
		}

		QThread::yieldCurrentThread();
	}

	// Set _working to false -> process cannot be aborted anymore
	m_mutex.lock();
	m_working = false;
	m_mutex.unlock();

	qDebug() << "ZeroMQHandler process stopped";// in Thread "<<thread()->currentThreadId();
}

ZeroMQHandler::ZeroMQHandler(DataHub::Core* core, QString ip, bool debug, zmq::context_t* context) : ThreadBase(core)
{
	m_IPadress = ip;
	m_context = context;
	m_debug = debug;
	m_stop = false;
	m_working = false;

	connect(core, SIGNAL(tickSecond(int)), this, SLOT(createSyncMessage(int)), Qt::DirectConnection);
}

void ZeroMQHandler::createSyncMessage(int time)
{
	m_mutex.lock();
	m_syncMessage[0] = m_targetHostID;
	m_syncMessage[1] = time;
	m_syncMessage[2] = MessageType::SYNC;
	m_mutex.unlock();

	m_waitContition->wakeOne();

	// increase local time for controlling client timeouts
	m_time++;
}

void ZeroMQHandler::run()
{
	zmq::socket_t socket(*m_context, ZMQ_SUB);
	socket.bind(QString("tcp://" + m_IPadress + ":5557").toLatin1().data());
	socket.setsockopt(ZMQ_SUBSCRIBE, "client", 0);

	zmq::socket_t sender(*m_context, ZMQ_PUB);
	sender.bind(QString("tcp://" + m_IPadress + ":5556").toLatin1().data());

	zmq::pollitem_t item = { static_cast<void*>(socket), 0, ZMQ_POLLIN, 0 };

	m_waitContition = new QWaitCondition();
	ZeroMQPoller zeroMQPoller(ZeroMQPoller(m_core, &item, m_waitContition));
	
	m_zeroMQPollerThread = new QThread(this);
	zeroMQPoller.moveToThread(m_zeroMQPollerThread);

	QObject::connect(m_zeroMQPollerThread, &QThread::started, &zeroMQPoller, &ZeroMQPoller::run);

	m_zeroMQPollerThread->start();
	zeroMQPoller.requestStart();

	qDebug() << "Starting ZeroMQHandler";// in Thread " << thread()->currentThreadId();

	while (true) {

		// checks if process should be aborted
		m_mutex.lock();
		bool stop = m_stop;
		m_mutex.unlock();

		m_pauseMutex.lock();
		m_waitContition->wait(&m_pauseMutex);

		//qDebug() << "Thread woked up!";

		//try to receive a zeroMQ message
		//zmq::poll(&item, 1, -1);
		
		zmq::message_t message;

		if (m_syncMessage[2] != MessageType::EMPTY)
		{
			sender.send(m_syncMessage, 3);
			qDebug() << "Time:" << m_syncMessage[1];
			m_syncMessage[2] = MessageType::EMPTY;
		}
		if (item.revents & ZMQ_POLLIN)
		{
			//try to receive a zeroMQ message
			qDebug() << "REC";
			socket.recv(&message);
		}

		//check if recv timed out
		if (message.size() > 0)
		{
			QByteArray msgArray = QByteArray((char*)message.data(), static_cast<int>(message.size()));

			const unsigned char clientID = msgArray[0];
			// char time = msgArray[1];
			const MessageType msgType = static_cast<MessageType>(msgArray[2]);
			//char sceneID = msgArray[3];
			//short sceneObjectID = CharToShort(&msgArray[4]);
			//short parameterID = CharToShort(&msgArray[6]);

			if (m_debug)
			{
				std::cout << "Msg: ";
				std::cout << "cID: " << msgArray[0] << " "; //ClientID
				std::cout << "t: " << msgArray[1] << " "; //Time
				std::cout << "mtype: " << static_cast<MessageType>(msgArray[2]) << " "; //MessageType
				if (msgType == PARAMETERUPDATE)
				{
					std::cout << "sID: " << msgArray[3] << " "; //SceneID
					std::cout << "oID: " << CharToShort(&msgArray[4]) << " "; //SceneObjectID
					std::cout << "pID: " << CharToShort(&msgArray[6]); //ParamID
				}
				std::cout << std::endl;
			}

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
				newMessage[2] = MessageType::DATAHUB;
				newMessage[3] = 0; // data hub type 0 = connection status update
				newMessage[4] = 1; // new client registered
				newMessage[5] = clientID; // new client ID

				message = zmq::message_t(newMessage.constData(), static_cast<size_t>(newMessage.length()));
				sender.send(message);
				
				qInfo() << "New client registered: " << clientID;
			}

			switch (msgType)
			{
				case MessageType::RESENDUPDATE:
				{
					qInfo() << "RESENDING UPDATES";

					QByteArray newMessage((qsizetype)3, Qt::Uninitialized);
					newMessage[0] = m_targetHostID;
					newMessage[1] = m_core->m_time;
					newMessage[2] = MessageType::PARAMETERUPDATE;

					foreach(QByteArray objectState, m_objectStateMap)
					{
						newMessage.append(objectState);
						if (m_debug)
						{
							std::cout << "OutMsg (" << objectState.length() << "):";
							foreach(const char c, objectState)
							{
								std::cout << " " << (int)c;
							}
							std::cout << std::endl;
						}
					}

					message = zmq::message_t(newMessage.constData(), static_cast<size_t>(newMessage.length()));
					sender.send(message);
					break;
				}
				case MessageType::LOCK:
				{
					//store locked object for each client

					QList<QByteArray> lockedIDs = m_lockMap.values(clientID);
					QByteArray newValue = msgArray.sliced(3, 3);

					if (lockedIDs.isEmpty())
					{
						if (msgArray[6])
						{
							m_lockMap.insert(clientID, newValue);
						}
					}
					else
					{
						if (msgArray[6])
						{
							if (lockedIDs.contains(newValue))
							{
								if (m_debug)
									std::cout << "Object " << 1234 << "already locked!";
							}
							else
								m_lockMap.insert(clientID, newValue);
						}
						else
						{
							if (lockedIDs.contains(newValue))
								m_lockMap.remove(clientID, newValue);
							else if (m_debug)
								std::cout << "Unknown Lock release request from client: " << 256 + (int)clientID;
						}
					}

					if (m_debug)
					{
						std::cout << "LockMsg: ";
						std::cout << "cID: " << 256 + msgArray[0] << " "; //ClientID
						std::cout << "t: " << msgArray[1] << " "; //Time
						std::cout << "sID: " << msgArray[3]; //SceneObjectID
						std::cout << "oID: " << CharToShort(&msgArray[4]); //SceneObjectID
						std::cout << "state: " << msgArray[6]; //SceneObjectID
						std::cout << std::endl;
					}
					sender.send(message);
					break;
				}
				case MessageType::PARAMETERUPDATE:
				{
					int start = 3;
					while (start < msgArray.size())
					{
						const int length = msgArray[start + 6];
						m_objectStateMap.insert(msgArray.sliced(start, 5), msgArray.sliced(start, length));
						start += length;
					}
					sender.send(message);
					break;
				}
				case MessageType::UNDOREDOADD:
				case MessageType::RESETOBJECT:
				case MessageType::SYNC:
					sender.send(message);
					break;
				}
		}

		//check if ping timed out for any client
		foreach(const unsigned int time, m_pingMap) {
			if (m_time-time > m_pingTimeout)
			{
				//connection to client lost
				byte clientID = m_pingMap.key(time);
				m_pingMap.remove(clientID);

				QByteArray newMessage((qsizetype)6, Qt::Uninitialized);
				newMessage[0] = m_targetHostID;
				newMessage[1] = m_core->m_time;
				newMessage[2] = MessageType::DATAHUB;
				newMessage[3] = 0; // data hub type 0 = connection status update
				newMessage[4] = 0; // client lost
				newMessage[5] = clientID; // new client ID

				message = zmq::message_t(newMessage.constData(), static_cast<size_t>(newMessage.length()));
				sender.send(message);

				qInfo().nospace() << "Lost connection to: " << (int)clientID;

				//check if client had lock
				QList<QByteArray> values = m_lockMap.values(clientID);
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
						lockReleaseMsg[2] = static_cast<char>(MessageType::LOCK);
						lockReleaseMsg[3] = value[0]; // sID
						//memcpy(lockReleaseMsg + 4, value + 1, 2);
						lockReleaseMsg[4] = value[1]; // oID part1
						lockReleaseMsg[5] = value[2]; // oID part2
						lockReleaseMsg[6] = static_cast<char>(false);

						sender.send(lockReleaseMsg, 7);
						m_lockMap.remove(clientID);
					}
				}
			}
		}

		if (stop) {
			qDebug() << "Stopping ZeroMQHandler";// in Thread "<<thread()->currentThreadId();
			break;
		}
		m_pauseMutex.unlock();

		QThread::yieldCurrentThread();
	}

	//zeroMQPoller.requestStop();
	//m_zeroMQPollerThread->exit();

	// Set _working to false -> process cannot be aborted anymore
	m_mutex.lock();
	m_working = false;
	m_mutex.unlock();

	qDebug() << "ZeroMQHandler process stopped";// in Thread "<<thread()->currentThreadId();

	emit stopped();
}


CommandHandler::CommandHandler(DataHub::Core* core, QString ip, bool debug, zmq::context_t* context) : ThreadBase(core)
{
	m_IPadress = ip;
	m_context = context;
	m_debug = debug;
	m_stop = false;
	m_working = false;
}

void CommandHandler::run()
{
	zmq::socket_t socket(*m_context, ZMQ_REP);
	socket.bind(QString("tcp://" + m_IPadress + ":5558").toLatin1().data());

	qDebug() << "Starting CommandHandler";// in Thread " << thread()->currentThreadId();

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
				case ZeroMQHandler::MessageType::PING:
				{
					char responseMsg[3];
					responseMsg[0] = m_targetHostID;
					responseMsg[1] = m_core->m_time;
					responseMsg[2] = ZeroMQHandler::MessageType::PING;

					socket.send(responseMsg, 3);
					break;
				}
				case ZeroMQHandler::MessageType::DATAHUB:
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
