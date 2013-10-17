#ifndef SPEEDYSHARE_H
#define SPEEDYSHARE_H

#if __QT5__
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QProgressBar>
#else
#include <QtGui/QMainWindow>
#include <QtGui/QFileDialog>
#include <QtGui/QProgressBar>
#endif

#include "senderthread.h"
#include "receiverthread.h"
#include "ui_speedyshare.h"
#include "ui_connectdialog.h"

class SpeedyShare : public QMainWindow
{
	Q_OBJECT

public:
	/// constructor
	SpeedyShare(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	/// destructor
	~SpeedyShare();

	/// called whenever the window is closed
	void closeEvent(QCloseEvent* e);

signals:
	/// emitted when a file is accepted
	void FileAccepted(QString file, int index);
	/// emitted when a file is denied
	void FileDenied(QString file, int index);
private slots:
	/// called whenever the connect button is pressed
	void OnConnectPressed();
public slots:
	/// called whenever the sender is connected
	void OnSenderConnected();
	/// called whenever the sender is disconnected
	void OnSenderDisconnected();
	/// called whenever a file is requested
	void OnFileRequested(QString file, int index);

	/// called whenever a file is done
	void OnFileReceiveDone(QString file, int index);
	/// called when a file is progressing
	void OnFileReceiveProgress(QString file, const QByteArray& chunk, int index);
	/// called whenever a file progress is done
	void OnFileReceiveStarted(QString file, int chunks, int index);

	/// called whenever a file is done
	void OnFileSendDone(QString file);
	/// called when a file is progressing
	void OnFileSendProgress(QString file, int numBytes);
	/// called whenever a file progress is done
	void OnFileSendStarted(QString file, int chunks);

	/// called whenever the send button is pressed
	void OnSendPressed();

private:
	ReceiverThread receiverThread;
	SenderThread senderThread;
	QDialog connectDialog;
	QFileDialog fileDialog;
	Ui::ConnectDialog dialogUi;
	Ui::SpeedyShareClass ui;
	QMap<int, QMap<QString, QFile*> > fileReceiveMap;
	QMap<int, QMap<QString, QProgressBar*> > progressReceiveMap;
	QMap<int, QMap<QString, QLabel*> > labelReceiveMap;
	QMap<QString, QFile*> fileSendMap;
	QMap<QString, QProgressBar*> progressSendMap;
	QMap<QString, QLabel*> labelSendMap;
	bool isConnected;
};

#endif // SPEEDYSHARE_H
