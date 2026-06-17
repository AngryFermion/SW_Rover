'use strict';

// ── Config ────────────────────────────────────────────────────────────────────
// SIGNAL_VIEW_MACRO: default view for both rover cards on load.
// 'full'   → Speed gauge + Distance, RPM, Throttle, Brake, Steering
// 'simple' → Speed gauge + Distance only
const DEFAULT_SIGNAL_VIEW = 'full'; // 'full' | 'simple'

// API_BASE: set by config.js when hosted on an external site (e.g. ancitconsulting.com).
// Leave empty when running locally via server.py — relative paths work fine.
const API_BASE = (typeof SW_API_BASE !== 'undefined' && SW_API_BASE) ? SW_API_BASE.replace(/\/$/, '') : '';

const MQTT_URL  = 'wss://broker.emqx.io:8084/mqtt';
const MQTT_OPTS = {
  username: 'rw',
  password: 'readwrite',
  clean: true,
  reconnectPeriod: 4000,
  clientId: 'swdash_' + Math.random().toString(16).slice(2, 10)
};

// VIN identifiers — must match TELEMATICS_DEVICE_ID in telematics_config.h
const VIN_ALPHA = 'MA3ZZ20G0T1234567';
const VIN_BETA  = 'WROOM_002';

// Topics matching the ESP32 firmware (telematics_config.h, ancit_mqtt_client.cpp)
const T_TELEM   = 'SmartKit/Ultra';
const T_HEALTH  = 'SmartKit/device_health';
const T_FOTA_P  = 'SmartWheelsNS/device/+/fota/progress';
// LWT topic: broker publishes "online"/"offline" here on connect/disconnect.
// Built from clientId "sdv-vehicle-001" hardcoded in ancit_mqtt_client.cpp:262.
const T_LWT     = 'SmartWheelsNS/telematics/status/sdv-vehicle-001';

// Device-ID → rover slot (matches TELEMATICS_DEVICE_ID in firmware)
const DEV_MAP = { [VIN_ALPHA]: 1, [VIN_BETA]: 2 };

// ── Gauge geometry ────────────────────────────────────────────────────────────
// SVG gauge: centre (100,95), radius 72, sweep -135°→+135° (270° total)
const CX = 100, CY = 95, R = 72;
const G_START = -135, G_END = 135, G_SWEEP = 270;
const MAX_SPEED = 120, MAX_RPM = 6000, MAX_DIST = 400;

function polarToXY(deg, r) {
  const rad = (deg - 90) * Math.PI / 180;
  return { x: CX + r * Math.cos(rad), y: CY + r * Math.sin(rad) };
}

function arcPath(startDeg, endDeg, r) {
  const s = polarToXY(startDeg, r);
  const e = polarToXY(endDeg, r);
  const large = (endDeg - startDeg) > 180 ? 1 : 0;
  return `M ${s.x.toFixed(2)} ${s.y.toFixed(2)} A ${r} ${r} 0 ${large} 1 ${e.x.toFixed(2)} ${e.y.toFixed(2)}`;
}

function initTicks(n) {
  const g = document.getElementById(`ticks-${n}`);
  const NS = 'http://www.w3.org/2000/svg';
  [0, 30, 60, 90, 120].forEach(v => {
    const ang = G_START + (v / MAX_SPEED) * G_SWEEP;
    const isMajor = v % 60 === 0;
    const p1 = polarToXY(ang, R - 7);
    const p2 = polarToXY(ang, R + 7);
    const line = document.createElementNS(NS, 'line');
    line.setAttribute('x1', p1.x.toFixed(2)); line.setAttribute('y1', p1.y.toFixed(2));
    line.setAttribute('x2', p2.x.toFixed(2)); line.setAttribute('y2', p2.y.toFixed(2));
    line.setAttribute('stroke', isMajor ? '#252d48' : '#1a2238');
    line.setAttribute('stroke-width', isMajor ? '2' : '1.5');
    g.appendChild(line);

    if (isMajor) {
      const lp = polarToXY(ang, R + 19);
      const t = document.createElementNS(NS, 'text');
      t.setAttribute('x', lp.x.toFixed(2));
      t.setAttribute('y', (lp.y + 4).toFixed(2));
      t.setAttribute('text-anchor', 'middle');
      t.setAttribute('fill', '#283040');
      t.setAttribute('font-size', '9');
      t.setAttribute('font-family', 'system-ui,sans-serif');
      t.textContent = v;
      g.appendChild(t);
    }
  });
}

