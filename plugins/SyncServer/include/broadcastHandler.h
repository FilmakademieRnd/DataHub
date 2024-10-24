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
#ifndef BROADCASTHANDLER_H
#define BROADCASTHANDLER_H

#include "zeroMQHandler.h"
#include <iostream>
#include <QMultiMap>


class BroadcastHandler : public ZeroMQHandler
{
    Q_OBJECT
public:
    //! 
    //! Constructor
    //! 
    //! @param core A reference to the DataHub core.
    //! @param IPAdress The IP adress the BroadcastHandler shall connect to. 
    //! @param debug Flag determin wether debug informations shall be printed.
    //! @param context The ZMQ context used by the BroadcastHandler.
    //! 
    explicit BroadcastHandler(DataHub::Core* core, QString IPAdress = "", bool debug = false, bool parameterHistory = true, bool lockHistory = true, zmq::context_t* context = NULL);

    //~BroadcastHandler()
    //{
    //    m_sender->close();
    //}

private:

    bool m_parameterHistory;
    bool m_lockHistory;

    zmq::socket_t *m_sender;

    //! Mutex used for pausing the BroadcastHandler thread.
    QMutex m_pauseMutex;
    QMutex m_messageListMutex;

    //! Wait condition used for pausing the BroadcastHandler thread.
    QWaitCondition* m_waitContition;

    //! Co thread for polling.
    QThread* m_zeroMQPollerThread;

    //! Buffer used for broadcasting sync messages.
    byte m_syncMessage[3] = { m_targetHostID,0,MessageType::EMPTY };

    //! Buffer used for broadcasting general purpose messages.
    QByteArray m_broadcastMessage;

    //! The map storing last scene object states. 
    QMap<QByteArray, QByteArray> m_objectStateMap;

    //! The map storing scene objects lock status. 
    QMultiMap<byte, QByteArray> m_lockMap;

    // client | bundeles | bundle | more & message
    QMap<byte, QList<QList<QPair<bool, QByteArray>>>> m_messageMap;

    //! 
    //! Helper function to convert characters to shorts. 
    //! 
    //! @param buf The char buffer to be converted.
    //! @return The converted and copyed buffer as short.
    //! 
    inline const short CharToShort(const char* buf) const
    {
        short val;
        std::memcpy(&val, buf, 2);
        return val;
    }

    inline void QueMessage(QByteArray message, bool more)
    {
        m_messageListMutex.lock();
        // CLIENT //
        const byte clienID = message[0];
        QList<QList<QPair<bool, QByteArray>>> &clientMessageBundles = m_messageMap[clienID];
        //qDebug() << clientMessageBundles.size();
        //qDebug() << more;
        
        // MESSAGE BUNDLES //
        if (clientMessageBundles.isEmpty()) {
            clientMessageBundles.append(
                QList< QPair<bool, QByteArray> >( {QPair< bool, QByteArray >(more, message)} )
            );
        }
		else {
            QList<QPair<bool, QByteArray>> &lastMessageBundle = clientMessageBundles.last();

			// MESSAGE //
			//if last message in bundle has more tag
            if (lastMessageBundle.last().first) {
                lastMessageBundle.append(QPair<bool, QByteArray>(more, message));
                //qDebug() << "more";
            }
            else {
                clientMessageBundles.append(
                    QList< QPair<bool, QByteArray> >({ QPair<bool, QByteArray>(more, message) })
                );
                //qDebug() << "not more";
            }
		}

        m_messageListMutex.unlock();
    }

public:
    //!
    //! Function to broadcast a messagte to all connected clients.
    //!
    //! @param message The message to be broadcasted. 
    //!
    void BroadcastMessage(QByteArray message);

    void ClientLost(byte clientID);

    //!
    //! Function to get a reference to the Lock map.
    //!
    //! @return The reference to the lockMap.
    //!
    QMultiMap<byte, QByteArray> GetLockMap();

signals:
    //!
    //! Signal emitted when process is finished.
    //!
    void stopped();

public slots:
    //!
    //! The broadcast thread's main worker loop.
    //!
    void run();

private slots:
    //!
    //! Slot to create a new sync message.
    //!
    //! @param time The current tracer time.
    //!
    inline void createSyncMessage(int time)
    {
        m_mutex.lock();
        m_syncMessage[0] = m_targetHostID;
        m_syncMessage[1] = time;
        m_syncMessage[2] = MessageType::SYNC;
        m_mutex.unlock();

        std::cout << "\r" << "Time: " << time << " ";

        m_waitContition->wakeOne();
    }

    inline void SendMessages()
    {
        m_mutex.lock();
        m_messageListMutex.lock();

        if (!m_messageMap.isEmpty())
        {
            for (auto clientIt = m_messageMap.begin(); clientIt != m_messageMap.end(); clientIt++)
            {
                for (auto bundlesIt = clientIt->begin(); bundlesIt != clientIt->end();)
                {
                    if (bundlesIt->last().first) {
                        ++bundlesIt;
                    }
                    else {
                        for (auto bundleIt = bundlesIt->begin(); bundleIt != bundlesIt->end(); bundleIt++)
                            m_sender->send(bundleIt->second.data(), bundleIt->second.size(), bundleIt->first * ZMQ_SNDMORE);
                        bundlesIt = clientIt->erase(bundlesIt);
                        //bundlesIt->clear();
                    }
                }
            }
        }

        //if (count > 0)
        //{
        //    if (count > 1)
        //    {
        //        for (int i = 0; i < count - 1; i++)
        //        {
        //            QByteArray message = m_messageQue[i];
        //            m_sender->send(message.data(), message.size(), ZMQ_SNDMORE);
        //        }
        //        QByteArray message = m_messageQue.last();
        //        m_sender->send(message.data(), message.size(), 0);
        //    }
        //    else
        //    {
        //        QByteArray message = m_messageQue[0];
        //        m_sender->send(message.data(), message.size(), 0);
        //    }
        //    //for (int i = 0; i < count; i++)
        //      //  m_messageQue[i].clear();
        //    m_messageQue.clear();
        //}

        m_messageListMutex.unlock();
        m_mutex.unlock();
    }
};


#endif // BROADCASTHANDLER_H