#pragma once
#include <QObject>
#include <QtNetwork/QTcpServer>
#include "datareceiverhandler.h"
#include "filepackage.h"

//------------------------------------------------------------------------------------
/**
	Implements the server-side data receiver
*/
//------------------------------------------------------------------------------------

class DataReceiver :
	public QTcpServer
{
	Q_OBJECT
public:
	/// constructor
	DataReceiver(void);
	/// destructor
	~DataReceiver(void);

	/// open receiver
	bool Open();
	/// closes receiver
	void Close();
	/// updates receiver
	void Update();

	/// called whenever the receiver thread should accept a file
	void OnAcceptFile(QString file, int index);
	/// called whenever the receiver thread should deny the file
	void OnDenyFile(QString file, int index);
private slots:
	/// called whenever a connection dies
	void OnConnectionDied();
	/// called whenever a file gets requested
	void OnFileRequested(QString file);
	/// called whenever a file transaction is done
	void OnFileDone(QString file);
	/// called from handler when progress is performed
	void OnFileStart(QString file, int chunks);
	/// called from handler when progress is progressed
	void OnFileProgress(QString file, const QByteArray& chunk);
signals:
	/// called whenever a new file transaction is ready
	void NewRequest(QString name, int index);
	/// called whenever a new connection is established
	void FileDone(QString file, int index);
	/// emitted when a file has started downloading
	void FileStarted(QString file, int chunks, int index);
	/// emitted when a file is progressing
	void FileProgress(QString file, const QByteArray& chunk, int index);

private:

	QMap<QString, QTcpSocket*> pendingFiles;
	QList<QTcpSocket*> connections;
	QList<DataReceiverHandler*> dataHandlers;
};
