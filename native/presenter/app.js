async function connectDevice(deviceId, controls) {
  const buttons = controls.map(name => document.querySelector(`#${deviceId}-${name}`));
  // Fixed physical-actuator transport only. Page/focus decisions and pixels live in C.
  const image = document.querySelector(`#${deviceId}-frame`);
  const status = document.querySelector(`#${deviceId}-status`);
  let revision = 0;
  let visibleRevision = 0;
  let drawGeneration = 0;
  let currentBlob;
  let requestedRevision = 0;
  let commands = Promise.resolve();
  let transportGeneration = 0;
  let inputBlocked = false;

  async function stopAfterTransportFailure() {
    if (inputBlocked) return;
    inputBlocked = true;
    ++transportGeneration;
    ++drawGeneration;
    held.clear();
    buttons.forEach(button => button.classList.remove('held'));
    status.textContent = 'Transport interrupted; activation stopped. Reload to reconnect with a fresh gesture.';
    // A down may have reached C even when its response was lost. Never send a queued up.
    try {
      const response = await fetch('/api/device-input', { method: 'POST', headers: { 'Content-Type': 'application/json', 'X-Requested-With': 'CritterLab' }, body: JSON.stringify({ device: deviceId, event: 'cancel', revision: visibleRevision }) });
      if (!response.ok) return;
      await response.json();
    } catch {
      // Remain blocked even when native cancellation cannot be confirmed.
    }
  }

  function send(event, requestedFrame = visibleRevision) {
    if (inputBlocked) return;
    const generation = transportGeneration;
    commands = commands.then(async () => {
      if (inputBlocked || generation !== transportGeneration) return;
      const response = await fetch('/api/device-input', { method: 'POST', headers: { 'Content-Type': 'application/json', 'X-Requested-With': 'CritterLab' }, body: JSON.stringify({ device: deviceId, event, revision: requestedFrame }) });
      if (!response.ok) throw new Error('Native input transport unavailable.');
      const state = await response.json();
      if (generation === transportGeneration && !inputBlocked) receive(state);
    }).catch(() => generation === transportGeneration ? stopAfterTransportFailure() : undefined);
  }

  function receive(state) {
    revision = state.revision;
    (deviceId === 'lab' ? ['research', 'critters', 'library', 'habitat'] : []).forEach((name, index) => {
      document.querySelector(`#${deviceId}-${name}`).setAttribute('aria-pressed', String(state.workspace === index));
    });
    status.textContent = `${state.focus} · ${state.transfer}`;
    const link = document.querySelector(`#${deviceId}-link`);
    if (link) link.checked = state.online;
    if (visibleRevision !== revision && requestedRevision !== revision) draw(revision);
  }

  async function draw(frame) {
    requestedRevision = frame;
    const generation = ++drawGeneration;
    let url;
    try {
      const response = await fetch(`/api/devices/${deviceId}/frame?revision=${frame}`);
      if (response.status === 409) return; // A newer native frame superseded this request.
      if (!response.ok) throw new Error('Native frame unavailable.');
      url = URL.createObjectURL(await response.blob());
      const decoded = new Image();
      decoded.src = url;
      await decoded.decode();
      if (generation !== drawGeneration || frame !== revision) return;
      image.src = url;
      await image.decode();
      if (generation !== drawGeneration || frame !== revision) return;
      const previous = currentBlob;
      currentBlob = url;
      url = undefined;
      if (previous) URL.revokeObjectURL(previous);
      requestAnimationFrame(() => requestAnimationFrame(() => {
        if (generation === drawGeneration && frame === revision && !document.hidden) {
          visibleRevision = frame;
          send('ready', frame);
        }
      }));
    } catch {
      if (generation === drawGeneration) await stopAfterTransportFailure();
    } finally {
      if (url) URL.revokeObjectURL(url);
    }
  }

  const held = new Map();
  for (const name of controls) {
    const button = document.querySelector(`#${deviceId}-${name}`);
    button.addEventListener('pointerdown', event => {
      if (inputBlocked || event.button !== 0 || held.has(name)) return;
      event.preventDefault();
      if (held.size) {
        for (const gesture of held.values()) gesture.cancelled = true;
        held.set(name, { pointer: event.pointerId, frame: visibleRevision, cancelled: true });
        button.setPointerCapture(event.pointerId);
        buttons.forEach(control => control.classList.remove('held'));
        send('cancel');
        return;
      }
      held.set(name, { pointer: event.pointerId, frame: visibleRevision });
      button.setPointerCapture(event.pointerId);
      button.classList.add('held');
      send(`${name}-down`, visibleRevision);
    });
    button.addEventListener('pointerup', event => {
      const gesture = held.get(name);
      if (gesture?.pointer !== event.pointerId) return;
      held.delete(name);
      button.classList.remove('held');
      const bounds = button.getBoundingClientRect();
      const inside = event.clientX >= bounds.left && event.clientX <= bounds.right && event.clientY >= bounds.top && event.clientY <= bounds.bottom;
      if (!gesture.cancelled) send(inside ? `${name}-up` : 'cancel', gesture.frame);
    });
    for (const type of ['pointercancel', 'lostpointercapture']) button.addEventListener(type, () => {
      if (held.delete(name)) {
        for (const gesture of held.values()) gesture.cancelled = true;
        buttons.forEach(control => control.classList.remove('held'));
        send('cancel');
      }
    });
    button.addEventListener('keydown', event => event.preventDefault());
  }

  function suspend() {
    held.clear();
    buttons.forEach(button => button.classList.remove('held'));
    send('suspend');
  }
  function resume() { if (!document.hidden) send('resume'); }
  window.addEventListener('blur', suspend);
  window.addEventListener('focus', resume);
  document.addEventListener('visibilitychange', () => document.hidden ? suspend() : resume());
  try {
    const response = await fetch(`/api/devices/${deviceId}/status`);
    if (!response.ok) throw new Error('Native C process unavailable.');
    receive(await response.json());
    if (!document.hidden) send('resume');
  } catch { await stopAfterTransportFailure(); }



  // Poll native time-driven state; timing and gameplay stay in C.
  const pollTimer = setInterval(() => {
    if (inputBlocked || document.hidden || held.size) return;
    commands = commands.then(async () => {
      const response = await fetch(`/api/devices/${deviceId}/status`);
      if (!response.ok) throw new Error('Native device unavailable');
      receive(await response.json());
    }).catch(stopAfterTransportFailure);
  }, 1000);

  // Node transport tests should not be held open by the browser polling timer.
  pollTimer.unref?.();
}

