#include "rbfm.h"
#include<stdio.h>
#include<unistd.h>

RecordBasedFileManager* RecordBasedFileManager::_rbf_manager = 0;

RecordBasedFileManager* RecordBasedFileManager::instance()
{
    if(!_rbf_manager)
        _rbf_manager = new RecordBasedFileManager();

    return _rbf_manager;
}

RecordBasedFileManager::RecordBasedFileManager()
{
}

RecordBasedFileManager::~RecordBasedFileManager()
{
}

RC RecordBasedFileManager::createFile(const string &fileName) {
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

RC RecordBasedFileManager::destroyFile(const string &fileName) {
    return -1;
}

RC RecordBasedFileManager::openFile(const string &fileName, FileHandle &fileHandle) {
	//if not exist
	if(-1 == access(fileName.c_str(),F_OK))
	{
		return -1;
	}
	FILE* fp = NULL;
	fp = fopen(fileName.c_str(), "r+");
	if(NULL == fp)
	{
		return -1;
	}else{
		fileHandle.setFileHandle(fp);
		fp = NULL;
	}
	return 0;
}

RC RecordBasedFileManager::closeFile(FileHandle &fileHandle) {
    return -1;
}

RC RecordBasedFileManager::insertRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid) {
    return -1;
}

RC RecordBasedFileManager::readRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data) {
    return -1;
}

RC RecordBasedFileManager::printRecord(const vector<Attribute> &recordDescriptor, const void *data) {
    return -1;
}
