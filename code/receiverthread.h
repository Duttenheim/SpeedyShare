#pragma once
#include <QThread>
#include "datareceiver.h"

//------------------------------------------------------------------------------------
/**
	Thread which receives data
*/
//------------------------------------------------------------------------------------
class ReceiverThread :
	public QThread
{
	Q_OBJECT
public:
	/// constructor
	ReceiverThread(void);
	/// destructor
	~ReceiverThread(void);

	/// starts thread
	void Start();
	/// stops thread
	void Stop();

public slots:
	/// called whenever a file is accepted
	void OnFileAccepted(QString file, int index);
	/// called whenever a file is denied
	void OnFileDenied(QString file, int index);
private slots:
	/// called whenever a file is requested
	void OnFileRequested(QString file, int index);
	/// called whenever a file is done
	void OnFileDone(QString file, int index);
	/// called whenever a file progress is done
	void OnFileProgress(QString file, const QByteArray& chunk, int index);
	/// called when a file has started
	void OnFileStarted(QString file, int chunks, int index);
signals:
	/// emitted when a file is requested
	void FileRequested(QString file, int index);
	/// emitted when a file is done
	void FileDone(QString file, int index);
	/// emitted when a file progress is done
	void FileProgress(QString file, const QByteArray& chunk, int index);
	/// emitted when a file is started
	void FileStarted(QString file, int chunks, int index);
private:
	/// runs thread
	void run();

	QList<QTcpSocket*> connections;
	DataReceiver* receiver;
	bool shouldStop;
};
