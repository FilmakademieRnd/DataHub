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

#include "sceneReceiver.h"

SceneReceiver::SceneReceiver(DataHub::Core* core, QString IPAdress, bool debug, zmq::context_t* context) 
	: ZeroMQHandler(core, IPAdress, debug, false, context)
{
	m_requests = { "header", "nodes", "parameterobjects", "objects", "characters", "textures", "materials" };
}

SceneReceiver::~SceneReceiver()
{
	delete m_sceneData;
}


void SceneReceiver::run()
{
	zmq::socket_t socket(*m_context, ZMQ_REQ);
	socket.setsockopt(ZMQ_SNDTIMEO, 200);

	QString address = "tcp://" + m_IPadress + ":5555";
	socket.bind(address.toLatin1().data());

	m_sceneData = new SceneDataHandler(m_IPadress, "./");

	startInfo(address);

	for (int i = 0; i < m_requests.count(); i++)
	{
		if (!socket.send(zmq::message_t(m_requests[i])))
			continue;

		zmq::message_t recvMessage;
		socket.recv(recvMessage);

		switch (i)
		{
		case 1: // header
			m_sceneData->headerByteData = QByteArray(static_cast<char*>(recvMessage.data()), static_cast<int>(recvMessage.size()));
			break;
		case 2: // nodes
			m_sceneData->nodesByteData = QByteArray(static_cast<char*>(recvMessage.data()), static_cast<int>(recvMessage.size()));
			break;
		case 3: // parameterobjects
			m_sceneData->parameterObjectsByteData = QByteArray(static_cast<char*>(recvMessage.data()), static_cast<int>(recvMessage.size()));
			break;
		case 4: // objects
			m_sceneData->objectsByteData = QByteArray(static_cast<char*>(recvMessage.data()), static_cast<int>(recvMessage.size()));
			break;
		case 5: // characters
			m_sceneData->characterByteData = QByteArray(static_cast<char*>(recvMessage.data()), static_cast<int>(recvMessage.size()));
			break;
		case 6: // textures
			m_sceneData->texturesByteData = QByteArray(static_cast<char*>(recvMessage.data()), static_cast<int>(recvMessage.size()));
			break;
		case 7: // materials
			m_sceneData->materialsByteData = QByteArray(static_cast<char*>(recvMessage.data()), static_cast<int>(recvMessage.size()));
			break;
		}
	}

	if (!m_sceneData->isEmpty())
	{
		//m_sceneData->serverID = m_IPadress;
		m_sceneData->writeToDisk();
	}

	// Set _working to false -> process cannot be aborted anymore
	m_mutex.lock();
	m_working = false;
	m_mutex.unlock();

	stopInfo(address);

	emit stopped(this);
}