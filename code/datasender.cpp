#include "datasender.h"

#define MAXPACKAGESIZE 65535

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

	// setup stream
	this->stream.setDevice(this);

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
	// read stuff if we have bytes waiting, do not do waitForReadyRead here since it will only be true once for some reason...
	qint32 available = this->bytesAvailable();
	if (available > 0)
	{
		this->Read();
	}	

	this->Write();
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

		// add message to queue
		this->messages.append(message);
	}
}

//------------------------------------------------------------------------------
/**
*/
void 
DataSender::Read()
{
	// read magic
	qint32 magic;
	this->stream >> magic;

	if (magic == 'ACPF')
	{
		// get response
		bool answer;
		this->stream >> answer;

		// get file name
		QString name;
		this->stream >> name;

		if (answer)
		{
			// get package
			Q_ASSERT(this->pendingPackages.contains(name));
			FilePackage package = this->pendingPackages[name];		

			// get file
			QFile* fileHandle = package.GetFile();
			quint64 dataSize = fileHandle->size();

			// split package into smaller packages
			qint32 numPackages = (dataSize / MAXPACKAGESIZE) + 1;

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
				// clear package for next loop
				dataPackage.clear();	

				QByteArray fileData = fileHandle->read(MAXPACKAGESIZE);
				qint32 packageSize = qMin(MAXPACKAGESIZE, fileData.size());

				// send size first
				this->write((char*)&packageSize, sizeof(qint32));
				this->waitForBytesWritten(-1);
				
				// then package the actual data
				dataPackage.append(fileData);

				// write package
				this->write(dataPackage.constData(), packageSize);
				this->waitForBytesWritten(-1);

				// trigget data progress
				emit FileProgress(package.GetName(), packageSize);		
			}

			// remove pending package
			this->pendingPackages.remove(name);

			// close file
			fileHandle->close();

			// trigger data send done
			emit FileDone(package.GetName());
		}
		else
		{
			// emit that the file should not be sent
			emit FileDenied(name);
		}
	}
}

//------------------------------------------------------------------------------
/**
*/
void 
DataSender::Write()
{
	if (this->messages.size() > 0) do
	{
		const QByteArray& data = this->messages[0];
		this->write(data);
		this->waitForBytesWritten(-1);
		this->messages.removeFirst();
	} 
	while (this->messages.size() > 0);
}