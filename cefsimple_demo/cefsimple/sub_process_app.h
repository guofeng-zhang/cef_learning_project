// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
//
// SubProcessApp: the CefApp that is handed to CefExecuteProcess().
//
// It is only consumed in NON-Browser processes (Renderer / GPU / Utility /
// Zygote / Crashpad). In the Browser process CefExecuteProcess() returns -1
// immediately (see libcef/browser/main_runner.cc:186-188) without touching
// the CefApp you pass — so whatever is registered here is effectively
// invisible to the Browser process.
//
// See browser_app.h for the counterpart.

#ifndef CEFSIMPLE_DEMO_SUB_PROCESS_APP_H_
#define CEFSIMPLE_DEMO_SUB_PROCESS_APP_H_

#include "include/cef_app.h"
#include "include/cef_render_process_handler.h"

class SubProcessApp : public CefApp, public CefRenderProcessHandler {
 public:
  SubProcessApp();
  ~SubProcessApp() override;

  // CefApp methods:
  void OnBeforeCommandLineProcessing(
      const CefString& process_type,
      CefRefPtr<CefCommandLine> command_line) override;

  // Intentionally NOT overriding GetBrowserProcessHandler(): even if a
  // sub-process asked for it, this CefApp should not provide one.

  CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() override {
    return this;
  }

  // CefRenderProcessHandler methods: the ones below run inside the renderer
  // process. None of them can ever be triggered in the Browser process.
  void OnBrowserCreated(CefRefPtr<CefBrowser> browser,
                        CefRefPtr<CefDictionaryValue> extra_info) override;
  void OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) override;
  void OnContextCreated(CefRefPtr<CefBrowser> browser,
                        CefRefPtr<CefFrame> frame,
                        CefRefPtr<CefV8Context> context) override;
  void OnContextReleased(CefRefPtr<CefBrowser> browser,
                         CefRefPtr<CefFrame> frame,
                         CefRefPtr<CefV8Context> context) override;

 private:
  IMPLEMENT_REFCOUNTING(SubProcessApp);
};

#endif  // CEFSIMPLE_DEMO_SUB_PROCESS_APP_H_
