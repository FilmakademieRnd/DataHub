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
	~SceneDataHandler()  
	{
		headerByteData.clear();
		thumbnailByteData.clear();
		nodesByteData.clear();
		parameterObjectsByteData.clear();
		objectsByteData.clear();
		characterByteData.clear();
		splatByteData.clear();
		texturesByteData.clear();
		materialsByteData.clear();
	}

public:
	QByteArray headerByteData;
	QByteArray thumbnailByteData;
	QByteArray nodesByteData;
	QByteArray parameterObjectsByteData;
	QByteArray objectsByteData;
	QByteArray characterByteData;
	QByteArray splatByteData;
	QByteArray texturesByteData;
	QByteArray materialsByteData;

private:
	inline static const QString headerString {".header"};
	inline static const QString thumbnailString {".jpg"};
	inline static const QString nodesString {".nodesByteData"};
	inline static const QString parameterObjectsString {".parameterObjectsByteData"};
	inline static const QString objectsString {".objectsByteData"};
	inline static const QString characterString {".characterByteData"};
	inline static const QString splatString {".splatByteData"};
	inline static const QString texturesString {".texturesByteData"};
	inline static const QString materialsString {".materialsByteData"};

public:
	static QList<QStringList> infoFromDisk(QString path, QString serverID)
	{
		QList<QStringList> returnvalue;

		QDir dir(path + serverID + "/");
		returnvalue.append(dir.entryList(QStringList() << "*" + headerString, QDir::Files));
		returnvalue.append(dir.entryList(QStringList() << "*" + thumbnailString, QDir::Files));
		returnvalue.append(dir.entryList(QStringList() << "*" + nodesString, QDir::Files));
		returnvalue.append(dir.entryList(QStringList() << "*" + parameterObjectsString, QDir::Files));
		returnvalue.append(dir.entryList(QStringList() << "*" + objectsString, QDir::Files));
		returnvalue.append(dir.entryList(QStringList() << "*" + characterString, QDir::Files));
		returnvalue.append(dir.entryList(QStringList() << "*" + splatString, QDir::Files));
		returnvalue.append(dir.entryList(QStringList() << "*" + texturesString, QDir::Files));
		returnvalue.append(dir.entryList(QStringList() << "*" + materialsString, QDir::Files));

		return returnvalue;
	}

	static QList<QByteArray> readPackagesFromDisk(QString path, QString serverID, int package = 0)
	{
		QList<QStringList> fileNames = infoFromDisk(path, serverID);

		QString filePath = path + serverID + "/";
		QList<QByteArray> thumbnails;

		foreach(QString fileName, fileNames[package])
			thumbnails.append(readFile(filePath, fileName));

		return thumbnails;
	}

	void writeToDisk(QString path, QString senderID, QString stamp)
	{
		QDir dir(path + senderID);
		
		if (!dir.exists())
			dir.mkpath(".");

		QString filePath = path + senderID + "/" + stamp;

		writeFile(&headerByteData, filePath + headerString);
		writeFile(&thumbnailByteData, filePath + thumbnailString);
		writeFile(&nodesByteData, filePath + nodesString);
		writeFile(&parameterObjectsByteData, filePath + parameterObjectsString);
		writeFile(&objectsByteData, filePath + objectsString);
		writeFile(&characterByteData, filePath + characterString);
		writeFile(&splatByteData, filePath + splatString);
		writeFile(&texturesByteData, filePath + texturesString);
		writeFile(&materialsByteData, filePath + materialsString);
	}

	void readFromDisk(QString path, QString serverID, int entryNbr = 0)
	{
		QList<QStringList> fileNames = infoFromDisk(path, serverID);

		QString filePath = path + serverID + "/";
		
		headerByteData = readFile(filePath, fileNames[0], entryNbr);
		thumbnailByteData = readFile(filePath, fileNames[1], entryNbr);
		nodesByteData = readFile(filePath, fileNames[2], entryNbr);
		parameterObjectsByteData = readFile(filePath, fileNames[3], entryNbr);
		objectsByteData = readFile(filePath, fileNames[4], entryNbr);
		characterByteData = readFile(filePath, fileNames[5], entryNbr);
		splatByteData = readFile(filePath, fileNames[5], entryNbr);
		texturesByteData = readFile(filePath, fileNames[6], entryNbr);
		materialsByteData = readFile(filePath, fileNames[7], entryNbr);
	}

	bool isComplete()
	{
		return
			headerByteData.isEmpty() &&
			nodesByteData.isEmpty() &&
			parameterObjectsByteData.isEmpty() &&
			objectsByteData.isEmpty() &&
			characterByteData.isEmpty() &&
			splatByteData.isEmpty() &&
			texturesByteData.isEmpty() &&
			materialsByteData.isEmpty();
	}

private:
	void writeFile(QByteArray* data, QString filePath)
	{
		if (!data->isEmpty())
		{
			QFile file(filePath);
			file.open(QIODevice::WriteOnly);
			file.write(data->data(), data->size());
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
		else return QByteArray();
	}

	static QByteArray readFile(QString path, QStringList fileNames, int index)
	{
		if (fileNames.size() > index)
		{
			QFile file(path + "/" + fileNames[index]);
			if (file.exists() && file.open(QIODevice::ReadOnly)) {
				return file.readAll();
			}
		}
		else
			return QByteArray();
	}

	static QByteArray readFile(QString path, QString fileName)
	{
		QFile file(path + "/" + fileName);
		if (file.exists() && file.open(QIODevice::ReadOnly)) {
			return file.readAll();
		}
		return QByteArray();
	}
};

#endif // end SCENEDADAHANDLER_H