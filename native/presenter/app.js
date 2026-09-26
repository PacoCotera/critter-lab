// Presentation transport only. Native descriptors supply every playable action.
const screen = document.querySelector('#frame');
const statusLine = document.querySelector('#status');
const sizeButton = document.querySelector('#native-size');
let nativeSize = false;
function updateDisplaySize() {
  document.querySelector('#instrument').classList.toggle('native-size', nativeSize);
  sizeButton.setAttribute('aria-pressed', String(nativeSize));
  sizeButton.textContent = nativeSize ? 'Fit complete screen' : '1:1 native pixels';
  document.querySelector('#viewport-note').textContent = nativeSize
    ? 'Native pixels shown at 1:1. Scroll inside the screen if needed.'
    : 'The complete screen fits this view. Use 1:1 to inspect native pixels.';
}
sizeButton.addEventListener('click', () => {
  nativeSize = !nativeSize;
  updateDisplaySize();
});
const retryButton = document.querySelector('#retry');
let device = ['lab', 'probe', 'companion'].includes(localStorage.getItem('critterDevice')) ? localStorage.getItem('critterDevice') : 'lab', snapshot = null, ready = false, busy = false, generation = 0, pending = null;
let held = false;
const setStatus = text => { statusLine.textContent = text; };
function lockControls() {
  document.querySelectorAll('[data-command]').forEach(button => { button.disabled = !ready || busy || Boolean(pending); });
}
async function show(state) {
  const request = ++generation;
  snapshot = state; ready = false; lockControls();
  const response = await fetch(`/api/frame?device=${device}&revision=${state.revision}`, {cache:'no-store'});
  if (!response.ok) throw new Error('Saved state changed. Refresh to load its screen.');
  const url = URL.createObjectURL(await response.blob());
  const image = new Image(); image.src = url;
  try { await image.decode(); } catch (error) { URL.revokeObjectURL(url); throw error; }
  if (request !== generation) { URL.revokeObjectURL(url); return; }
  const old = screen.src; screen.src = url;
  await screen.decode();
  if (old.startsWith('blob:')) URL.revokeObjectURL(old);
  if (request !== generation) return;
  document.querySelector('#instrument').className = 'instrument ' + device;
  updateDisplaySize();
  screen.width = device === 'probe' ? 122 : device === 'companion' ? 368 : 1024;
  screen.height = device === 'probe' ? 250 : device === 'companion' ? 448 : 600;
  for (const id of ['lab', 'probe', 'companion']) document.querySelector(`#${id}`).setAttribute('aria-pressed', String(id === device));
  for (const id of ['controls', 'engineering']) document.querySelector(`#${id}`).replaceChildren();
  state.actions.filter(action => action.device === device || (action.device === 'engineering' && device !== 'companion')).forEach(action => {
    const button = document.createElement('button'); button.textContent = action.label; button.dataset.command = action.name;
    button.addEventListener('click', () => activate(action.name));
    document.querySelector(action.device === 'engineering' ? '#engineering' : '#controls').append(button);
  });
  ready = true; lockControls();
  document.querySelector('#complete').hidden = !state.finding;
  if (!busy) setStatus('Screen ready. Changes are saved after each action.');
}
async function refresh() {
  if (busy || pending) return;
  busy = true; ready = false; lockControls();
  try { const response = await fetch('/api/status', {cache:'no-store'}); const value = await response.json(); if(!response.ok) throw new Error(value.error); await show(value); }
  catch(error) {setStatus(error.message);}
  finally {busy = false;lockControls();if(ready) setStatus('Screen ready. Changes are saved after each action.');}
}
async function sendPending() {
  if (busy || !pending) return;
  busy = true; ready = false; retryButton.hidden = true; lockControls(); setStatus('Saving action...');
  try {
    const response = await fetch('/api/command', {method:'POST',headers:{'Content-Type':'application/json','X-Requested-With':'CritterLab'},body:JSON.stringify(pending)});
    const value = await response.json();
    if(!response.ok) {
      if(response.status < 500) {pending = null; sessionStorage.removeItem('critterPending');}
      throw new Error(value.error);
    }
    pending = null; sessionStorage.removeItem('critterPending'); await show(value);
  } catch(error) {setStatus(error.message + (pending ? ' Retry keeps the same action.' : ' Refresh to continue.'));}
  finally {busy = false;retryButton.hidden = !pending;lockControls();if(ready) setStatus('Screen ready. Changes are saved after each action.');}
}
function activate(name) {
  if (!ready || busy || pending) return;
  const random = new Uint32Array(4); crypto.getRandomValues(random);
  pending = {name, revision:snapshot.revision, operation_id:Array.from(random,n=>n.toString(16).padStart(8,'0')).join('')};
  sessionStorage.setItem('critterPending',JSON.stringify(pending)); sendPending();
}
for (const id of ['lab','probe','companion']) document.querySelector(`#${id}`).addEventListener('click',async()=>{
  if(busy || pending) return; device=id; localStorage.setItem('critterDevice', device); if(snapshot) {try{await show(snapshot);}catch(error){setStatus(error.message);}}
});
document.querySelector('#refresh').addEventListener('click',refresh);
retryButton.addEventListener('click',sendPending);
document.addEventListener('keydown',event=>{
  if(!['ArrowLeft','ArrowRight','Enter'].includes(event.key)) return;
  event.preventDefault(); if(held||event.repeat) return; held=true;
  if(!ready||busy||pending) return;
  if(event.key==='Enter' && event.target.matches('button')) {event.target.click();return;}
  const buttons=Array.from(document.querySelectorAll('#controls button:not(:disabled)'));
  if(!buttons.length) return;
  const index=buttons.indexOf(document.activeElement);
  if(event.key==='Enter') {if(index>=0) buttons[index].click();else buttons[0].focus();}
  else buttons[(index+(event.key==='ArrowRight'?1:-1)+buttons.length)%buttons.length].focus();
});
document.addEventListener('keyup',()=>{held=false;}); window.addEventListener('blur',()=>{held=false;});
try {pending=JSON.parse(sessionStorage.getItem('critterPending'));} catch {sessionStorage.removeItem('critterPending');}
if(pending) {retryButton.hidden=false;setStatus('An action may still need confirmation. Retry it safely.');} else refresh();

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