function setGauge(n, speed) {
  const pct = Math.min(Math.max(speed, 0), MAX_SPEED) / MAX_SPEED;
  const el = document.getElementById(`gauge-val-${n}`);
  if (pct < 0.005) { el.setAttribute('d', ''); return; }
  el.setAttribute('d', arcPath(G_START, G_START + pct * G_SWEEP, R));
  // Gradient hue: green(120) → yellow(60) → red(0)
  const hue = Math.round(120 - pct * 120);
  const col = `hsl(${hue},85%,55%)`;
  el.style.stroke = col;
  el.style.filter = `drop-shadow(0 0 5px ${col})`;
}

// ── Rover state ───────────────────────────────────────────────────────────────
// Rover 1 (Alpha) = real ESP32 device — no local simulation, waits for MQTT.
// Rover 2 (Beta)  = demo simulation until a real WROOM_002 device appears.
const rovers = {
  1: { speed: 0, rpm: 0, throttle: 0, brake: 0, steering: 0, distance: 0,
       isLive: false, lastSeen: 0, lastVinBeat: 0, view: DEFAULT_SIGNAL_VIEW,
       sim: { spd: { val: 0, tgt: 0, t: 0 }, dst: { val: 0, tgt: 0, t: 0 } } },
  2: { speed: 0, rpm: 0, throttle: 0, brake: 0, steering: 0, distance: 150,
       isLive: false, lastSeen: 0, lastVinBeat: 0, view: DEFAULT_SIGNAL_VIEW,
       sim: { spd: { val: 55, tgt: 55, t: 0 }, dst: { val: 150, tgt: 150, t: 0 } } }
};

// ── DOM helpers ───────────────────────────────────────────────────────────────
function setBar(id, pct, color) {
  const el = document.getElementById(id);
  el.style.width = `${Math.min(100, Math.max(0, pct * 100)).toFixed(1)}%`;
  if (color) el.style.backgroundColor = color;
}

function setText(id, v) { document.getElementById(id).textContent = v; }

function updateUI(n) {
  const s = rovers[n];

  // Rover Alpha (n=1) shows dashes until the first real MQTT message arrives.
  const waiting = (n === 1 && !s.isLive);

  setGauge(n, waiting ? 0 : s.speed);
  setText(`speed-num-${n}`, waiting ? '—' : Math.round(s.speed));

  // Distance bar: near = full bar, colour shifts green→amber→red
  if (waiting) {
    setBar(`dist-bar-${n}`, 0, '#2e3550');
    setText(`dist-val-${n}`, '— cm');
  } else {
    const distPct = 1 - Math.min(s.distance, MAX_DIST) / MAX_DIST;
    const distCol = s.distance < 50 ? '#ff5252' : s.distance < 100 ? '#ffab40' : '#00e676';
    setBar(`dist-bar-${n}`, distPct, distCol);
    setText(`dist-val-${n}`, `${Math.round(s.distance)} cm`);
  }

  setBar(`rpm-bar-${n}`, waiting ? 0 : s.rpm / MAX_RPM);
  setText(`rpm-val-${n}`, waiting ? '—' : Math.round(s.rpm));

  setBar(`thr-bar-${n}`, waiting ? 0 : s.throttle / 100);
  setText(`thr-val-${n}`, waiting ? '— %' : `${Math.round(s.throttle)} %`);

  setBar(`brk-bar-${n}`, waiting ? 0 : s.brake / 100);
  setText(`brk-val-${n}`, waiting ? '— %' : `${Math.round(s.brake)} %`);

  // Steering: map -90…+90 → 0…100%
  setBar(`str-bar-${n}`, waiting ? 0 : (s.steering + 90) / 180);
  setText(`str-val-${n}`, waiting ? '—' : `${s.steering >= 0 ? '+' : ''}${Math.round(s.steering)}°`);

  // ACC badge
  const badge = document.getElementById(`acc-badge-${n}`);
  const accTxt = document.getElementById(`acc-text-${n}`);
  badge.className = 'acc-badge';
  if (waiting) {
    accTxt.textContent = 'ACC WAITING';
  } else if (s.distance < 40) {
    badge.classList.add('braking');
    accTxt.textContent = 'ACC BRAKING';
  } else if (s.distance < 100) {
    badge.classList.add('adaptive');
    accTxt.textContent = 'ACC ADAPTIVE';
  } else {
    accTxt.textContent = 'ACC CRUISE';
  }
}

