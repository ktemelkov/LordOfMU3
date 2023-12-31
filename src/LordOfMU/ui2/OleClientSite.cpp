/* 
   OleClientSite.cpp

   Copyright (C) René Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch
*/

#include "stdafx.h"

#include "OleClientSite.h"

OleClientSite::OleClientSite (
        IOleInPlaceSite*    in_place,
        IDocHostUIHandler*  doc_host_ui_handler,
        DWebBrowserEvents2* web_browser_events) :

          in_place_           (in_place           ),
          doc_host_ui_handler_(doc_host_ui_handler),
          web_browser_events_ (web_browser_events ) 
      {}

HRESULT OleClientSite::QueryInterface(REFIID riid, void ** ppvObject) {
  if (!memcmp((const void*) &riid, (const void*)&IID_IUnknown,       sizeof(GUID)) || 
      !memcmp((const void*) &riid, (const void*)&IID_IOleClientSite, sizeof(GUID))) {

    *ppvObject = this;//static_cast<IOleClientSite*>(this);//&((_IOleClientSiteEx *)This)->client;
    return S_OK;
  }
  else if (!memcmp((const void*)&riid, &IID_IOleInPlaceSite, sizeof(GUID))) {
    *ppvObject = in_place_;//static_cast<IOleInPlaceSite*>(this);
    return S_OK;
  }
  else if (!memcmp((const void*)&riid, &IID_IDocHostUIHandler, sizeof(GUID))) {
    *ppvObject = doc_host_ui_handler_;
    return S_OK;
  }

  // TODO: DIID_DWebBrowserEvents2 und IID_IDispatch koennen wohl raus
  else if (riid == DIID_DWebBrowserEvents2) {
    *ppvObject = web_browser_events_;
    return S_OK;
  }
  else if (riid == IID_IDispatch) {
    *ppvObject = web_browser_events_;
    return S_OK;
  }
  else {
    *ppvObject = 0;
    return E_NOINTERFACE;
  }
}
