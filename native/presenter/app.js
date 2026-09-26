// Presentation transport only. Native descriptors own actions and native pixels.
const screen = document.querySelector('#frame');
const deck = document.querySelector('#controls');
const housing = document.querySelector('#housing');
const statusLine = document.querySelector('#status');
const retryButton = document.querySelector('#retry');
const confirmation = document.querySelector('#reset-confirmation');
const deviceNames = ['lab', 'probe', 'companion'];
const savedDevice = localStorage.getItem('critterDevice');
let device = deviceNames.includes(savedDevice) ? savedDevice : 'lab';
let snapshot = null;
let ready = false;
let busy = false;
let generation = 0;
let pending = null;
let presented = null;
let pointerGesture = null;
let keyboardGesture = null;
let blockedKeyboardRelease = false;
let focusAfterUpdate = null;
const heldKeys = new Set();

function setStatus(text) {
  statusLine.textContent = text;
}
function cancelGestures() {
  pointerGesture = null;
  keyboardGesture = null;
}
function invalidatePresentation() {
  ready = false;
  if (heldKeys.size) blockedKeyboardRelease = true;
  cancelGestures();
  lockControls();
}
function lockControls() {
  const unavailable = !ready || busy || Boolean(pending);
  const confirming = !confirmation.hidden;
  document.querySelectorAll('[data-command]').forEach(button => {
    button.disabled = unavailable || confirming;
  });
  deck.setAttribute('aria-busy', String(unavailable));
  for (const id of deviceNames) document.querySelector(`#${id}`).disabled = unavailable || confirming;
  for (const id of ['refresh', 'reset-sandbox']) document.querySelector(`#${id}`).disabled = unavailable || confirming;
  for (const id of ['confirm-reset', 'cancel-reset']) document.querySelector(`#${id}`).disabled = unavailable;
  document.querySelector('#cancel-reset').disabled = busy || Boolean(pending);
  // Refresh is the recovery path after a failed frame request.
  document.querySelector('#refresh').disabled = busy || Boolean(pending) || confirming;
}
function samePresentation(identity) {
  return ready && !busy && !pending && confirmation.hidden && presented
    && identity.generation === generation && identity.device === presented.device
    && identity.revision === presented.revision;
}
function bindingAvailable(binding) {
  return samePresentation(binding.identity) && snapshot.actions.some(action =>
    action.name === binding.name && action.device === binding.actionDevice
    && (action.device === binding.identity.device
      || (action.device === 'engineering' && binding.identity.device !== 'companion')));
}
function rememberFocus(name = null) {
  const focused = document.activeElement;
  if (name || deck.contains(focused)) {
    focusAfterUpdate = { name: name || focused.dataset.command || null };
  }
}
function restoreFocus() {
  if (!focusAfterUpdate || !ready || busy || pending) return;
  const target = Array.from(deck.querySelectorAll('button')).find(button =>
    button.dataset.command === focusAfterUpdate.name && !button.disabled);
  (target || (deck.childElementCount ? deck : housing)).focus({preventScroll: true});
  focusAfterUpdate = null;
}
function bindAction(action, identity) {
  const button = document.createElement('button');
  button.textContent = action.label;
  button.dataset.command = action.name;
  const binding = {name: action.name, actionDevice: action.device, identity};
  button.addEventListener('pointerdown', event => {
    if (event.button !== 0 || !bindingAvailable(binding)) return;
    pointerGesture = {button, binding, pointerId: event.pointerId, released: false};
    keyboardGesture = null;
  });
  button.addEventListener('pointerup', event => {
    if (!pointerGesture || pointerGesture.button !== button
        || pointerGesture.pointerId !== event.pointerId || !bindingAvailable(binding)) {
      pointerGesture = null;
      return;
    }
    pointerGesture.released = true;
  });
  for (const type of ['pointerleave', 'pointercancel']) {
    button.addEventListener(type, () => { pointerGesture = null; });
  }
  button.addEventListener('lostpointercapture', () => {
    if (pointerGesture && !pointerGesture.released) pointerGesture = null;
  });
  button.addEventListener('keydown', event => {
    if (!['Enter', ' '].includes(event.key)) return;
    if (event.repeat || heldKeys.has(event.key) || !bindingAvailable(binding)) {
      event.preventDefault();
      return;
    }
    heldKeys.add(event.key);
    blockedKeyboardRelease = false;
    keyboardGesture = {button, binding, key: event.key};
    pointerGesture = null;
  });
  button.addEventListener('click', event => {
    const pointerValid = pointerGesture && pointerGesture.button === button
      && pointerGesture.released && pointerGesture.binding === binding;
    const keyboardValid = keyboardGesture && keyboardGesture.button === button
      && keyboardGesture.binding === binding;
    // detail=0 without a held key also supports assistive-technology activation.
    const accessibleClick = event.detail === 0 && !heldKeys.size && !keyboardGesture && !blockedKeyboardRelease;
    if (!bindingAvailable(binding) || !(event.detail > 0 ? pointerValid : keyboardValid || accessibleClick)) {
      cancelGestures();
      return;
    }
    cancelGestures();
    button.classList.add('acknowledged');
    rememberFocus(action.device === identity.device ? action.name : null);
    activate(action.name, binding);
  });
  return button;
}
async function show(state) {
  const identity = {generation: ++generation, device, revision: state.revision};
  invalidatePresentation();
  const response = await fetch(`/api/frame?device=${identity.device}&revision=${identity.revision}`, {cache: 'no-store'});
  if (!response.ok) throw new Error('Saved state changed. Refresh to load its screen.');
  const url = URL.createObjectURL(await response.blob());
  const image = new Image();
  image.src = url;
  try {
    await image.decode();
    if (identity.generation !== generation) return false;
    const old = screen.src;
    screen.src = url;
    // The already decoded resource is installed before exposing its bindings.
    const sizes = {lab: [1024, 600], probe: [122, 250], companion: [368, 448]};
    [screen.width, screen.height] = sizes[identity.device];
    document.querySelector('#instrument').className = `instrument ${identity.device}`;
    for (const id of deviceNames) document.querySelector(`#${id}`).setAttribute('aria-pressed', String(id === identity.device));
    snapshot = state;
    presented = identity;
    deck.replaceChildren(...state.actions.filter(action => action.device === identity.device).map(action => bindAction(action, identity)));
    document.querySelector('#engineering').replaceChildren(...state.actions.filter(action =>
      action.device === 'engineering' && identity.device !== 'companion').map(action => bindAction(action, identity)));
    deck.setAttribute('aria-label', `${identity.device === 'lab' ? 'Lab' : identity.device === 'probe' ? 'Probe' : 'Companion'} controls`);
    document.querySelector('#mapping-note').hidden = !deck.childElementCount;
    document.querySelector('#complete').hidden = !state.finding;
    ready = true;
    if (old.startsWith('blob:')) URL.revokeObjectURL(old);
    return true;
  } finally {
    if (identity.generation !== generation || screen.src !== url) URL.revokeObjectURL(url);
  }
}
async function refresh() {
  if (busy || pending || !confirmation.hidden) return;
  rememberFocus();
  busy = true;
  invalidatePresentation();
  setStatus('Updating the device…');
  try {
    const response = await fetch('/api/status', {cache: 'no-store'});
    const value = await response.json();
    if (!response.ok) throw new Error(value.error);
    if (await show(value)) setStatus('Device updated.');
  } catch (error) {
    setStatus(error.message);
  } finally {
    busy = false;
    lockControls();
    restoreFocus();
  }
}
async function sendPending() {
  if (busy || !pending) return;
  busy = true;
  invalidatePresentation();
  retryButton.hidden = true;
  setStatus('Updating the device…');
  try {
    const response = await fetch('/api/command', {
      method: 'POST',
      headers: {'Content-Type': 'application/json', 'X-Requested-With': 'CritterLab'},
      body: JSON.stringify(pending)
    });
    const value = await response.json();
    if (!response.ok) {
      if (response.status < 500) {
        pending = null;
        sessionStorage.removeItem('critterPending');
      }
      throw new Error(value.error);
    }
    const resetCompleted = pending.name === 'reset';
    pending = null;
    sessionStorage.removeItem('critterPending');
    if (resetCompleted) {
      device = 'lab';
      localStorage.setItem('critterDevice', device);
      confirmation.hidden = true;
      focusAfterUpdate = {name: null};
    }
    if (await show(value)) setStatus('Device updated.');
  } catch (error) {
    setStatus(error.message + (pending ? ' Retry keeps the same action.' : ' Refresh to continue.'));
  } finally {
    busy = false;
    retryButton.hidden = !pending;
    lockControls();
    restoreFocus();
  }
}
function activate(name, binding = null) {
  if (!ready || busy || pending) return;
  if (binding ? !bindingAvailable(binding) : name !== 'reset' || confirmation.hidden) return;
  const random = new Uint32Array(4);
  crypto.getRandomValues(random);
  pending = {
    name,
    revision: snapshot.revision,
    operation_id: Array.from(random, value => value.toString(16).padStart(8, '0')).join('')
  };
  sessionStorage.setItem('critterPending', JSON.stringify(pending));
  sendPending();
}
for (const id of deviceNames) {
  document.querySelector(`#${id}`).addEventListener('click', async () => {
    if (!ready || busy || pending || !confirmation.hidden || id === device) return;
    busy = true;
    device = id;
    localStorage.setItem('critterDevice', device);
    focusAfterUpdate = null;
    invalidatePresentation();
    setStatus('Updating the device…');
    try {
      if (await show(snapshot)) setStatus('Device updated.');
    } catch (error) {
      setStatus(error.message);
    } finally {
      busy = false;
      lockControls();
      if (ready) document.querySelector(`#${id}`).focus({preventScroll: true});
    }
  });
}
deck.addEventListener('keydown', event => {
  const arrows = presented && presented.device === 'probe'
    ? ['ArrowLeft', 'ArrowRight', 'ArrowUp', 'ArrowDown'] : ['ArrowLeft', 'ArrowRight'];
  if (!arrows.includes(event.key) || !samePresentation(presented)) return;
  const buttons = Array.from(deck.querySelectorAll('button:not(:disabled)'));
  const index = buttons.indexOf(document.activeElement);
  if (index < 0) return;
  event.preventDefault();
  const step = ['ArrowRight', 'ArrowDown'].includes(event.key) ? 1 : -1;
  buttons[Math.max(0, Math.min(buttons.length - 1, index + step))].focus();
});
document.addEventListener('keyup', event => {
  heldKeys.delete(event.key);
  // Space's native click follows keyup; retain its token for that default action.
  setTimeout(() => {
    if (keyboardGesture && keyboardGesture.key === event.key) keyboardGesture = null;
    if (['Enter', ' '].includes(event.key)) blockedKeyboardRelease = false;
  }, 0);
});
window.addEventListener('blur', () => {
  if (heldKeys.size) blockedKeyboardRelease = true;
  cancelGestures(); heldKeys.clear();
});
document.addEventListener('visibilitychange', () => {
  if (document.hidden) {
    if (heldKeys.size) blockedKeyboardRelease = true;
    cancelGestures(); heldKeys.clear();
  }
});
document.querySelector('#refresh').addEventListener('click', refresh);
document.querySelector('#reset-sandbox').addEventListener('click', () => {
  if (!ready || busy || pending) return;
  if (heldKeys.size) blockedKeyboardRelease = true;
  cancelGestures();
  confirmation.hidden = false;
  lockControls();
  document.querySelector('#confirm-reset').focus();
});
document.querySelector('#cancel-reset').addEventListener('click', () => {
  confirmation.hidden = true;
  lockControls();
  document.querySelector('#reset-sandbox').focus();
});
document.querySelector('#confirm-reset').addEventListener('click', () => activate('reset'));
retryButton.addEventListener('click', sendPending);
try {
  pending = JSON.parse(sessionStorage.getItem('critterPending'));
} catch {
  sessionStorage.removeItem('critterPending');
}
lockControls();
if (pending) {
  retryButton.hidden = false;
  setStatus('An action may still need confirmation. Retry it safely.');
} else {
  refresh();
}

async function showRelease() {
  const label = document.querySelector('#release');
  try {
    const response = await fetch('/api/release', {cache: 'no-store'});
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
    // Missing metadata leaves the honest development label intact.
  }
}
showRelease();