// ── Signal view toggle ────────────────────────────────────────────────────────
// applyView syncs the card class and button label to rovers[n].view.
function applyView(n) {
  const card = document.getElementById(`rover-card-${n}`);
  const btn  = document.getElementById(`view-btn-${n}`);
  if (rovers[n].view === 'simple') {
    card.classList.add('simple-view');
    btn.textContent = '⊞ Full';
  } else {
    card.classList.remove('simple-view');
    btn.textContent = '⊟ Simple';
  }
}

// toggleView is called by the per-card button (onclick="toggleView(n)").
function toggleView(n) {
  rovers[n].view = rovers[n].view === 'full' ? 'simple' : 'full';
  applyView(n);
}

function setHealth(n, state, label) {
  const dot = document.getElementById(`health-dot-${n}`);
  dot.className = `health-dot ${state}`; // online | warn | offline
  setText(`health-label-${n}`, label);
}

function bumpVin(n) {
  rovers[n].lastVinBeat = Date.now();
  setText(`vin-beat-${n}`, 'heartbeat: just now');
}

// ── Health monitor (runs every 2 s) ──────────────────────────────────────────
function monitorHealth() {
  const now = Date.now();
  [1, 2].forEach(n => {
    const s = rovers[n];
    const ref = s.isLive ? s.lastSeen : s.lastVinBeat;

    // ref===0 means this rover has never sent a message
    if (ref === 0) {
      if (n === 1) setHealth(1, 'warn', `Waiting for VIN ${VIN_ALPHA}…`);
      return;
    }

    const age = (now - ref) / 1000;
    const ago = age < 60
      ? `${Math.round(age)}s ago`
      : `${Math.floor(age / 60)}m ${Math.round(age % 60)}s ago`;

    if (age > 1) setText(`vin-beat-${n}`, `heartbeat: ${ago}`);

    if (age > 30)      setHealth(n, 'offline', `Offline (${ago})`);
    else if (age > 15) setHealth(n, 'warn',    `Warning (${ago})`);
    else               setHealth(n, 'online',  'Online');
  });
}

// ── Demo simulation (Rover Beta only) ────────────────────────────────────────
function stepSim(n) {
  if (n === 1) return;  // Rover Alpha always uses real MQTT data — no local sim
  const s = rovers[n];
  if (s.isLive) return;

  const now = Date.now();
  const sim = s.sim;

  // New speed target every 4–8 s (stagger rovers)
  if (now - sim.spd.t > 4000 + n * 1200 + Math.random() * 4000) {
    sim.spd.tgt = 15 + Math.random() * 75;
    sim.spd.t   = now;
  }
  // New distance target every 2–5 s
  if (now - sim.dst.t > 2000 + Math.random() * 3000) {
    sim.dst.tgt = 30 + Math.random() * 350;
    sim.dst.t   = now;
  }
  // ACC: braking when obstacle close
  if (sim.dst.val < 60) sim.spd.tgt = Math.max(0, sim.spd.tgt * 0.5);

  // Smooth exponential approach
  sim.spd.val += (sim.spd.tgt - sim.spd.val) * 0.04;
  sim.dst.val += (sim.dst.tgt - sim.dst.val) * 0.05;

  s.speed    = Math.max(0, sim.spd.val + (Math.random() - 0.5) * 1.5);
  s.distance = Math.max(5, sim.dst.val + (Math.random() - 0.5) * 4);
  s.rpm      = Math.max(0, s.speed * 47 + (Math.random() - 0.5) * 60);
  s.throttle = Math.max(0, Math.min(100, (s.speed / 80) * 80 + (Math.random() - 0.5) * 8));
  s.brake    = s.distance < 60 ? Math.min(100, (60 - s.distance) * 1.8) : 0;
  s.steering = Math.sin(now / 4500 + n * 1.8) * 18 + (Math.random() - 0.5) * 4;

  // VIN heartbeat every 10 s
  if (now - s.lastVinBeat > 10000) bumpVin(n);
}

function animLoop() {
  [1, 2].forEach(n => { stepSim(n); updateUI(n); });
  requestAnimationFrame(animLoop);
}

