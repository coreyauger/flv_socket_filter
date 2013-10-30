//-----------------------------------------------------------------------------
//
//	AfterCAD RTMP Writer Properties Page
//
//	Author : Kenney Wong
//
//-----------------------------------------------------------------------------

#include "stdafx.h"


void MakeNiceSize(__int64 size, CString &v)
{
	int r=0;
	__int64	c=size;
	LPCTSTR		rady[] = {
		_T("bytes"),
		_T("KB"),
		_T("MB"),
		_T("GB"),
		_T("TB")
	};

	// spocitame rad
	while (c > 1024 && r<4) {
		r++;
		c = c / 1024;
	}

	c=size;
	for (int i=1; i<r; i++) { c = c/1024; }
	double d=c / 1024.0;

	v.Format(_T("%5.3f %s"), (float)d, rady[r]);
}

void MakeNiceSpeed(__int64 bps, CString &v)
{
	int r=0;
	__int64	c=bps;
	LPCTSTR		rady[] = {
		_T("bps"),
		_T("kbps"),
		_T("mbps"),
		_T("gbps"),
		_T("tbps")
	};

	// spocitame rad
	while (c > 1000 && r<4) {
		r++;
		c = c / 1000;
	}

	c=bps;
	for (int i=1; i<r; i++) { c = c/1000; }
	double d=c / 1000.0;

	v.Format(_T("%5.3f %s"), (float)d, rady[r]);
}

void MakeNiceTime(__int64 time_ms, CString &v)
{
	__int64		ms = time_ms%1000;	
	time_ms -= ms;
	time_ms /= 1000;

	int		h, m, s;
	h = time_ms / 3600;		time_ms -= h*3600;
	m = time_ms / 60;		time_ms -= m*60;
	s = time_ms;

	v.Format(_T("%.2d:%.2d:%.2d,%.3d"), h, m, s, ms);
}


#define MAX_TEXT_LENGTH			1024
#define WM_UPDATE_VISUALS		(WM_USER + 1003)
#define ITEM(x)					(GetDlgItem(m_Dlg, x))

//-----------------------------------------------------------------------------
//
//	CVideoRTMPSendPropertyPage class
//
//-----------------------------------------------------------------------------

CUnknown *CSocketSendPropertyPage::CreateInstance(LPUNKNOWN lpUnk, HRESULT *phr)
{
	return new CSocketSendPropertyPage(lpUnk, phr);
}

CSocketSendPropertyPage::CSocketSendPropertyPage(LPUNKNOWN lpUnk, HRESULT *phr) :
	CBasePropertyPage(NAME("Socket Writer Property Page"), lpUnk, IDD_PAGE_RTMP, IDS_PAGE_RTMP),
	socketSendFilter(NULL)
{
}

CSocketSendPropertyPage::~CSocketSendPropertyPage()
{
}

HRESULT CSocketSendPropertyPage::OnConnect(IUnknown *pUnknown)
{
	ASSERT(!socketSendFilter);

	HRESULT hr = pUnknown->QueryInterface(IID_ISocketSendFilter, (void**)&socketSendFilter);
	if (FAILED(hr)) return hr;

	// okay
	return NOERROR;
}

HRESULT CSocketSendPropertyPage::OnDisconnect()
{
	if (socketSendFilter) {
		socketSendFilter->Release();
		socketSendFilter = NULL;
	}
	return NOERROR;
}

HRESULT CSocketSendPropertyPage::OnActivate()
{
	SetTimer(m_Dlg, 0, 300, NULL);
    GetRtmpServerInfo();
	UpdateStreamInfo();
	return NOERROR;
}

HRESULT CSocketSendPropertyPage::OnDeactivate()
{
	KillTimer(m_Dlg, 0);
	return NOERROR;
}

INT_PTR CSocketSendPropertyPage::OnReceiveMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_TIMER:
		{
			UpdateStreamInfo();
            PutRtmpServerInfo();
		}
		break;
	}
	return __super::OnReceiveMessage(hwnd, uMsg, wParam, lParam);
}

void CSocketSendPropertyPage::GetRtmpServerInfo()
{
	if (!socketSendFilter) return ;
	/*
	RTMP_Send_Info		info;
	socketSendFilter->GetRTMPSendInfo(&info);

	// destination rtmp server info
	Edit_SetText(ITEM(IDC_EDIT_SERVER_ADDRESS), info.host_name);
	Edit_SetText(ITEM(IDC_EDIT_APP_NAME), info.app_name);
	Edit_SetText(ITEM(IDC_EDIT_STREAM_NAME), info.play_path);
    TCHAR hostPortStr[256];
    swprintf_s( hostPortStr, 256, _T("%d"), info.host_port );
	Edit_SetText(ITEM(IDC_EDIT_SERVER_PORT), hostPortStr);
	*/
}

void CSocketSendPropertyPage::PutRtmpServerInfo()
{
	if (!socketSendFilter) return ;
	/*
	RTMP_Send_Info		info;

	// destination rtmp server info
	Edit_GetText(ITEM(IDC_EDIT_SERVER_ADDRESS), info.host_name, 256);
	Edit_GetText(ITEM(IDC_EDIT_APP_NAME), info.app_name, 256);
	Edit_GetText(ITEM(IDC_EDIT_STREAM_NAME), info.play_path, 256);
    TCHAR hostPortStr[256];
    int hostPort;
	Edit_GetText(ITEM(IDC_EDIT_SERVER_PORT), hostPortStr, 256);
    if ( swscanf_s( hostPortStr, _T("%d"), &hostPort ) == 1 )
        info.host_port = hostPort;

	socketSendFilter->PutRTMPSendInfo(&info);
	*/
}

void CSocketSendPropertyPage::UpdateStreamInfo()
{
	if (!socketSendFilter) return ;
/*
	RTMP_Send_Info		info;
	socketSendFilter->GetRTMPSendInfo(&info);

	// stream descriptions
	Static_SetText(ITEM(IDC_STATIC_AUDIO), info.audio_stream);
	Static_SetText(ITEM(IDC_STATIC_VIDEO), info.video_stream);

	double	rate = 0;
	if (info.duration_sec > 0) {
		rate = info.file_size * 8.0 / info.duration_sec;
	}

	CString		s;
	MakeNiceSpeed((__int64)rate, s);
	Static_SetText(ITEM(IDC_STATIC_DATARATE), s);

	// duration
	__int64 ms = (__int64)(info.duration_sec * 1000.0);
	MakeNiceTime(ms, s);
	Static_SetText(ITEM(IDC_STATIC_DURATION), s);

	// size
	MakeNiceSize(info.file_size, s);
	Static_SetText(ITEM(IDC_STATIC_SIZE), s);
	*/
}

