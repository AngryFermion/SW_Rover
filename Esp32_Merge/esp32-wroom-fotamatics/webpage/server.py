#!/usr/bin/env python3
"""
SmartWheels FOTA Demo — local web server
Serves the static dashboard and exposes three API endpoints:
  GET  /api/srec/list      — list .srec files in ../temp/
  POST /api/fota/trigger   — run publish_srec.py then bootload_publisher.py
  GET  /api/fota/stream    — Server-Sent Events log stream for the active FOTA run
"""
import os
import sys
import json
import glob
import queue
import subprocess
import threading
import urllib.parse
from http import HTTPStatus
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from datetime import datetime

BASE_DIR   = os.path.dirname(os.path.abspath(__file__))
PARENT_DIR = os.path.dirname(BASE_DIR)
SREC_DIR   = os.path.join(PARENT_DIR, 'temp')

_fota_queue: queue.Queue = queue.Queue()
_fota_lock = threading.Lock()


class Handler(SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=BASE_DIR, **kwargs)

    def log_message(self, fmt, *args):
        pass  # silence access log for cleaner terminal

    # ── routing ──────────────────────────────────────────────────────────────

    def do_GET(self):
        path = urllib.parse.urlparse(self.path).path
        if path == '/api/srec/list':
            self._srec_list()
        elif path == '/api/fota/stream':
            self._fota_stream()
        else:
            super().do_GET()

    def do_POST(self):
        path = urllib.parse.urlparse(self.path).path
        if path == '/api/fota/trigger':
            length = int(self.headers.get('Content-Length', 0))
            body = json.loads(self.rfile.read(length) or b'{}')
            self._fota_trigger(body)
        else:
            self.send_error(HTTPStatus.NOT_FOUND)

    def do_OPTIONS(self):
        self.send_response(200)
        self._cors()
        self.end_headers()

    # ── helpers ───────────────────────────────────────────────────────────────

    def _cors(self):
        self.send_header('Access-Control-Allow-Origin', '*')
        self.send_header('Access-Control-Allow-Methods', 'GET, POST, OPTIONS')
        self.send_header('Access-Control-Allow-Headers', 'Content-Type')

    def _json(self, data, status=200):
        body = json.dumps(data).encode()
        self.send_response(status)
        self._cors()
        self.send_header('Content-Type', 'application/json')
        self.send_header('Content-Length', str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    # ── endpoints ─────────────────────────────────────────────────────────────

    def _srec_list(self):
        files = glob.glob(os.path.join(SREC_DIR, '*.srec'))
        result = [
            {'name': os.path.basename(f), 'path': f, 'size': os.path.getsize(f)}
            for f in sorted(files)
        ]
        self._json(result)

    def _fota_trigger(self, body):
        srec_path = body.get('srec_path', '')
        if not os.path.isfile(srec_path):
            files = glob.glob(os.path.join(SREC_DIR, '*.srec'))
            if not files:
                self._json({'error': 'No .srec file found in temp/'}, 400)
                return
            srec_path = sorted(files)[0]

        with _fota_lock:
            while not _fota_queue.empty():
                try:
                    _fota_queue.get_nowait()
                except queue.Empty:
                    break

        threading.Thread(target=self._run_fota, args=(srec_path,), daemon=True).start()
        self._json({'status': 'started', 'file': os.path.basename(srec_path)})

    def _run_fota(self, srec_path):
        def emit(stage, msg):
            _fota_queue.put(json.dumps({
                'stage': stage,
                'msg': msg,
                'ts': datetime.now().strftime('%H:%M:%S')
            }))

        emit('upload', f'Publishing {os.path.basename(srec_path)} → broker.emqx.io')

        p1 = subprocess.Popen(
            [sys.executable, os.path.join(PARENT_DIR, 'publish_srec.py'), srec_path],
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, bufsize=1
        )
        for line in p1.stdout:
            line = line.rstrip()
            if line:
                emit('upload', line)
        p1.wait()

        if p1.returncode != 0:
            emit('error', f'publish_srec.py failed (exit {p1.returncode})')
            _fota_queue.put(None)
            return

        emit('bootload', 'publish_srec.py complete — sending bootload command...')

        p2 = subprocess.Popen(
            [sys.executable, os.path.join(PARENT_DIR, 'bootload_publisher.py'), 'serial'],
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, bufsize=1
        )
        for line in p2.stdout:
            line = line.rstrip()
            if line:
                emit('bootload', line)
        p2.wait()

        if p2.returncode == 0:
            emit('complete', 'FOTA sequence complete — S32K144 bootloader running')
        else:
            emit('error', f'bootload_publisher.py failed (exit {p2.returncode})')

        _fota_queue.put(None)  # sentinel — tells stream to close

    def _fota_stream(self):
        self.send_response(200)
        self._cors()
        self.send_header('Content-Type', 'text/event-stream')
        self.send_header('Cache-Control', 'no-cache')
        self.send_header('X-Accel-Buffering', 'no')
        self.end_headers()

        try:
            while True:
                item = _fota_queue.get(timeout=120)
                if item is None:
                    self.wfile.write(b'data: {"stage":"done"}\n\n')
                    self.wfile.flush()
                    break
                self.wfile.write(f'data: {item}\n\n'.encode())
                self.wfile.flush()
        except (BrokenPipeError, ConnectionResetError, queue.Empty):
            pass


if __name__ == '__main__':
    port = 5000
    print(f'\n  SmartWheels Dashboard  →  http://localhost:{port}\n')
    server = ThreadingHTTPServer(('0.0.0.0', port), Handler)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print('\n  Server stopped.')
