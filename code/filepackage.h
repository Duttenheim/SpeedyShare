#pragma once
#include <QObject>
#include <QMutex>
#include <QByteArray>
#include <QString>
#include <QMetaType>
#include <QFile>
//------------------------------------------------------------------------------------
/**
	A package which contains all data necessary to be sent to another host
*/
//------------------------------------------------------------------------------------

class FilePackage
{
public:
	/// constructor
	FilePackage(void);
	/// destructor
	~FilePackage(void);
	/// copy-constructor
	FilePackage(const FilePackage& package);
	
	/// assignment operator
	void operator=(const FilePackage& package);

	/// sets data
	void SetData(const QByteArray& data);
	/// gets the data
	const QByteArray& GetData() const;
	/// sets the file
	void SetFile(QFile* file);
	/// gets the file
	QFile* GetFile() const;
	/// sets the file name
	void SetName(const QString& name);
	/// gets the file name
	const QString& GetName() const;
private:
	QString name;
	QByteArray data;
	QFile* file;
};

Q_DECLARE_METATYPE(FilePackage);
//------------------------------------------------------------------------------
/**
*/
inline void 
FilePackage::SetData( const QByteArray& data )
{
	QByteArray intermediate = data;
	this->data = intermediate;
}

//------------------------------------------------------------------------------
/**
*/
inline const QByteArray& 
FilePackage::GetData() const
{
	return this->data;
}

//------------------------------------------------------------------------------
/**
*/
inline void 
FilePackage::SetFile( QFile* file )
{
	Q_ASSERT(file);
	this->file = file;
}

//------------------------------------------------------------------------------
/**
*/
inline QFile* 
FilePackage::GetFile() const
{
	return this->file;
}

//------------------------------------------------------------------------------
/**
*/
inline void 
FilePackage::SetName( const QString& name )
{
	QString intermediate = name;
	this->name = intermediate; 
}

//------------------------------------------------------------------------------
/**
*/
inline const QString& 
FilePackage::GetName() const
{
	return this->name;
}