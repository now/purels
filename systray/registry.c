
#define WIN32_LEAN_AND_MEAN
#define STRICT

#include <windows.h>
#include "registry.h"

//
// RegQueryBinaryValue
//

UINT WINAPI RegQueryBinaryValue( HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValueName, LPBYTE pbBuffer, UINT nBufferSize )
{
	DWORD dwDataType;
	DWORD dwDataSize = nBufferSize;
	HKEY hSubKey = NULL;
	LONG lErrorCode;
	
	if( pszSubKey )
	{
		lErrorCode = RegOpenKeyEx( hKey, pszSubKey, 0, KEY_QUERY_VALUE, &hSubKey );
		
		if( lErrorCode != ERROR_SUCCESS )
			return 0;
		
		hKey = hSubKey;
	}
	
	lErrorCode = RegQueryValueEx( hKey, pszValueName, NULL, &dwDataType,
		pbBuffer, &dwDataSize );
	
	if( hSubKey )
		RegCloseKey( hSubKey );
	
	if( dwDataType != REG_BINARY )
		return 0;
		
	return lErrorCode != ERROR_SUCCESS ? 0 : (UINT) dwDataSize;
}

//
// RegQueryLongValue
//

LONG WINAPI RegQueryLongValue( HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValueName, LONG lDefaultValue )
{
	DWORD dwDataType;
	DWORD dwData;
	DWORD dwDataSize = sizeof(DWORD);
	HKEY hSubKey = NULL;
	LONG lErrorCode;
	
	if( pszSubKey )
	{
		lErrorCode = RegOpenKeyEx( hKey, pszSubKey, 0, KEY_QUERY_VALUE, &hSubKey );
		
		if( lErrorCode != ERROR_SUCCESS )
			return lDefaultValue;
		
		hKey = hSubKey;
	}
	
	lErrorCode = RegQueryValueEx( hKey, pszValueName, NULL, &dwDataType,
		(LPBYTE) &dwData, &dwDataSize );
		
	if( hSubKey )
		RegCloseKey( hSubKey );
	
	if( dwDataType != REG_DWORD || dwDataSize != sizeof(DWORD) )
		return lDefaultValue;
	
	return lErrorCode != ERROR_SUCCESS ? lDefaultValue : (LONG) dwData;
}

//
// RegQueryStringValue
//

LPTSTR WINAPI RegQueryStringValue( HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValueName, LPTSTR pszBuffer, UINT nBufferSize, LPCTSTR pszDefaultValue )
{
	DWORD dwDataType;
	DWORD dwDataSize = nBufferSize;
	HKEY hSubKey = NULL;
	LONG lErrorCode;
	
	if( pszSubKey )
	{
		lErrorCode = RegOpenKeyEx( hKey, pszSubKey, 0, KEY_QUERY_VALUE, &hSubKey );
		
		if( lErrorCode != ERROR_SUCCESS )
		{
			lstrcpyn( pszBuffer, pszDefaultValue, nBufferSize );
			return pszBuffer;
		}
		
		hKey = hSubKey;
	}
	
	lErrorCode = RegQueryValueEx( hKey, pszValueName, NULL, &dwDataType,
		(LPBYTE) pszBuffer, &dwDataSize );
	
	if( hSubKey )
		RegCloseKey( hSubKey );
		
	if( dwDataType != REG_SZ || lErrorCode != ERROR_SUCCESS )
	{
		lstrcpyn( pszBuffer, pszDefaultValue, nBufferSize );
		return pszBuffer;
	}
	
	return pszBuffer;
}

//
// RegSetBinaryValue
//

BOOL WINAPI RegSetBinaryValue( HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValueName, CONST BYTE *pbBuffer, UINT nBufferSize )
{
	HKEY hSubKey = NULL;
	LONG lErrorCode;
	
	if( pszSubKey )
	{
		lErrorCode = RegCreateKeyEx( hKey, pszSubKey, 0, NULL, 0, KEY_SET_VALUE,
			NULL, &hSubKey, NULL );
		
		if( lErrorCode != ERROR_SUCCESS )
			return FALSE;
		
		hKey = hSubKey;
	}
	
	lErrorCode = RegSetValueEx( hKey, pszValueName, 0, REG_BINARY,
		pbBuffer, (DWORD) nBufferSize );
	
	if( hSubKey )
		RegCloseKey( hSubKey );
	
	return lErrorCode != ERROR_SUCCESS ? FALSE : TRUE;
}

//
// RegSetLongValue
//

BOOL WINAPI RegSetLongValue( HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValueName, LONG lValue )
{
	HKEY hSubKey = NULL;
	LONG lErrorCode;
	
	if( pszSubKey )
	{
		lErrorCode = RegCreateKeyEx( hKey, pszSubKey, 0, NULL, 0, KEY_SET_VALUE,
			NULL, &hSubKey, NULL );
		
		if( lErrorCode != ERROR_SUCCESS )
			return FALSE;
		
		hKey = hSubKey;
	}
	
	lErrorCode = RegSetValueEx( hKey, pszValueName, 0, REG_DWORD,
		(CONST BYTE *) &lValue, sizeof(DWORD) );
	
	if( hSubKey )
		RegCloseKey( hSubKey );
	
	return lErrorCode != ERROR_SUCCESS ? FALSE : TRUE;
}

//
// RegSetStringValue
//

BOOL WINAPI RegSetStringValue( HKEY hKey, LPCTSTR pszSubKey, LPCTSTR pszValueName, LPCTSTR pszValue )
{
	HKEY hSubKey = NULL;
	LONG lErrorCode;
	
	if( pszSubKey )
	{
		lErrorCode = RegCreateKeyEx( hKey, pszSubKey, 0, NULL, 0, KEY_SET_VALUE,
			NULL, &hSubKey, NULL );
		
		if( lErrorCode != ERROR_SUCCESS )
			return FALSE;
		
		hKey = hSubKey;
	}
	
	lErrorCode = RegSetValueEx( hKey, pszValueName, 0, REG_SZ,
		(CONST BYTE *) pszValue, (DWORD) lstrlen( pszValue ) );
	
	if( hSubKey )
		RegCloseKey( hSubKey );
	
	return lErrorCode != ERROR_SUCCESS ? FALSE : TRUE;
}
