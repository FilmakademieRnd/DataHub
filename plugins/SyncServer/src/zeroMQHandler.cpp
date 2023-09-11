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

ZeroMQHandler::ZeroMQHandler(DataHub::Core* core, QString ip, bool debug, zmq::context_t* context)
{
	IPadress = ip;
	context_ = context;
	_debug = debug;
	_stop = false;
	_working = false;
	_core = core;

	connect(core, SIGNAL(tickSecond(int)), this, SLOT(createSyncMessage(int)), Qt::DirectConnection);
}

void ZeroMQHandler::requestStart()
{
	mutex.lock();
	_working = true;
	_stop = false;
	qInfo() << "ZeroMQHandler requested to start"; // in Thread "<<thread()->currentThreadId();
	mutex.unlock();

	emit startRequested();
}

void ZeroMQHandler::requestStop()
{
	mutex.lock();
	if (_working) {
		_stop = true;
		qInfo() << "ZeroMQHandler stopping"; // in Thread "<<thread()->currentThreadId();
	}
	mutex.unlock();
}

void ZeroMQHandler::createSyncMessage(int time)
{
	syncMessage[0] = targetHostID;
	syncMessage[1] = time;
	syncMessage[2] = MessageType::SYNC;

	// increase local time for controlling client timeouts
	m_time++;
}

