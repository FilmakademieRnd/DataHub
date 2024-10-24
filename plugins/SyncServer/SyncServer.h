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

//! @file "SyncServer.h"
//! @brief Datahub Plugin: Sync Server defines the network bridge between TRACER clients and servers.
//! @author Simon Spielmann
//! @version 1
//! @date 03.07.2023

#ifndef SYNCSERVER_H
#define SYNCSERVER_H

#include <QThread>
#include <QMutex>
#include "plugininterface.h"
#include "broadcastHandler.h"
#include "commandHandler.h"


namespace DataHub {
	
	class PLUGININTERFACESHARED_EXPORT SyncServer : public PluginInterface
	{
		Q_OBJECT
		Q_PLUGIN_METADATA(IID "de.datahub.PluginInterface" FILE "metadata.json")
		Q_INTERFACES(DataHub::PluginInterface)

	public:
		SyncServer() : m_ownIP(""), m_debug(false), m_lockHistory(true), m_paramHistory(true), m_context(new zmq::context_t(1)), m_broadcastHandler(0), m_broadcastHandlerThread(0) { }
	
	public:
		virtual void run();
		virtual void stop();

	private:
		QString m_ownIP;
		bool m_debug;
		bool m_lockHistory;
		bool m_paramHistory;
		zmq::context_t *m_context;
		QThread* m_broadcastHandlerThread;
		QThread* m_commandHandlerThread;
		BroadcastHandler* m_broadcastHandler;
		CommandHandler* m_commandHandler;
	protected:
		void init();
	private:
		void InitServer();
		void printHelp();
	};

}

#endif //SYNCSERVER_H