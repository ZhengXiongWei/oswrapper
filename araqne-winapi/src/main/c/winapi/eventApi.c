#include <windows.h>
#include <sddl.h>
#include <stdio.h>
#include <winevt.h>

#include <jni.h>
#include "EventApi.h"

#define maxlog 128

EVT_HANDLE (WINAPI *_EvtSubscribe)(
    EVT_HANDLE Session,
    HANDLE SignalEvent,
    LPCWSTR ChannelPath,
    LPCWSTR Query,
    EVT_HANDLE Bookmark,
    PVOID context,
    EVT_SUBSCRIBE_CALLBACK Callback,
    DWORD Flags
    );

EVT_HANDLE (WINAPI *_EvtCreateBookmark)(
    LPCWSTR BookmarkXml
    );

BOOL (WINAPI *_EvtUpdateBookmark)(
    EVT_HANDLE Bookmark,
    EVT_HANDLE Event
    );

BOOL (WINAPI *_EvtRender)(
    EVT_HANDLE Context,
    EVT_HANDLE Fragment,
    DWORD Flags,                        // EVT_RENDER_FLAGS
    DWORD BufferSize,
    PVOID Buffer,
    PDWORD BufferUsed,
    PDWORD PropertyCount
    );

BOOL (WINAPI *_EvtClose)(
    EVT_HANDLE Object
    );

BOOL (WINAPI *_EvtNext)(
    EVT_HANDLE ResultSet,
    DWORD EventsSize,
    PEVT_HANDLE Events,
    DWORD Timeout,
    DWORD Flags,
    PDWORD Returned
    );

EVT_HANDLE (WINAPI *_EvtCreateRenderContext)(
    DWORD ValuePathsCount,
    LPCWSTR* ValuePaths,
    DWORD Flags                         // EVT_RENDER_CONTEXT_FLAGS
    );

BOOL (WINAPI *_EvtFormatMessage)(
    EVT_HANDLE PublisherMetadata,       // Except for forwarded events
    EVT_HANDLE Event,
    DWORD MessageId,
    DWORD ValueCount,
    PEVT_VARIANT Values,
    DWORD Flags,
    DWORD BufferSize,
    LPWSTR Buffer,
    PDWORD BufferUsed
    );

EVT_HANDLE (WINAPI *_EvtOpenPublisherMetadata)(
    EVT_HANDLE Session,
    LPCWSTR PublisherId,
    LPCWSTR LogFilePath,
    LCID Locale,
    DWORD Flags
    );

EVT_HANDLE (WINAPI *_EvtOpenChannelEnum)(
    EVT_HANDLE Session,
    DWORD Flags
    );

BOOL (WINAPI *_EvtNextChannelPath)(
    EVT_HANDLE ChannelEnum,
    DWORD ChannelPathBufferSize,
    LPWSTR ChannelPathBuffer,
    PDWORD ChannelPathBufferUsed
    );


HMODULE hModule;

