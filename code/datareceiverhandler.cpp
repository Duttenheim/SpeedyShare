#include "datareceiverhandler.h"

#if __QT5__
#include <QtWidgets/QApplication>
#else
#include <QtGui/QApplication>
#endif

#include <QtNetwork/QHostAddress>

#define MAXPACKAGESIZE 65535

//------------------------------------------------------------------------------
/**
*/
DataReceiverHandler::DataReceiverHandler( QTcpSocket* socket )
{
	this->socket = socket;
	this->stream.setDevice(socket);
}

//------------------------------------------------------------------------------
/**
*/
DataReceiverHandler::~DataReceiverHandler()
{
	this->socket = 0;
}

//------------------------------------------------------------------------------
/**
*/
void 
DataReceiverHandler::Kill()
{
	this->socket->disconnectFromHost();
	this->socket->close();
}

//------------------------------------------------------------------------------
/**
*/
void 
DataReceiverHandler::Update()
{
	// read stuff, this differs from the sender because for some reason incoming sockets needs to wait for a ready read...
	if (this->socket->waitForReadyRead(0))
	{
		this->Read();
	}

	// write stuff
	this->Write();
}

//------------------------------------------------------------------------------
/**
*/
void 
DataReceiverHandler::Read()
{
	// read int
	qint32 magic;
	this->stream >> magic;

	// get name
	QString file;
	this->stream >> file;

	if (magic == 'REQF')
	{
		this->pendingFiles.append(file);
		emit NewRequest(file, this->socket->peerAddress().toString());
	}
	else if (magic == 'PACK')
	{
		// read number of packages
		qint32 numPackages;
		this->stream >> numPackages;

		// start transaction
		emit this->TransactionStarted(file, numPackages);
		QApplication::processEvents();

		int i;
		for (i = 0; i < numPackages; i++)
		{
			// wait until we can read the package size
			while (this->socket->bytesAvailable() < sizeof(qint32))
			{
				QApplication::processEvents();
			}

			// read size of package
			qint32 size;
			this->socket->read((char*)&size, sizeof(qint32));

			// wait until the package can be read
			while (this->socket->bytesAvailable() < size)
			{
				QApplication::processEvents();
			}

			// read chunk
			QByteArray chunk = this->socket->read(size);
			emit this->TransactionProgress(file, chunk);
		}


		// stop transaction
		emit this->TransactionDone(file);
		QApplication::processEvents();
	}
}


//------------------------------------------------------------------------------
/**
*/
void 
DataReceiverHandler::Write()
{
	if (this->messages.size() > 0) do
	{
		const QByteArray& data = this->messages[0];
		this->socket->write(data);
		this->socket->waitForBytesWritten(-1);
		this->messages.removeFirst();
	} 
	while (this->messages.size() > 0);
}

//------------------------------------------------------------------------------
/**
*/
void 
DataReceiverHandler::OnAcceptFile( const QString& file )
{
	// assume we have a connection for this file already...
	Q_ASSERT(this->pendingFiles.contains(file));

	// remove pending package
	this->pendingFiles.removeOne(file);

	// write package
	QByteArray package;
	QDataStream stream(&package, QIODevice::WriteOnly);

	// package stream
	stream << 'ACPF';
	stream << true;
	stream << file;

	// send answer
	this->messages.append(package);
}

//------------------------------------------------------------------------------
/**
*/
void 
DataReceiverHandler::OnDenyFile( const QString& file )
{
	// assume we have a connection for this file already...
	Q_ASSERT(this->pendingFiles.contains(file));

	// remove pending package
	this->pendingFiles.removeOne(file);

	// write package
	QByteArray package;
	QDataStream stream(&package, QIODevice::WriteOnly);

	// package stream
	stream << 'ACPF';
	stream << false;
	stream << file;

	// send answer
	this->messages.append(package);
}
