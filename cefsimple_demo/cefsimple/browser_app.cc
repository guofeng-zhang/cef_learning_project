// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
//
// Implementation of BrowserApp. All log lines are tagged [BrowserApp] so
// you can filter them out of combined stderr:
//
//     ./cefsimple 2>&1 | grep -E '^\[(BrowserApp|SubProcessApp|main)\]'
//
// The key observation: every line tagged [BrowserApp] MUST come from the
// Browser (main) process. If you ever see it in a sub-process log, something
// is wrong with the role separation.

#include "browser_app.h"

#include <sys/types.h>
#include <unistd.h>

#include <cstdio>
#include <string>

#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"
#include "simple_handler.h"

#if defined(CEF_X11)
#include <X11/Xlib.h>
#endif

namespace {

// Views-framework helpers (unchanged from the upstream sample) -----------

class SimpleWindowDelegate : public CefWindowDelegate {
 public:
  SimpleWindowDelegate(CefRefPtr<CefBrowserView> browser_view,
                       cef_runtime_style_t runtime_style,
                       cef_show_state_t initial_show_state)
      : browser_view_(browser_view),
        runtime_style_(runtime_style),
        initial_show_state_(initial_show_state) {}

  SimpleWindowDelegate(const SimpleWindowDelegate&) = delete;
  SimpleWindowDelegate& operator=(const SimpleWindowDelegate&) = delete;

  void OnWindowCreated(CefRefPtr<CefWindow> window) override {
    window->AddChildView(browser_view_);
    if (initial_show_state_ != CEF_SHOW_STATE_HIDDEN) {
      window->Show();
    }
  }

  void OnWindowDestroyed(CefRefPtr<CefWindow> window) override {
    browser_view_ = nullptr;
  }

  bool CanClose(CefRefPtr<CefWindow> window) override {
    CefRefPtr<CefBrowser> browser = browser_view_->GetBrowser();
    if (browser) {
      return browser->GetHost()->TryCloseBrowser();
    }
    return true;
  }

  CefSize GetPreferredSize(CefRefPtr<CefView> view) override {
    return CefSize(800, 600);
  }

  cef_show_state_t GetInitialShowState(CefRefPtr<CefWindow> window) override {
    return initial_show_state_;
  }

  cef_runtime_style_t GetWindowRuntimeStyle() override {
    return runtime_style_;
  }

 private:
  CefRefPtr<CefBrowserView> browser_view_;
  const cef_runtime_style_t runtime_style_;
  const cef_show_state_t initial_show_state_;

  IMPLEMENT_REFCOUNTING(SimpleWindowDelegate);
};

class SimpleBrowserViewDelegate : public CefBrowserViewDelegate {
 public:
  explicit SimpleBrowserViewDelegate(cef_runtime_style_t runtime_style)
      : runtime_style_(runtime_style) {}

  SimpleBrowserViewDelegate(const SimpleBrowserViewDelegate&) = delete;
  SimpleBrowserViewDelegate& operator=(const SimpleBrowserViewDelegate&) =
      delete;

  bool OnPopupBrowserViewCreated(CefRefPtr<CefBrowserView> browser_view,
                                 CefRefPtr<CefBrowserView> popup_browser_view,
                                 bool is_devtools) override {
    CefWindow::CreateTopLevelWindow(new SimpleWindowDelegate(
        popup_browser_view, runtime_style_, CEF_SHOW_STATE_NORMAL));
    return true;
  }

  cef_runtime_style_t GetBrowserRuntimeStyle() override {
    return runtime_style_;
  }

 private:
  const cef_runtime_style_t runtime_style_;

  IMPLEMENT_REFCOUNTING(SimpleBrowserViewDelegate);
};

}  // namespace

// ---------------------------------------------------------------------------
// BrowserApp
// ---------------------------------------------------------------------------

BrowserApp::BrowserApp() {
  std::fprintf(stderr,
               "[BrowserApp] ctor     pid=%d (this object only does useful "
               "work in the Browser process)\n",
               static_cast<int>(::getpid()));
}

BrowserApp::~BrowserApp() {
  std::fprintf(stderr, "[BrowserApp] dtor     pid=%d\n",
               static_cast<int>(::getpid()));
}

void BrowserApp::OnBeforeCommandLineProcessing(
    const CefString& process_type,
    CefRefPtr<CefCommandLine> command_line) {
  // process_type is empty for the Browser process, and "renderer" / "gpu-
  // process" / "utility" / "zygote" etc. for sub-processes. Because this
  // CefApp only exists in the Browser process's address space, we expect
  // process_type to always be empty here.
  std::fprintf(stderr,
               "[BrowserApp] OnBeforeCommandLineProcessing pid=%d "
               "process_type='%s' (expect '')\n",
               static_cast<int>(::getpid()),
               process_type.ToString().c_str());
}

void BrowserApp::OnContextInitialized() {
  CEF_REQUIRE_UI_THREAD();
  std::fprintf(stderr,
               "[BrowserApp] OnContextInitialized pid=%d (Browser-only "
               "callback; sub-processes never see this)\n",
               static_cast<int>(::getpid()));

  CefRefPtr<CefCommandLine> command_line =
      CefCommandLine::GetGlobalCommandLine();

  cef_runtime_style_t runtime_style = CEF_RUNTIME_STYLE_DEFAULT;
  const bool use_alloy_style = command_line->HasSwitch("use-alloy-style");
  if (use_alloy_style) {
    runtime_style = CEF_RUNTIME_STYLE_ALLOY;
  }

  CefRefPtr<SimpleHandler> handler(new SimpleHandler(use_alloy_style));
  CefBrowserSettings browser_settings;

  std::string url = command_line->GetSwitchValue("url");
  if (url.empty()) {
    url = "https://www.google.com";
  }

  const bool use_views = !command_line->HasSwitch("use-native");

  if (use_views) {
    CefRefPtr<CefBrowserView> browser_view = CefBrowserView::CreateBrowserView(
        handler, url, browser_settings, nullptr, nullptr,
        new SimpleBrowserViewDelegate(runtime_style));

    cef_show_state_t initial_show_state = CEF_SHOW_STATE_NORMAL;
    const std::string& show_state_value =
        command_line->GetSwitchValue("initial-show-state");
    if (show_state_value == "minimized") {
      initial_show_state = CEF_SHOW_STATE_MINIMIZED;
    } else if (show_state_value == "maximized") {
      initial_show_state = CEF_SHOW_STATE_MAXIMIZED;
    }

    CefWindow::CreateTopLevelWindow(new SimpleWindowDelegate(
        browser_view, runtime_style, initial_show_state));
  } else {
    CefWindowInfo window_info;
    window_info.runtime_style = runtime_style;
    CefBrowserHost::CreateBrowser(window_info, handler, url, browser_settings,
                                  nullptr, nullptr);
  }
}

CefRefPtr<CefClient> BrowserApp::GetDefaultClient() {
  // Invoked when a new Browser is created via Chrome-style UI (e.g. the
  // user opens a new tab). Only meaningful in the Browser process.
  return SimpleHandler::GetInstance();
}
