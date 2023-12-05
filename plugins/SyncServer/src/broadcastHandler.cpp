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

#include "BroadcastHandler.h"
#include <iostream>

BroadcastHandler::BroadcastHandler(DataHub::Core* core, QString IPAdress, bool debug, bool parameterHistory, bool lockHistory, zmq::context_t* context) : 
									m_parameterHistory(parameterHistory), m_lockHistory(lockHistory), ZeroMQHandler(core, IPAdress, debug, context)
{
	connect(core, SIGNAL(tickSecond(int)), this, SLOT(createSyncMessage(int)), Qt::DirectConnection);
}

void BroadcastHandler::createSyncMessage(int time)
{
	m_mutex.lock();
	m_syncMessage[0] = m_targetHostID;
	m_syncMessage[1] = time;
	m_syncMessage[2] = MessageType::SYNC;
	m_mutex.unlock();

	m_waitContition->wakeOne();
}

void BroadcastHandler::BroadcastMessage(QByteArray message)
{
	m_mutex.lock();
	m_broadcastMessage = message;
	m_mutex.unlock();

	m_waitContition->wakeOne();
}

QMultiMap<byte, QByteArray> BroadcastHandler::GetLockMap()
{
	return m_lockMap;
}

void BroadcastHandler::run()
{
	zmq::socket_t socket(*m_context, ZMQ_SUB);
	socket.bind(QString("tcp://" + m_IPadress + ":5557").toLatin1().data());
	socket.setsockopt(ZMQ_SUBSCRIBE, "client", 0);

	zmq::socket_t sender(*m_context, ZMQ_PUB);
	sender.bind(QString("tcp://" + m_IPadress + ":5556").toLatin1().data());

	zmq::pollitem_t item = { static_cast<void*>(socket), 0, ZMQ_POLLIN, 0 };

	m_waitContition = new QWaitCondition();
	BroadcastPoller zeroMQPoller(BroadcastPoller(m_core, &item, m_waitContition));

	m_zeroMQPollerThread = new QThread(this);
	zeroMQPoller.moveToThread(m_zeroMQPollerThread);

	QObject::connect(m_zeroMQPollerThread, &QThread::started, &zeroMQPoller, &BroadcastPoller::run);

	m_zeroMQPollerThread->start();
	zeroMQPoller.requestStart();

	qDebug() << "Starting " << metaObject()->className();

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

		m_mutex.lock();
		if (m_syncMessage[2] != MessageType::EMPTY)
		{
			sender.send(m_syncMessage, 3);
			m_syncMessage[2] = MessageType::EMPTY;
		}
		if (!m_broadcastMessage.isNull() && !m_broadcastMessage.isEmpty())
		{
			sender.send(m_broadcastMessage.constData(), static_cast<size_t>(m_broadcastMessage.length()));
			m_broadcastMessage.clear();
		}
		m_mutex.unlock();
		
		if (item.revents & ZMQ_POLLIN)
		{
			//try to receive a zeroMQ message
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
				if (m_lockHistory)
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
									std::cout << "Object " << newValue << "already locked!";
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
				}
				sender.send(message);
				break;
			}
			case MessageType::PARAMETERUPDATE:
			{
				if (m_parameterHistory)
				{
					int start = 3;
					while (start < msgArray.size())
					{
						const int length = msgArray[start + 6];
						m_objectStateMap.insert(msgArray.sliced(start, 5), msgArray.sliced(start, length));
						start += length;
					}
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

		if (stop) {
			qDebug() << "Stopping " << metaObject()->className();
			break;
		}
		m_pauseMutex.unlock();

		QThread::yieldCurrentThread();
	}

	zeroMQPoller.requestStop();
	m_zeroMQPollerThread->exit();

	// Set _working to false -> process cannot be aborted anymore
	m_mutex.lock();
	m_working = false;
	m_mutex.unlock();

	qDebug() << metaObject()->className() << " process stopped";// in Thread "<<thread()->currentThreadId();

	emit stopped();
}