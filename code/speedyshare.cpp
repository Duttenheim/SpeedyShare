#include "speedyshare.h"

#if __QT5__
#include <QtWidgets/QMessageBox>
#else
#include <QtGui/QMessageBox>
#endif

#include <QtCore/QUrl>
#include <QtCore/QDataStream>
#include <QtCore/QMimeData>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QHostInfo>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QNetworkInterface>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QList>
#include "config.h"
#include "version.h"

//------------------------------------------------------------------------------
/**
*/
SpeedyShare::SpeedyShare(QWidget *parent, Qt::WindowFlags flags) : 
	QMainWindow(parent, flags),
	isConnected(false)
{
	this->ui.setupUi(this);
	this->dialogUi.setupUi(&this->connectDialog);
    this->browserUi.setupUi(&this->networkBrowser);
    this->setAcceptDrops(true);

    QString version;
    version.sprintf(" (build %d.%d)", speedyshare_VERSION_MAJOR, speedyshare_VERSION_MINOR);
    this->setWindowTitle(this->windowTitle() + version);

	// setup file dialog
	this->fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
	this->fileDialog.setFileMode(QFileDialog::ExistingFiles);
	this->dialogUi.port->setValue(MAINPORT);

    // setup service announcement socket
    this->serviceSocket.bind(SERVICEPORT);
    this->serviceUpdateTimer.setInterval(16);
    connect(&this->serviceUpdateTimer, SIGNAL(timeout()), this, SLOT(OnServiceUpdate()));
    this->serviceUpdateTimer.start();

	// connect signals for threads
	connect(this->ui.connectButton, SIGNAL(pressed()), this, SLOT(OnConnectPressed()));
	connect(this->ui.sendButton, SIGNAL(pressed()), this, SLOT(OnSendPressed()));
	connect(&this->receiverThread, SIGNAL(FileRequested(const QString&, const QString&, int)), this, SLOT(OnFileRequested(const QString&, const QString&, int)));
	connect(&this->receiverThread, SIGNAL(FileDone(const QString&, int)), this, SLOT(OnFileReceiveDone(const QString&, int)));
	connect(&this->receiverThread, SIGNAL(FileProgress(const QString&, const QByteArray&, int)), this, SLOT(OnFileReceiveProgress(const QString&, const QByteArray&, int)));
	connect(&this->receiverThread, SIGNAL(FileStarted(const QString&, int, int)), this, SLOT(OnFileReceiveStarted(const QString&, int, int)));
	connect(this, SIGNAL(FileAccepted(const QString&, int)), &this->receiverThread, SLOT(OnFileAccepted(const QString&, int)));
	connect(this, SIGNAL(FileDenied(const QString&, int)), &this->receiverThread, SLOT(OnFileDenied(const QString&, int)));
	connect(&this->senderThread, SIGNAL(ConnectionSuccessful()), this, SLOT(OnSenderConnected()));
	connect(&this->senderThread, SIGNAL(Disconnected()), this, SLOT(OnSenderDisconnected()));	

    // file transfering
	connect(&this->senderThread, SIGNAL(FileDenied(const QString&)), this, SLOT(OnFileDenied(const QString&)));
	connect(&this->senderThread, SIGNAL(FileDone(const QString&)), this, SLOT(OnFileSendDone(const QString&)));
	connect(&this->senderThread, SIGNAL(FileProgress(const QString&, int)), this, SLOT(OnFileSendProgress(const QString&, int)));
	connect(&this->senderThread, SIGNAL(FileStarted(const QString&, int)), this, SLOT(OnFileSendStarted(const QString&, int)));    

    // network browsing
    connect(this->ui.networkBrowse, SIGNAL(pressed()), this, SLOT(OnBrowseNetwork()));
    connect(this, SIGNAL(SpeedySharePeerFound(const QHostAddress&)), this, SLOT(OnAddPeer(const QHostAddress&)));
    connect(this->browserUi.peerList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(OnPeerClicked(QListWidgetItem*)));

	// connect to Google DNS to resolve local IP
	QTcpSocket socket;
	socket.connectToHost("8.8.8.8", 53);
	if (socket.waitForConnected())
	{
        this->localAddress = socket.localAddress();
		this->ui.ipLabel->setText("Local IP: " + socket.localAddress().toString());

        // fetch IP from amazon 
        QNetworkRequest request(QUrl("http://checkip.amazonaws.com/"));
        QNetworkReply* reply = this->networkManager.get(request);
        connect(reply, SIGNAL(readyRead()), this, SLOT(OnPublicIp()));
	}
	else
	{
		this->ui.ipLabel->setText("No internet access");
        this->ui.publicIp->setText("No internet access");
	}

	// open receiver
	this->receiverThread.Start();
}