void ZeroMQHandler::run()
{
	socket_ = new zmq::socket_t(*context_, ZMQ_SUB);
	socket_->bind(QString("tcp://" + IPadress + ":5557").toLatin1().data());
	socket_->setsockopt(ZMQ_SUBSCRIBE, "client", 0);

	sender_ = new zmq::socket_t(*context_, ZMQ_PUB);
	sender_->bind(QString("tcp://" + IPadress + ":5556").toLatin1().data());

	zmq::pollitem_t item = { static_cast<void*>(*socket_), 0, ZMQ_POLLIN, 0 };

	qDebug() << "Starting ZeroMQHandler";// in Thread " << thread()->currentThreadId();

	while (true) {

		// checks if process should be aborted
		mutex.lock();
		bool stop = _stop;
		mutex.unlock();

		zmq::message_t message;

		//try to receive a zeroMQ message
		zmq::poll(&item, 1, -1);

		if (item.revents & ZMQ_POLLIN)
		{
			//try to receive a zeroMQ message
			socket_->recv(&message);
		}

		//check if recv timed out
		if (message.size() != 0)
		{
			QByteArray msgArray = QByteArray((char*)message.data(), static_cast<int>(message.size()));

			const unsigned char clientID = msgArray[0];
			// char time = msgArray[1];
			const MessageType msgType = static_cast<MessageType>(msgArray[2]);
			//char sceneID = msgArray[3];
			//short sceneObjectID = CharToShort(&msgArray[4]);
			//short parameterID = CharToShort(&msgArray[6]);

			if (_debug)
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
			auto client = pingMap.find(clientID);
			if (client != pingMap.end())
			{
				client.value() = m_time;
			}
			else
			{
				pingMap.insert(clientID, m_time);
				
				QByteArray newMessage((qsizetype)6, Qt::Uninitialized);
				newMessage[0] = targetHostID;
				newMessage[1] = _core->m_time;
				newMessage[2] = MessageType::DATAHUB;
				newMessage[3] = 0; // data hub type 0 = connection status update
				newMessage[4] = 1; // new client registered
				newMessage[5] = clientID; // new client ID

				message = zmq::message_t(newMessage.constData(), static_cast<size_t>(newMessage.length()));
				sender_->send(message);
				
				qInfo() << "New client registered: " << clientID;
			}

			switch (msgType)
			{
				case MessageType::RESENDUPDATE:
				{
					qInfo() << "RESENDING UPDATES";

					QByteArray newMessage((qsizetype)3, Qt::Uninitialized);
					newMessage[0] = targetHostID;
					newMessage[1] = _core->m_time;
					newMessage[2] = MessageType::PARAMETERUPDATE;

					foreach(QByteArray objectState, objectStateMap)
					{
						newMessage.append(objectState);
						if (_debug)
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
					sender_->send(message);
					break;
				}
				case MessageType::LOCK:
				{
					//store locked object for each client

					QList<QByteArray> lockedIDs = lockMap.values(clientID);
					QByteArray newValue = msgArray.sliced(3, 3);

					if (lockedIDs.isEmpty())
					{
						if (msgArray[6])
						{
							lockMap.insert(clientID, newValue);
						}
					}
					else
					{
						if (msgArray[6])
						{
							if (lockedIDs.contains(newValue))
							{
								if (_debug)
									std::cout << "Object " << 1234 << "already locked!";
							}
							else
								lockMap.insert(clientID, newValue);
						}
						else
						{
							if (lockedIDs.contains(newValue))
								lockMap.remove(clientID, newValue);
							else if (_debug)
								std::cout << "Unknown Lock release request from client: " << 256 + (int)clientID;
						}
					}

					if (_debug)
					{
						std::cout << "LockMsg: ";
						std::cout << "cID: " << 256 + msgArray[0] << " "; //ClientID
						std::cout << "t: " << msgArray[1] << " "; //Time
						std::cout << "sID: " << msgArray[3]; //SceneObjectID
						std::cout << "oID: " << CharToShort(&msgArray[4]); //SceneObjectID
						std::cout << "state: " << msgArray[6]; //SceneObjectID
						std::cout << std::endl;
					}
					sender_->send(message);
					break;
				}
				case MessageType::PARAMETERUPDATE:
				{
					int start = 3;
					while (start < msgArray.size())
					{
						const int length = msgArray[start + 6];
						objectStateMap.insert(msgArray.sliced(start, 5), msgArray.sliced(start, length));
						start += length;
					}
					sender_->send(message);
					break;
				}
				case MessageType::UNDOREDOADD:
				case MessageType::RESETOBJECT:
				case MessageType::SYNC:
					sender_->send(message);
					break;
				}
		}

		if (syncMessage[2] != MessageType::EMPTY)
		{
			//sender_->send(syncMessage, 3);
			//syncMessage[2] = MessageType::EMPTY;
		}

		//check if ping timed out for any client
		foreach(unsigned int time, pingMap) {
			if (m_time-time > m_pingTimeout)
			{
				//connection to client lost
				byte clientID = pingMap.key(time);
				pingMap.remove(clientID);

				QByteArray newMessage((qsizetype)6, Qt::Uninitialized);
				newMessage[0] = targetHostID;
				newMessage[1] = _core->m_time;
				newMessage[2] = MessageType::DATAHUB;
				newMessage[3] = 0; // data hub type 0 = connection status update
				newMessage[4] = 0; // client lost
				newMessage[5] = clientID; // new client ID

				message = zmq::message_t(newMessage.constData(), static_cast<size_t>(newMessage.length()));
				sender_->send(message);

				qInfo().nospace() << "Lost connection to: " << (int)clientID;

				//check if client had lock
				QList<QByteArray> values = lockMap.values(clientID);
				if (!values.isEmpty())
				{
					//release lock
					qInfo() << "Resetting lock!";
					char lockReleaseMsg[7];
					for (int i = 0; i < values.count(); i++)
					{
						const char* value = values[i].constData();
						lockReleaseMsg[0] = static_cast<char>(targetHostID);
						lockReleaseMsg[1] = static_cast<char>(_core->m_time);  // time
						lockReleaseMsg[2] = static_cast<char>(MessageType::LOCK);
						lockReleaseMsg[3] = value[0]; // sID
						//memcpy(lockReleaseMsg + 4, value + 1, 2);
						lockReleaseMsg[4] = value[1]; // oID part1
						lockReleaseMsg[5] = value[2]; // oID part2
						lockReleaseMsg[6] = static_cast<char>(false);

						sender_->send(lockReleaseMsg, 7);
						lockMap.remove(clientID);
					}
				}
			}
		}

		if (stop) {
			qDebug() << "Stopping ZeroMQHandler";// in Thread "<<thread()->currentThreadId();
			break;
		}
	}

	// Set _working to false -> process cannot be aborted anymore
	mutex.lock();
	_working = false;
	mutex.unlock();

	qDebug() << "ZeroMQHandler process stopped";// in Thread "<<thread()->currentThreadId();

	emit stopped();
}
