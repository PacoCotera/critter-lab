import test from 'node:test';
import assert from 'node:assert/strict';

test('lost down response cancels native hold and discards queued release', async () => {
  class Element extends EventTarget {
    constructor() { super(); this.textContent = ''; this.classList = { add() {}, remove() {} }; this.style = {}; }
    getBoundingClientRect() { return { left: 0, right: 100, top: 0, bottom: 100, width: 100, height: 100 }; }
    setPointerCapture() {}
    querySelector() { return new Element(); }
    decode() { return Promise.resolve(); }
  }
  const elements = Object.fromEntries(['frame', 'status', 'knob', 'confirm', 'back'].map(name => [`#${name}`, new Element()]));
  const document = new EventTarget();
  document.hidden = false;
  document.querySelector = selector => elements[selector];
  document.querySelectorAll = () => [elements['#confirm'], elements['#back']];
  const requests = [];
  let heldInNative = false;
  let nativeRevision = 1;
  const state = () => ({ revision: nativeRevision, page: 'study', focus: 'start', ready: true, boundary: 'test transport' });
  const originals = Object.fromEntries(['document', 'window', 'Image', 'fetch', 'requestAnimationFrame'].map(key => [key, globalThis[key]]));
  Object.assign(globalThis, {
    document,
    window: new EventTarget(),
    Image: Element,
    requestAnimationFrame: callback => queueMicrotask(callback),
    fetch: async (url, options) => {
      if (url === '/api/status') return { ok: true, json: async () => state() };
      if (url.startsWith('/api/frame')) return { ok: true, status: 200, blob: async () => new Blob() };
      const input = JSON.parse(options.body);
      requests.push(input.event);
      if (input.event === 'resume') ++nativeRevision;
      if (input.event === 'confirm-down') {
        heldInNative = true;
        throw new Error('Response lost after native down');
      }
      if (input.event === 'cancel') heldInNative = false;
      return { ok: true, json: async () => state() };
    }
  });
  async function until(predicate) {
    for (let index = 0; index < 100 && !predicate(); ++index) await new Promise(resolve => setImmediate(resolve));
    assert(predicate(), 'Transport test did not reach its expected boundary');
  }
  const pointer = type => {
    const event = new Event(type, { cancelable: true });
    Object.assign(event, { button: 0, pointerId: 1, clientX: 50, clientY: 50 });
    return event;
  };
  try {
    await import('./bridge.mjs');
    await until(() => requests.includes('ready'));
    elements['#confirm'].dispatchEvent(pointer('pointerdown'));
    elements['#confirm'].dispatchEvent(pointer('pointerup'));
    await until(() => requests.includes('cancel'));
    assert.equal(heldInNative, false);
    assert(!requests.includes('confirm-up'), 'Queued release reached C after failed down response');
    const count = requests.length;
    elements['#confirm'].dispatchEvent(pointer('pointerdown'));
    elements['#confirm'].dispatchEvent(pointer('pointerup'));
    await new Promise(resolve => setImmediate(resolve));
    assert.equal(requests.length, count, 'Input resumed without an explicit reload after failure');
    assert.match(elements['#status'].textContent, /activation stopped/);
  } finally {
    for (const [key, value] of Object.entries(originals)) {
      if (value === undefined) delete globalThis[key]; else globalThis[key] = value;
    }
  }
});
