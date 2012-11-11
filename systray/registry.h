
////
/// registry functions
//

#ifndef __REGISTRY_H
#define __REGISTRY_H

#ifdef __cplusplus
extern "C" {
#endif

UINT WINAPI RegQueryBinaryValue( HKEY, LPCTSTR, LPCTSTR, LPBYTE, UINT );
LONG WINAPI RegQueryLongValue( HKEY, LPCTSTR, LPCTSTR, LONG );
LPTSTR WINAPI RegQueryStringValue( HKEY, LPCTSTR, LPCTSTR, LPTSTR, UINT, LPCTSTR );

BOOL WINAPI RegSetBinaryValue( HKEY, LPCTSTR, LPCTSTR, CONST BYTE *, UINT );
BOOL WINAPI RegSetLongValue( HKEY, LPCTSTR, LPCTSTR, LONG );
BOOL WINAPI RegSetStringValue( HKEY, LPCTSTR, LPCTSTR, LPCTSTR );

#ifdef __cplusplus
}
#endif

#endif