const profiles = {
  lab: ['up','down','left','right','research','critters','library','habitat','confirm','back'],
  companion: ['up','down','left','right','confirm','back'],
  dock: ['up','down','confirm','research','critters']
};
await Promise.all(Object.entries(profiles).map(([device, buttons]) => connectDevice(device, buttons)));
for (const device of ['companion','dock']) {
  document.querySelector(`#${device}-link`).addEventListener('change', async event => {
    const label = document.querySelector('#link-status');
    try {
      const response = await fetch('/api/link', {method:'POST',headers:{'Content-Type':'application/json','X-Requested-With':'CritterLab'},body:JSON.stringify({device,online:event.target.checked})});
      if (!response.ok) throw new Error();
      const state = await response.json();
      event.target.checked = state.online;
      label.textContent = `${device}: ${state.online ? 'link available' : 'link interrupted'} (simulated).`;
    } catch { label.textContent = 'Simulation link control unavailable. Reload to verify state.'; }
  });
}
async function showRelease() {
  const label = document.querySelector('#release');
  try {
    const response = await fetch('/api/release', { cache: 'no-store' });
    if (!response.ok) return;
    const release = await response.json();
    if (!/^[0-9a-f]{40}$/.test(release.commit || '') || !release.deployed_at) return;
    const timestamp = new Date(release.deployed_at);
    if (!Number.isFinite(timestamp.getTime())) return;
    const formatted = new Intl.DateTimeFormat('en-GB', {
      timeZone: 'America/Mexico_City', day: '2-digit', month: 'short', year: 'numeric',
      hour: '2-digit', minute: '2-digit', second: '2-digit', hourCycle: 'h23'
    }).format(timestamp);
    label.textContent = `Release ${release.commit.slice(0, 7)} | ${formatted} Mexico City`;
  } catch {
    // Missing metadata retains the honest development label.
  }
}
showRelease();
