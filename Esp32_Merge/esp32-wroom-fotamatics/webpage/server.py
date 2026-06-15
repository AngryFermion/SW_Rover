#!/usr/bin/env python3
"""
SmartWheels FOTA Demo — local web server
Serves the static dashboard and exposes API endpoints:
  GET  /api/releases        — list firmware versions from GitLab Package Registry (or local temp/)
  POST /api/deploy          — download from GitLab, verify SHA256, publish via MQTT, bootload
  GET  /api/srec/list       — (legacy) list .srec files in ../temp/
  POST /api/fota/trigger    — (legacy) run publish_srec.py then bootload_publisher.py
  GET  /api/fota/stream     — Server-Sent Events log stream for the active FOTA run

GitLab configuration (set as environment variables before starting server.py):
  GITLAB_URL          — e.g. https://gitlab.com  (default: https://gitlab.com)
  GITLAB_PROJECT_ID   — numeric project ID or namespace/project
  GITLAB_DEPLOY_USER  — deploy token username  (e.g. gitlab+deploy-token-XXXXXXXX)
  GITLAB_TOKEN        — deploy token password  (e.g. gldt-xxxx…)
"""
import os
import sys
import json
import glob
import queue
import base64
import hashlib
import subprocess
import threading
import urllib.parse
import urllib.request
import urllib.error
from http import HTTPStatus
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from datetime import datetime

BASE_DIR   = os.path.dirname(os.path.abspath(__file__))
PARENT_DIR = os.path.dirname(BASE_DIR)
SREC_DIR   = os.path.join(PARENT_DIR, 'temp')

# ── GitLab Generic Package Registry config ────────────────────────────────────
# Auth uses a Deploy Token (read_package_registry scope).
# Set these environment variables before starting server.py — never hard-code.
#
#   $env:GITLAB_URL         = "https://gitlab.com"
#   $env:GITLAB_PROJECT_ID  = "<numeric project ID>"
#   $env:GITLAB_DEPLOY_USER = "gitlab+deploy-token-XXXXXXXX"
#   $env:GITLAB_TOKEN       = "gldt-xxxxxxxxxxxxxxxxxxxx"
GITLAB_URL     = os.getenv('GITLAB_URL',         'https://gitlab.com')
GITLAB_PROJECT = os.getenv('GITLAB_PROJECT_ID',  '83355782')
GITLAB_USER    = os.getenv('GITLAB_DEPLOY_USER', 'gitlab+deploy-token-14069615')
GITLAB_TOKEN   = os.getenv('GITLAB_TOKEN',       '')  # set via env — never commit
GL_PKG_NAME    = 'smartwheels-s32k144-sm-rover'
GL_FIRMWARE    = 'Image_SM_Rover_V1_padded.srec'

