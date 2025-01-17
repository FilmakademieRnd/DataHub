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

#ifndef SCENEDADAHANDLER_H
#define SCENEDADAHANDLER_H

typedef unsigned char byte;

class SceneDataHandler
{
public:
	SceneDataHandler(QString serverID, QString path) : m_serverID(serverID), m_path(path) {}

public:
	QByteArray headerByteData;
	QByteArray nodesByteData;
	QByteArray parameterObjectsByteData;
	QByteArray objectsByteData;
	QByteArray characterByteData;
	QByteArray texturesByteData;
	QByteArray materialsByteData;

public:
	void writeToDisk()
	{
		QString filePath = m_path + m_serverID;

		writeFile(&headerByteData, filePath + "_header");
		writeFile(&nodesByteData, filePath + "_nodesByteData");
		writeFile(&parameterObjectsByteData, filePath + "_parameterObjectsByteData");
		writeFile(&objectsByteData, filePath + "_objectsByteData");
		writeFile(&characterByteData, filePath + "_characterByteData");
		writeFile(&texturesByteData, filePath + "_texturesByteData");
		writeFile(&materialsByteData, filePath + "_materialsByteData");
	}

	void readFromDisk()
	{
		QString filePath = m_path + m_serverID;
		
		headerByteData = readFile(filePath + "_header");
		nodesByteData = readFile(filePath + "_nodesByteData");
		parameterObjectsByteData = readFile(filePath + "_parameterObjectsByteData");
		objectsByteData = readFile(filePath + "_objectsByteData");
		characterByteData = readFile(filePath + "_characterByteData");
		texturesByteData = readFile(filePath + "_texturesByteData");
		materialsByteData = readFile(filePath + "_materialsByteData");
	}

	bool isEmpty()
	{
		return
			headerByteData.isEmpty() &&
			nodesByteData.isEmpty() &&
			parameterObjectsByteData.isEmpty() &&
			objectsByteData.isEmpty() &&
			characterByteData.isEmpty() &&
			texturesByteData.isEmpty() &&
			materialsByteData.isEmpty();
	}

private:
	QString m_serverID;
	QString m_path;

private:
	void writeFile(QByteArray* data, QString filePath)
	{
		if (!data->isEmpty())
		{
			QFile file(filePath);
			file.open(QIODevice::WriteOnly);
			file.write(data->data());
			file.flush();
			file.close();
		}
	}

	QByteArray readFile(QString filePath)
	{
		QFile file(filePath);
		if (file.exists() && file.open(QIODevice::ReadOnly)) {
			return file.readAll();
		}
	}
};

#endif // end SCENEDADAHANDLER_H