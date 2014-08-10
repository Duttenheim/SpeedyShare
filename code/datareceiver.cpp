#include "datareceiver.h"

#if __QT5__
#include <QtWidgets/QMessageBox>
#else
#include <QtGui/QMessageBox>
#endif

#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QNetworkProxy>
#include "config.h"

//------------------------------------------------------------------------------
/**
*/
DataReceiver::DataReceiver(void)
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
DataReceiver::~DataReceiver(void)
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
bool 
DataReceiver::Open()
{
	return this->listen(QHostAddress::Any, MAINPORT);
}

//------------------------------------------------------------------------------
/**
*/
bool
DataReceiver::ReOpen(uint port)
{
    this->close();
    return this->listen(QHostAddress::Any, port);
}

//------------------------------------------------------------------------------
/**
*/
void 
DataReceiver::Close()
{
    int i;
    for (i = 0; i < this->dataHandlers.size(); i++)
    {
        // disconnect to avoid double signals
        disconnect(this->dataHandlers[i], SIGNAL(ConnectionDied()), this, SLOT(OnConnectionDied()));
        this->dataHandlers[i]->Kill();
        delete this->dataHandlers[i];
    }
    this->dataHandlers.clear();
	this->close();
}

//------------------------------------------------------------------------------
/**
*/
void 
DataReceiver::Update()
{
	// poll incoming connections
	if (this->waitForNewConnection(0))
	{
		// get socket
		QTcpSocket* newSocket = this->nextPendingConnection();
		DataReceiverHandler* dataHandler = new DataReceiverHandler(newSocket);
		connect(dataHandler, SIGNAL(NewRequest(const QString&, const QString&)), this, SLOT(OnFileRequested(const QString&, const QString&)));
		connect(dataHandler, SIGNAL(TransactionDone(const QString&)), this, SLOT(OnFileDone(const QString&)));
		connect(dataHandler, SIGNAL(TransactionProgress(const QString&, const QByteArray&)), this, SLOT(OnFileProgress(const QString&, const QByteArray&)));
		connect(dataHandler, SIGNAL(TransactionStarted(const QString&, int)), this, SLOT(OnFileStart(const QString&, int)));
        connect(dataHandler, SIGNAL(ConnectionDied()), this, SLOT(OnConnectionDied()));
		this->dataHandlers.append(dataHandler);
	}

	// update connections
	int i;
	for (i = 0; i < this->dataHandlers.size(); i++)
	{
		this->dataHandlers[i]->Update();
	}
}

//------------------------------------------------------------------------------
/**
*/
void 
DataReceiver::OnConnectionDied()
{
	// get handler
	DataReceiverHandler* handler = static_cast<DataReceiverHandler*>(this->sender());
	
	// get index of connection
	int index = this->dataHandlers.indexOf(handler);
    delete handler;
	this->dataHandlers.removeAt(index);
}

//------------------------------------------------------------------------------
/**
*/
void 
DataReceiver::OnFileRequested( const QString& file, const QString& peer )
{
	// get handler
	DataReceiverHandler* handler = static_cast<DataReceiverHandler*>(this->sender());
	int index = this->dataHandlers.indexOf(handler);
	emit this->NewRequest(file, peer, index);
}

//------------------------------------------------------------------------------
/**
*/
void 
DataReceiver::OnFileDone( const QString& file )
{
	// get handler
	DataReceiverHandler* handler = static_cast<DataReceiverHandler*>(this->sender());
	int index = this->dataHandlers.indexOf(handler);
	emit this->FileDone(file, index);
}

//------------------------------------------------------------------------------
/**
*/
void 
DataReceiver::OnFileStart( const QString& file, int chunks )
{
	// get handler
	DataReceiverHandler* handler = static_cast<DataReceiverHandler*>(this->sender());
	int index = this->dataHandlers.indexOf(handler);
	emit this->FileStarted(file, chunks, index);
}

//------------------------------------------------------------------------------
/**
*/
void 
DataReceiver::OnFileProgress( const QString& file, const QByteArray& chunk )
{
	// get handler
	DataReceiverHandler* handler = static_cast<DataReceiverHandler*>(this->sender());
	int index = this->dataHandlers.indexOf(handler);
	emit this->FileProgress(file, chunk, index);
}

//------------------------------------------------------------------------------
/**
*/
void 
DataReceiver::OnAcceptFile( const QString& file, int index )
{
	// pass through the notification
	DataReceiverHandler* dataHandler = this->dataHandlers[index];
	dataHandler->OnAcceptFile(file);
}

//------------------------------------------------------------------------------
/**
*/
void 
DataReceiver::OnDenyFile( const QString& file, int index )
{
	// pass through the notification
	DataReceiverHandler* dataHandler = this->dataHandlers[index];
	dataHandler->OnDenyFile(file);
}