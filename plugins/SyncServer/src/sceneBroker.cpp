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

#include "sceneBroker.h"

SceneBroker::SceneBroker(DataHub::Core* core, QString clientAddress, bool debug, zmq::context_t* context)
	: ZeroMQHandler(core, "0.0.0.0", debug, false, context), m_clientAddress(clientAddress)
{
}

SceneBroker::~SceneBroker()
{
}


void SceneBroker::run()
{
	QString address = "tcp://" + m_clientAddress + ":5555";
	
	m_sceneClients = zmq::socket_t(*m_context, ZMQ_ROUTER);
	m_sceneClients.bind(address.toLatin1().data());
	//m_sceneClients.setsockopt(ZMQ_LINGER, 0);

	m_sceneWorkers = zmq::socket_t(*m_context, ZMQ_DEALER);
	m_sceneWorkers.bind("inproc://TransceiverWorkers");
	//m_sceneWorkers.setsockopt(ZMQ_LINGER, 0);

	startInfo(address);

	// zmq::proxy only terminates when zmq::context will be closed.
	// It also always leads to an exeption we catch here to be able
	// to cleanup the entire app correctly.

	try
	{
		zmq::proxy(m_sceneWorkers, m_sceneClients, nullptr);
	}
	catch (...) {}
	
	m_mutex.lock();
	m_working = false;
	m_mutex.unlock();

	stopInfo(address);

	emit stopped(this);
}