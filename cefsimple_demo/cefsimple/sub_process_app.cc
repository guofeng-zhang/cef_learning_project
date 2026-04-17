// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
//
// Implementation of SubProcessApp. Every log line is tagged
// [SubProcessApp] so you can confirm it only appears in sub-process stderr.
//
// The three most useful observations to make from these logs:
//
//   1. ctor and OnBeforeCommandLineProcessing fire in EVERY sub-process
//      (renderer, gpu-process, utility, zygote ...), and the process_type
//      string reveals which one.
//   2. OnBrowserCreated / OnContextCreated only fire in the RENDERER
//      process — never in gpu-process, utility, or zygote.
//   3. None of these ever fire in the Browser process, which proves that
//      the CefApp passed to CefExecuteProcess is indeed sub-process-only.

#include "sub_process_app.h"

#include <sys/types.h>
#include <unistd.h>

#include <cstdio>

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/cef_frame.h"
#include "include/cef_v8.h"

namespace {

// Reads /proc/self/cmdline on Linux so we can print the full argv of the
// current sub-process regardless of what the launcher rewrote argv[0] to.
// (argv inside sub-processes is sometimes rewritten by Chromium's zygote.)
std::string ReadProcCmdline() {
  std::FILE* f = std::fopen("/proc/self/cmdline", "rb");
  if (!f) {
    return "<no /proc>";
  }
  std::string out;
  char buf[256];
  size_t n = 0;
  while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) {
    for (size_t i = 0; i < n; ++i) {
      out.push_back(buf[i] ? buf[i] : ' ');
    }
  }
  std::fclose(f);
  return out;
}

}  // namespace

SubProcessApp::SubProcessApp() {
  std::fprintf(stderr,
               "[SubProcessApp] ctor     pid=%d cmdline='%s'\n",
               static_cast<int>(::getpid()),
               ReadProcCmdline().c_str());
}

SubProcessApp::~SubProcessApp() {
  std::fprintf(stderr, "[SubProcessApp] dtor     pid=%d\n",
               static_cast<int>(::getpid()));
}

void SubProcessApp::OnBeforeCommandLineProcessing(
    const CefString& process_type,
    CefRefPtr<CefCommandLine> command_line) {
  // process_type carries the --type= value: "renderer", "gpu-process",
  // "utility", "zygote", "crashpad-handler", etc. For this CefApp it
  // should never be empty, because the Browser process never reaches us.
  std::fprintf(stderr,
               "[SubProcessApp] OnBeforeCommandLineProcessing pid=%d "
               "process_type='%s' (expect one of: renderer / gpu-process / "
               "utility / zygote / ...)\n",
               static_cast<int>(::getpid()),
               process_type.ToString().c_str());
}

void SubProcessApp::OnBrowserCreated(
    CefRefPtr<CefBrowser> browser,
    CefRefPtr<CefDictionaryValue> /*extra_info*/) {
  // Only invoked in the RENDERER process after a Browser's main frame is
  // bound on this renderer side. Even though other sub-processes hold a
  // SubProcessApp too, OnBrowserCreated is a CefRenderProcessHandler
  // method and the CEF runtime only calls it on renderers.
  std::fprintf(stderr,
               "[SubProcessApp] OnBrowserCreated    pid=%d "
               "browser_id=%d (renderer-side)\n",
               static_cast<int>(::getpid()),
               browser ? browser->GetIdentifier() : -1);
}

void SubProcessApp::OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) {
  std::fprintf(stderr,
               "[SubProcessApp] OnBrowserDestroyed  pid=%d browser_id=%d\n",
               static_cast<int>(::getpid()),
               browser ? browser->GetIdentifier() : -1);
}

void SubProcessApp::OnContextCreated(CefRefPtr<CefBrowser> browser,
                                     CefRefPtr<CefFrame> frame,
                                     CefRefPtr<CefV8Context> /*context*/) {
  // Fires in the RENDERER process each time a V8 context is bound to a
  // frame — the earliest safe place to register JS<->C++ bindings.
  std::fprintf(
      stderr,
      "[SubProcessApp] OnContextCreated    pid=%d browser_id=%d url='%s'\n",
      static_cast<int>(::getpid()), browser ? browser->GetIdentifier() : -1,
      frame ? frame->GetURL().ToString().c_str() : "");
}

void SubProcessApp::OnContextReleased(CefRefPtr<CefBrowser> browser,
                                      CefRefPtr<CefFrame> frame,
                                      CefRefPtr<CefV8Context> /*context*/) {
  std::fprintf(
      stderr,
      "[SubProcessApp] OnContextReleased   pid=%d browser_id=%d url='%s'\n",
      static_cast<int>(::getpid()), browser ? browser->GetIdentifier() : -1,
      frame ? frame->GetURL().ToString().c_str() : "");
}