//------------------------------------------------------------------------------
/**
*/
SpeedyShare::~SpeedyShare()
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
void 
SpeedyShare::closeEvent( QCloseEvent* e )
{
    // close service announcer
    this->serviceUpdateTimer.stop();
    this->serviceSocket.close();

	// close receiver
	this->receiverThread.Stop();
	this->senderThread.Stop();
	
	// wait for threads to die
	while (this->receiverThread.isRunning());
	while (this->senderThread.isRunning());
}

//------------------------------------------------------------------------------
/**
*/
void 
SpeedyShare::dragEnterEvent(QDragEnterEvent* e)
{
    if (e->mimeData()->hasUrls())
    {
        e->acceptProposedAction();
    }
}

//------------------------------------------------------------------------------
/**
*/
void 
SpeedyShare::dropEvent(QDropEvent* e)
{
    const QMimeData* data = e->mimeData();
    if (this->isConnected)
    {
        if (data->hasUrls())
        {
            QStringList pathList;
            QList<QUrl> urlList = data->urls();

            // extract the local paths of the files
            for (int i = 0; i < urlList.size(); ++i)
            {
                this->SendFile(urlList.at(i).toLocalFile());
            }
        }
    }
}

//------------------------------------------------------------------------------
/**
*/
void 
SpeedyShare::OnConnectPressed()
{
	if (this->isConnected)
	{
        this->senderThread.Stop();
        this->receiverThread.Stop();
	}
	else
	{
		// stop thread
		if (this->senderThread.Stop())
        {
            // open dialog
            int res = this->connectDialog.exec();
            if (res == QDialog::Accepted)
            {
                // construct address
                QHostAddress address;
                address.setAddress(this->dialogUi.address->text());
                quint16 port = this->dialogUi.port->value();

                // set data in thread
                this->senderThread.SetAddress(address);
                this->senderThread.SetPort(port);

                // open thread
                this->senderThread.Start();
            }
        }
	}
}

//------------------------------------------------------------------------------
/**
*/
void 
SpeedyShare::OnServiceUpdate()
{
    if (this->serviceSocket.bytesAvailable() > 0)
    {
        char* buf = new char[256];
        QHostAddress receiver;
        qint64 size = this->serviceSocket.readDatagram(buf, 256, &receiver);
        QByteArray data(buf, size);
        QDataStream instream(&data, QIODevice::ReadOnly);

        qint32 magic;
        instream >> magic;
        
        // if you get asked if you are speedyshare, respond with YES
        if (magic == ARE_YOU_SPEEDYSHARE)
        {
            // get ip from sender
            QString sender;
            instream >> sender;

            // clear buffer and use the byte array to write some new stuff
            QByteArray data;
            QDataStream outstream(&data, QIODevice::WriteOnly);
            outstream << PEER_IS_SPEEDYSHARE;     

            // we write our local address because the response we will get from the broadcast is the router
            outstream << this->localAddress.toString();
            this->serviceSocket.writeDatagram(data, QHostAddress(sender), SERVICEPORT);
            this->serviceSocket.waitForBytesWritten(-1);
        }
        else if (magic == PEER_IS_SPEEDYSHARE)
        {
            // get address from string
            QString me = this->localAddress.toString();
            QString them;
            instream >> them;
            if (me != them)
            {
                QHostAddress addr(them);
                this->localPeers.append(addr);
                emit this->SpeedySharePeerFound(addr);
            }            
        }

        delete [] buf;
    }
}

//------------------------------------------------------------------------------
/**
*/
void 
SpeedyShare::OnBrowseNetwork()
{
    // clear network browser
    this->browserUi.peerList->clear();
    this->networkBrowser.show();
    this->networkBrowser.raise();

    // broadcast message
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << ARE_YOU_SPEEDYSHARE;
    stream << this->localAddress.toString();

    QUdpSocket broadcastSocket;
    broadcastSocket.writeDatagram(data, QHostAddress::Broadcast, SERVICEPORT);
    broadcastSocket.waitForBytesWritten(-1);
}

//------------------------------------------------------------------------------
/**
*/
void 
SpeedyShare::OnPublicIp()
{
    QNetworkReply* reply = (QNetworkReply*)this->sender();
    QByteArray data = reply->readAll();
    QString label("Public IP: ");
    label.append(data);
    this->ui.publicIp->setText(label);
    disconnect(reply, SIGNAL(readyRead()));
}

//------------------------------------------------------------------------------
/**
*/
void 
SpeedyShare::OnAddPeer(const QHostAddress& addr)
{
    // add to list
    this->browserUi.peerList->addItem(addr.toString());
}

