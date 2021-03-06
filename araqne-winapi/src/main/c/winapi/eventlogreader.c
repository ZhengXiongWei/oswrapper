/*
 * Copyright 2010 NCHOVY
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <windows.h>
#include <strsafe.h>
#include "EventLogReader.h"

#define MAX_RECORD_BUFFER_SIZE 0x10000

JNIEXPORT jobject JNICALL Java_org_araqne_winapi_EventLogReader_readAllEventLogs(JNIEnv *env, jobject obj, jstring jLogName, jint begin) {
	jclass clzList = (*env)->FindClass(env, "java/util/ArrayList");
	jmethodID listInit = (*env)->GetMethodID(env, clzList, "<init>", "()V");
	jmethodID listAdd = (*env)->GetMethodID(env, clzList, "add", "(Ljava/lang/Object;)Z");
	jobject list = (*env)->NewObject(env, clzList, listInit);
	
	DWORD dwReadFlags = 0;
	LPVOID lpBuffer = (LPVOID)malloc(MAX_RECORD_BUFFER_SIZE);
	DWORD nNumberOfBytesToRead = MAX_RECORD_BUFFER_SIZE;
	DWORD pnBytesRead = 0;
	DWORD pnMinNumberOfBytesNeeded = 0;	

	LPTSTR lpLogName;
	HANDLE hEventLog;

	memset(lpBuffer, 0, MAX_RECORD_BUFFER_SIZE);

	if ( jLogName == (jstring)NULL )
	{
		jclass exceptionClass = (*env)->FindClass(env, "java/lang/NullPointerException");
		(*env)->ThrowNew(env, exceptionClass, "Should not input \"NULL\"");
		free(lpBuffer);
		return 0;
	}

	lpLogName = (LPTSTR)(*env)->GetStringChars(env, jLogName, JNI_FALSE);
	hEventLog = OpenEventLog(NULL, lpLogName);

	if(hEventLog == NULL) {
//		fwprintf(stderr, L"Error in OpenEventLog: 0x%x\n", GetLastError());
		free(lpBuffer);
		(*env)->ReleaseStringChars(env, jLogName, lpLogName);
		return 0;
	}

	dwReadFlags = EVENTLOG_SEQUENTIAL_READ | EVENTLOG_FORWARDS_READ;

	for(;;) {
		if(!ReadEventLog(hEventLog, dwReadFlags, begin, lpBuffer, nNumberOfBytesToRead, &pnBytesRead, &pnMinNumberOfBytesNeeded)) {
			DWORD error = GetLastError();
			LPVOID tempBuffer = NULL;

			if(error == ERROR_HANDLE_EOF || nNumberOfBytesToRead == 0)
				break;

			if(error == ERROR_INSUFFICIENT_BUFFER) {
				fwprintf(stderr, L"Error in ReadEventLog: 0x%x\n", error);
				CloseEventLog(hEventLog);
				free(lpBuffer);
				(*env)->ReleaseStringChars(env, jLogName, lpLogName);

				return 0;
			}

			tempBuffer = (LPVOID)realloc(lpBuffer, nNumberOfBytesToRead);
			if(tempBuffer == NULL) {
				fwprintf(stderr, L"Failed to reallocate the memory for the record buffer (%d bytes)\n", pnMinNumberOfBytesNeeded);
				CloseEventLog(hEventLog);
				free(lpBuffer);
				(*env)->ReleaseStringChars(env, jLogName, lpLogName);

				return 0;
			}
			lpBuffer = tempBuffer;
			nNumberOfBytesToRead = pnMinNumberOfBytesNeeded;
		} else {
			PBYTE pRead = lpBuffer;
			PBYTE pBufferEnd = pRead + pnBytesRead;

			while(pRead < pBufferEnd) {
				PEVENTLOGRECORD record = (PEVENTLOGRECORD)pRead;

				pRead += record->Length;
				if((signed)record->RecordNumber < begin)
					continue;

				(*env)->CallVoidMethod(env, list, listAdd, getEventLogObject(env, lpLogName, record));
				begin = record->RecordNumber + 1;
			}
		}
	}
	(*env)->ReleaseStringChars(env, jLogName, lpLogName);

	if(lpBuffer)
		free(lpBuffer);

	if(!CloseEventLog(hEventLog))
		fwprintf(stderr, L"Error in CloseEventLog: 0x%x\n", GetLastError());

	return list;
}

JNIEXPORT jobject JNICALL Java_org_araqne_winapi_EventLogReader_readEventLog(JNIEnv *env, jobject obj, jstring jLogName, jint begin) {
	jobject record = NULL;		
	DWORD dwReadFlags = EVENTLOG_SEEK_READ | EVENTLOG_FORWARDS_READ;
	LPVOID lpBuffer = (LPVOID)malloc(MAX_RECORD_BUFFER_SIZE);
	DWORD nNumberOfBytesToRead = MAX_RECORD_BUFFER_SIZE;
	DWORD pnBytesRead = 0;
	DWORD pnMinNumberOfBytesNeeded = 0;	
	LPTSTR lpLogName;
	HANDLE hEventLog;	

	memset(lpBuffer, 0, MAX_RECORD_BUFFER_SIZE);

	if ( jLogName == (jstring)NULL )
	{
		jclass exceptionClass = (*env)->FindClass(env, "java/lang/NullPointerException");
		(*env)->ThrowNew(env, exceptionClass, "Should not input \"NULL\"");
		free(lpBuffer);
		return 0;
	}

	lpLogName = (LPTSTR)(*env)->GetStringChars(env, jLogName, JNI_FALSE);
	hEventLog = OpenEventLog(NULL, lpLogName);

	if(lpLogName == NULL)
	{
		free(lpBuffer);
		fwprintf(stderr, L"Error in OpenEventLog: 0x%x\n", GetLastError());		
		return 0;
	}

	if(hEventLog == NULL) {
		free(lpBuffer);
		(*env)->ReleaseStringChars(env, jLogName, lpLogName);
		fwprintf(stderr, L"Error in OpenEventLog: 0x%x\n", GetLastError());
		return 0;
	}

	if(!ReadEventLog(hEventLog, dwReadFlags, begin, lpBuffer, nNumberOfBytesToRead, &pnBytesRead, &pnMinNumberOfBytesNeeded)) {
		DWORD error = GetLastError();
		LPVOID tempBuffer = NULL;

		if(error == ERROR_HANDLE_EOF) {
			free(lpBuffer);
			(*env)->ReleaseStringChars(env, jLogName, lpLogName);
			CloseEventLog(hEventLog);
			return NULL;
		}

		if(error == ERROR_INSUFFICIENT_BUFFER) {
			fwprintf(stderr, L"Error in ReadEventLog: 0x%x\n", error);
			free(lpBuffer);
			(*env)->ReleaseStringChars(env, jLogName, lpLogName);
			CloseEventLog(hEventLog);
			return NULL;
		}

		tempBuffer = (LPVOID)realloc(lpBuffer, nNumberOfBytesToRead);
		if(tempBuffer == NULL) {
			fwprintf(stderr, L"Failed to reallocate the memory for the record buffer (%d bytes)\n", pnMinNumberOfBytesNeeded);
			free(lpBuffer);
			(*env)->ReleaseStringChars(env, jLogName, lpLogName);
			CloseEventLog(hEventLog);
			return NULL;
		}
		lpBuffer = tempBuffer;
	}

	if(pnBytesRead == 0) {
		dwReadFlags = EVENTLOG_SEQUENTIAL_READ | EVENTLOG_FORWARDS_READ;

		if(!ReadEventLog(hEventLog, dwReadFlags, begin, lpBuffer, nNumberOfBytesToRead, &pnBytesRead, &pnMinNumberOfBytesNeeded)) {
			DWORD error = GetLastError();
			LPVOID tempBuffer = NULL;

			if(error == ERROR_HANDLE_EOF) {
				free(lpBuffer);
				(*env)->ReleaseStringChars(env, jLogName, lpLogName);
				CloseEventLog(hEventLog);
				return NULL;
			}

			if(error == ERROR_INSUFFICIENT_BUFFER) {
				fwprintf(stderr, L"Error in ReadEventLog: 0x%x\n", error);
				free(lpBuffer);
				(*env)->ReleaseStringChars(env, jLogName, lpLogName);
				CloseEventLog(hEventLog);
				return NULL;
			}

			tempBuffer = (LPVOID)realloc(lpBuffer, nNumberOfBytesToRead);
			if(tempBuffer == NULL) {
				fwprintf(stderr, L"Failed to reallocate the memory for the record buffer (%d bytes)\n", pnMinNumberOfBytesNeeded);
				free(lpBuffer);
				(*env)->ReleaseStringChars(env, jLogName, lpLogName);
				CloseEventLog(hEventLog);
				return NULL;
			}
			lpBuffer = tempBuffer;
		}

		if(pnBytesRead == 0 || ((PEVENTLOGRECORD)lpBuffer)->RecordNumber < (DWORD)begin) { 
			free(lpBuffer);
			(*env)->ReleaseStringChars(env, jLogName, lpLogName);
			CloseEventLog(hEventLog);

			return NULL;
		}
	}

	record = getEventLogObject(env, lpLogName, (PEVENTLOGRECORD)lpBuffer);

	(*env)->ReleaseStringChars(env, jLogName, lpLogName);

	if(lpBuffer)
		free(lpBuffer);

	if(!CloseEventLog(hEventLog))
		fwprintf(stderr, L"Error in CloseEventLog: 0x%x\n", GetLastError());

	return record;
}

jobject getEventLogObject(JNIEnv *env, LPTSTR lpLogName, PEVENTLOGRECORD record) {
	jclass clzString = (*env)->FindClass(env, "java/lang/String");
	jclass clzEventLog = (*env)->FindClass(env, "org/araqne/winapi/EventLog");
	jmethodID eventLogInit = (*env)->GetMethodID(env, clzEventLog, "<init>", "(IILorg/araqne/winapi/EventType;IILjava/lang/String;Ljava/lang/String;[BLjava/lang/String;Ljava/lang/String;[B)V");

	LPTSTR lpEventType, lpSourceName, lpEventCategory, pStrings, lpMessage;
	jint recordNumber, eventId, generated, written;
	jobject eventType;
	jstring sourceName, eventCategory;

	jbyteArray sid = NULL;
	jstring user = NULL;
	jobject message = NULL;
	
	jbyteArray data = NULL;
	
	lpEventType = getEventType(record->EventType);
	lpSourceName = (LPTSTR)((PBYTE)&(record->DataOffset)+4);
	lpEventCategory = getMessageString(lpLogName, lpSourceName, L"CategoryMessageFile", record->EventCategory, 0, NULL);

	pStrings = (LPTSTR)((PBYTE)record+record->StringOffset);
	lpMessage = getMessageString(lpLogName, lpSourceName, L"EventMessageFile", record->EventID, record->NumStrings, pStrings);

	recordNumber = record->RecordNumber;
	eventId = record->EventID & 0xFFFF;
	eventType = (*env)->NewString(env, lpEventType, (jsize)wcslen(lpEventType));
	generated = record->TimeGenerated;
	written = record->TimeWritten;
	sourceName = (*env)->NewString(env, lpSourceName, (jsize)wcslen(lpSourceName));
	eventCategory = lpEventCategory ? (*env)->NewString(env, lpEventCategory, (jsize)wcslen(lpEventCategory)) : NULL;
	message = lpMessage ? (*env)->NewString(env, lpMessage, (jsize)wcslen(lpMessage)) : NULL;

	if (lpEventCategory != NULL) 
	{
		LocalFree(lpEventCategory);
		lpEventCategory = NULL;
	}

	if (lpMessage != NULL)
	{
		LocalFree(lpMessage);
		lpMessage = NULL;
	}

	if(record->UserSidLength > 0) {
		LPTSTR lpName = NULL;
		LPTSTR lpDomain = NULL;
		DWORD dwNameSize = 0;
		DWORD dwDomainSize = 0;
		SID_NAME_USE nUse;

		sid = (*env)->NewByteArray(env, record->UserSidLength);
		(*env)->SetByteArrayRegion(env, sid, 0, record->UserSidLength, (PBYTE)record+record->UserSidOffset);

		LookupAccountSid(NULL, (PSID)((PBYTE)record+record->UserSidOffset), NULL, &dwNameSize, NULL, &dwDomainSize, &nUse);

		lpName = (LPTSTR)malloc(sizeof(TCHAR)*dwNameSize);
		lpDomain = (LPTSTR)malloc(sizeof(TCHAR)*dwDomainSize);

		memset(lpName, 0, sizeof(TCHAR)*dwNameSize);
		memset(lpDomain, 0, sizeof(TCHAR)*dwDomainSize);

		if(!LookupAccountSid(NULL, (PSID)((PBYTE)record+record->UserSidOffset), lpName, &dwNameSize, lpDomain, &dwDomainSize, &nUse)) {
			fprintf(stderr, "Error in LookupAccountSid: 0x%x\n", GetLastError());			
		} else {
			user = (*env)->NewString(env, lpName, (jsize)wcslen(lpName));
		}

		free(lpName);
		free(lpDomain);
	}
	if(record->DataLength > 0) {
		data = (*env)->NewByteArray(env, record->DataLength);
		(*env)->SetByteArrayRegion(env, data, 0, record->DataLength, (PBYTE)record+record->DataOffset);
	}

	return (*env)->NewObject(env, clzEventLog, eventLogInit, recordNumber, eventId, eventType, generated, written, sourceName, eventCategory, sid, user, message, data);
}

LPTSTR getEventType(WORD wEventType) {
	switch(wEventType) {
	case 0x01:
		return L"Error";
	case 0x02:
		return L"Warning";
	case 0x04:
		return L"Information";
	case 0x08:
		return L"AuditSuccess";
	case 0x10:
		return L"AuditFailure";
	}
	return L"Information";
}

LPTSTR getMessageString(LPTSTR lpLogName, LPTSTR lpSourceName, LPTSTR lpValueName, DWORD dwMessageId, WORD numStrings, LPTSTR pStrings) {
	DWORD dwFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY | FORMAT_MESSAGE_MAX_WIDTH_MASK;// | FORMAT_MESSAGE_IGNORE_INSERTS;
	LPCVOID lpSource = NULL;
	DWORD dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT);
	LPTSTR lpBuffer = NULL;
	wchar_t *pch = NULL;
	wchar_t seps[] = L";";
	wchar_t *context = NULL;

	void **Arguments = NULL;
	WORD i;
	int index = 0;
	int allocIndex = 0;
	int freeIndex = 0;

	// semicolon separated paths
	LPCTSTR lpFileNames = NULL;
	HANDLE hResources[100];

	memset(hResources, 0, sizeof(hResources));

	lpFileNames = getResource(lpLogName, lpSourceName, lpValueName);
	if(!lpFileNames) {
//		fwprintf(stderr, L"getResource failed\n");
		return NULL;
	}

	pch = wcstok_s(lpFileNames, seps, &context);
	while (pch != NULL){
		if (allocIndex >= 100)
			break;

		hResources[allocIndex++] = LoadLibraryEx(pch, NULL, DONT_RESOLVE_DLL_REFERENCES|LOAD_IGNORE_CODE_AUTHZ_LEVEL|LOAD_LIBRARY_AS_DATAFILE);
		pch = wcstok_s(NULL, seps, &context);
	}

	if(numStrings >= 0) {
		Arguments = (void**)malloc(sizeof(void*)*(numStrings + 20));
		memset(Arguments, 0, sizeof(void*)*(numStrings + 20));
		for(i=0; i<numStrings; i++) {
			*(Arguments + i) = (void*)pStrings;
			pStrings += wcslen(pStrings) + 1;
		}		
	}	

	for (index = 0; index < allocIndex; index++) {
		if(FormatMessage(dwFlags, hResources[index], dwMessageId, dwLanguageId, (LPTSTR)&lpBuffer, 0, (va_list*)Arguments))
			break;
		//else
		//	fwprintf(stderr, L"Error in FormatMessage: 0x%x, %s(%d)\n", GetLastError(), lpSourceName, dwMessageId);
	}

	for (freeIndex = 0; freeIndex < allocIndex; freeIndex++)
		FreeLibrary(hResources[freeIndex]);

	if(Arguments)
		free(Arguments);

	free((void*)lpFileNames);

	return lpBuffer;
}

LPTSTR getResource(LPTSTR lpLogName, LPTSTR lpSourceName, LPTSTR lpValueName) {
	HANDLE hResource = NULL;
	LPTSTR lpFileName = NULL;
	LPBYTE lpSrc = NULL;
	DWORD nSize = 0;
	HKEY hKey;
	DWORD lpcbData = 0;
	LPTSTR lpSubKey = NULL;
	size_t nSubKeySize = 0;

	nSubKeySize = sizeof(TCHAR)*(45+wcslen(lpLogName)+wcslen(lpSourceName));
	lpSubKey = (LPTSTR)malloc(nSubKeySize);
	memset(lpSubKey, 0, nSubKeySize);
	StringCbPrintf(lpSubKey, nSubKeySize, L"SYSTEM\\CurrentControlSet\\services\\eventlog\\%s\\%s", lpLogName, lpSourceName);

	if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpSubKey, 0, KEY_READ, &hKey) != 0 ) {
		// fallback to System
		StringCbPrintf(lpSubKey, nSubKeySize, L"SYSTEM\\CurrentControlSet\\services\\eventlog\\System\\%s", lpSourceName);
		if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpSubKey, 0, KEY_READ, &hKey) != 0 ) {
			// fallback to Application
			StringCbPrintf(lpSubKey, nSubKeySize, L"SYSTEM\\CurrentControlSet\\services\\eventlog\\Application\\%s", lpSourceName);
			if ( RegOpenKeyEx(HKEY_LOCAL_MACHINE, lpSubKey, 0, KEY_READ, &hKey) != 0 ) {
				free(lpSubKey);
				return NULL;
			}
		}
	}

	RegQueryValueEx(hKey, lpValueName, NULL, NULL, NULL, &lpcbData);
	if(lpcbData == 0) {
//		fwprintf(stderr, L"cannot query value %s\n", lpValueName);
		free(lpSubKey);
		RegCloseKey(hKey);
		return NULL;
	}

	lpSrc = (LPBYTE)malloc(lpcbData);
	memset(lpSrc, 0, lpcbData);

	RegQueryValueEx(hKey, lpValueName, NULL, NULL, lpSrc, &lpcbData);
	RegCloseKey(hKey);

	nSize = ExpandEnvironmentStrings((LPCTSTR)lpSrc, NULL, 0);
	lpFileName = (LPTSTR)malloc(sizeof(TCHAR)*nSize);
	memset(lpFileName, 0, sizeof(TCHAR)*nSize);
	ExpandEnvironmentStrings((LPCTSTR)lpSrc, (LPTSTR)lpFileName, nSize);

//	hResource = LoadLibrary(lpFileName);
//	free((void*)lpFileName);
	free(lpSrc);
	free(lpSubKey);

//	return (LPCVOID)hResource;
	return lpFileName;
}