def _gl_auth_header():
    """Return Authorization header for the configured GitLab credential type.
    Deploy token  → Basic base64(user:token)
    PAT/job token → PRIVATE-TOKEN  (fallback when no deploy user is set)
    """
    if GITLAB_USER and GITLAB_TOKEN:
        creds = base64.b64encode(f'{GITLAB_USER}:{GITLAB_TOKEN}'.encode()).decode()
        return {'Authorization': f'Basic {creds}'}
    return {'PRIVATE-TOKEN': GITLAB_TOKEN}

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
        if path == '/api/releases':
            self._releases()
        elif path == '/api/srec/list':
            self._srec_list()
        elif path == '/api/fota/stream':
            self._fota_stream()
        else:
            super().do_GET()

    def do_POST(self):
        path = urllib.parse.urlparse(self.path).path
        length = int(self.headers.get('Content-Length', 0))
        body = json.loads(self.rfile.read(length) or b'{}')
        if path == '/api/deploy':
            self._deploy(body)
        elif path == '/api/fota/trigger':
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

    def _releases(self):
        """List firmware versions from GitLab Package Registry, or fall back to local temp/."""
        if not GITLAB_PROJECT or not GITLAB_TOKEN:
            # Fallback: expose local .srec files the same way the legacy endpoint does
            files = glob.glob(os.path.join(SREC_DIR, '*.srec'))
            result = [
                {'source': 'local', 'version': os.path.basename(f),
                 'path': f, 'size': os.path.getsize(f)}
                for f in sorted(files, reverse=True)
            ]
            if not result:
                result = []
            self._json({'source': 'local', 'releases': result,
                        'warning': 'GITLAB_PROJECT_ID / GITLAB_TOKEN not set — showing local files'})
            return

        url = (f'{GITLAB_URL}/api/v4/projects/{urllib.parse.quote(str(GITLAB_PROJECT), safe="")}'
               f'/packages?package_name={urllib.parse.quote(GL_PKG_NAME)}'
               f'&sort=desc&per_page=20&order_by=created_at')
        req = urllib.request.Request(url, headers=_gl_auth_header())
        try:
            with urllib.request.urlopen(req, timeout=10) as resp:
                packages = json.loads(resp.read())
            releases = [
                {'source': 'gitlab', 'version': p['version'],
                 'created_at': p.get('created_at', ''), 'id': p.get('id'),
                 'package_name': GL_PKG_NAME}
                for p in packages
            ]
            self._json({'source': 'gitlab', 'releases': releases,
                        'package_name': GL_PKG_NAME})
        except urllib.error.HTTPError as e:
            # Include the URL (no token) so the operator can diagnose host/project issues
            safe_url = url.split('?')[0]
            self._json({
                'error': f'GitLab API {e.code}: {e.reason}',
                'url_tried': safe_url,
                'hint': (
                    'If 401/403: deploy token may lack read_api scope — use manual version entry below. '
                    'If 404: check GITLAB_URL (default is https://gitlab.com) and GITLAB_PROJECT_ID.'
                ),
                'manual_entry': True
            }, 502)
        except Exception as e:
            self._json({'error': str(e), 'manual_entry': True}, 502)

    def _deploy(self, body):
        """Download from GitLab, verify, then run existing publish/bootload scripts."""
        vin          = body.get('vin', '')
        package_name = body.get('package_name', GL_PKG_NAME)
        version      = body.get('version', '').strip()

        if not version:
            self._json({'error': 'version is required'}, 400)
            return
        if not GITLAB_PROJECT or not GITLAB_TOKEN:
            self._json({'error': 'GITLAB_PROJECT_ID and GITLAB_TOKEN must be set'}, 400)
            return

        with _fota_lock:
            while not _fota_queue.empty():
                try: _fota_queue.get_nowait()
                except queue.Empty: break

        threading.Thread(
            target=self._run_deploy, args=(vin, package_name, version), daemon=True
        ).start()
        self._json({'status': 'started', 'vin': vin, 'version': version})

    def _run_deploy(self, vin, package_name, version):
        def emit(stage, msg):
            _fota_queue.put(json.dumps(
                {'stage': stage, 'msg': msg, 'ts': datetime.now().strftime('%H:%M:%S')}
            ))

        base = (f'{GITLAB_URL}/api/v4/projects/{urllib.parse.quote(str(GITLAB_PROJECT), safe="")}'
                f'/packages/generic/{urllib.parse.quote(package_name)}/{urllib.parse.quote(version)}')
        hdrs = _gl_auth_header()

        def gl_get(filename, label, *, binary=False):
            """Download one file from GitLab; returns bytes or None on failure."""
            emit('upload', f'Downloading {label} from GitLab…')
            req = urllib.request.Request(f'{base}/{urllib.parse.quote(filename)}', headers=hdrs)
            try:
                with urllib.request.urlopen(req, timeout=60) as resp:
                    data = resp.read()
                emit('upload', f'{label} fetched ({len(data):,} bytes)')
                return data
            except urllib.error.HTTPError as e:
                emit('error', f'GitLab {e.code} fetching {filename}: {e.reason}')
                return None
            except Exception as e:
                emit('error', f'Download failed for {filename}: {e}')
                return None

        emit('upload', f'Deploy {package_name}@{version} → VIN {vin}')

        # 1. metadata.json — validate target MCU
        meta_raw = gl_get('metadata.json', 'metadata.json')
        if meta_raw is None:
            _fota_queue.put(None); return
        try:
            metadata = json.loads(meta_raw)
        except Exception:
            emit('error', 'metadata.json is not valid JSON')
            _fota_queue.put(None); return

        target = metadata.get('target', '')
        if target != 'S32K144':
            emit('error', f'metadata target mismatch: expected S32K144, got {target!r}')
            _fota_queue.put(None); return
        emit('upload', f'Metadata OK — target: {target}  fw: {metadata.get("version", "?")}')

        # 2. checksum.txt
        cksum_raw = gl_get('checksum.txt', 'checksum.txt')
        if cksum_raw is None:
            _fota_queue.put(None); return
        # Support "sha256:<hash>", "<hash>  filename", or bare "<hash>"
        cksum_text = cksum_raw.decode(errors='replace').strip()
        expected_sha = cksum_text.lstrip('sha256:').split()[0].lower()
        if len(expected_sha) != 64:
            emit('error', f'Unexpected checksum format: {cksum_text[:80]}')
            _fota_queue.put(None); return

        # 3. firmware .srec
        srec_data = gl_get(GL_FIRMWARE, GL_FIRMWARE, binary=True)
        if srec_data is None:
            _fota_queue.put(None); return

        # 4. SHA-256 verification
        actual_sha = hashlib.sha256(srec_data).hexdigest()
        if actual_sha != expected_sha:
            emit('error', f'SHA-256 mismatch — expected {expected_sha[:16]}… got {actual_sha[:16]}…')
            _fota_queue.put(None); return
        emit('upload', f'SHA-256 verified ✓  ({actual_sha[:16]}…)')

        # 5. Save to temp/
        os.makedirs(SREC_DIR, exist_ok=True)
        srec_filename = f'{package_name}-{version}.srec'
        srec_path = os.path.join(SREC_DIR, srec_filename)
        with open(srec_path, 'wb') as fh:
            fh.write(srec_data)
        emit('upload', f'Saved → temp/{srec_filename}  ({len(srec_data)/1024:.1f} KB)')

        # 6. publish_srec.py — chunks firmware to ESP32 over MQTT
        emit('upload', f'Publishing {srec_filename} → broker.emqx.io')
        p1 = subprocess.Popen(
            [sys.executable, os.path.join(PARENT_DIR, 'publish_srec.py'), srec_path],
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, bufsize=1
        )
        for line in p1.stdout:
            line = line.rstrip()
            if line: emit('upload', line)
        p1.wait()

        if p1.returncode != 0:
            emit('error', f'publish_srec.py failed (exit {p1.returncode})')
            _fota_queue.put(None); return

        emit('bootload', 'publish_srec.py complete — sending bootload command…')

        # 7. bootload_publisher.py — triggers S32K144 UART bootloader via ESP32
        p2 = subprocess.Popen(
            [sys.executable, os.path.join(PARENT_DIR, 'bootload_publisher.py'), 'serial'],
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, bufsize=1
        )
        for line in p2.stdout:
            line = line.rstrip()
            if line: emit('bootload', line)
        p2.wait()

        if p2.returncode == 0:
            emit('complete', f'FOTA complete — S32K144 running {version}  VIN {vin}')
        else:
            emit('error', f'bootload_publisher.py failed (exit {p2.returncode})')

        _fota_queue.put(None)

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
