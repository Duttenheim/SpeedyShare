#include "filepackage.h"
//------------------------------------------------------------------------------
/**
*/
FilePackage::FilePackage(void)
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
FilePackage::FilePackage( const FilePackage& package )
{
	this->data = package.data;
	this->name = package.name;
	this->file = package.file;
}

//------------------------------------------------------------------------------
/**
*/
FilePackage::~FilePackage(void)
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
void
FilePackage::operator=( const FilePackage& package )
{
	this->data = package.data;
	this->name = package.name;
	this->file = package.file;
}

