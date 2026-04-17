// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.
//
// main() for the cefsimple demo.
//
// This version is deliberately instrumented to showcase the asymmetry
// between CefExecuteProcess() and CefInitialize() with respect to the
// CefApp they receive:
//
//     int main(...) {
//         CefExecuteProcess(args, SubProcessApp, ...);  // sub-process CefApp
//         if (exit_code >= 0) return exit_code;         // sub-process exits here
//         CefInitialize(args, settings, BrowserApp, ...);// browser CefApp
//         ...
//     }
//
// Expected runtime behaviour:
//
//   * Browser process (no --type=)
//         - [main] banner prints pid and argv.
//         - [SubProcessApp] is never constructed.
//         - CefExecuteProcess returns -1 (see libcef/browser/main_runner.cc:
//           the `if (!command_line.HasSwitch(kProcessType)) return -1;` branch).
//         - BrowserApp is constructed and its OnContextInitialized fires.
//
//   * Renderer / GPU / Utility / Zygote sub-processes (--type=...)
//         - [main] banner prints the same pid/argv.
//         - SubProcessApp is constructed and OnBeforeCommandLineProcessing
//           fires with process_type != "".
//         - BrowserApp is never constructed (we `return exit_code;` first).
//         - Only the renderer sub-process goes on to fire OnBrowserCreated
//           and OnContextCreated.

#include "browser_app.h"
#include "sub_process_app.h"

#include <sys/types.h>
#include <unistd.h>

#include <cstdio>
#include <string>

#if defined(CEF_X11)
#include <X11/Xlib.h>
#endif

#include "include/base/cef_logging.h"
#include "include/cef_app.h"
#include "include/cef_command_line.h"

#if defined(CEF_X11)
namespace {

int XErrorHandlerImpl(Display* display, XErrorEvent* event) {
  LOG(WARNING) << "X error received: " << "type " << event->type << ", "
               << "serial " << event->serial << ", " << "error_code "
               << static_cast<int>(event->error_code) << ", " << "request_code "
               << static_cast<int>(event->request_code) << ", " << "minor_code "
               << static_cast<int>(event->minor_code);
  return 0;
}

int XIOErrorHandlerImpl(Display* display) {
  return 0;
}

}  // namespace
#endif  // defined(CEF_X11)

namespace {

// Prints a header identifying the current process before we decide whether
// it's a Browser or a sub-process. Run as `./cefsimple 2>&1 | grep '^\['` to
// see just the instrumentation.
void LogStartupBanner(int argc, char* argv[]) {
  std::string argv_joined;
  for (int i = 0; i < argc; ++i) {
    if (i > 0) argv_joined.push_back(' ');
    argv_joined.append(argv[i]);
  }
  std::fprintf(
      stderr,
      "[main] ====== process start pid=%d argc=%d argv='%s'\n",
      static_cast<int>(::getpid()), argc, argv_joined.c_str());
}

}  // namespace

NO_STACK_PROTECTOR
int main(int argc, char* argv[]) {
  LogStartupBanner(argc, argv);

  CefMainArgs main_args(argc, argv);

  // ------------------------------------------------------------------
  // STEP 1: hand a SubProcessApp to CefExecuteProcess.
  //
  // In the Browser process, this call returns -1 immediately without
  // ever using sub_app. So constructing sub_app in the Browser is
  // technically wasted work — we do it anyway to make the asymmetry
  // visible in logs.
  // ------------------------------------------------------------------
  std::fprintf(stderr,
               "[main] pid=%d constructing SubProcessApp and calling "
               "CefExecuteProcess ...\n",
               static_cast<int>(::getpid()));
  CefRefPtr<SubProcessApp> sub_app(new SubProcessApp);

  int exit_code = CefExecuteProcess(main_args, sub_app.get(), nullptr);
  std::fprintf(stderr,
               "[main] pid=%d CefExecuteProcess returned %d "
               "(<0 means Browser process, >=0 means sub-process is done)\n",
               static_cast<int>(::getpid()), exit_code);

  if (exit_code >= 0) {
    // Sub-process path. SubProcessApp was the ONLY CefApp this process
    // ever used. We never reach CefInitialize below.
    std::fprintf(stderr,
                 "[main] pid=%d sub-process exiting with code=%d "
                 "(BrowserApp was never constructed here)\n",
                 static_cast<int>(::getpid()), exit_code);
    return exit_code;
  }

  // --- From here on, we are definitely in the Browser process. --------

#if defined(CEF_X11)
  XSetErrorHandler(XErrorHandlerImpl);
  XSetIOErrorHandler(XIOErrorHandlerImpl);
#endif

  CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
  command_line->InitFromArgv(argc, argv);

  CefSettings settings;
#if !defined(CEF_USE_SANDBOX)
  settings.no_sandbox = true;
#endif

  // ------------------------------------------------------------------
  // STEP 2: hand a BrowserApp to CefInitialize.
  //
  // This CefApp is stored in CefContext::application_ and consulted for
  // the lifetime of the Browser process (GetBrowserProcessHandler,
  // OnBeforeCommandLineProcessing, GetDefaultClient, ...).
  // ------------------------------------------------------------------
  std::fprintf(stderr,
               "[main] pid=%d constructing BrowserApp and calling "
               "CefInitialize ...\n",
               static_cast<int>(::getpid()));
  CefRefPtr<BrowserApp> browser_app(new BrowserApp);

  if (!CefInitialize(main_args, settings, browser_app.get(), nullptr)) {
    std::fprintf(stderr,
                 "[main] pid=%d CefInitialize FAILED exit_code=%d\n",
                 static_cast<int>(::getpid()), CefGetExitCode());
    return CefGetExitCode();
  }

  std::fprintf(stderr,
               "[main] pid=%d entering CefRunMessageLoop (Browser process)\n",
               static_cast<int>(::getpid()));
  CefRunMessageLoop();

  std::fprintf(stderr,
               "[main] pid=%d calling CefShutdown (Browser process)\n",
               static_cast<int>(::getpid()));
  CefShutdown();

  return 0;
}
