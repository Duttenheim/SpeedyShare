#include "speedyshare.h"
#include <QtWidgets/QMessageBox>


//------------------------------------------------------------------------------------
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

	/*
	QRegExpValidator *v = new QRegExpValidator(this);
	QRegExp rx("((1{0,1}[0-9]{0,2}|2[0-4]{1,1}[0-9]{1,1}|25[0-5]{1,1})\\.){3,3}(1{0,1}[0-9]{0,2}|2[0-4]{1,1}[0-9]{1,1}|25[0-5]{1,1})");
	v->setRegExp(rx);
	this->dialogUi.address->setValidator(v);
	*/

	connect(this->ui.connectButton, SIGNAL(pressed()), this, SLOT(OnConnectPressed()));
	connect(this->ui.sendButton, SIGNAL(pressed()), this, SLOT(OnSendPressed()));
	connect(&this->receiverThread, SIGNAL(FileRequested(const QString&, int)), this, SLOT(OnFileRequested(const QString&, int)));
	connect(&this->receiverThread, SIGNAL(FileDone(const QString&, int)), this, SLOT(OnFileDone(const QString&, int)));
	connect(&this->receiverThread, SIGNAL(FileProgress(const QString&, const QByteArray&, int)), this, SLOT(OnFileProgress(const QString&, const QByteArray&, int)));
	connect(&this->receiverThread, SIGNAL(FileStarted(const QString&, int, int)), this, SLOT(OnFileStarted(const QString&, int, int)));
	connect(this, SIGNAL(FileAccepted(const QString&, int)), &this->receiverThread, SLOT(OnFileAccepted(const QString&, int)));
	connect(this, SIGNAL(FileDenied(const QString&, int)), &this->receiverThread, SLOT(OnFileDenied(const QString&, int)));
	connect(&this->senderThread, SIGNAL(ConnectionSuccessful()), this, SLOT(OnSenderConnected()));
	connect(&this->senderThread, SIGNAL(Disconnected()), this, SLOT(OnSenderDisconnected()));	

	// open receiver
	this->receiverThread.Start();
}

//------------------------------------------------------------------------------------
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
SpeedyShare::OnFileRequested(const QString& file, int index)
{
	QMessageBox box;
	box.setText("File: " + file + " is requested for download");
	box.setStandardButtons(QMessageBox::Save | QMessageBox::Abort);
	int result = box.exec();

	if (result == QMessageBox::Save)
	{
		QFileDialog dialog;
		dialog.setFileMode(QFileDialog::AnyFile);
		dialog.setAcceptMode(QFileDialog::AcceptSave);

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
				this->progressMap[index][file] = bar;
				this->fileMap[index][file] = fileHandle;
				this->labelMap[index][file] = label;
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
SpeedyShare::OnFileDone( const QString& file, int index )
{
	Q_ASSERT(this->fileMap[index].contains(file));

	// delete and remove label
	QLabel* label = this->labelMap[index][file];

	// update text
	label->setText("Writing: " + file + " to file...");
	QApplication::processEvents();

	// get file handle
	QFile* fileHandle = this->fileMap[index][file];
	fileHandle->close();
	delete fileHandle;

	// delete and remove progress bar
	QProgressBar* progressBar = this->progressMap[index][file];
	this->ui.downloadLayout->removeWidget(progressBar);
	delete progressBar;

	this->ui.downloadLayout->removeWidget(label);
	delete label;
}

//------------------------------------------------------------------------------
/**
*/
void 
SpeedyShare::OnFileProgress( const QString& file, const QByteArray& chunk, int index )
{
	Q_ASSERT(this->fileMap[index].contains(file));

	// get file
	QFile* fileHandle = this->fileMap[index][file];
	Q_ASSERT(fileHandle->isOpen());
	fileHandle->write(chunk);

	// get progress bar
	QProgressBar* bar = this->progressMap[index][file];
	bar->setValue(bar->value() + 1);


}

//------------------------------------------------------------------------------
/**
*/
void 
SpeedyShare::OnFileStarted( const QString& file, int chunks, int index )
{
	Q_ASSERT(this->fileMap[index].contains(file));

	// get progress bar
	QProgressBar* progressBar = this->progressMap[index][file];
	progressBar->setMaximum(chunks);

	// set text of label
	QLabel* label = this->labelMap[index][file];
	label->setText("Downloading: " + file);
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

					this->senderThread.Enqueue(package);
				}
			}
			else
			{
				qErrnoWarning("Could not open file: %s/n", file.toUtf8().constData());
			}			
		}
		
	}
}
