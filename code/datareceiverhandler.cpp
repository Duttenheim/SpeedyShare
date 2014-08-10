#include "datareceiverhandler.h"

#if __QT5__
#include <QtWidgets/QApplication>
#else
#include <QtGui/QApplication>
#endif

#include <QtNetwork/QHostAddress>
#include "config.h"

#define WAIT_FOR_BYTES(socket, num) \
    while(socket->bytesAvailable() < num && !this->stopRequested) \
    { \
        QApplication::processEvents(); \
    }

//------------------------------------------------------------------------------
/**
*/
DataReceiverHandler::DataReceiverHandler( QTcpSocket* socket ) :
    stopRequested(false)
{
	this->socket = socket;
	this->stream.setDevice(socket);
    connect(this->socket, SIGNAL(disconnected()), this, SLOT(OnSocketDisconnect()));
}

//------------------------------------------------------------------------------
/**
*/
DataReceiverHandler::~DataReceiverHandler()
{
    disconnect(this->socket, SIGNAL(disconnected()));
	this->socket = NULL;
    this->stream.unsetDevice();
}

//------------------------------------------------------------------------------
/**
*/
void 
DataReceiverHandler::Kill()
{
	this->socket->disconnectFromHost();
	this->socket->close();
    this->stopRequested = true;
}

//------------------------------------------------------------------------------
/**
*/
void 
DataReceiverHandler::Update()
{
	// read incoming messages, this differs from the sender because for some reason incoming sockets needs to wait for a ready read upon each read...
	if (this->socket->waitForReadyRead(0))
	{
		this->Read();
	}

	// write pending messages
    if (!this->stopRequested)
    {
	    this->Write();
    }
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

	if (magic == REQUEST_FILE)
	{
		this->pendingFiles.append(file);
		emit NewRequest(file, this->socket->peerAddress().toString());
	}
	else if (magic == PACKAGE)
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
			// wait until we can read package type
            WAIT_FOR_BYTES(this->socket, sizeof(quint32));

			// read package type
			qint32 type;			
			this->stream >> type;			

			// only read chunk if we actually have a chunk
			if (type == CHUNK)
			{
                // wait until we can read the package size
                WAIT_FOR_BYTES(this->socket, sizeof(quint32));

				// read size of package
				qint32 size;
				this->stream >> size;

                // wait until the package can be read
                WAIT_FOR_BYTES(this->socket, size);

				// read chunk
				QByteArray chunk = this->socket->read(size);
				emit this->TransactionProgress(file, chunk);
			}
			else if (type == ABORT)
			{
				// abort!!!
				emit this->TransactionAborted(file);
				break;
			}
			else
			{
				// woops, unknown FourCC means something went wrong
				emit this->TransactionCorrupted(file);
				break;
			}
		}

		// stop transaction
		emit this->TransactionDone(file);
		QApplication::processEvents();
	}
	else
	{
		// woops, unknown FourCC means something went wrong
		emit this->TransactionCorrupted(file);
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
	stream << ACCEPT_FILE;
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
	stream << ACCEPT_FILE;
	stream << false;
	stream << file;

	// send answer
	this->messages.append(package);
}

//------------------------------------------------------------------------------
/**
*/
void 
DataReceiverHandler::OnSocketDisconnect()
{
    emit ConnectionDied();
}