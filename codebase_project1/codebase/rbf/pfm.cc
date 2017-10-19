#include "pfm.h"
#include<stdio.h>
#include<unistd.h>

PagedFileManager* PagedFileManager::_pf_manager = 0;

PagedFileManager* PagedFileManager::instance()
{
    if(!_pf_manager)
        _pf_manager = new PagedFileManager();

    return _pf_manager;
}


PagedFileManager::PagedFileManager()
{
}


PagedFileManager::~PagedFileManager()
{
}


RC PagedFileManager::createFile(const string &fileName)
{
	if(0 == access(fileName.c_str(),F_OK))
	{
		return -1;
	}
	FILE *fp = NULL;
	fp = fopen(fileName.c_str(), "a+");
	if(NULL == fp)
	{
		return -1;
	}
	fclose(fp);
	fp = NULL;
    return 0;
}


RC PagedFileManager::destroyFile(const string &fileName)
{
	//if not exist
	if(-1 == access(fileName.c_str(),F_OK))
	{
		return -1;
	}
	if(-1 == remove(fileName.c_str()))
	{
		return -1;
	}

    return 0;
}


RC PagedFileManager::openFile(const string &fileName, FileHandle &fileHandle)
{
	//if not exist
	if(-1 == access(fileName.c_str(),F_OK))
	{
		return -1;
	}
	FILE* fp = NULL;
	fp = fopen(fileName.c_str(), "a+");
	if(NULL == fp)
	{
		return -1;
	}else{
		fileHandle.setFileHandle(fp);
		fp = NULL;
	}
	return 0;
}


RC PagedFileManager::closeFile(FileHandle &fileHandle)
{
	FILE* fp = fileHandle.getFileHandle();
	if(NULL == fp)
	{
		return -1;
	}else{
		fclose(fp);
		fp = NULL;
		fileHandle.setFileHandle(NULL);
	}
    return 0;
}

FileHandle::FileHandle()
{
    readPageCounter = 0;
    writePageCounter = 0;
    appendPageCounter = 0;

    _fileHandle = NULL;
    _numberOfPages = -1;
}


FileHandle::~FileHandle()
{
}


RC FileHandle::readPage(PageNum pageNum, void *data)
{
    return -1;
}


RC FileHandle::writePage(PageNum pageNum, const void *data)
{
    return -1;
}


RC FileHandle::appendPage(const void *data)
{
    return -1;
}


unsigned FileHandle::getNumberOfPages()
{
	if(-1 == this->_numberOfPages)
	{
		unsigned long filesize = -1;
		if(NULL == this->_fileHandle)
		{
			return -1;
		}
		fseek(this->_fileHandle, 0L, SEEK_END);
		filesize = ftell(this->_fileHandle);

		this->_numberOfPages = filesize/PAGE_SIZE;
	}
	return this->_numberOfPages;
}

RC FileHandle::collectCounterValues(unsigned &readPageCount, unsigned &writePageCount, unsigned &appendPageCount)
{
	readPageCount = this->readPageCounter;
	writePageCount = this->writePageCounter;
	appendPageCount = this->appendPageCounter;

    return 0;
}

RC FileHandle::setFileHandle(FILE* fh)
{
	this->_fileHandle = fh;
	return 0;
}

FILE* FileHandle::getFileHandle()
{
	return this->_fileHandle;
}
