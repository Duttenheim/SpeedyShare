#include "receiverthread.h"

//------------------------------------------------------------------------------
/**
*/
ReceiverThread::ReceiverThread(void)
{
	// empty
	
}

//------------------------------------------------------------------------------
/**
*/
ReceiverThread::~ReceiverThread(void)
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
void 
ReceiverThread::Start()
{
	this->shouldStop = false;
	this->start();
}

//------------------------------------------------------------------------------
/**
*/
void 
ReceiverThread::Stop()
{
	this->shouldStop = true;
}

//------------------------------------------------------------------------------
/**
*/
void 
ReceiverThread::run()
{
	// create receiver
	this->receiver = new DataReceiver;

	// connect
	connect(this->receiver, SIGNAL(NewRequest(const QString&, const QString&, int)), this, SLOT(OnFileRequested(const QString&, const QString&, int)));
	connect(this->receiver, SIGNAL(FileDone(const QString&, int)), this, SLOT(OnFileDone(const QString&, int)));
	connect(this->receiver, SIGNAL(FileProgress(const QString&, const QByteArray&, int)), this, SLOT(OnFileProgress(const QString&, const QByteArray&, int)));
	connect(this->receiver, SIGNAL(FileStarted(const QString&, int, int)), this, SLOT(OnFileStarted(const QString&, int, int)));

	// open receiver
	bool isListening = this->receiver->Open();

	// run receiver thread loop
	while(!this->shouldStop)
	{
		this->receiver->Update();
		QThread::yieldCurrentThread();
		QThread::msleep(5);
	}

	this->shouldStop = false;

	// close receiver
	this->receiver->Close();

	// delete receiver
	delete this->receiver;
}

//------------------------------------------------------------------------------
/**
*/
void 
ReceiverThread::OnFileRequested( const QString& file, const QString& peer, int index )
{
	emit this->FileRequested(file, peer, index);
}

//------------------------------------------------------------------------------
/**
*/
void 
ReceiverThread::OnFileDone( const QString& file, int index )
{
	emit this->FileDone(file, index);
}

//------------------------------------------------------------------------------
/**
*/
void 
ReceiverThread::OnFileProgress( const QString& file, const QByteArray& chunk, int index )
{
	emit this->FileProgress(file, chunk, index);
}

//------------------------------------------------------------------------------
/**
*/
void 
ReceiverThread::OnFileStarted( const QString& file, int chunks, int index )
{
	emit this->FileStarted(file, chunks, index);
}

//------------------------------------------------------------------------------
/**
*/
void 
ReceiverThread::OnFileAccepted( const QString& file, int index )
{
	this->receiver->OnAcceptFile(file, index);
}

//------------------------------------------------------------------------------
/**
*/
void 
ReceiverThread::OnFileDenied( const QString& file, int index )
{
	this->receiver->OnDenyFile(file, index);
}

