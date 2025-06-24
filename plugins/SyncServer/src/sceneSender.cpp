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

#include "sceneSender.h"

SceneSender::SceneSender(DataHub::Core* core, QString serverAddress, QString clientAddress, bool debug, zmq::context_t* context)
	: ZeroMQHandler(core, serverAddress, debug, false, context), m_clientAddress(clientAddress)
{
	m_sceneData = new SceneDataHandler();
}

SceneSender::~SceneSender()
{
	delete m_sceneData;
}

bool SceneSender::loadData()
{
	m_sceneData->readFromDisk("./", m_clientAddress, 0 /*file list entry number*/);

	if (m_sceneData->isEmpty())
		false;

	m_responses.insert("header", &m_sceneData->headerByteData);
	m_responses.insert("nodes", &m_sceneData->nodesByteData);
	m_responses.insert("parameterobjects", &m_sceneData->parameterObjectsByteData);
	m_responses.insert("objects", &m_sceneData->objectsByteData);
	m_responses.insert("characters", &m_sceneData->characterByteData);
	m_responses.insert("textures", &m_sceneData->texturesByteData);
	m_responses.insert("materials", &m_sceneData->materialsByteData);

	return true;
}


void SceneSender::run()
{
	if (!loadData()) {
		qInfo("Error loading Scene Files!");
		return;
	}

	zmq::socket_t socket(*m_context, ZMQ_REP);
	//socket.setsockopt(ZMQ_RCVTIMEO, 100);

	QString address = "tcp://" + m_IPadress + ":5555";
	socket.bind(address.toLatin1().data());

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
			std::string request = message.to_string();
			
			qInfo() << "Received request: " << request;
			
			auto response = m_responses.find(request);
			if (response != m_responses.end())
			{
				const QByteArray* dataArray = response.value();
				socket.send(zmq::message_t(dataArray->data(), dataArray->size()));

				qInfo() << request << " with size: " << dataArray->size() << + " sended.";
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