// ── MQTT ──────────────────────────────────────────────────────────────────────
function connectMqtt() {
  let client;
  try { client = mqtt.connect(MQTT_URL, MQTT_OPTS); }
  catch (e) { setMqttStatus('error', 'MQTT unavailable'); return; }

  client.on('connect', () => {
    setMqttStatus('connected', 'Live');
    client.subscribe([T_TELEM, T_HEALTH, T_FOTA_P, T_LWT]);
  });
  client.on('error',     () => setMqttStatus('error',   'Connection error'));
  client.on('reconnect', () => setMqttStatus('offline', 'Reconnecting…'));
  client.on('offline',   () => setMqttStatus('offline', 'Offline'));

  client.on('message', (topic, payload) => {
    const raw = payload.toString();
    // LWT publishes plain "online"/"offline" strings — not JSON
    if (topic === T_LWT) { handleMsg(topic, raw); return; }
    try { handleMsg(topic, JSON.parse(raw)); } catch (_) {}
  });
}

function handleMsg(topic, data) {
  if (topic === T_TELEM) {
    const n = DEV_MAP[data.device_id];
    if (!n) return;
    const s = rovers[n];
    s.isLive  = true;
    s.lastSeen = Date.now();
    const sig = data.signal;
    if (sig === 'Speed')          s.speed    = data.value;
    else if (sig === 'ENGINE_RPM')     s.rpm      = data.value;
    else if (sig === 'THROTTLE')       s.throttle = data.value;
    else if (sig === 'BRAKE')          s.brake    = data.value;
    else if (sig === 'STEERING_ANGLE') s.steering = data.value;
    else if (sig === 'Distance' || sig === 'ULTRASONIC') s.distance = data.value;
    bumpVin(n);
    const badge = document.getElementById(`mode-${n}`);
    badge.textContent = 'LIVE';
    badge.className = 'mode-badge live'; // replaces 'waiting' class if present
  }

  if (topic === T_HEALTH) {
    const devId = data.device_id || data.vin;
    const n = DEV_MAP[devId];
    if (n) bumpVin(n);
  }

  // LWT: broker fires this on clean connect ("online") or unexpected disconnect ("offline").
  // data is a plain string here, not an object.
  if (topic === T_LWT) {
    if (data === 'online') {
      rovers[1].lastVinBeat = Date.now();
      setHealth(1, 'online', `VIN ${VIN_ALPHA} connected`);
    } else {
      rovers[1].isLive = false;
      setHealth(1, 'offline', `VIN ${VIN_ALPHA} offline`);
      const badge = document.getElementById('mode-1');
      badge.textContent = 'OFFLINE';
      badge.className = 'mode-badge';
    }
  }

  if (topic.includes('/fota/progress')) {
    logFota(data.status || JSON.stringify(data), 'bootload');
  }
}

function setMqttStatus(cls, text) {
  const el = document.getElementById('mqtt-status');
  el.className = `mqtt-status ${cls}`;
  el.querySelector('.status-text').textContent = text;
}

// ── FOTA log ──────────────────────────────────────────────────────────────────
function logFota(msg, cls) {
  const log = document.getElementById('fota-log');
  const ph  = log.querySelector('.log-placeholder');
  if (ph) ph.remove();

  const ts = new Date().toTimeString().slice(0, 8);
  const line = document.createElement('div');
  if (cls === 'error') line.classList.add('log-line-error');
  line.innerHTML = `<span class="log-ts">[${ts}]</span><span class="${cls ? 'log-' + cls : ''}">${esc(msg)}</span>`;
  log.appendChild(line);
  log.scrollTop = log.scrollHeight;
}

function toggleDiagnostics(show) {
  document.getElementById('fota-log').classList.toggle('hide-errors', !show);
}

function esc(s) {
  return String(s).replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;');
}

// ── CI/CD pipeline ────────────────────────────────────────────────────────────
const N_STAGES = 6;

function resetPipeline() {
  for (let i = 0; i < N_STAGES; i++) {
    document.getElementById(`stage-${i}`).className = 'pipeline-stage';
    setText(`ss-${i}`, '');
  }
}

function setStage(i, status) {
  document.getElementById(`stage-${i}`).className = `pipeline-stage ${status}`;
  const sym = { active: '⏳', done: '✓', failed: '✗' };
  setText(`ss-${i}`, sym[status] || '');
}

function runPipeline(uploadOk, bootOk) {
  resetPipeline();
  const delay = ms => new Promise(r => setTimeout(r, ms));
  (async () => {
    for (let i = 0; i < 3; i++) {
      setStage(i, 'active'); await delay(380); setStage(i, 'done');
    }
    setStage(3, 'active'); await delay(uploadOk ? 700 : 350);
    setStage(3, uploadOk ? 'done' : 'failed');
    if (!uploadOk) return;
    setStage(4, 'active'); await delay(bootOk ? 1000 : 350);
    setStage(4, bootOk ? 'done' : 'failed');
    if (!bootOk) return;
    await delay(500); setStage(5, 'active'); await delay(700); setStage(5, 'done');
  })();
}

