// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
//
// BrowserApp: the CefApp that is handed to CefInitialize().
//
// It is ONLY consumed by the Browser (main) process. Sub-processes never
// reach CefInitialize(), so even if you `new` this class, the sub-process
// side instance is never exercised.
//
// See sub_process_app.h for the counterpart.

#ifndef CEFSIMPLE_DEMO_BROWSER_APP_H_
#define CEFSIMPLE_DEMO_BROWSER_APP_H_

#include "include/cef_app.h"

class BrowserApp : public CefApp, public CefBrowserProcessHandler {
 public:
  BrowserApp();
  ~BrowserApp() override;

  // CefApp methods:
  void OnBeforeCommandLineProcessing(
      const CefString& process_type,
      CefRefPtr<CefCommandLine> command_line) override;

  CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() override {
    return this;
  }

  // Intentionally NOT overriding GetRenderProcessHandler(): it defaults to
  // nullptr. Even if the Browser process were asked for a render handler,
  // it shouldn't provide one.

  // CefBrowserProcessHandler methods:
  void OnContextInitialized() override;
  CefRefPtr<CefClient> GetDefaultClient() override;

 private:
  IMPLEMENT_REFCOUNTING(BrowserApp);
};

#endif  // CEFSIMPLE_DEMO_BROWSER_APP_H_
