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

SceneReceiver::SceneReceiver(DataHub::Core* core, QString clientAddress, bool debug, zmq::context_t* context)
	: ZeroMQHandler(core, "0.0.0.0", debug, false, context), m_clientAddress(clientAddress)
{
	m_sceneData = new SceneDataHandler();
	m_packageNames = { "header", "thumbnail" , "nodes", "parameterobjects", "objects", "characters", "textures", "materials"};
}

SceneReceiver::~SceneReceiver()
{
	delete m_sceneData;
}


void SceneReceiver::run()
{
	zmq::socket_t socket(*m_context, ZMQ_REP);
	socket.setsockopt(ZMQ_RCVTIMEO, 1000);

	//QString address = "tcp://" + m_IPadress + ":5555";
	QString address = "inproc://TransceiverWorkers";
	socket.connect(address.toLatin1().data());

	startInfo(address);

	zmq::multipart_t messages;

	int packageCount = 0;
	while (true)
	{
		m_mutex.lock();
		bool stop = m_stop;
		m_mutex.unlock();

		if (zmq::recv_multipart(socket, std::back_inserter(messages), zmq::recv_flags::none).has_value())
		{
			QString pkName = QString::fromStdString(messages[0].to_string());

			qInfo() << "Package:" << pkName << "received with" << messages[1].size() << "bytes.";

			switch (m_packageNames.indexOf(pkName))
			{
			case 0: // header
				m_sceneData->headerByteData = toByteArray(messages[1]);
				packageCount++;
				break;
			case 1: // header
				m_sceneData->thumbnailByteData = toByteArray(messages[1]);
				packageCount++;
				break;
			case 2: // nodes
				m_sceneData->nodesByteData = toByteArray(messages[1]);
				packageCount++;
				break;
			case 3: // parameterobjects
				m_sceneData->parameterObjectsByteData = toByteArray(messages[1]);
				packageCount++;
				break;
			case 4: // objects
				m_sceneData->objectsByteData = toByteArray(messages[1]);
				packageCount++;
				break;
			case 5: // characters
				m_sceneData->characterByteData = toByteArray(messages[1]);
				packageCount++;
				break;
			case 6: // textures
				m_sceneData->texturesByteData = toByteArray(messages[1]);
				packageCount++;
				break;
			case 7: // materials
				m_sceneData->materialsByteData = toByteArray(messages[1]);
				packageCount++;
				break;
			default:
				qInfo() << "Unknown scene package" << pkName << "received! Package skipped.";
				socket.send(zmq::message_t("0"));
				continue;
			}
			
			socket.send(zmq::message_t("1"));
			messages.clear();

			if (packageCount == m_packageNames.size())
			{
				qInfo("Scene complete, write Files to Disk!");
				m_sceneData->writeToDisk("./", m_clientAddress, QDateTime::currentDateTime().toString("dd-MM-yyyy_hh-mm-ss"));
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