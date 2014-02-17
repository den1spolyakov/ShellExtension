#include "Extension.h"
#include "resource.h"
#include <strsafe.h>
#include <Shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

extern HINSTANCE g_hInst;
extern long g_cDllRef;

#define IDM_DISPLAY             0  // The command's identifier offset

Extension::Extension(void) : m_cRef(1), 
    m_pszMenuText(L"&Calculate the sum"),
    m_pszVerb("sumcalc"),
    m_pwszVerb(L"sumcalc"),
    m_pszVerbCanonicalName("cppsumcalc"),
    m_pwszVerbCanonicalName(L"cppsumcalc"),
    m_pszVerbHelpText("Calculate the sum"),
    m_pwszVerbHelpText(L"Calculate the sum")
{
    InterlockedIncrement(&g_cDllRef);
}

Extension::~Extension(void)
{
    InterlockedDecrement(&g_cDllRef);
}

void processLogging(std::vector<std::wstring> & records)
{
	std::wstring message = L"Checksums have been written!";
	std::wstring ext = L"ShellExtension";
	FileProcessor fp(records);

	fp.log();

	MessageBox(NULL, message.c_str(), ext.c_str(), MB_OK | MB_ICONINFORMATION);
}

#pragma region IUnknown

// Query to the interface the component supported.
IFACEMETHODIMP Extension::QueryInterface(REFIID riid, void **ppv)
{
    static const QITAB qit[] = 
    {
        QITABENT(Extension, IContextMenu),
        QITABENT(Extension, IShellExtInit), 
        { 0 },
    };
    return QISearch(this, qit, riid, ppv);
}

// Increase the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG) Extension::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

// Decrease the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG) Extension::Release()
{
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if (cRef == 0)
    {
        delete this;
    }
    return cRef;
}

#pragma endregion

#pragma region IShellExtInit

// Initialize the context menu handler.
IFACEMETHODIMP Extension::Initialize(
    LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hKeyProgID)
{
    if (pDataObj == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = E_FAIL;
    FORMATETC fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM stm;

    if (SUCCEEDED(pDataObj->GetData(&fe, &stm)))
    {
        HDROP hDrop = static_cast<HDROP>(GlobalLock(stm.hGlobal));
        if (hDrop != NULL)
        {
			UINT nFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
			wchar_t selectedFile[MAX_PATH];
			for (UINT i = 0; i < nFiles; i++)
			{
				if (DragQueryFile(hDrop, i, selectedFile, ARRAYSIZE(selectedFile)) != 0
					&& !PathIsDirectoryW(selectedFile))
				{
					paths.push_back(selectedFile);
					hr = S_OK;
				}
			}
            GlobalUnlock(stm.hGlobal);
        }
        ReleaseStgMedium(&stm);
    }
    return hr;
}

#pragma endregion

#pragma region IContextMenu

IFACEMETHODIMP Extension::QueryContextMenu(
    HMENU hMenu, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast, UINT uFlags)
{
    if (CMF_DEFAULTONLY & uFlags)
    {
        return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(0));
    }

    MENUITEMINFO mii = { sizeof(mii) };
    mii.fMask = MIIM_BITMAP | MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_STATE;
    mii.wID = idCmdFirst + IDM_DISPLAY;
    mii.fType = MFT_STRING;
    mii.dwTypeData = m_pszMenuText;
	mii.fState = MFS_ENABLED;
    if (!InsertMenuItem(hMenu, indexMenu, TRUE, &mii))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    MENUITEMINFO sep = { sizeof(sep) };
    sep.fMask = MIIM_TYPE;
    sep.fType = MFT_SEPARATOR;
    if (!InsertMenuItem(hMenu, indexMenu + 1, TRUE, &sep))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    return MAKE_HRESULT(SEVERITY_SUCCESS, 0, USHORT(IDM_DISPLAY + 1));
}

IFACEMETHODIMP Extension::InvokeCommand(LPCMINVOKECOMMANDINFO pici)
{
    BOOL fUnicode = FALSE;
	BOOL ready = FALSE;

    if (pici->cbSize == sizeof(CMINVOKECOMMANDINFOEX))
    {
        if (pici->fMask & CMIC_MASK_UNICODE)
        {
            fUnicode = TRUE;
        }
    } 
    if (!fUnicode && HIWORD(pici->lpVerb))
    {
        if (StrCmpIA(pici->lpVerb, m_pszVerb) == 0)
        {
			ready = TRUE;
        }
        else
        {
            return E_FAIL;
        }
    }
    else if (fUnicode && HIWORD(((CMINVOKECOMMANDINFOEX*)pici)->lpVerbW))
    {
        if (StrCmpIW(((CMINVOKECOMMANDINFOEX*)pici)->lpVerbW, m_pwszVerb) == 0)
        {
			ready = TRUE;
        }
        else
        {
            return E_FAIL;
        }
    }
    else
    {
        if (LOWORD(pici->lpVerb) == IDM_DISPLAY)
        {
			ready = TRUE;
        }
        else
        {
            return E_FAIL;
        }
    }

	if (ready == TRUE)
	{
		std::thread t(processLogging, paths);
		t.detach();
	}

    return S_OK;
}

IFACEMETHODIMP Extension::GetCommandString(UINT_PTR idCommand, 
    UINT uFlags, UINT *pwReserved, LPSTR pszName, UINT cchMax)
{
    HRESULT hr = E_INVALIDARG;

    if (idCommand == IDM_DISPLAY)
    {
        switch (uFlags)
        {
        case GCS_HELPTEXTW:
            hr = StringCchCopy(reinterpret_cast<PWSTR>(pszName), cchMax, 
                m_pwszVerbHelpText);
            break;

        case GCS_VERBW:
            hr = StringCchCopy(reinterpret_cast<PWSTR>(pszName), cchMax, 
                m_pwszVerbCanonicalName);
            break;

        default:
            hr = S_OK;
        }
    }
    return hr;
}

#pragma endregion