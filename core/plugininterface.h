/*
-----------------------------------------------------------------------------
This source file is part of TRACER -
Toolset for Realtime Animation, Collaboration & Extended Reality
https://animationsinstitut.de/en/research/tools/tracer
http://github.com/FilmakademieRnd/VPET

Copyright (c) 2023 Filmakademie Baden-Wuerttemberg, Animationsinstitut R&D Lab

This project has been initiated in the scope of the EU funded project MAX-R
(https://www.upf.edu/web/max-r) under grant agreement no 101070072 in the years
2022-2024.

Since the DataHub is available for free, Filmakademie shall only be liable for
intent and gross negligence; warranty is limited to malice. DataHub may under
no circumstances be used for racist, sexual or any illegal purposes. In all
non-commercial productions, scientific publications, prototypical non-commercial
software tools, etc. using the DataHub Filmakademie has to be named as follows:
"TRACER - Toolset for Realtime Animation, Collaboration & Extended Reality by
Filmakademie Baden-Wuerttemberg, Animationsinstitut
(http://research.animationsinstitut.de)".

In case a company or individual would like to use DataHub in a commercial
surrounding or for commercial purposes, software based on these components or
any part thereof, the company/individual will have to contact Filmakademie
(research<at>filmakademie.de).
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

namespace DataHub
{
	//!
	//! \brief Interface for plugins for the DataHub
	//!
	class PLUGININTERFACESHARED_EXPORT PluginInterface : public QObject
	{
	public:
		QString name() { return metaObject()->className(); }
		virtual void run() = 0;
		virtual void stop() = 0;
	};


}  // end namespace DataHub

#define PluginInterface_iid "de.datahub.PluginInterface"
Q_DECLARE_INTERFACE(DataHub::PluginInterface, PluginInterface_iid)

#endif // PLUGININTERFACE_H