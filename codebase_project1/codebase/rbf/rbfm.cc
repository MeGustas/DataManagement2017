#include "rbfm.h"
#include<stdio.h>
#include<unistd.h>
#include<math.h>
#include<stdlib.h>
#include<string.h>
#include<iostream>

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
	PagedFileManager *pfm = PagedFileManager::instance();
	return pfm->createFile(fileName);
}

RC RecordBasedFileManager::destroyFile(const string &fileName) {
	PagedFileManager *pfm = PagedFileManager::instance();
	return pfm->destroyFile(fileName);
}

RC RecordBasedFileManager::openFile(const string &fileName, FileHandle &fileHandle) {
	PagedFileManager *pfm = PagedFileManager::instance();
	return pfm->openFile(fileName, fileHandle);
}

RC RecordBasedFileManager::closeFile(FileHandle &fileHandle) {
	PagedFileManager *pfm = PagedFileManager::instance();
	return pfm->closeFile(fileHandle);
}

RC RecordBasedFileManager::insertRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const void *data, RID &rid) {
	/* Suppose you have five fields and their types are varchar(20), integer, varchar(20), real, and string.
	 * If a record is ("Tom", 25, "UCIrvine", 3.1415, 100),
	 * then the format of the record should be:
	 * [1 byte for the null-indicators for the fields: bit 00000000]
	 * [4 bytes for the length 3]
	 * [3 bytes for the string "Tom"]
	 * [4 bytes for the integer value 25]
	 * [4 bytes for the length 8]
	 * [8 bytes for the string "UCIrvine"]
	 * [4 bytes for the float value 3.1415]
	 * [4 bytes for the integer value 100].
	 * Note that integer and real type fields do not have an associated length value in front of them;
	 * this is because each of these types always occupies 4 bytes.
	 */
	//calculate the record's length
	int offset = 0;
	// Null-indicators
	int nullFieldsIndicatorActualSize = ceil((double) recordDescriptor.size() / CHAR_BIT);
	unsigned char *nullsIndicator = (unsigned char *) malloc(nullFieldsIndicatorActualSize);
	memset(nullsIndicator, 0, nullFieldsIndicatorActualSize);
	// Null-indicator for the fields
	memcpy(nullsIndicator,(char *)data + offset, nullFieldsIndicatorActualSize);
	offset += nullFieldsIndicatorActualSize;

	bool nullBit = false;
	int iLength = 0;
	for(unsigned int iLoop = 0; iLoop < recordDescriptor.size(); iLoop++ )	{
		// Is this field not-NULL?
		nullBit = nullsIndicator[0] & (1 << (nullFieldsIndicatorActualSize - iLoop));
		if (!nullBit) {
			switch(recordDescriptor[iLoop].type) {
			case TypeInt:
				offset += sizeof(int);
				break;
			case TypeReal:
				offset += sizeof(float);
				break;
			case TypeVarChar:
				memcpy(&iLength, (char *)data + offset, sizeof(int));
				offset += sizeof(int);
				offset += iLength;
				break;
			default:
				return -1;
			}
		}
	}
	int iRecSize = offset;

	unsigned pageCount = fileHandle.getNumberOfPages();
	void *buffer = malloc(PAGE_SIZE);
	if(0 != pageCount){
		if( 0 != fileHandle.readPage(pageCount - 1, buffer)){
			return -1;
		}
		int ifreespace = 0;
		memcpy(&ifreespace, (char*)buffer, sizeof(int));
		if(ifreespace >= iRecSize){
			int ioffset = PAGE_SIZE - ifreespace;
			memcpy((char*)buffer + ioffset, data, iRecSize);

			rid.pageNum = pageCount -1;
			rid.slotNum = (PAGE_SIZE - ifreespace - sizeof(int))/iRecSize;

			ifreespace = ifreespace - iRecSize;
			memcpy(buffer, &ifreespace, sizeof(int));
			free(buffer);
			return 0;
		}
	}

	int ifreespace = PAGE_SIZE - sizeof(int) - iRecSize;
	memcpy(buffer, &ifreespace, sizeof(int));
	memcpy((char *)buffer + sizeof(int), (char *)data, iRecSize);

	if(0 != fileHandle.appendPage(buffer)){
		return -1;
	}
	rid.pageNum = fileHandle.getNumberOfPages() -1;
	rid.slotNum = 0;

	free(buffer);
	return 0;
}

RC RecordBasedFileManager::readRecord(FileHandle &fileHandle, const vector<Attribute> &recordDescriptor, const RID &rid, void *data) {

	void *buffer = malloc(PAGE_SIZE);
	if( 0 != fileHandle.readPage(rid.pageNum, buffer)){
		return -1;
	}

	int iRecSize = 0;
	for(unsigned int iLoop = 0; iLoop < recordDescriptor.size(); iLoop++ )	{
		iRecSize += recordDescriptor[iLoop].length;
	}

	int ioffset = sizeof(int) + rid.slotNum* iRecSize;
	memcpy((char *)data, (char *)buffer + ioffset, iRecSize);
	free(buffer);

	return 0;
}

RC RecordBasedFileManager::printRecord(const vector<Attribute> &recordDescriptor, const void *data) {

	int offset = 0;

    // Null-indicators
    int nullFieldsIndicatorActualSize = ceil((double) recordDescriptor.size() / CHAR_BIT);
    unsigned char *nullsIndicator = (unsigned char *) malloc(nullFieldsIndicatorActualSize);
    memset(nullsIndicator, 0, nullFieldsIndicatorActualSize);

    // Null-indicator for the fields
	memcpy(nullsIndicator,(char *)data + offset, nullFieldsIndicatorActualSize);
	offset += nullFieldsIndicatorActualSize;

	// Beginning of the actual data
	// Note that the left-most bit represents the first field. Thus, the offset is 7 from right, not 0.
	// e.g., if a record consists of four fields and they are all nulls, then the bit representation will be: [11110000]
	bool nullBit = false;
	int iVar = 0;
	float fVar = 0.0;
	int iLength = 0;
	unsigned char* content;
	for(unsigned int iLoop = 0; iLoop < recordDescriptor.size(); iLoop++ )	{
		// Is this field not-NULL?
		nullBit = nullsIndicator[0] & (1 << (7 - iLoop));
		if (!nullBit) {
			switch(recordDescriptor[iLoop].type) {
			case TypeInt:
				memcpy( &iVar,(char *)data + offset, sizeof(int));
				offset += sizeof(int);
				cout<<recordDescriptor[iLoop].name<<" : "<<iVar<<endl;
				break;
			case TypeReal:
				memcpy( &fVar,(char *)data + offset, sizeof(int));
				offset += sizeof(float);
				cout<<recordDescriptor[iLoop].name<<" : "<<fVar<<endl;
				break;
			case TypeVarChar:
				memcpy(&iLength, (char *)data + offset, sizeof(int));
				offset += sizeof(int);
				content = (unsigned char *) malloc(iLength+1);
				memset(content, 0, iLength);
				memcpy(content, (char *)data + offset, iLength);
				content[iLength] = '\0';
				offset += iLength;
				cout<<recordDescriptor[iLoop].name<<" : "<<content<<endl;
				free(content);
				break;
			default:
				return -1;
			}
		}
	}
	return 0;
}