static int loadLibrary( JNIEnv *env )
{
	if ( hModule != NULL )
		return 0;

	hModule = LoadLibrary( L"wevtapi.dll" );
	if ( hModule == NULL )
	{
		(*env)->ThrowNew( env, (*env)->FindClass( env, "java/lang/UnsupportedOperationException" ), "cannot load wevtapi.dll" );
		return -1;
	}

	_EvtSubscribe = (EVT_HANDLE (WINAPI *)(
		EVT_HANDLE Session,
		HANDLE SignalEvent,
		LPCWSTR ChannelPath,
		LPCWSTR Query,
		EVT_HANDLE Bookmark,
		PVOID context,
		EVT_SUBSCRIBE_CALLBACK Callback,
		DWORD Flags
	))GetProcAddress( hModule, "EvtSubscribe" );
	
	_EvtCreateBookmark = (EVT_HANDLE (WINAPI *)(
		LPCWSTR BookmarkXml
	))GetProcAddress( hModule, "EvtCreateBookmark" );
	
	_EvtUpdateBookmark = (BOOL (WINAPI *)(
		EVT_HANDLE Bookmark,
		EVT_HANDLE Event
	))GetProcAddress( hModule, "EvtUpdateBookmark" );
	
	_EvtRender = (BOOL (WINAPI *)(
		EVT_HANDLE Context,
		EVT_HANDLE Fragment,
		DWORD Flags,                        // EVT_RENDER_FLAGS
		DWORD BufferSize,
		PVOID Buffer,
		PDWORD BufferUsed,
		PDWORD PropertyCount
	))GetProcAddress( hModule, "EvtRender" );
	
	_EvtClose = (BOOL (WINAPI *)(
		EVT_HANDLE Object
	))GetProcAddress( hModule, "EvtClose" );

	_EvtNext = (BOOL (WINAPI *)(
		EVT_HANDLE ResultSet,
		DWORD EventsSize,
		PEVT_HANDLE Events,
		DWORD Timeout,
		DWORD Flags,
		PDWORD Returned
	))GetProcAddress( hModule, "EvtNext" );

	_EvtCreateRenderContext = (EVT_HANDLE (WINAPI *)(
		DWORD ValuePathsCount,
		LPCWSTR* ValuePaths,
		DWORD Flags                         // EVT_RENDER_CONTEXT_FLAGS
	))GetProcAddress( hModule, "EvtCreateRenderContext" );

	_EvtFormatMessage = (BOOL (WINAPI *)(
		EVT_HANDLE PublisherMetadata,       // Except for forwarded events
		EVT_HANDLE Event,
		DWORD MessageId,
		DWORD ValueCount,
		PEVT_VARIANT Values,
		DWORD Flags,
		DWORD BufferSize,
		LPWSTR Buffer,
		PDWORD BufferUsed
	))GetProcAddress( hModule, "EvtFormatMessage" );

	_EvtOpenPublisherMetadata = (EVT_HANDLE (WINAPI *)(
		EVT_HANDLE Session,
		LPCWSTR PublisherId,
		LPCWSTR LogFilePath,
		LCID Locale,
		DWORD Flags
    ))GetProcAddress( hModule, "EvtOpenPublisherMetadata" );

	_EvtOpenChannelEnum = (EVT_HANDLE (WINAPI *)(
		EVT_HANDLE Session,
		DWORD Flags
	))GetProcAddress( hModule, "EvtOpenChannelEnum" );

	_EvtNextChannelPath = (BOOL (WINAPI *)(
		EVT_HANDLE ChannelEnum,
		DWORD ChannelPathBufferSize,
		LPWSTR ChannelPathBuffer,
		PDWORD ChannelPathBufferUsed
	))GetProcAddress( hModule, "EvtNextChannelPath" );


	if ( !( _EvtSubscribe && _EvtCreateBookmark && _EvtUpdateBookmark && _EvtRender && _EvtClose && _EvtNext &&
		_EvtCreateRenderContext && _EvtFormatMessage && _EvtOpenPublisherMetadata && _EvtOpenChannelEnum && _EvtNextChannelPath ) )
	{
		CloseHandle( hModule );
		hModule = NULL;
		(*env)->ThrowNew( env, (*env)->FindClass( env, "java/lang/UnsupportedOperationException" ), "invalid proc address" );
		return -2;
	}
	return 0;
}

struct eventapi_t {
	HANDLE hSignal;
    EVT_HANDLE hSubscription;
    EVT_HANDLE hBookmark;
	EVT_HANDLE hRender;
};