// ── Manual version entry (fallback when GitLab listing is unavailable) ────────
function enableManualVersionEntry(n) {
  const wrap = document.getElementById(`version-row-${n}`);
  if (!wrap) return;
  const roverName = n === 1 ? 'Alpha' : 'Beta';
  wrap.innerHTML = `
    <input id="version-input-${n}" class="ctrl-select card-fota-select" type="text"
           placeholder="e.g. 1.0.42" style="flex:1"
           oninput="syncManualVersion(this.value, ${n})" />
    <button class="ctrl-refresh-btn" onclick="loadReleases()" title="Retry GitLab">↻</button>`;
  const hidden = document.createElement('select');
  hidden.id    = `version-select-${n}`;
  hidden.style.display = 'none';
  wrap.appendChild(hidden);
  logFota(`Rover ${roverName}: enter version manually (e.g. 1.0.42) — listing unavailable.`, 'error');
}

function syncManualVersion(val, n) {
  const sel = document.getElementById(`version-select-${n}`);
  if (!sel) return;
  sel.innerHTML = '';
  if (val.trim()) {
    const opt = document.createElement('option');
    opt.value = JSON.stringify({
      source: 'gitlab', version: val.trim(),
      package_name: 'smartwheels-s32k144-sm-rover'
    });
    opt.selected = true;
    sel.appendChild(opt);
  }
}

// ── FOTA trigger ──────────────────────────────────────────────────────────────
const fotaBusy = { 1: false, 2: false };

async function triggerFota(n) {
  if (fotaBusy[n]) return;

  const verSel  = document.getElementById(`version-select-${n}`);
  const verRaw  = verSel.value;
  if (!verRaw) { logFota('No firmware version selected.', 'error'); return; }

  const release    = JSON.parse(verRaw);
  const targetVin  = n === 1 ? VIN_ALPHA : VIN_BETA;
  const roverName  = n === 1 ? 'Alpha' : 'Beta';
  const btn        = document.getElementById(`fota-btn-${n}`);
  const progWrap   = document.getElementById('fota-progress-wrap');
  const progBar    = document.getElementById('fota-bar');
  const progLbl    = document.getElementById('fota-progress-lbl');

  fotaBusy[n] = true;
  btn.disabled = true;
  progWrap.style.display = 'block';
  progBar.style.width = '5%';
  progLbl.textContent = `Starting FOTA → Rover ${roverName}…`;
  logFota(`── Rover ${roverName} | VIN: ${targetVin}  |  Version: ${release.version} ──`, '');

  // Route: GitLab release → /api/deploy; local file → /api/fota/trigger (legacy)
  let ok = false;
  try {
    let resp;
    if (release.source === 'gitlab') {
      resp = await fetch(API_BASE + '/api/deploy', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          vin:          targetVin,
          package_name: release.package_name || 'smartwheels-s32k144-sm-rover',
          version:      release.version
        })
      });
    } else {
      resp = await fetch(API_BASE + '/api/fota/trigger', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ srec_path: release.path })
      });
    }
    const json = await resp.json();
    if (json.error) throw new Error(json.error);
    ok = true;
  } catch (err) {
    logFota(`Cannot reach server.py: ${err.message}`, 'error');
    logFota('Run:  python server.py  in the webpage/ folder, then retry.', 'error');
    progLbl.textContent = 'Error — server.py not running';
    fotaBusy[n] = false;
    btn.disabled = false;
    return;
  }

  // Animate stages 0-2 immediately (git push / build / sign already done in CI)
  resetPipeline();
  for (let i = 0; i < 3; i++) {
    setTimeout(() => { setStage(i, 'active'); setTimeout(() => setStage(i, 'done'), 360); }, i * 400);
  }

  let uploadDone = false, bootDone = false;

  const es = new EventSource(API_BASE + '/api/fota/stream');

  es.onmessage = (e) => {
    const d = JSON.parse(e.data);
    const { stage, msg } = d;

    if (stage === 'done') {
      es.close();
      fotaBusy[n] = false;
      btn.disabled = false;
      progBar.style.width = '100%';
      progLbl.textContent = `Rover ${roverName} — Complete ✓`;
      runPipeline(uploadDone, bootDone);
      return;
    }

    logFota(msg, stage);

    if (stage === 'upload') {
      progBar.style.width = '25%';
      progLbl.textContent = 'Uploading firmware chunks to MQTT…';
      setStage(3, 'active');
      uploadDone = true;
    }
    if (stage === 'bootload') {
      progBar.style.width = '60%';
      progLbl.textContent = 'Bootloading S32K144 via UART…';
      setStage(3, 'done');
      setStage(4, 'active');
      bootDone = true;
    }
    if (stage === 'complete') {
      progBar.style.width = '88%';
      progLbl.textContent = 'Verifying VIN ACK…';
      setStage(4, 'done');
      setStage(5, 'active');
    }
    if (stage === 'error') {
      es.close();
      fotaBusy[n] = false;
      btn.disabled = false;
      progLbl.textContent = `Error: ${msg}`;
      runPipeline(uploadDone, false);
    }
  };

  es.onerror = () => {
    if (fotaBusy[n]) {
      es.close();
      logFota('SSE stream closed', 'error');
      fotaBusy[n] = false;
      btn.disabled = false;
    }
  };
}

