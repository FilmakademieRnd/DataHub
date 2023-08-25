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

//! @file "core.cpp"
//! @brief DataHub core implementation. Central class for DataHub initalization. Also manages all plugins.
//! @author Simon Spielmann
//! @version 1
//! @date 03.07.2023

#include "core.h"
#include <QPluginLoader>
#include <cstdlib>

namespace DataHub {

	Core::Core() 
	{
        m_timesteps = (int)((s_timestepsBase / s_framerate) * s_framerate);

        TimerThread tthread(1000.f/m_timesteps);
        connect(&tthread, SIGNAL(tick()), this, SLOT(updateTime()), Qt::DirectConnection);
        tthread.start();
        tthread.setPriority(QThread::HighPriority);

		loadPlugins();
	}

    Core::~Core()
    {
        qDebug() << "Exiting all Threads...";

        foreach(PluginInterface * plugin, s_plugins)
        {
            plugin->stop();
        }
    }

	//!
    //! Function for increasing and resetting the time variable.
    //!
	void Core::updateTime()
	{
		m_time = (m_time > (m_timesteps - 2) ? (char)0 : m_time += 1);

        if ((m_time % s_framerate) == 0) 
            emit tickSecond(m_time);
	}

    void Core::loadPlugins()
    {
        // search for plugins
        QDir pluginsDir(QDir::currentPath() + "/plugins");
        pluginsDir.setNameFilters(QStringList() << "*.dll");

        const QStringList entries = pluginsDir.entryList();

        for (const QString& fileName : entries) {
            QString filePath = pluginsDir.absoluteFilePath(fileName);
            QPluginLoader pluginLoader(filePath);
            QObject* plugin = pluginLoader.instance();
            if (plugin) {
                PluginInterface* pluginInterface = qobject_cast<PluginInterface*>(plugin);
                if (pluginInterface)
                {
                    s_plugins.insert(pluginInterface->name(), pluginInterface);
                    // init plugin
                    qDebug() << "Plugin " + filePath + " loaded.";
                    pluginInterface->setCore(this);
                    pluginInterface->init();
                    pluginInterface->run();
                }
                else
                    pluginLoader.unload();
            }
            else
                qDebug() << "Plugin " + filePath + " could not be loaded.";
        }

    }

}