JNIEXPORT jlong JNICALL Java_org_araqne_winapi_EventApi_subscribe
  (JNIEnv *env, jobject jobj, jstring channelPath, jstring query, jstring bookmark)
{
	HANDLE hSignal = CreateEvent( NULL, FALSE, FALSE, NULL );
    EVT_HANDLE hSubscription = NULL;
    EVT_HANDLE hBookmark = NULL;
	EVT_HANDLE hRender = NULL;
    LPWSTR pwsPath = NULL;
    LPWSTR pwsQuery = NULL;
    LPWSTR pBookmarkXml = NULL;
	struct eventapi_t *evt;
	char *str;
	int size;

	if ( loadLibrary( env ) )
		return 0;

	if ( channelPath != NULL )
	{
		str = (*env)->GetStringUTFChars( env, channelPath, NULL );
		size = MultiByteToWideChar( CP_UTF8, 0, str, -1, NULL, 0 );
		pwsPath = malloc( size * sizeof(wchar_t) );
		size = MultiByteToWideChar( CP_UTF8, 0, str, -1, pwsPath, size );
		(*env)->ReleaseStringUTFChars( env, channelPath, str );
	}
	if ( query != NULL )
	{
		str = (*env)->GetStringUTFChars( env, query, NULL );
		size = MultiByteToWideChar( CP_UTF8, 0, str, -1, NULL, 0 );
		pwsQuery = malloc( size * sizeof(wchar_t) );
		size = MultiByteToWideChar( CP_UTF8, 0, str, -1, pwsQuery, size );
		(*env)->ReleaseStringUTFChars( env, query, str );
	}

	if ( bookmark != NULL )
	{
		str = (*env)->GetStringUTFChars( env, bookmark, NULL );
		size = MultiByteToWideChar( CP_UTF8, 0, str, -1, NULL, 0 );
		pBookmarkXml = malloc( size * sizeof(wchar_t) );
		size = MultiByteToWideChar( CP_UTF8, 0, str, -1, pBookmarkXml, size );
		(*env)->ReleaseStringUTFChars( env, bookmark, str );
	}

	hRender = _EvtCreateRenderContext( 0, NULL, EvtRenderContextSystem );
    if (NULL == hRender)
    {
        wprintf(L"EvtCreateRenderContext failed with %lu\n", GetLastError());
        goto cleanup;
    }

	hBookmark = _EvtCreateBookmark(pBookmarkXml);
    if (NULL == hBookmark)
    {
        wprintf(L"EvtCreateBookmark failed with %lu\n", GetLastError());
        goto cleanup;
    }
	
    hSubscription = _EvtSubscribe(NULL, hSignal, pwsPath, pwsQuery, hBookmark, (PVOID)hBookmark, NULL, EvtSubscribeStartAfterBookmark);
    if (NULL == hSubscription)
    {
        wprintf(L"EvtSubscribe failed with %lu.\n", GetLastError());
        goto cleanup;
    }

	evt = (struct eventapi_t *)malloc( sizeof(struct eventapi_t) );
	evt->hSubscription = hSubscription;
	evt->hSignal = hSignal;
	evt->hBookmark = hBookmark;
	evt->hRender = hRender;
	return evt;

cleanup:
    if (hSubscription)
        _EvtClose(hSubscription);

    if (hBookmark)
        _EvtClose(hBookmark);

	if (hRender)
		_EvtClose(hRender);

	if (hSignal)
		CloseHandle(hSignal);

	return 0;
}

JNIEXPORT void JNICALL Java_org_araqne_winapi_EventApi_close
  (JNIEnv *env, jobject jobj, jlong handle)
{
	struct eventapi_t *evt = (struct eventapi_t *)handle;

	if ( evt == NULL )
		return NULL;

    if ( evt->hSubscription )
        _EvtClose( evt->hSubscription );

    if ( evt->hBookmark )
        _EvtClose( evt->hBookmark );

	if ( evt->hSignal )
		CloseHandle( evt->hSignal );

	free( evt );
	return NULL;
}

static char *GetXMLString(EVT_HANDLE hEvent)
{
    DWORD status = ERROR_SUCCESS;
    DWORD dwBufferSize = 0;
    DWORD dwBufferUsed = 0;
    DWORD dwPropertyCount = 0;
    LPWSTR pRenderedContent = NULL;
	char *outbuf = NULL;
	int size;

    if (!_EvtRender(NULL, hEvent, EvtRenderEventXml, dwBufferSize, pRenderedContent, &dwBufferUsed, &dwPropertyCount))
    {
        if (ERROR_INSUFFICIENT_BUFFER == (status = GetLastError()))
        {
            dwBufferSize = dwBufferUsed;
            pRenderedContent = (LPWSTR)malloc(dwBufferSize);
            if (pRenderedContent)
            {
                _EvtRender(NULL, hEvent, EvtRenderEventXml, dwBufferSize, pRenderedContent, &dwBufferUsed, &dwPropertyCount);
            }
            else
            {
                wprintf(L"malloc failed\n");
                status = ERROR_OUTOFMEMORY;
                goto cleanup;
            }
        }

        if (ERROR_SUCCESS != (status = GetLastError()))
        {
            wprintf(L"EvtRender failed with %d\n", status);
            goto cleanup;
        }
    }

//    wprintf(L"%s\n\n", pRenderedContent);
	size = WideCharToMultiByte( CP_UTF8, 0, pRenderedContent, -1, NULL, 0, NULL, NULL );
	if ( size <= 0 )
		goto cleanup;

	outbuf = malloc( size );
	WideCharToMultiByte( CP_UTF8, 0, pRenderedContent, -1, outbuf, size, NULL, NULL );

cleanup:

    if (pRenderedContent)
        free(pRenderedContent);

    return outbuf;
}

