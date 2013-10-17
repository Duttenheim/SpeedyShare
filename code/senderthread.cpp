#include "senderthread.h"

//------------------------------------------------------------------------------------
/**
*/
SenderThread::SenderThread(void)
{
	// empty
}

//------------------------------------------------------------------------------------
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

	// connect stuff
	connect(this->sender, SIGNAL(connected()), this, SLOT(OnConnected()));
	connect(this->sender, SIGNAL(disconnected()), this, SLOT(OnDisconnected()));

	this->sender->SetAddress(this->address);
	this->sender->SetPort(this->port);
	this->sender->Open();
	
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
		QThread::yieldCurrentThread();
		QThread::msleep(500);
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