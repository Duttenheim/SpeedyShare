#include "datareceiverhandler.h"

#if __QT5__
#include <QtWidgets/QApplication>
#else
#include <QtGui/QApplication>
#endif
#include <QDataStream>
#define MAXPACKAGESIZE 65535

//------------------------------------------------------------------------------
/**
*/
DataReceiverHandler::DataReceiverHandler( QTcpSocket* socket )
{
	this->socket = socket;
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
	if (this->socket->waitForReadyRead(0))
	{
		this->Read();
	}
}

//------------------------------------------------------------------------------
/**
*/
void 
DataReceiverHandler::Read()
{
	// create data stream
	QDataStream stream(this->socket);

	// read int
	qint32 magic;
	stream >> magic;

	// get name
	QString file;
	stream >> file;

	if (magic == 'REQF')
	{
		this->pendingFiles.append(file);
		emit NewRequest(file);
	}
	else if (magic == 'PACK')
	{
		// read number of packages
		qint32 numPackages;
		stream >> numPackages;

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
	qint32 written = this->socket->write(package);
	this->socket->flush();
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
	this->socket->write(package);
	this->socket->flush();
}