static EVT_VARIANT *GetProvider(EVT_HANDLE hRender, EVT_HANDLE hEvent)
{
    DWORD status = ERROR_SUCCESS;
    DWORD dwBufferSize = 0;
    DWORD dwBufferUsed = 0;
    DWORD dwPropertyCount = 0;
    LPWSTR pRenderedContent = NULL;
	EVT_VARIANT *evt;

	if (!_EvtRender(hRender, hEvent, EvtRenderEventValues, dwBufferSize, pRenderedContent, &dwBufferUsed, &dwPropertyCount))
    {
        if (ERROR_INSUFFICIENT_BUFFER == (status = GetLastError()))
        {
            dwBufferSize = dwBufferUsed;
            pRenderedContent = (LPWSTR)malloc(dwBufferSize);
            if (pRenderedContent)
            {
                _EvtRender(hRender, hEvent, EvtRenderEventValues, dwBufferSize, pRenderedContent, &dwBufferUsed, &dwPropertyCount);
            }
            else
            {
                wprintf(L"malloc failed\n");
                status = ERROR_OUTOFMEMORY;
                goto cleanup;
            }
        }

        if (ERROR_SUCCESS != (status = GetLastError()))
        {
            wprintf(L"EvtRender failed with %d\n", status);
            goto cleanup;
        }
    }

	evt = (EVT_VARIANT *)pRenderedContent;

	return evt;

cleanup:
	if (pRenderedContent)
		free(pRenderedContent);
	return NULL;
}

LPWSTR GetMessageString(EVT_HANDLE hMetadata, EVT_HANDLE hEvent, EVT_FORMAT_MESSAGE_FLAGS FormatId)
{
    LPWSTR pBuffer = NULL;
    DWORD dwBufferSize = 0;
    DWORD dwBufferUsed = 0;
    DWORD status = 0;

    if (!_EvtFormatMessage(hMetadata, hEvent, 0, 0, NULL, FormatId, dwBufferSize, pBuffer, &dwBufferUsed))
    {
        status = GetLastError();
        if (ERROR_INSUFFICIENT_BUFFER == status)
        {
            // An event can contain one or more keywords. The function returns keywords
            // as a list of keyword strings. To process the list, you need to know the
            // size of the buffer, so you know when you have read the last string, or you
            // can terminate the list of strings with a second null terminator character 
            // as this example does.
            if ((EvtFormatMessageKeyword == FormatId))
                pBuffer[dwBufferSize-1] = L'\0';
            else
                dwBufferSize = dwBufferUsed;

            pBuffer = (LPWSTR)malloc(dwBufferSize * sizeof(WCHAR));

            if (pBuffer)
            {
                _EvtFormatMessage(hMetadata, hEvent, 0, 0, NULL, FormatId, dwBufferSize, pBuffer, &dwBufferUsed);

                // Add the second null terminator character.
                if ((EvtFormatMessageKeyword == FormatId))
                    pBuffer[dwBufferUsed-1] = L'\0';
            }
            else
            {
                wprintf(L"malloc failed\n");
            }
        }
        else if (ERROR_EVT_MESSAGE_NOT_FOUND == status || ERROR_EVT_MESSAGE_ID_NOT_FOUND == status)
            ;
        else
        {
            wprintf(L"EvtFormatMessage failed with %u\n", status);
        }
    }
	return pBuffer;
}

static char *GetEventString(struct eventapi_t *evt, EVT_HANDLE hEvent)
{
	EVT_VARIANT *var = GetProvider( evt->hRender, hEvent );
	int len;
	char *str = NULL;
	LPWSTR message = NULL;
	EVT_HANDLE hPub = NULL;

	if ( var == NULL )
		return NULL;

	hPub = _EvtOpenPublisherMetadata( NULL, var->StringVal, NULL, 0, 0 );
    if (NULL == hPub)
    {
        wprintf(L"EvtOpenPublisherMetadata failed with %lu\n", GetLastError());
		goto cleanup;
    }

	message = GetMessageString(hPub ,hEvent, EvtFormatMessageEvent);
	if ( message == NULL )
		goto cleanup;

	len = WideCharToMultiByte(CP_UTF8, 0, message, -1, NULL, 0, NULL, NULL);
	str = malloc(len);
	WideCharToMultiByte(CP_UTF8, 0, message, -1, str, len, NULL, NULL);

cleanup:;
	if ( message != NULL )
		free( message );
	if ( var != NULL )
		free( var );
	if ( hPub != NULL )
		_EvtClose( hPub );

	return str;
}

