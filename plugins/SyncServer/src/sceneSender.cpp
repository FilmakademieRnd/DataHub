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

SceneSender::SceneSender(DataHub::Core* core, QString clientAddress, int fileListNbr, bool debug, zmq::context_t* context)
	: ZeroMQHandler(core, "0.0.0.0", debug, false, context), m_clientAddress(clientAddress), m_fileListNbr(fileListNbr)
{
	//m_sceneData = new SceneDataHandler();
}

SceneSender::~SceneSender()
{
	//m_responses.clear();
	//delete m_sceneData;
}

bool SceneSender::loadData(int fileListNbr)
{
	m_sceneData.readFromDisk("./", m_clientAddress, fileListNbr);

	m_responses.insert("header", &m_sceneData.headerByteData);
	//m_responses.insert("thumbnail", &m_sceneData.thumbnailByteData);  // currently not requested by TRACER (Beware, thread will not stop if activated!)
	m_responses.insert("nodes", &m_sceneData.nodesByteData);
	m_responses.insert("parameterobjects", &m_sceneData.parameterObjectsByteData);
	m_responses.insert("objects", &m_sceneData.objectsByteData);
	m_responses.insert("characters", &m_sceneData.characterByteData);
	m_responses.insert("splats", &m_sceneData.splatByteData);
	m_responses.insert("textures", &m_sceneData.texturesByteData);
	m_responses.insert("materials", &m_sceneData.materialsByteData);

	return true;
}

void SceneSender::run()
{
	if (!loadData(m_fileListNbr)) {
		qInfo("Error loading Scene Files!");
		return;
	}

	zmq::socket_t socket(*m_context, ZMQ_REP);
	//socket.setsockopt(ZMQ_RCVTIMEO, 1000);

	//QString address = "tcp://" + m_IPadress + ":5555";
	QString address = "inproc://TransceiverWorkers";
	socket.connect(address.toLatin1().data());

	startInfo(address);
	int packageCount = 0;

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
			
			qInfo() << "Received request:" << request;
			
			QMap<std::string, QByteArray*>::const_iterator response = m_responses.find(request);
			if (response != m_responses.end())
			{
				QByteArray* dataArray = response.value();
				socket.send(dataArray->data(), dataArray->size());
				packageCount++;
				qInfo() << request << "with size:" << dataArray->size() << + "sended.";
			}
			else {
				qInfo() << "Requested package" << request << "not part of the scene!";
				continue;
			}
		}

		if (packageCount == m_responses.size() || stop)
			break;

		QThread::yieldCurrentThread();
	}

	m_responses.clear();

	m_mutex.lock();
	m_working = false;
	m_mutex.unlock();

	stopInfo(address);

	emit stopped(this);
}