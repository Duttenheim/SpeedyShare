#include "senderthread.h"
#include <QApplication>

//------------------------------------------------------------------------------
/**
*/
SenderThread::SenderThread(void) :
	connectionOpen(false)
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
SenderThread::~SenderThread(void)
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
void 
SenderThread::Start()
{
	this->shouldStop = false;
	this->start();
}

//------------------------------------------------------------------------------
/**
*/
void 
SenderThread::Stop()
{
	this->shouldStop = true;
}

//------------------------------------------------------------------------------
/**
*/
void 
SenderThread::AbortCurrent()
{
	this->mutex.lock();
	this->sender->AbortCurrent();
	this->mutex.unlock();
}

//------------------------------------------------------------------------------
/**
*/
void 
SenderThread::Enqueue( const FilePackage& data )
{
	this->mutex.lock();
	FilePackage intermediate(data);
	this->dataQueue.enqueue(intermediate);
	this->mutex.unlock();
}

//------------------------------------------------------------------------------
/**
*/
void 
SenderThread::run()
{
	// create sender
	this->sender = new DataSender;

	// connect connection/disconnection signals
	connect(this->sender, SIGNAL(connected()), this, SLOT(OnConnected()));
	connect(this->sender, SIGNAL(disconnected()), this, SLOT(OnDisconnected()));

	// connect data sent signal
	connect(this->sender, SIGNAL(FileDenied(const QString&)), this, SLOT(OnFileDenied(const QString&)));
	connect(this->sender, SIGNAL(FileStarted(const QString&, int)), this, SLOT(OnFileStarted(const QString&, int)));
	connect(this->sender, SIGNAL(FileDone(const QString&)), this, SLOT(OnFileDone(const QString&)));
	connect(this->sender, SIGNAL(FileProgress(const QString&, int)), this, SLOT(OnFileProgress(const QString&, int)));

	// set address, port and open the sender
	this->sender->SetAddress(this->address);
	this->sender->SetPort(this->port);
	this->sender->Open();
	
	// execute sender thread loop
	while(!this->shouldStop)
	{
		if (this->connectionOpen)
		{
			if (!this->dataQueue.isEmpty())
			{
				this->mutex.lock();
				this->sender->Send(this->dataQueue.dequeue());
				this->mutex.unlock();
			}
			
			this->sender->Update();
		}
		QApplication::processEvents();
		QThread::yieldCurrentThread();
		QThread::msleep(5);
	}

	this->shouldStop = false;

	// close sender
	this->sender->Close();

	// delete sender
	delete this->sender;
}

//------------------------------------------------------------------------------
/**
*/
void 
SenderThread::OnConnected()
{
	this->connectionOpen = true;
	emit ConnectionSuccessful();
}

//------------------------------------------------------------------------------
/**
*/
void 
SenderThread::OnDisconnected()
{
	this->quit();
	this->connectionOpen = false;
	emit Disconnected();
}

//------------------------------------------------------------------------------
/**
*/
void 
SenderThread::OnFileDenied( const QString& file )
{
	emit FileDenied(file);
}

//------------------------------------------------------------------------------
/**
*/
void 
SenderThread::OnFileDone( const QString& file )
{
	emit FileDone(file);
}

//------------------------------------------------------------------------------
/**
*/
void 
SenderThread::OnFileProgress( const QString& file, int numBytes )
{
	emit FileProgress(file, numBytes);
}

//------------------------------------------------------------------------------
/**
*/
void 
SenderThread::OnFileStarted( const QString& file, int chunks )
{
	emit FileStarted(file, chunks);
}