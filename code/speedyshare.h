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
#include <QtGui/QDropEvent>
#endif

#include <QtCore/QTimer>
#include <QtNetwork/QUdpSocket>
#include <QtNetwork/QNetworkAccessManager>
#include "senderthread.h"
#include "receiverthread.h"
#include "ui_speedyshare.h"
#include "ui_connectdialog.h"
#include "ui_networkbrowser.h"

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

    /// called whenever a drag enters the window
    void dragEnterEvent(QDragEnterEvent* e);    
    /// called when we get a file dragged and dropped
    void dropEvent(QDropEvent* e);

signals:
	/// emitted when a file is accepted
	void FileAccepted(const QString& file, int index);
	/// emitted when a file is denied
	void FileDenied(const QString& file, int index);
    /// emitted when we get a new peer
    void SpeedySharePeerFound(const QHostAddress& addr);
private slots:
	/// called whenever the connect button is pressed
	void OnConnectPressed();
    /// called whenever the service announcement socket needs updating
    void OnServiceUpdate();
    /// called when the browse network button gets pressed
    void OnBrowseNetwork();
    /// called whenever the amazon web service responded with our public ip
    void OnPublicIp();
    /// called whenever we find a peer on the network which uses speedyshare
    void OnAddPeer(const QHostAddress& addr);
    /// called whenever the user clicks on a peer in the network browser
    void OnPeerClicked(QListWidgetItem* item);
public slots:
	/// called whenever the sender is connected
	void OnSenderConnected();
	/// called whenever the sender is disconnected
	void OnSenderDisconnected();
	/// called whenever a file is requested
	void OnFileRequested(const QString& file, const QString& peer, int index);

	/// called whenever a file is done
	void OnFileReceiveDone(const QString& file, int index);
	/// called when a file is progressing
	void OnFileReceiveProgress(const QString& file, const QByteArray& chunk, int index);
	/// called whenever a file progress is done
	void OnFileReceiveStarted(const QString& file, int chunks, int index);

	/// called whenever the receiver denies a file
	void OnFileDenied(const QString& file);
	/// called whenever a file is done
	void OnFileSendDone(const QString& file);
	/// called when a file is progressing
	void OnFileSendProgress(const QString& file, int numBytes);
	/// called whenever a file progress is done
	void OnFileSendStarted(const QString& file, int chunks);
	/// called whenever the current file is aborted
	void OnFileSendAborted();

	/// called whenever the send button is pressed
	void OnSendPressed();

private:
    /// start a file send
    bool SendFile(const QString& path);

    QNetworkAccessManager networkManager;

	ReceiverThread receiverThread;
	SenderThread senderThread;
	QDialog connectDialog;
	QFileDialog fileDialog;
	Ui::ConnectDialog dialogUi;
	Ui::SpeedyShareClass ui;
    Ui::NetworkBrowser browserUi;
	QMap<int, QMap<QString, QFile*> > fileReceiveMap;
	QMap<int, QMap<QString, QProgressBar*> > progressReceiveMap;
	QMap<int, QMap<QString, QLabel*> > labelReceiveMap;
	QMap<QString, QFile*> fileSendMap;
	QMap<QString, QProgressBar*> progressSendMap;
	QMap<QString, QLabel*> labelSendMap;
	QMap<QString, QPushButton*> abortSendMap;

    QTimer serviceUpdateTimer;
    QUdpSocket serviceSocket;
    QHostAddress localAddress;
    QList<QHostAddress> localPeers;
    QDialog networkBrowser;

	bool isConnected;
};

#endif // SPEEDYSHARE_H
