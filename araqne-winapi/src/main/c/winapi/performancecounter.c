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
#include <pdh.h>
#include <PDHMsg.h>
#include "jni_common.h"
#include "PerformanceCounter.h"

JNIEXPORT jobjectArray JNICALL Java_org_araqne_winapi_PerformanceCounter_getMachines(JNIEnv *env, jobject obj) {
	jobjectArray machineList = NULL;
	LPTSTR lpMachineNameList = NULL;
	DWORD dwBufferLength = 0;
	PDH_STATUS stat = 0;

	PdhEnumMachines(NULL, NULL, &dwBufferLength);
	if(dwBufferLength == 0) {
		throwExc(env, "Error in PdhEnumMachines\n");
		return NULL;
	}

	lpMachineNameList = (LPTSTR)malloc(sizeof(TCHAR)*dwBufferLength);
	stat = PdhEnumMachines(NULL, lpMachineNameList, &dwBufferLength);
	if(stat != ERROR_SUCCESS) {
		free(lpMachineNameList);
		throwExc(env, "Error in PdhEnumMachines: 0x%x", stat);
		return NULL;
	}

	machineList = convertStringArray(env, lpMachineNameList, dwBufferLength);

	free(lpMachineNameList);

	return machineList;
}

JNIEXPORT jobjectArray JNICALL Java_org_araqne_winapi_PerformanceCounter_getCategories(JNIEnv *env, jobject obj, jstring machine, jint detail) {
	jobjectArray categoryList = NULL;
	LPTSTR machineName = machine ? (LPTSTR)(*env)->GetStringChars(env, machine, JNI_FALSE) : NULL;
	LPTSTR lpCategoryNameList = NULL;
	DWORD dwBufferLength = 0;
	PDH_STATUS stat = 0;

	if(machineName) {
		stat = PdhConnectMachine(machineName);
		if(stat != ERROR_SUCCESS) {
			throwExc(env, "Error in PdhConnectMachine:, 0x%x", stat);
			(*env)->ReleaseStringChars(env, machine, machineName);
			return NULL;
		}
	}

	PdhEnumObjects(NULL, machineName, NULL, &dwBufferLength, detail, TRUE);
	if(dwBufferLength == 0) {
		if (machineName)
			(*env)->ReleaseStringChars(env, machine, machineName);
		throwExc(env, "Error in PdhEnumObjects");
		return NULL;
	}

	lpCategoryNameList = (LPTSTR)malloc(sizeof(TCHAR)*dwBufferLength);
	stat = PdhEnumObjects(NULL, machineName, lpCategoryNameList, &dwBufferLength, detail, TRUE);
	if(stat != ERROR_SUCCESS) {
		free(lpCategoryNameList);
		if (machineName)
			(*env)->ReleaseStringChars(env, machine, machineName);
		throwExc(env, "Error in PdhEnumObjects: 0x%x", stat);
		return NULL;
	}
	if(machineName)
		(*env)->ReleaseStringChars(env, machine, machineName);

	categoryList = convertStringArray(env, lpCategoryNameList, dwBufferLength);

	free(lpCategoryNameList);

	return categoryList;
}

