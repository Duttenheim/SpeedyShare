#include "speedyshare.h"

#if __QT5__
#include <QtWidgets/QMessageBox>
#else
#include <QtGui/QMessageBox>
#endif

#include <QtNetwork/QNetworkProxy>

//------------------------------------------------------------------------------
/**
*/
SpeedyShare::SpeedyShare(QWidget *parent, Qt::WindowFlags flags) : 
	QMainWindow(parent, flags),
	isConnected(false)
{
	this->ui.setupUi(this);
	this->dialogUi.setupUi(&this->connectDialog);

	// setup file dialog
	this->fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
	this->fileDialog.setFileMode(QFileDialog::ExistingFiles);


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

	connect(&this->senderThread, SIGNAL(FileDenied(const QString&)), this, SLOT(OnFileDenied(const QString&)));
	connect(&this->senderThread, SIGNAL(FileDone(const QString&)), this, SLOT(OnFileSendDone(const QString&)));
	connect(&this->senderThread, SIGNAL(FileProgress(const QString&, int)), this, SLOT(OnFileSendProgress(const QString&, int)));
	connect(&this->senderThread, SIGNAL(FileStarted(const QString&, int)), this, SLOT(OnFileSendStarted(const QString&, int)));

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
	// close receiver
	this->receiverThread.Stop();
	this->senderThread.Stop();
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
		this->OnSenderDisconnected();
	}
	else
	{
		// stop thread
		this->senderThread.Stop();

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

	// get progress bar
	QProgressBar* bar = this->progressReceiveMap[index][file];

	// rescale progress bar
	bar->setMinimum(0);
	bar->setMaximum(chunks);
	bar->setValue(0);

	// set text of label
	QLabel* label = this->labelReceiveMap[index][file];
	label->setText("Downloading: " + file);
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
			// get file
			QString file = files[i];
			file = file.split("/").back();

			// read files
			QFile* fileHandle = new QFile(files[i]);
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

					this->ui.uploadLayout->addWidget(bar);
					this->ui.uploadLayout->addWidget(label);
					this->fileSendMap[file] = fileHandle;
					this->progressSendMap[file] = bar;
					this->labelSendMap[file] = label;
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
		}
		
	}
}
