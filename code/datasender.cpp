#include "datasender.h"
#include <QSslSocket>
#include <QDataStream>

#define MAXPACKAGESIZE 65535
#define MAXFILECHUNKSIZE 131072

//------------------------------------------------------------------------------------
/**
*/
DataSender::DataSender(void) :
	port(1467)
{
	// empty
}

//------------------------------------------------------------------------------------
/**
*/
DataSender::~DataSender(void)
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
bool 
DataSender::Open()
{
	// open connection
	this->connectToHost(this->address, this->port, QIODevice::ReadWrite);

	// wait until connection occurs
	return this->waitForConnected(1000);
}

//------------------------------------------------------------------------------
/**
*/
void 
DataSender::Close()
{
	// closes sender
	this->disconnectFromHost();
}

//------------------------------------------------------------------------------
/**
*/
void 
DataSender::Update()
{
	if (this->waitForReadyRead(0))
	{
		this->OnRead();
	}	
}

//------------------------------------------------------------------------------
/**
*/
void 
DataSender::Send( const FilePackage& data )
{
	// the sender has to be valid
	if (this->isValid())
	{
		// create full data package
		QByteArray package = data.GetData();
		QString name = data.GetName();

		// create message
		QByteArray message;

		// create data stream
		QDataStream stream(&message, QIODevice::WriteOnly);

		// write magic number
		stream << 'REQF';
		stream << name;

		// add to pending packages
		this->pendingPackages[name] = data;

		// write message
		this->write(message);
		this->flush();
	}
}

//------------------------------------------------------------------------------
/**
*/
void 
DataSender::OnRead()
{
	// read bytes
	QDataStream stream(this);

	// read magic
	qint32 magic;
	stream >> magic;

	if (magic == 'ACPF')
	{
		// get response
		bool answer;
		stream >> answer;

		// get file name
		QString name;
		stream >> name;

		if (answer)
		{
			// get package
			FilePackage package = this->pendingPackages[name];		

			// get file
			QFile* fileHandle = package.GetFile();
			int dataSize = fileHandle->size();

			// split package into smaller packages
			int numPackages = (dataSize / MAXPACKAGESIZE) + 1;

			// create new header
			QByteArray dataPackage;
			QDataStream dataStream(&dataPackage, QIODevice::WriteOnly);			

			// write magic
			dataStream << 'PACK';
			dataStream << name;
			dataStream << numPackages;

			// write header
			this->write(dataPackage);
			this->waitForBytesWritten(-1);

			// clear package
			dataPackage.clear();

			// trigger data send start
			emit FileStarted(package.GetName(), numPackages);

			// go through packages
			int i;
			for (i = 0; i < numPackages; i++)
			{
				QByteArray fileData = fileHandle->read(MAXPACKAGESIZE);
				int packageSize = qMin(MAXPACKAGESIZE, fileData.size() - i * MAXPACKAGESIZE);
				dataPackage.append(fileData);

				// write package
				this->write(dataPackage);
				this->waitForBytesWritten(-1);

				// trigget data progress
				emit FileProgress(package.GetName(), packageSize);

				// clear package for next loop
				dataPackage.clear();			
			}

			// close file
			fileHandle->close();

			// trigger data send done
			emit FileDone(package.GetName());
		}
		else
		{
			// do nothing, the send was denied
		}
	}
}