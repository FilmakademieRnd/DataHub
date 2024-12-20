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
#ifndef ZEROMQHANDLER_H
#define ZEROMQHANDLER_H

#include <QObject>
#include <QMutex>
#include <zmq.hpp>
#include <zmq_addon.hpp>

#include "core.h"

typedef unsigned char byte;


class ZeroMQHandler : public QObject
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
    explicit ZeroMQHandler(DataHub::Core* core, QString IPAdress, bool debug, bool webSockets, zmq::context_t* context) : m_core(core), m_IPadress(IPAdress), m_debug(debug), m_context(context) 
    {
        m_stop = false;
        m_working = false;

        if (webSockets) {
            m_addressPrefix = "ws://";
            m_addressPortBase = ":550";
        }
        else {
            m_addressPrefix = "tcp://";
            m_addressPortBase = ":555";
        }
    }

	//! Tracer message types.
	enum MessageType
    {
        PARAMETERUPDATE, LOCK, // node
        SYNC, PING, RESENDUPDATE, // sync
        UNDOREDOADD, RESETOBJECT, // undo redo
        DATAHUB, // DataHub
        RPC, // RPC
        EMPTY = 255
    };

    //! 
    //! Helper function to convert characters to shorts. 
    //! 
    //! @param buf The char buffer to be converted.
    //! @return The converted and copyed buffer as short.
    //! 
    static inline const short CharToShort(const char* buf) 
    {
        short val;
        std::memcpy(&val, buf, 2);
        return val;
    }

    //! 
    //! Helper function to convert characters to integer. 
    //! 
    //! @param buf The char buffer to be converted.
    //! @return The converted and copyed buffer as integer.
    //! 
    static inline const int CharToInt(const char* buf) 
    {
        int val;
        std::memcpy(&val, buf, 4);
        return val;
    }

public:
    //! Request this process to start working.
    void requestStart()
    {
        m_mutex.lock();
        m_working = true;
        m_stop = false;
        qInfo() << metaObject()->className() << " requested to start";
        m_mutex.unlock();
    }

    //! Request this process to stop working.
    void requestStop()
    {
        m_mutex.lock();
        if (m_working) {
            m_stop = true;
            qInfo() << metaObject()->className() << " stopping";
        }
        m_mutex.unlock();
    }

protected:
    //! ID displayed as clientID for messages redistributed through syncServer.
    byte m_targetHostID = 0;

    //! ZeroMQ context.
    zmq::context_t* m_context;

    //! The server IP adress.
    QString m_IPadress;

    //! Default mutex used to lock the thread.
    QMutex m_mutex;

    //! If true process is stopped.
    bool m_stop;

    //! If true process is running.
    bool m_working;

    //! Shall debug messages be printed.
    bool m_debug;

    QString m_addressPrefix;
    QString m_addressPortBase;

    //! A reference to the DataHub core.
    DataHub::Core* m_core;

signals:
    //!
    //! Signal emitted when process is finished.
    //!
    void stopped();

public slots:
    //! Default thread worker loop.
    virtual void run() = 0;
};


#endif // ZEROMQHANDLER_H
