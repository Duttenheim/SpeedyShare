#include "datasender.h"
#include "config.h"
#include <QBuffer>

//------------------------------------------------------------------------------------
/**
*/
DataSender::DataSender(void) :
	port(MAINPORT),
	abortCurrent(false),
    isSending(false),
    isPaused(false)
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

    // unpause
    this->isPaused = false;

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
    if (this->isPaused) return;

	// read incoming messages if we have bytes waiting, do not do waitForReadyRead here since it will only be true once for some reason...
	qint32 available = this->bytesAvailable();
	if (available > 0)
	{
		this->Read();
	}	

	// write pending messages
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
		stream << REQUEST_FILE;
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

	if (magic == ACCEPT_FILE)
	{
		// get response
		bool answer;
		this->stream >> answer;

		// get file name
		QString name;
		this->stream >> name;

		if (answer)
		{
            // check bool saying we are sending
            this->isSending = true;

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
			dataStream << PACKAGE;
			dataStream << name;
			dataStream << numPackages;

			// write header
			this->write(dataPackage);
			this->waitForBytesWritten(-1);

			// trigger data send start
			emit FileStarted(package.GetName(), numPackages);

			// go through packages
			int i;
			for (i = 0; i < numPackages; i++)
			{
				// clear package and reset stream for next loop
				dataPackage.clear();					
				dataStream.device()->reset();

				if (!this->abortCurrent)
				{
					QByteArray fileData = fileHandle->read(MAXPACKAGESIZE);
					qint32 packageSize = qMin(MAXPACKAGESIZE, fileData.size());

					// reset data stream, then send chunk header				
					dataStream << CHUNK;
					dataStream << packageSize;

					// send type and size
					this->write(dataPackage);
					this->waitForBytesWritten(-1);

					// clear package
					dataPackage.clear();

					// then package the actual data
					dataPackage.append(fileData);

					// write package, restrict size to only write MAXPACKAGESIZE or less bytes
					this->write(dataPackage.constData(), packageSize);
					this->waitForBytesWritten(-1);

					// trigger data progress
					emit FileProgress(package.GetName(), packageSize);
				}
				else
				{
					// flip dat bool!
					this->abortCurrent = false;
					dataStream << ABORT;

					// send type
					this->write(dataPackage);
					this->waitForBytesWritten(-1);

					// break loop
					break;
				}		
			}

			// remove pending package
			this->pendingPackages.remove(name);

			// close file
			fileHandle->close();

            // inform we are not sending anymore
            this->isSending = false;

			// trigger data send done
			emit FileDone(package.GetName());
		}
		else
		{
            // inform we are not sending anymore
            this->isSending = false;

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

//------------------------------------------------------------------------------
/**
*/
void 
DataSender::TogglePause()
{
    this->isPaused = !this->isPaused;
}