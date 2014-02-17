#include <windows.h>
#include <Guiddef.h>
#include <new>
#include "ClassFactory.h"           // For the class factory
#include "Registration.h"

const CLSID CLSID_FileContextMenuExt = 
{ 0x26f92c18, 0x3bf3, 0x42bf, { 0xb1, 0x87, 0xdd, 0x51, 0xc3, 0x83, 0xef, 0x61 } };

HINSTANCE   g_hInst     = NULL;
long        g_cDllRef   = 0;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
        g_hInst = hModule;
        DisableThreadLibraryCalls(hModule);
        break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
 
STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
    HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

    if (IsEqualCLSID(CLSID_FileContextMenuExt, rclsid))
    {
        hr = E_OUTOFMEMORY;

        ClassFactory *pClassFactory = new (std::nothrow) ClassFactory();
        if (pClassFactory)
        {
            hr = pClassFactory->QueryInterface(riid, ppv);
            pClassFactory->Release();
        }
    }

    return hr;
}
 
STDAPI DllCanUnloadNow(void)
{
    return g_cDllRef > 0 ? S_FALSE : S_OK;
}
 
STDAPI DllRegisterServer(void)
{
    HRESULT hr;

    wchar_t szModule[MAX_PATH];
    if (GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule)) == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }

    // Register the component.
    hr = RegisterInprocServer(szModule, CLSID_FileContextMenuExt, 
        L"ShellExtension.Extension Class", 
        L"Apartment");
    if (SUCCEEDED(hr))
    {
        hr = RegisterShellExtContextMenuHandler(L"*", 
            CLSID_FileContextMenuExt, 
            L"ShellExtension.Extension");
		hr = RegisterShellExtContextMenuHandler(L"Directory", 
            CLSID_FileContextMenuExt, 
            L"ShellExtension.Extension");
    }

    return hr;
}
 
STDAPI DllUnregisterServer(void)
{
    HRESULT hr = S_OK;

    wchar_t szModule[MAX_PATH];
    if (GetModuleFileName(g_hInst, szModule, ARRAYSIZE(szModule)) == 0)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        return hr;
    }

    // Unregister the component.
    hr = UnregisterInprocServer(CLSID_FileContextMenuExt);
    if (SUCCEEDED(hr))
    {
        // Unregister the context menu handler.
        hr = UnregisterShellExtContextMenuHandler(L"*", 
            CLSID_FileContextMenuExt);
		hr = UnregisterShellExtContextMenuHandler(L"Directory", 
            CLSID_FileContextMenuExt);
    }

    return hr;
}