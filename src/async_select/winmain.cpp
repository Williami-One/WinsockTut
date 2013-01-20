﻿/**
 *  @author ichenq@gmail.com
 *  @date   Nov 24, 2011
 *  @brief  A simple echo server, use async select
 */


#include "resource.h"
#include "appdef.h"
#include <assert.h>
#include <ShellAPI.h>
#include "../common/logging.h"

#pragma comment(lib, "shell32.lib")

#pragma warning(disable: 4996)



static INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
static bool StartServer(const _tstring& strHost, const _tstring& strPort);
static void OnCommand(HWND hCtrl, DWORD dwNotifyCode);

static HWND     g_hDlg;  // global dialog handle


// main entry
int APIENTRY _tWinMain(HINSTANCE hInstance, 
                    HINSTANCE hPrevInstance, 
                    LPTSTR lpCmdLine, 
                    int nCmdShow)
{
    std::pair<_tstring, _tstring>* dlgParam = NULL;
    int nNumArgs = 0;    
    CommandLineToArgvW(GetCommandLineW(), &nNumArgs);
    if (nNumArgs == 3)
    {
        _tstring strCmdLine = lpCmdLine;
        size_t pos = strCmdLine.find(_T(' '));
        if (pos != _tstring::npos)
        {
            const _tstring& strHost = strCmdLine.substr(0, pos);
            const _tstring& strPort = strCmdLine.substr(pos+1);
            dlgParam = new std::pair<_tstring, _tstring>(strHost, strPort);
        }
    }
    return DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG_ASYNCSELECT), NULL, DlgProc, (LPARAM)dlgParam);
}


// dialog proc
INT_PTR CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    switch(uMsg)
    {
    case WM_INITDIALOG:
        {
            g_hDlg = hDlg; // initialize global handle

            std::pair<_tstring, _tstring>* dlgParam = (std::pair<_tstring, _tstring>*)lParam;
            if (dlgParam != NULL)
            {
                StartServer(dlgParam->first, dlgParam->second);
                delete dlgParam;
            }
        }
        break;

    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDOK:
        case IDCANCEL:
            {
                EndDialog(hDlg, 0);
            }
        default:
            OnCommand((HWND)lParam, HIWORD(wParam));
            break;
        }
        break;

    case WM_SOCKET:
        HandleNetEvents(hDlg, wParam, WSAGETSELECTEVENT(lParam), WSAGETSELECTERROR(lParam));
        break;

    }

    return 0;
}

void OnCommand(HWND hCtrl, DWORD dwNotifyCode)
{
    if (hCtrl == GetDlgItem(g_hDlg, IDC_BUTTO_START))
    {
        TCHAR szbuf[MAX_PATH];
        if (GetDlgItemText(g_hDlg, IDC_EDIT_HOST, szbuf, MAX_PATH) > 0)
        {
            _tstring strHost = szbuf;
            if (GetDlgItemText(g_hDlg, IDC_EDIT_PORT, szbuf, MAX_PATH) > 0)
            {
                _tstring strPort = szbuf;
                if (StartServer(strHost, strPort))
                    return ;
            }
        }
        _tstring msg = _T("start server failed.");
        AppendLogText(msg.data(), msg.length());
    }
    else if (hCtrl == GetDlgItem(g_hDlg, IDC_BUTTON_STOP))
    {
        CloseServer();
        EnableWindow(GetDlgItem(g_hDlg, IDC_EDIT_HOST), TRUE);
        EnableWindow(GetDlgItem(g_hDlg, IDC_EDIT_PORT), TRUE);
        EnableWindow(GetDlgItem(g_hDlg, IDC_BUTTO_START), TRUE);
        EnableWindow(GetDlgItem(g_hDlg, IDC_BUTTON_STOP), FALSE);
    }
}

bool StartServer(const _tstring& strHost, const _tstring& strPort)
{
    if (InitializeServer(g_hDlg, strHost, strPort))
    {
        
        SetDlgItemText(g_hDlg, IDC_EDIT_HOST, strHost.data());
        SetDlgItemText(g_hDlg, IDC_EDIT_PORT, strPort.data());
        EnableWindow(GetDlgItem(g_hDlg, IDC_EDIT_HOST), FALSE);
        EnableWindow(GetDlgItem(g_hDlg, IDC_EDIT_PORT), FALSE);
        EnableWindow(GetDlgItem(g_hDlg, IDC_BUTTO_START), FALSE);
        EnableWindow(GetDlgItem(g_hDlg, IDC_BUTTON_STOP), TRUE);
        _tstring msg = Now() + _T(", server start listen at ") + strHost + _T(":") + strPort;
        AppendLogText(msg.data(), msg.length());
        return true;
    }
    else
    {
        _tstring msg = _T("initialize server failed.");
        AppendLogText(msg.data(), msg.length());
        return false;
    }
}

// new text =  current text + appended text
bool AppendLogText(const TCHAR* text, int len)
{
    assert(text && len > 0);
    TCHAR textbuf[BUFSIZ];
    size_t count = GetDlgItemText(g_hDlg, IDC_EDIT_LOG, textbuf, BUFSIZ);
    if (count == 0 && GetLastError() != S_OK)
    {
        return false;
    }
    else
    {
        textbuf[count++] = ('\r');
        textbuf[count++] = ('\n');
    }
    if (count + len + 2 >= BUFSIZ)
    {
        return false;
    }

    _tcsncpy_s(textbuf + count, BUFSIZ-count, text, len);
    return SetDlgItemText(g_hDlg, IDC_EDIT_LOG, textbuf) == TRUE;
}