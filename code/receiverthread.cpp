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
	connect(this->receiver, SIGNAL(NewRequest(const QString&, int)), this, SLOT(OnFileRequested(const QString&, int)));
	connect(this->receiver, SIGNAL(FileDone(QString, int)), this, SLOT(OnFileDone(QString, int)));
	connect(this->receiver, SIGNAL(FileProgress(QString, const QByteArray&, int)), this, SLOT(OnFileProgress(QString, const QByteArray&, int)));
	connect(this->receiver, SIGNAL(FileStarted(QString, int, int)), this, SLOT(OnFileStarted(QString, int, int)));

	// open receiver
	bool isListening = this->receiver->Open();

	while(!this->shouldStop)
	{
		this->receiver->Update();
		QThread::yieldCurrentThread();
		QThread::msleep(500);
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
ReceiverThread::OnFileRequested( QString file, int index )
{
	emit this->FileRequested(file, index);
}

//------------------------------------------------------------------------------
/**
*/
void 
ReceiverThread::OnFileDone( QString file, int index )
{
	emit this->FileDone(file, index);
}

//------------------------------------------------------------------------------
/**
*/
void 
ReceiverThread::OnFileProgress( QString file, const QByteArray& chunk, int index )
{
	emit this->FileProgress(file, chunk, index);
}

//------------------------------------------------------------------------------
/**
*/
void 
ReceiverThread::OnFileStarted( QString file, int chunks, int index )
{
	emit this->FileStarted(file, chunks, index);
}

//------------------------------------------------------------------------------
/**
*/
void 
ReceiverThread::OnFileAccepted( QString file, int index )
{
	this->receiver->OnAcceptFile(file, index);
}

//------------------------------------------------------------------------------
/**
*/
void 
ReceiverThread::OnFileDenied( QString file, int index )
{
	this->receiver->OnDenyFile(file, index);
}

