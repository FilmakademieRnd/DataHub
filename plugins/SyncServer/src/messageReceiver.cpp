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

#include "messageReceiver.h"
#include <iostream>

MessageReceiver::MessageReceiver(DataHub::Core* core, QList<MessageSender*> messageSenders, QString IPAdress, bool debug, bool webSockets, bool parameterHistory, bool lockHistory, zmq::context_t* context) :
									m_senders(messageSenders), m_parameterHistory(parameterHistory), m_lockHistory(lockHistory), ZeroMQHandler(core, IPAdress, debug, webSockets, context)
{
}

void MessageReceiver::CheckLocks(byte clientID)
{
	//check if client had lock
	m_lockMapMutex.lock();
	QList<QByteArray> values = m_lockMap.values(clientID);

	if (!values.isEmpty())
	{
		//release lock
		qInfo() << "Resetting locks!";
		char lockReleaseMsg[7];
		for (int i = 0; i < values.count(); i++)
		{
			const char* value = values[i].constData();
			lockReleaseMsg[0] = static_cast<char>(m_targetHostID);
			lockReleaseMsg[1] = static_cast<char>(m_core->m_time);  // time
			lockReleaseMsg[2] = static_cast<char>(MessageReceiver::MessageType::LOCK);
			lockReleaseMsg[3] = value[0]; // sID
			//memcpy(lockReleaseMsg + 4, value + 1, 2);
			lockReleaseMsg[4] = value[1]; // oID part1
			lockReleaseMsg[5] = value[2]; // oID part2
			lockReleaseMsg[6] = static_cast<char>(false);

			QueBroadcastMessage(std::move(zmq::message_t(lockReleaseMsg, 7)));
			m_lockMap.remove(clientID);
		}
	}
	m_lockMapMutex.unlock();
}


void MessageReceiver::run()
{
	zmq::socket_t socket(*m_context, ZMQ_SUB);
	socket.setsockopt(ZMQ_SUBSCRIBE, "client", 0);
	socket.setsockopt(ZMQ_RCVTIMEO, 100);
	QString address = m_addressPrefix + m_IPadress + m_addressPortBase + "7";
	socket.bind(address.toLatin1().data());

	zmq::pollitem_t item = { static_cast<void*>(socket), 0, ZMQ_POLLIN, 0 };

	startInfo(address);

	while (true) {

		// checks if process should be aborted
		m_mutex.lock();
		bool stop = m_stop;
		
		zmq::multipart_t messages;

		//try to receive a zeroMQ message
		zmq::recv_multipart(socket, std::back_inserter(messages), zmq::recv_flags::none);
		m_mutex.unlock();

		for (auto messageIter = messages.begin(); messageIter != messages.end(); messageIter++)
		{
			QByteArray msgArray = QByteArray((char*)messageIter->data(), static_cast<int>(messageIter->size()));
			//QByteArray msgArray = QByteArray(static_cast<qsizetype>(messageIter->size()), Qt::Uninitialized);
			//memcpy(msgArray.data(), messageIter->data(), messageIter->size());

			const unsigned char clientID = msgArray[0];
			// char time = msgArray[1];
			const MessageType msgType = static_cast<MessageType>(msgArray[2]);
			//char sceneID = msgArray[3];
			//short sceneObjectID = CharToShort(&msgArray[4]);
			//short parameterID = CharToShort(&msgArray[6]);

			if (m_debug)
			{
				if (msgType == RPC)
				{
					std::cout << "RPCMsg: ";
					std::cout << "cID: " << (int)(char)msgArray[0] + 256 << " "; //ClientID
					std::cout << "t: " << (unsigned int)msgArray[1] << " "; //Time
					//std::cout << "mtype: " << static_cast<MessageType>(msgArray[2]) << " "; //MessageType
					std::cout << "sID: " << (int)(char)msgArray[3] + 256 << " "; //SceneID
					std::cout << "oID: " << CharToShort(&msgArray[4]) << " "; //SceneObjectID
					std::cout << "pID: " << CharToShort(&msgArray[6]); //ParamID
					std::cout << std::endl;
				}
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

				QueMessage(std::move(zmq::message_t(newMessage.data(), newMessage.size())));
				break;
			}
			case MessageType::LOCK:
			{
				if (m_lockHistory)
				{
					m_lockMapMutex.lock();
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
									std::cout << "Object " << 256 + (int)(char)msgArray[6] << "already locked!" << std::endl;
							}
							else
								m_lockMap.insert(clientID, newValue);
						}
						else
						{
							if (lockedIDs.contains(newValue))
								m_lockMap.remove(clientID, newValue);
							else if (m_debug)
								std::cout << "Unknown Lock release request from client: " << 256 + (int)clientID << std::endl;
						}
					}
					m_lockMapMutex.unlock();

					if (m_debug)
					{
						std::cout << "LockMsg: ";
						std::cout << "cID: " << (int)(char)msgArray[0] + 256 << " "; //ClientID
						std::cout << "t: " << (unsigned int)msgArray[1] << " "; //Time
						std::cout << "sID: " << (int)(char)msgArray[3] + 256 << " "; //SceneObjectID
						std::cout << "oID: " << CharToShort(&msgArray[4]) << " "; //SceneObjectID
						std::cout << "state: " << (unsigned int)msgArray[6]; //lock state
						std::cout << std::endl;
					}
				}
				QueMessage(std::move(*messageIter));
				break;
			}
			case MessageType::PARAMETERUPDATE:
			{
				if (m_parameterHistory)
				{
					int start = 3;
					while (start < msgArray.size())
					{
						const int length = CharToInt(msgArray.sliced(start + 6, 4));
						
						if (!m_objectStateMap.contains(msgArray.sliced(start, 5)))
							m_objectStateMap.insert(msgArray.sliced(start, 5), msgArray.sliced(start, length));
						
						start += length;
					}
				}
				QueMessage(std::move(*messageIter));
				break;
			}
			case MessageType::SYNC:
			case MessageType::UNDOREDOADD:
			case MessageType::RESETOBJECT:
			case MessageType::RPC:
				QueMessage(std::move(*messageIter));
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
