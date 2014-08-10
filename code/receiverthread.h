#pragma once
#include <QThread>
#include "datareceiver.h"

//------------------------------------------------------------------------------------
/**
	Thread which receives data
*/
//------------------------------------------------------------------------------------
class ReceiverThread : public QThread
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
	void OnFileAccepted(const QString& file, int index);
	/// called whenever a file is denied
	void OnFileDenied(const QString& file, int index);
    /// called whenever the port has changed
    void OnPortChanged(uint port);
private slots:
	/// called whenever a file is requested
	void OnFileRequested(const QString& file, const QString& peer, int index);
	/// called whenever a file is done
	void OnFileDone(const QString& file, int index);
	/// called whenever a file progress is done
	void OnFileProgress(const QString& file, const QByteArray& chunk, int index);
	/// called when a file has started
	void OnFileStarted(const QString& file, int chunks, int index);
signals:
	/// emitted when a file is requested
	void FileRequested(const QString& file, const QString& peer, int index);
	/// emitted when a file is done
	void FileDone(const QString& file, int index);
	/// emitted when a file progress is done
	void FileProgress(const QString& file, const QByteArray& chunk, int index);
	/// emitted when a file is started
	void FileStarted(const QString& file, int chunks, int index);
private:
	/// runs thread
	void run();

	QList<QTcpSocket*> connections;
	DataReceiver* receiver;
	bool shouldStop;
};