//------------------------------------------------------------------------------
/**
*/
void 
SpeedyShare::OnPeerClicked(QListWidgetItem* item)
{
    // stop thread
    if (this->senderThread.Stop())
    {
        // construct address
        QHostAddress address;
        address.setAddress(item->text());
        quint16 port = this->dialogUi.port->value();

        // set data in thread
        this->senderThread.SetAddress(address);
        this->senderThread.SetPort(port);

        // open thread
        this->senderThread.Start();
    }    
}

//------------------------------------------------------------------------------
/**
*/
void 
SpeedyShare::OnSenderConnected()
{
	QString string;
	string.sprintf("Disconnect from: %s", this->senderThread.GetAddress().toString().toUtf8().constData());
	this->ui.connectButton->setText(string);
	string.sprintf("Connected to: %s", this->senderThread.GetAddress().toString().toUtf8().constData());
	this->ui.statusLabel->setText(string);
	this->ui.sendButton->setEnabled(true);
	this->isConnected = true;
}

//------------------------------------------------------------------------------
/**
*/
void 
SpeedyShare::OnSenderDisconnected()
{
	this->ui.connectButton->setText("Connect...");
	this->ui.statusLabel->setText("Not connected");
	this->ui.sendButton->setEnabled(false);
	this->isConnected = false;
}

//------------------------------------------------------------------------------
/**
*/
void 
SpeedyShare::OnFileRequested(const QString& file, const QString& peer, int index)
{
	QMessageBox box;
	box.setText("File: " + file + " is requested to be uploaded from " + peer);
	box.setStandardButtons(QMessageBox::Save | QMessageBox::Abort);
	int result = box.exec();

	if (result == QMessageBox::Save)
	{
		QFileDialog dialog;
		dialog.setFileMode(QFileDialog::AnyFile);
		dialog.setAcceptMode(QFileDialog::AcceptSave);
		dialog.selectFile(file);
		result = dialog.exec();

		if (result == QDialog::Accepted)
		{
			QStringList files = dialog.selectedFiles();
			const QString& fileName = files[0];
			QFile* fileHandle = new QFile(fileName);
			if (fileHandle->open(QIODevice::WriteOnly))
			{
				QProgressBar* bar = new QProgressBar(this);
				QLabel* label = new QLabel(this);
				this->ui.downloadLayout->addWidget(bar);
				this->ui.downloadLayout->addWidget(label);
				this->progressReceiveMap[index][file] = bar;
				this->fileReceiveMap[index][file] = fileHandle;
				this->labelReceiveMap[index][file] = label;
				emit this->FileAccepted(file, index);
			}
			else
			{
				emit this->FileDenied(file, index);
			}
		}		
	}
	else
	{
		emit this->FileDenied(file, index);
	}
}

//------------------------------------------------------------------------------
/**
*/
void 
SpeedyShare::OnFileReceiveDone( const QString& file, int index )
{
	Q_ASSERT(this->fileReceiveMap[index].contains(file));

	// delete and remove label
	QLabel* label = this->labelReceiveMap[index][file];
	this->ui.downloadLayout->removeWidget(label);

	// update text
	QString msg;
	msg.sprintf("Writing %s to file", file.toUtf8().constData());
	label->setText(msg);
	QApplication::processEvents();

	// delete label
	delete label;

	// get file handle
	QFile* fileHandle = this->fileReceiveMap[index][file];
	fileHandle->close();
	delete fileHandle;

	// delete and remove progress bar
	QProgressBar* progressBar = this->progressReceiveMap[index][file];
	this->ui.downloadLayout->removeWidget(progressBar);
	delete progressBar;
}

//------------------------------------------------------------------------------
/**
*/
void 
SpeedyShare::OnFileReceiveProgress( const QString& file, const QByteArray& chunk, int index )
{
	Q_ASSERT(this->fileReceiveMap[index].contains(file));

	// get file
	QFile* fileHandle = this->fileReceiveMap[index][file];
	Q_ASSERT(fileHandle->isOpen());
	fileHandle->write(chunk);

	// get progress bar
	QProgressBar* bar = this->progressReceiveMap[index][file];
	bar->setValue(bar->value() + 1);
}

//------------------------------------------------------------------------------
/**
*/
void 
SpeedyShare::OnFileReceiveStarted( const QString& file, int chunks, int index )
{
	Q_ASSERT(this->fileReceiveMap[index].contains(file));
	
	// get file handle
	QFile* fileHandle = this->fileReceiveMap[index][file];

	// get progress bar
	QProgressBar* bar = this->progressReceiveMap[index][file];

	// rescale progress bar
	bar->setMinimum(0);
	bar->setMaximum(chunks);
	bar->setValue(0);

	// set text of label
	QLabel* label = this->labelReceiveMap[index][file];
	label->setTextFormat(Qt::RichText);
	label->setText("Downloading: " + file + "<br><b>to:</b><br>" + fileHandle->fileName());
}

