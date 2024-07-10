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
Baden-Württemberg, Animationsinstitut (http://research.animationsinstitut.de)".

In case a company or individual would like to use the Data Hub in a commercial
surrounding or for commercial purposes, software based on these components or
any part thereof, the company/individual will have to contact Filmakademie
(research<at>filmakademie.de) for an individual license agreement.
-----------------------------------------------------------------------------
*/

#include "zeroMQHandler.h"

#include <QDebug>

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
	qInfo() << metaObject()->className() << " requested to start"; 
	m_mutex.unlock();
}

void ThreadBase::requestStop()
{
	m_mutex.lock();
	if (m_working) {
		m_stop = true;
		qInfo() << metaObject()->className() << " stopping"; 
	}
	m_mutex.unlock();
}

void BroadcastPoller::run()
{
	while (true) {
		// checks if process should be aborted
		m_mutex.lock();

		bool stop = m_stop;

		//try to receive a zeroMQ message
		zmq::poll(m_item, 1, -1);

		//if (m_item->revents & ZMQ_POLLIN)
			m_waitCondition->wakeOne();

		m_mutex.unlock();

		if (stop) {
			qDebug() << "Stopping " << metaObject()->className();
			break;
		}

		QThread::yieldCurrentThread();
	}

	// Set _working to false -> process cannot be aborted anymore
	m_mutex.lock();
	m_working = false;
	m_mutex.unlock();

	qDebug() << metaObject()->className() << " process stopped"; // in Thread " << thread()->currentThreadId();
}