static char *SaveBookmark(EVT_HANDLE hBookmark)
{
    DWORD status = ERROR_SUCCESS;
    DWORD dwBufferSize = 0;
    DWORD dwBufferUsed = 0;
    DWORD dwPropertyCount = 0;
    LPWSTR pBookmarkXml = NULL;
	char *outbuf = NULL;
	int size;

    if (!_EvtRender(NULL, hBookmark, EvtRenderBookmark, dwBufferSize, pBookmarkXml, &dwBufferUsed, &dwPropertyCount))
    {
        if (ERROR_INSUFFICIENT_BUFFER == (status = GetLastError()))
        {
            dwBufferSize = dwBufferUsed;
            pBookmarkXml = (LPWSTR)malloc(dwBufferSize);
            if (pBookmarkXml)
            {
                _EvtRender(NULL, hBookmark, EvtRenderBookmark, dwBufferSize, pBookmarkXml, &dwBufferUsed, &dwPropertyCount);
            }
            else
            {
                wprintf(L"malloc failed\n");
                status = ERROR_OUTOFMEMORY;
                goto cleanup;
            }
        }

        if (ERROR_SUCCESS != (status = GetLastError()))
        {
            wprintf(L"EvtRender failed with %d\n", status);
            goto cleanup;
        }
    }

    // Persist bookmark to a file or the registry.
	size = WideCharToMultiByte( CP_UTF8, 0, pBookmarkXml, -1, NULL, 0, NULL, NULL );
	if ( size <= 0 )
		goto cleanup;

	outbuf = malloc( size );
	WideCharToMultiByte( CP_UTF8, 0, pBookmarkXml, -1, outbuf, size, NULL, NULL );

cleanup:

    if (pBookmarkXml)
        free(pBookmarkXml);

    return outbuf;
}


JNIEXPORT jobjectArray JNICALL Java_org_araqne_winapi_EventApi_read
  (JNIEnv *env, jobject jobj, jlong handle, jint timeout)
{
	struct eventapi_t *evt = (struct eventapi_t *)handle;
	EVT_HANDLE events[maxlog + 1];
	DWORD out = 0, i;
	jobjectArray joa;
	char *xml, *msg;

	do {
		_EvtNext( evt->hSubscription, maxlog, events, 1, 0, &out );
		if ( !out )
		{
			if ( timeout )
			{
				WaitForSingleObject( evt->hSignal, timeout );
				timeout = 0;
			}
			else
				return NULL;
		}
	} while( !out );
	joa = (*env)->NewObjectArray( env, out * 2 + 1, (*env)->FindClass( env, "java/lang/String" ), NULL );

	for ( i = 0; i < out; i++ )
	{
		xml = GetXMLString( events[i] );
		if ( xml == NULL )
			continue;

		(*env)->SetObjectArrayElement( env, joa, i * 2, (*env)->NewStringUTF( env, xml ) );
		free( xml );

		msg = GetEventString( evt, events[i] );
		if ( msg == NULL )
			(*env)->SetObjectArrayElement( env, joa, i * 2 + 1, NULL );
		else
		{
			(*env)->SetObjectArrayElement( env, joa, i * 2 + 1, (*env)->NewStringUTF( env, msg ) );
			free( msg );
		}
	}

	_EvtUpdateBookmark( evt->hBookmark, events[ out - 1 ] );
	msg = SaveBookmark( evt->hBookmark );
	(*env)->SetObjectArrayElement( env, joa, out * 2, (*env)->NewStringUTF( env, msg ) );
	free( msg );

	return joa;
}