JNIEXPORT jobject JNICALL Java_org_araqne_winapi_PerformanceCounter_getCounters(JNIEnv *env, jobject obj, jstring category, jstring machine, jint detail) {
	jclass clzHashMap = (*env)->FindClass(env, "java/util/HashMap");
	jmethodID hashMapInit = (*env)->GetMethodID(env, clzHashMap, "<init>", "()V");
	jmethodID hashMapPut = (*env)->GetMethodID(env, clzHashMap, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
	jobject counter = NULL;
	LPTSTR categoryName = (LPTSTR)(*env)->GetStringChars(env, category, JNI_FALSE);
	LPTSTR machineName = machine ? (LPTSTR)(*env)->GetStringChars(env, machine, JNI_FALSE) : NULL;
	jobjectArray counters = NULL;
	jobjectArray instances = NULL;
	LPTSTR lpCounterList = NULL;
	DWORD dwCounterListLength = 0;
	LPTSTR lpInstanceList = NULL;
	DWORD dwInstanceListLength = 0;
	DWORD stat = 0;

	if(machineName) {
		stat = PdhConnectMachine(machineName);
		if(stat != ERROR_SUCCESS) {
			(*env)->ReleaseStringChars(env, machine, machineName);
			throwExc(env, "Error in PdhConnectMachine:, 0x%x", stat);
			return NULL;
		}
	}

	PdhEnumObjectItems(NULL, machineName, categoryName, NULL, &dwCounterListLength, NULL, &dwInstanceListLength, detail, 0);

	lpCounterList = (LPTSTR)malloc(sizeof(TCHAR)*dwCounterListLength);
	lpInstanceList = (LPTSTR)malloc(sizeof(TCHAR)*dwInstanceListLength);
	stat = PdhEnumObjectItems(NULL, machineName, categoryName, lpCounterList, &dwCounterListLength, lpInstanceList, &dwInstanceListLength, detail, 0);
	(*env)->ReleaseStringChars(env, category, categoryName);
	if(machineName)
		(*env)->ReleaseStringChars(env, machine, machineName);
	if(stat != ERROR_SUCCESS) {
		free(lpCounterList);
		free(lpInstanceList);
		throwExc(env, "Error in PdhEnumObjectItems");
		return NULL;
	}

	counter = counter = (*env)->NewObject(env, clzHashMap, hashMapInit);
	counters = convertStringArray(env, lpCounterList, dwCounterListLength);
	instances = convertStringArray(env, lpInstanceList, dwInstanceListLength);

	free(lpCounterList);
	free(lpInstanceList);

	(*env)->CallObjectMethod(env, counter, hashMapPut, (*env)->NewStringUTF(env, "counters"), counters);
	(*env)->CallObjectMethod(env, counter, hashMapPut, (*env)->NewStringUTF(env, "instances"), instances);

	return counter;
}

jobjectArray convertStringArray(JNIEnv *env, LPTSTR source, DWORD dwLength) {
	jclass clzString = (*env)->FindClass(env, "java/lang/String");
	jobjectArray ary = NULL;
	LPTSTR tempStr = NULL;
	DWORD strCount = 0;
	DWORD i;

	for(tempStr=source; tempStr<source+dwLength;) {
		if(wcslen(tempStr) > 0)
			strCount++;
		tempStr += wcslen(tempStr) + 1;
	}

	ary = (*env)->NewObjectArray(env, strCount, clzString, NULL);
	tempStr = source;
	for(i=0; i<strCount; i++) {
		jstring str = NULL;

		str = (*env)->NewString(env, tempStr, (jsize)wcslen(tempStr));
		(*env)->SetObjectArrayElement(env, ary, i, str);
		do {
			tempStr += wcslen(tempStr) + 1;
		} while(wcslen(tempStr) == 0 && tempStr < source+dwLength);
	}

	return ary;
}

JNIEXPORT jint JNICALL Java_org_araqne_winapi_PerformanceCounter_open(JNIEnv *env, jobject obj) {
	PDH_HQUERY phQuery = NULL;
	PDH_STATUS stat = 0;

	stat = PdhOpenQuery(NULL, 0, &phQuery);
	if(stat != ERROR_SUCCESS) {
		throwExc(env, "Error in PdhOpenQuery: 0x%x", stat);
		return 0;
	}

	return (jint)phQuery;
}

JNIEXPORT jint JNICALL Java_org_araqne_winapi_PerformanceCounter_addCounterN(JNIEnv *env, jobject obj, jint queryHandle, jstring category, jstring counter, jstring instance, jstring machine) {
	PDH_HCOUNTER phCounter = NULL;
	PDH_HQUERY phQuery = (PDH_HQUERY)queryHandle;
	LPTSTR counterPath = NULL;
	PDH_COUNTER_PATH_ELEMENTS pathElement;
	DWORD dwSize = 0;
	PDH_STATUS stat = 0;
	jboolean isCopy = JNI_FALSE;

	memset(&pathElement, 0, sizeof(pathElement));
	pathElement.szObjectName = category ? (LPTSTR)(*env)->GetStringChars(env, category, &isCopy) : NULL;
	pathElement.szCounterName = counter ? (LPTSTR)(*env)->GetStringChars(env, counter, &isCopy) : NULL;
	pathElement.szInstanceName = instance ? (LPTSTR)(*env)->GetStringChars(env, instance, &isCopy) : NULL;
	pathElement.szMachineName = machine ? (LPTSTR)(*env)->GetStringChars(env, machine, &isCopy) : NULL;

	if(pathElement.szMachineName) {
		stat = PdhConnectMachine(pathElement.szMachineName);
		if(stat != ERROR_SUCCESS) {
			if(pathElement.szMachineName)
				(*env)->ReleaseStringChars(env, category, pathElement.szObjectName);
			(*env)->ReleaseStringChars(env, counter, pathElement.szCounterName);
			(*env)->ReleaseStringChars(env, instance, pathElement.szInstanceName);
			(*env)->ReleaseStringChars(env, machine, pathElement.szMachineName);
			return throwExc(env, "Error in PdhConnectMachine:, 0x%x", stat);
		}
	}

	PdhMakeCounterPath(&pathElement, NULL, &dwSize, 0);
	if(dwSize == 0) {
		(*env)->ReleaseStringChars(env, category, pathElement.szObjectName);
		(*env)->ReleaseStringChars(env, counter, pathElement.szCounterName);
		(*env)->ReleaseStringChars(env, instance, pathElement.szInstanceName);
		(*env)->ReleaseStringChars(env, machine, pathElement.szMachineName);
		return throwExc(env, "Error in PdhMakeCounterPath\n");
	}

	counterPath = (LPTSTR)malloc(sizeof(TCHAR)*dwSize);
	stat = PdhMakeCounterPath(&pathElement, counterPath, &dwSize, 0);
	(*env)->ReleaseStringChars(env, category, pathElement.szObjectName);
	(*env)->ReleaseStringChars(env, counter, pathElement.szCounterName);
	(*env)->ReleaseStringChars(env, instance, pathElement.szInstanceName);
	(*env)->ReleaseStringChars(env, machine, pathElement.szMachineName);
	if(stat != ERROR_SUCCESS) {
		free(counterPath);
		return throwExc(env, "Error in PdhMakeCounterPath: 0x%x\n", stat);
	}

	stat = PdhAddCounter(phQuery, counterPath, 0, &phCounter);
	if(stat != ERROR_SUCCESS) {
		free(counterPath);
		return throwExc(env, "Error in PdhAddCounter: 0x%x\n", stat);
	}
	free(counterPath);

	PdhCollectQueryData(phQuery);

	return (jint)phCounter;
}

JNIEXPORT jdoubleArray JNICALL Java_org_araqne_winapi_PerformanceCounter_queryAndGet(JNIEnv *env, jobject obj, jint queryHandle, jintArray counterHandles, jdoubleArray valueBuf) {
	PDH_HQUERY phQuery = (PDH_HQUERY)queryHandle;
	PDH_HCOUNTER phCounter = NULL;
	PDH_FMT_COUNTERVALUE pValue;
	PDH_STATUS stat = 0;
	jsize handleCnt, bufCnt;
	jint* pCounterHandle;
	jdouble* pValueBuf;
	int i;

	stat = PdhCollectQueryData(phQuery);
	if(stat != ERROR_SUCCESS) {
		if (stat == PDH_NO_DATA) {
			// no data available - hide stderr report
			return NULL;
		} else {
			throwExc(env, "PdhCollectQueryData failed: 0x%x\n", stat);
			return NULL;
		}
	}

	handleCnt = (*env)->GetArrayLength(env, counterHandles);
	bufCnt = (*env)->GetArrayLength(env, valueBuf);

	if (bufCnt < handleCnt) {
		throwExc(env, "PdhCollectQueryData - IllegalArgument\n");
		return NULL;
	}

	pCounterHandle = (*env)->GetIntArrayElements(env, counterHandles, 0);
	pValueBuf = (*env)->GetDoubleArrayElements(env, valueBuf, 0);
	for (i = 0; i < handleCnt; ++i) {
		stat = PdhGetFormattedCounterValue((PDH_HCOUNTER) pCounterHandle[i], PDH_FMT_DOUBLE, NULL, &pValue);
		if(stat != ERROR_SUCCESS) {
			pValueBuf[i] = -1.0;
		}
		else pValueBuf[i] = pValue.doubleValue;
	}
	(*env)->ReleaseIntArrayElements(env, counterHandles, pCounterHandle, (jint)NULL);
	(*env)->ReleaseDoubleArrayElements(env, valueBuf, pValueBuf, (jint)NULL);

	return valueBuf;
}

JNIEXPORT void JNICALL Java_org_araqne_winapi_PerformanceCounter_close(JNIEnv *env, jobject obj, jint queryHandle) {
	PDH_STATUS stat = 0;

	// removing counters. does not have to be here.
	//stat = PdhRemoveCounter((PDH_HCOUNTER)counterHandle);
	//if(stat != ERROR_SUCCESS) {
	//	fprintf(stderr, "Error in PdhRemoveCounter: 0x%x\n", stat);
	//	return;
	//}

	stat = PdhCloseQuery((PDH_HQUERY)queryHandle);
	if(stat != ERROR_SUCCESS)
		throwExc(env, "Error in PdhCloseQuery: 0x%x\n", stat);
}

JNIEXPORT jobjectArray JNICALL Java_org_araqne_winapi_PerformanceCounter_expandCounterPath(JNIEnv *env, jobject obj, jstring path) {
	jobjectArray categoryList = NULL;
	LPTSTR strPath = path ? (LPTSTR)(*env)->GetStringChars(env, path, JNI_FALSE) : NULL;
	LPTSTR lpCounterList = NULL;
	DWORD dwBufferLength = 0;
	PDH_STATUS stat = 0;

	if (wcsnlen(strPath, PDH_MAX_COUNTER_NAME + 1) > PDH_MAX_COUNTER_NAME) 
		return NULL;

	if (path) {
		stat = PdhExpandCounterPathW(strPath, NULL, &dwBufferLength);
		if (stat == PDH_MORE_DATA) {
			lpCounterList = (LPTSTR) malloc(dwBufferLength * sizeof(TCHAR));
			stat = PdhExpandCounterPathW(strPath, lpCounterList, &dwBufferLength);
			if (stat != ERROR_SUCCESS) {
				(*env)->ReleaseStringChars(env, path, strPath);
				free(lpCounterList);
				return NULL;
			}
		}
	}

	if(path)
		(*env)->ReleaseStringChars(env, path, strPath);

	categoryList = convertStringArray(env, lpCounterList, dwBufferLength);

	free(lpCounterList);

	return categoryList;
}
