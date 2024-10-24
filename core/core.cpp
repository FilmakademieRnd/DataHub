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

        TimerThread tthread(1000.f / s_framerate, false);
        TimerThread trandthread(1000.f, true);

        connect(&tthread, SIGNAL(tick()), this, SLOT(updateTime()), Qt::DirectConnection);
        connect(&trandthread, SIGNAL(tick()), this, SLOT(updateTimeRand()), Qt::DirectConnection);
        tthread.start();
        trandthread.start();
        tthread.setPriority(QThread::HighPriority);
        trandthread.setPriority(QThread::HighPriority);

        loadPlugins();
    }

	Core::Core(QStringList cmdlineArgs)
	{
        m_cmdlineArgs = cmdlineArgs;
       
        m_timesteps = (int)((s_timestepsBase / s_framerate) * s_framerate);

        TimerThread tthread(1000.f / s_framerate, false);
        TimerThread trandthread(1000.f, true);

        connect(&tthread, SIGNAL(tick()), this, SLOT(updateTime()), Qt::DirectConnection);
        connect(&trandthread, SIGNAL(tick()), this, SLOT(updateTimeRand()), Qt::DirectConnection);
        tthread.start();
        trandthread.start();
        tthread.setPriority(QThread::HighPriority);
        trandthread.setPriority(QThread::HighPriority);

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

    QStringList Core::getAppArguments()
    {
        return m_cmdlineArgs;
    }

	//!
    //! Function for increasing and resetting the time variable.
    //!
	void Core::updateTime()
	{
		m_time = (m_time > (m_timesteps - 2) ? (unsigned char)0 : m_time += 1);

        emit tickTick(m_time);

        if ((m_time % s_framerate) == 0) 
            emit tickSecond(m_time);

        //if ((m_time % 2) == 0)
          //  emit tickHalf(m_time);
	}

    //!
    //! Function for increasing and resetting the time variable.
    //!
    void Core::updateTimeRand()
    {
        emit tickSecondRandom(m_time);
    }

    void Core::storeData(QByteArray data)
    {
        emit storeDataSignal(data);
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
                    //pluginInterface->run();
                }
                else
                    pluginLoader.unload();
            }
            else
                qDebug() << "Plugin " + filePath + " could not be loaded.";
        }
        for (DataHub::PluginInterface* plugin : s_plugins) {
            plugin->run();
        }

    }

}