JNIEXPORT jobject JNICALL Java_org_araqne_winapi_EventApi_getChannelPaths
  (JNIEnv *env, jobject jobj)
{
	jclass clsArrayList = (*env)->FindClass(env, "java/util/ArrayList");
	jmethodID mCtor = (*env)->GetMethodID(env,  clsArrayList, "<init>", "()V");
	jmethodID mAdd = (*env)->GetMethodID(env, clsArrayList, "add", "(Ljava/lang/Object;)Z");

	jobject objArrayList = (*env)->NewObject(env, clsArrayList, mCtor);

    EVT_HANDLE hChannels = NULL;
    LPWSTR pBuffer = NULL;
    LPWSTR pTemp = NULL;
    DWORD dwBufferSize = 0;
    DWORD dwBufferUsed = 0;
    DWORD status = ERROR_SUCCESS;
	char *outbuf = NULL;
	int size;

	if ( loadLibrary( env ) )
		return 0;

    // Get a handle to an enumerator that contains all the names of the 
    // channels registered on the computer.
    hChannels = _EvtOpenChannelEnum(NULL, 0);

    if (NULL == hChannels)
    {
        wprintf(L"EvtOpenChannelEnum failed with %lu.\n", GetLastError());
        goto cleanup;
    }

    // Enumerate through the list of channel names. If the buffer is not big
    // enough reallocate the buffer. To get the configuration information for
    // a channel, call the EvtOpenChannelConfig function.
    while (1)
    {
        if (!_EvtNextChannelPath(hChannels, dwBufferSize, pBuffer, &dwBufferUsed))
        {
            status = GetLastError();

            if (ERROR_NO_MORE_ITEMS == status)
            {
                break;
            }
            else if (ERROR_INSUFFICIENT_BUFFER == status)
            {
                dwBufferSize = dwBufferUsed;
                pTemp = (LPWSTR)realloc(pBuffer, dwBufferSize * sizeof(WCHAR));
                if (pTemp)
                {
                    pBuffer = pTemp;
                    pTemp = NULL;
                    _EvtNextChannelPath(hChannels, dwBufferSize, pBuffer, &dwBufferUsed);
                }
                else
                {
                    wprintf(L"realloc failed\n");
                    status = ERROR_OUTOFMEMORY;
                    goto cleanup;
                }
            }
            else
            {
                wprintf(L"EvtNextChannelPath failed with %lu.\n", status);
				goto cleanup;
            }
        }

		size = WideCharToMultiByte( CP_UTF8, 0, pBuffer, -1, NULL, 0, NULL, NULL );
		if ( size <= 0 )
			goto cleanup;

		outbuf = malloc( size );
		WideCharToMultiByte( CP_UTF8, 0, pBuffer, -1, outbuf, size, NULL, NULL );
		(*env)->CallBooleanMethod(env, objArrayList, mAdd, (*env)->NewStringUTF( env, outbuf ));
		free( outbuf );	
	}

cleanup:

    if (hChannels)
        _EvtClose(hChannels);

    if (pBuffer)
        free(pBuffer);

	return objArrayList;
}

JNIEXPORT jobjectArray JNICALL Java_org_araqne_winapi_EventApi_lookupAccountSid
  (JNIEnv *env, jclass jobj, jstring sid)
{
	LPTSTR lpName = NULL;
	LPTSTR lpDomain = NULL;
	DWORD dwNameSize = 0;
	DWORD dwDomainSize = 0;
	SID_NAME_USE nUse;
	LPCWSTR StringSid = (LPCWSTR) (*env)->GetStringChars(env, sid, JNI_FALSE);
	PSTR Sid = NULL;
	jobject user, domain;
	jobjectArray joa = NULL;

	ConvertStringSidToSid( StringSid, &Sid );
	if ( Sid == NULL )
		return NULL;

	LookupAccountSid(NULL, Sid, NULL, &dwNameSize, NULL, &dwDomainSize, &nUse);

	lpName = (LPTSTR)malloc(sizeof(TCHAR)*dwNameSize);
	lpDomain = (LPTSTR)malloc(sizeof(TCHAR)*dwDomainSize);

	memset(lpName, 0, sizeof(TCHAR)*dwNameSize);
	memset(lpDomain, 0, sizeof(TCHAR)*dwDomainSize);

	if(!LookupAccountSid(NULL, Sid, lpName, &dwNameSize, lpDomain, &dwDomainSize, &nUse)) {
		fprintf(stderr, "Error in LookupAccountSid: 0x%x\n", GetLastError());
	} else {
		joa = (*env)->NewObjectArray( env, 2, (*env)->FindClass( env, "java/lang/String" ), NULL );
		(*env)->SetObjectArrayElement( env, joa, 0, (*env)->NewString(env, lpName, (jsize)wcslen(lpName)) );
		(*env)->SetObjectArrayElement( env, joa, 1, (*env)->NewString(env, lpDomain, (jsize)wcslen(lpDomain)) );
	}

	LocalFree( Sid );
	free(lpName);
	free(lpDomain);

	return joa;
}