//------------------------------------------------------------------------------
/**
*/
void 
SpeedyShare::OnFileDenied( const QString& file )
{
	Q_ASSERT(this->fileSendMap.contains(file));

	// get file
	QFile* fileHandle = this->fileSendMap[file];
	fileHandle->close();
	delete fileHandle;

	// get progress bar
	QProgressBar* bar = this->progressSendMap[file];
	this->ui.uploadLayout->removeWidget(bar);
	delete bar;

	// get label
	QLabel* label = this->labelSendMap[file];
	this->ui.uploadLayout->removeWidget(label);
	delete label;

    // get button
    QPushButton* abort = this->abortSendMap[file];
    this->ui.uploadLayout->removeWidget(abort);
    delete abort;
}

//------------------------------------------------------------------------------
/**
*/
void 
SpeedyShare::OnFileSendDone( const QString& file )
{
	Q_ASSERT(this->fileSendMap.contains(file));

	// get file
	QFile* fileHandle = this->fileSendMap[file];
	fileHandle->close();
	delete fileHandle;

	// get progress bar
	QProgressBar* bar = this->progressSendMap[file];
	this->ui.uploadLayout->removeWidget(bar);
	delete bar;

	// get label
	QLabel* label = this->labelSendMap[file];
	this->ui.uploadLayout->removeWidget(label);
	delete label;

	// get button
	QPushButton* button = this->abortSendMap[file];
	this->ui.uploadLayout->removeWidget(button);
	delete button;
}

//------------------------------------------------------------------------------
/**
*/
void 
SpeedyShare::OnFileSendProgress( const QString& file, int numBytes )
{
	Q_ASSERT(this->fileSendMap.contains(file));

	// get progress bar and update
	QProgressBar* bar = this->progressSendMap[file];
	bar->setValue(bar->value() + 1);
}

//------------------------------------------------------------------------------
/**
*/
void 
SpeedyShare::OnFileSendStarted( const QString& file, int chunks )
{
	Q_ASSERT(this->fileSendMap.contains(file));

	// get progress bar
	QProgressBar* bar = this->progressSendMap[file];
	bar->setMinimum(0);
	bar->setMaximum(chunks);
	bar->setValue(0);

	// get label
	QLabel* label = this->labelSendMap[file];
	label->setText("Uploading: " + file);
}

//------------------------------------------------------------------------------
/**
*/
void 
SpeedyShare::OnFileSendAborted()
{
	// abort current file
	this->senderThread.AbortCurrent();
}

//------------------------------------------------------------------------------
/**
*/
void 
SpeedyShare::OnSendPressed()
{
	int res = this->fileDialog.exec();
	if (res == QDialog::Accepted)
	{
		// get files
		QStringList files = this->fileDialog.selectedFiles();

		int i;
		for (i = 0; i < files.size(); i++)
		{
			// get file and send
			QString file = files[i];
            this->SendFile(file);			
		}
	}
}

//------------------------------------------------------------------------------
/**
*/
bool
SpeedyShare::SendFile(const QString& path)
{
    QString file = path.split("/").back();

    // read files
    QFile* fileHandle = new QFile(path);
    if (fileHandle->open(QIODevice::ReadOnly))
    {
        // only create package if thread is connected
        if (this->senderThread.IsConnected())
        {
            // create package
            FilePackage package;
            package.SetName(file);
            package.SetFile(fileHandle);

            // enqueue package in sender thread
            this->senderThread.Enqueue(package);

            // create UI for sender files
            QProgressBar* bar = new QProgressBar(this);
            QLabel* label = new QLabel(this);
            QPushButton* button = new QPushButton(this);

            // connect abort button
            connect(button, SIGNAL(pressed()), this, SLOT(OnFileSendAborted()));
            button->setText("Abort");

            this->ui.uploadLayout->addWidget(bar);
            this->ui.uploadLayout->addWidget(button);
            this->ui.uploadLayout->addWidget(label);
            this->fileSendMap[file] = fileHandle;
            this->progressSendMap[file] = bar;
            this->labelSendMap[file] = label;
            this->abortSendMap[file] = button;
            return true;
        }
        else
        {
            delete fileHandle;
        }
    }
    else
    {
        delete fileHandle;
        qErrnoWarning("Could not open file: %s/n", file.toUtf8().constData());
    }
    return false;
}