// ── Firmware release list (GitLab Package Registry or local fallback) ─────────
async function loadReleases() {
  const sels = [
    document.getElementById('version-select-1'),
    document.getElementById('version-select-2')
  ];
  sels.forEach(s => { s.innerHTML = '<option value="">Loading…</option>'; });

  let resp;
  try {
    resp = await fetch(API_BASE + '/api/releases');
  } catch (_) {
    sels.forEach(s => { s.innerHTML = '<option value="">server.py not running</option>'; });
    logFota('Cannot reach server.py — open http://localhost:5000 (not file://)', 'error');
    logFota('Then run:  python webpage/server.py', 'error');
    return;
  }

  try {
    const data = await resp.json();

    if (data.error) {
      logFota(`GitLab API error: ${data.error}`, 'error');
      if (data.url_tried) logFota(`URL tried: ${data.url_tried}`, 'error');
      if (data.hint)      logFota(`Hint: ${data.hint}`, 'error');
      if (data.manual_entry) {
        enableManualVersionEntry(1);
        enableManualVersionEntry(2);
      } else {
        sels.forEach(s => { s.innerHTML = '<option value="">GitLab error — see log</option>'; });
      }
      return;
    }

    const list = data.releases || [];

    if (list.length === 0) {
      const msg = `<option value="">No releases found${data.source === 'local' ? ' in temp/' : ''}</option>`;
      sels.forEach(s => { s.innerHTML = msg; });
      if (data.warning) logFota(data.warning, 'error');
      return;
    }

    sels.forEach(sel => {
      sel.innerHTML = '';
      list.forEach(r => {
        const opt = document.createElement('option');
        opt.value = JSON.stringify(r);
        if (r.source === 'local') {
          opt.textContent = `${r.version}  (${(r.size / 1024).toFixed(1)} KB)  [local]`;
        } else {
          const date = r.created_at ? r.created_at.slice(0, 10) : '';
          opt.textContent = `${r.version}  ·  ${date}  [GitLab]`;
        }
        sel.appendChild(opt);
      });
    });

    if (data.warning) logFota(data.warning, 'error');
  } catch (e) {
    sels.forEach(s => { s.innerHTML = '<option value="">Parse error — see log</option>'; });
    logFota(`Unexpected response from server.py: ${e.message}`, 'error');
  }
}

// ── Init ──────────────────────────────────────────────────────────────────────
function init() {
  initTicks(1);
  initTicks(2);

  const now = Date.now();

  // Rover Alpha: wait for real ESP32 data — no sim, no fake "online" state
  rovers[1].sim.spd.t = now;
  rovers[1].sim.dst.t = now;
  rovers[1].lastVinBeat = 0;           // 0 = never received anything
  setHealth(1, 'warn', `Waiting for VIN ${VIN_ALPHA}…`);
  setText('vin-beat-1', 'waiting for MQTT…');

  // Rover Beta: demo mode — sim runs, health starts online
  rovers[2].sim.spd.t = now;
  rovers[2].sim.dst.t = now;
  rovers[2].lastVinBeat = now;
  setHealth(2, 'online', 'Demo mode');

  applyView(1);
  applyView(2);
  animLoop();
  setInterval(monitorHealth, 2000);
  connectMqtt();
  loadReleases();
}

document.addEventListener('DOMContentLoaded', init);
