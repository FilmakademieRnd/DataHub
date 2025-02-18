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

//! @file "plugininterface.h"
//! @brief DataHub Core: Class implementing the DataHub plugin interface.
//! @author Simon Spielmann
//! @version 1
//! @date 03.07.2023

#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QObject>
#include <QString>
#include <QtCore/qglobal.h>

#if defined(PLUGININTERFACE_LIBRARY)
#define PLUGININTERFACESHARED_EXPORT Q_DECL_EXPORT
#else
#define PLUGININTERFACESHARED_EXPORT Q_DECL_IMPORT
#endif

#if defined(CORE_LIBRARY)
#define CORESHARED_EXPORT Q_DECL_EXPORT
#else
#define CORESHARED_EXPORT Q_DECL_IMPORT
#endif


namespace DataHub
{
	// Forewar deleclaration of core class.
	class Core;

	//!
	//! \brief Interface for plugins for the DataHub
	//!
	class CORESHARED_EXPORT PluginInterface : public QObject
	{
	public:
		QString name() { return metaObject()->className(); }
		virtual void run() = 0;
		virtual void stop() = 0;
	private:
		Core* m_core = 0;
	public:
		virtual void init() {};
		void setCore(Core* core) { m_core = core; };
	public:
		Core* core() const { return m_core; };

		friend class Core;
	};


}  // end namespace DataHub

#define PluginInterface_iid "de.datahub.PluginInterface"
Q_DECLARE_INTERFACE(DataHub::PluginInterface, PluginInterface_iid)

#endif // PLUGININTERFACE_H