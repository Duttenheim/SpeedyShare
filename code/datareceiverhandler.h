#pragma once
#include <QTcpSocket>
#include "filepackage.h"

//------------------------------------------------------------------------------------
/**
	Implements a per-connection receiver thread
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

	/// called whenever a file is accepted
	void OnAcceptFile(const QString& file);
	/// called whenever a file is denied
	void OnDenyFile(const QString& file);
signals:
	/// emitted when a file is requested
	void NewRequest(const QString& file);
	/// starts transaction
	void TransactionStarted(const QString& file, int chunks);
	/// reports progress
	void TransactionProgress(const QString& file, const QByteArray& chunk);
	/// emitted when a file has downloaded
	void TransactionDone(const QString& file);
	/// emitted when a transaction has failed
	void TransactionFailed(const QString& file);
private:
	QTcpSocket* socket;
	QList<QString> pendingFiles;
};
