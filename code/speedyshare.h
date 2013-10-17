#ifndef SPEEDYSHARE_H
#define SPEEDYSHARE_H

#include <QtWidgets/QMainWindow>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QProgressBar>
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
	void FileAccepted(const QString& file, int index);
	/// emitted when a file is denied
	void FileDenied(const QString& file, int index);
private slots:
	/// called whenever the connect button is pressed
	void OnConnectPressed();
public slots:
	/// called whenever the sender is connected
	void OnSenderConnected();
	/// called whenever the sender is disconnected
	void OnSenderDisconnected();
	/// called whenever a file is requested
	void OnFileRequested(const QString& file, int index);
	/// called whenever a file is done
	void OnFileDone(const QString& file, int index);
	/// called when a file is progressing
	void OnFileProgress(const QString& file, const QByteArray& chunk, int index);
	/// called whenever a file progress is done
	void OnFileStarted(const QString& file, int chunks, int index);
	/// called whenever the send button is pressed
	void OnSendPressed();
private:
	ReceiverThread receiverThread;
	SenderThread senderThread;
	QDialog connectDialog;
	QFileDialog fileDialog;
	Ui::ConnectDialog dialogUi;
	Ui::SpeedyShareClass ui;
	QMap<int, QMap<QString, QFile*> > fileMap;
	QMap<int, QMap<QString, QProgressBar*> > progressMap;
	QMap<int, QMap<QString, QLabel*> > labelMap;
	bool isConnected;
};

#endif // SPEEDYSHARE_H
