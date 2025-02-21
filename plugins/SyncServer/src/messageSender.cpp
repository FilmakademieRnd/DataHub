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

#include "messageSender.h"
#include <iostream>

MessageSender::MessageSender(DataHub::Core* core, QString IPAdress, bool debug, bool webSockets, zmq::context_t* context) :
    ZeroMQHandler(core, IPAdress, debug, webSockets, context)
{
	connect(core, SIGNAL(tickSecondRandom(int)), this, SLOT(createSyncMessage(int)), Qt::DirectConnection);
}

//!
//! Slot to create a new sync message.
//!
//! @param time The current tracer time.
//!
void MessageSender::createSyncMessage(int time)
{
    m_mutex.lock();
    m_syncMessage[0] = m_targetHostID;
    m_syncMessage[1] = time;
    m_syncMessage[2] = MessageType::SYNC;
    m_mutex.unlock();

    std::cout << "\r" << "Time: " << time << " ";

    //m_waitContition->wakeOne();
}

//!
//! The broadcast thread's main worker loop.
//!
void MessageSender::run()
{
	zmq::socket_t sender(*m_context, ZMQ_PUB);
    QString address = m_addressPrefix + m_IPadress + m_addressPortBase + "6";
	sender.bind(address.toLatin1().data());

    startInfo(address);

    while (true) {

        // checks if process should be aborted
        m_mutex.lock();
        bool stop = m_stop;

        if (m_syncMessage[2] != MessageType::EMPTY)
        {
            sender.send(m_syncMessage, 3);
            m_syncMessage[2] = MessageType::EMPTY;
        }

        if (!m_broadcastMessageList.empty())
        {
            zmq::send_multipart(sender, std::move(m_broadcastMessageList));
            m_broadcastMessageList.clear();
        }

        if (!m_messageList.empty()) {
            zmq::send_multipart(sender, std::move(m_messageList));
            m_messageList.clear();
        }

        m_mutex.unlock();
        
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

