#pragma once
#include <QTcpSocket>
#include <QDataStream>
#include "filepackage.h"

//------------------------------------------------------------------------------------
/**
	Implements a per-connection receiver handler
*/
//------------------------------------------------------------------------------------

class DataReceiverHandler : public QObject
{
	Q_OBJECT
public:
	/// constructor
	DataReceiverHandler(QTcpSocket* socket);
	/// destructor
	~DataReceiverHandler();

	/// kills handler
	void Kill();
	/// updates handler
	void Update();

	/// reads from socket
	void Read();
	/// writes to socket
	void Write();

	/// called whenever a file is accepted
	void OnAcceptFile(const QString& file);
	/// called whenever a file is denied
	void OnDenyFile(const QString& file);
signals:
	/// emitted when a file is requested
	void NewRequest(const QString& file, const QString& peer);
	/// starts transaction
	void TransactionStarted(const QString& file, int chunks);
	/// reports progress
	void TransactionProgress(const QString& file, const QByteArray& chunk);
	/// emitted when a file has downloaded
	void TransactionDone(const QString& file);
	/// emitted when a transaction has failed
	void TransactionFailed(const QString& file);
	/// emitted when a  transaction has been remotely aborted
	void TransactionAborted(const QString& file);
	/// emitted if an unknown package has been received
	void TransactionCorrupted(const QString& file);
private:
	bool stopRequested;
	QTcpSocket* socket;
	QList<QString> pendingFiles;
	QList<QByteArray> messages;
	QDataStream stream;
};
