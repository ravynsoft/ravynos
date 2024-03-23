#!/usr/bin/env python3

import http.server
import os
import threading
import webbrowser

port = 8000

for i in range(10):
    try:
        server = http.server.ThreadingHTTPServer(("localhost", port), http.server.SimpleHTTPRequestHandler)
        break
    except OSError:
        port += 1

thread = threading.Thread(target = server.serve_forever)
thread.daemon = True
thread.start()

webbrowser.open_new("http://localhost:" + str(port) + "/index.html")
input("\nPress Enter to exit\n\n")
