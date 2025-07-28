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
#include "zeroMQHandler.h"



namespace DataHub {
	
	class PLUGININTERFACESHARED_EXPORT SyncServer : public PluginInterface
	{
		Q_OBJECT
		Q_PLUGIN_METADATA(IID "de.datahub.PluginInterface" FILE "metadata.json")
		Q_INTERFACES(DataHub::PluginInterface)

	public:
		SyncServer();
	
	public:
        typedef QPair<bool, byte> cID;
		virtual void run();
		virtual void stop();
		void requestScene(byte clientID);
		void sendScene(QString ip, byte clientID);

		static int addClient(const int64_t mac)
		{
			// hard limit of 250 clients revome old inactive clients first
			if (m_clientIDs.size() > 249)
			{
				if (m_clientsInactive.size() > 0) {
					qDebug() << "List full, remive Client ID: " << m_clientsInactive[0] << " reactivated.";
					m_clientIDs.remove(m_clientsInactive.takeFirst());
				}
				else
					return 255;
			}

			auto valueIter = m_clientIDs.find(mac);

			if (valueIter != m_clientIDs.end())   // ip in list 
			{
				if (valueIter->first)    // ip active
				{
					qDebug() << "Client ID: " << valueIter->second << " still active.";
					return valueIter->second;
				}
				else   // if ip not active or lost then reactivate
				{
					m_clientsInactive.removeAt(m_clientsInactive.indexOf(mac));
					valueIter->first = true;
					qDebug() << "Client ID: " << valueIter->second << " reactivated.";
					return valueIter->second;
				}
			}
			else  // add complete new client
			{
				int next = nextFreeID() + 1; // because 0 is DataHub
				//qDebug() << "New Client added to list with ID: " << next;
				m_clientIDs.insert(mac, cID(true, next));
				return next;
			}
		}

		static bool removeClient(byte id)
		{
			int64_t ip = getMacInt(id);

			if (m_clientIDs.size() > 249) 
			{
				// remove oldest, inactive client
				return m_clientIDs.remove(m_clientsInactive.takeFirst());
			}
			else if (ip != -1)
			{
				m_clientsInactive.append(ip);
				m_clientIDs[ip].first = false;
				return true;
			}
			else
				return false;
        }

		static int ipToInt(byte first, byte second, byte third, byte fourth)
		{
			return (first << 24) | (second << 16) | (third << 8) | (fourth);
		}

		static int64_t macToInt(byte first, byte second, byte third, byte fourth, byte fith, byte sixth)
		{
			return (static_cast<uint64_t>(first) << 40) | 
				(static_cast<uint64_t>(second) << 32) | 
				(static_cast<uint64_t>(third) << 24) |
				(static_cast<uint64_t>(fourth) << 16) |
				(static_cast<uint64_t>(fith) << 8) |
				static_cast<uint64_t>(sixth);
		}

		static QString IntToIp(int ip)
		{
			byte bytes[4];

			bytes[0] = (ip >> 24) & 0xFF;
			bytes[1] = (ip >> 16) & 0xFF;
			bytes[2] = (ip >> 8) & 0xFF;
			bytes[3] = ip & 0xFF;

			return QString::number(bytes[0]) + 
				"." + QString::number(bytes[1]) + 
				"." + QString::number(bytes[2]) + 
				"." + QString::number(bytes[3]);
		}

		static QString IntToMac(int64_t mac)
		{
			byte bytes[6];

			bytes[0] = (mac >> 40) & 0xFF;
			bytes[1] = (mac >> 32) & 0xFF;
			bytes[2] = (mac >> 24) & 0xFF;
			bytes[3] = (mac >> 16) & 0xFF;
			bytes[4] = (mac >> 8) & 0xFF;
			bytes[5] = mac & 0xFF;

			return QString::number(bytes[0]) + 
				"." + QString::number(bytes[1]) + 
				"." + QString::number(bytes[2]) + 
				"." + QString::number(bytes[3]);
				"." + QString::number(bytes[4]);
				"." + QString::number(bytes[5]);
		}

	private:
		QString m_ownIP;
		bool m_debug;
		bool m_webSockets;
		bool m_lockHistory;
		bool m_paramHistory;
		bool m_isRunning;
		zmq::context_t *m_context;
		QList<ZeroMQHandler*> m_handlerlist;
        static QHash<int64_t, cID> m_clientIDs;
		static QList<int64_t> m_clientsInactive;

        static byte nextFreeID()
        {
            int i = 0;

            if (!m_clientIDs.empty())
            {
                QList<cID> keys = m_clientIDs.values();
				std::sort(keys.begin(), keys.end(), [&](const cID& c1, const cID& c2) { return c1.second < c2.second; });
                for (; i < keys.size() - 1;)
                {
                    if (keys[i + 1].second - keys[i].second > 1)
                        break;
                    i++;
                }

                i++;
            }

            return i;
        }

		static int64_t getMacInt(const byte clientID)
		{
			return m_clientIDs.key(cID(true, clientID), -1);
		}

		static QString getIPString(const byte clientID)
		{
			//return IntToIp(getIPInt(clientID)
			return IntToIp(0); // not defined because of gateway problem! 
		}

		static QString getMACString(const byte clientID)
		{
			return IntToMac(getMacInt(clientID));
		}
		
	protected:
		void init();

	private:
		void initServer();
		void initHandler(ZeroMQHandler *handler);
		void cleanupHandler(ZeroMQHandler* handler);
		void printHelp();
	
	signals:
		void sceneReceived(QString);
		void sceneSend(QString);
	};

}

#endif //SYNCSERVER_H