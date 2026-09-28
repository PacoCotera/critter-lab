// Fixed physical-actuator transport only. Page/focus decisions and pixels live in C.
const image = document.querySelector('#frame');
const status = document.querySelector('#status');
const knob = document.querySelector('#knob');
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
  document.querySelectorAll('button').forEach(button => button.classList.remove('held'));
  dial = null;
  status.textContent = 'Transport interrupted; activation stopped. Reload to reconnect with a fresh gesture.';
  // A down may have reached C even when its response was lost. Never send a queued up.
  try {
    const response = await fetch('/api/input', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ event: 'cancel', revision: visibleRevision }) });
    if (!response.ok) return;
    await response.json();
  } catch {
    // Remain blocked even when native cancellation cannot be confirmed.
  }
}

function send(event, requestedFrame = visibleRevision, delta) {
  if (inputBlocked) return;
  const generation = transportGeneration;
  commands = commands.then(async () => {
    if (inputBlocked || generation !== transportGeneration) return;
    const response = await fetch('/api/input', { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ event, revision: requestedFrame, ...(delta === undefined ? {} : { delta }) }) });
    if (!response.ok) throw new Error('Native input transport unavailable.');
    const state = await response.json();
    if (generation === transportGeneration && !inputBlocked) receive(state);
  }).catch(() => generation === transportGeneration ? stopAfterTransportFailure() : undefined);
}

function receive(state) {
  revision = state.revision;
  status.textContent = `Native page: ${state.page} · focus: ${state.focus} · ${state.ready ? 'frame ready' : 'waiting for frame'} · ${state.boundary}`;
  if (visibleRevision !== revision && requestedRevision !== revision) draw(revision);
}

async function draw(frame) {
  requestedRevision = frame;
  const generation = ++drawGeneration;
  let url;
  try {
    const response = await fetch(`/api/frame?revision=${frame}`);
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
for (const [name, button] of [['confirm', document.querySelector('#confirm')], ['back', document.querySelector('#back')]]) {
  button.addEventListener('pointerdown', event => {
    if (inputBlocked || event.button !== 0 || held.has(name)) return;
    event.preventDefault();
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
    send(inside ? `${name}-up` : 'cancel', gesture.frame);
  });
  for (const type of ['pointercancel', 'lostpointercapture']) button.addEventListener(type, () => {
    if (held.delete(name)) { button.classList.remove('held'); send('cancel'); }
  });
  button.addEventListener('keydown', event => event.preventDefault());
}

let dial;
let degrees = 0;
function angle(event) {
  const bounds = knob.getBoundingClientRect();
  return Math.atan2(event.clientY - bounds.top - bounds.height / 2, event.clientX - bounds.left - bounds.width / 2) * 180 / Math.PI;
}
function rotate(delta) {
  if (inputBlocked) return;
  degrees += delta * 24;
  knob.querySelector('span').style.transform = `rotate(${degrees}deg)`;
  send('rotate', visibleRevision, delta);
}
knob.addEventListener('pointerdown', event => {
  if (inputBlocked || event.button !== 0 || dial) return;
  event.preventDefault();
  knob.setPointerCapture(event.pointerId);
  dial = { pointer: event.pointerId, previous: angle(event), accumulated: 0 };
});
knob.addEventListener('pointermove', event => {
  if (dial?.pointer !== event.pointerId) return;
  const current = angle(event);
  let difference = current - dial.previous;
  if (difference > 180) difference -= 360;
  if (difference < -180) difference += 360;
  dial.previous = current;
  dial.accumulated += difference;
  while (Math.abs(dial.accumulated) >= 24) {
    const delta = Math.sign(dial.accumulated);
    dial.accumulated -= delta * 24;
    rotate(delta);
  }
});
for (const type of ['pointerup', 'pointercancel', 'lostpointercapture']) knob.addEventListener(type, () => { dial = null; });
knob.addEventListener('wheel', event => { event.preventDefault(); if (event.deltaY) rotate(Math.sign(event.deltaY)); }, { passive: false });

function suspend() {
  held.clear();
  document.querySelectorAll('button').forEach(button => button.classList.remove('held'));
  dial = null;
  send('suspend');
}
function resume() { if (!document.hidden) send('resume'); }
window.addEventListener('blur', suspend);
window.addEventListener('focus', resume);
document.addEventListener('visibilitychange', () => document.hidden ? suspend() : resume());
try {
  const response = await fetch('/api/status');
  if (!response.ok) throw new Error('Native C process unavailable.');
  receive(await response.json());
  if (!document.hidden) send('resume');
} catch { await stopAfterTransportFailure(); }
