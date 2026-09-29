import test from 'node:test';
import assert from 'node:assert/strict';

async function withTransport(failDown, scenario) {
  class Element extends EventTarget {
    constructor() { super(); this.textContent = ''; this.classList = { add() {}, remove() {} }; this.style = {}; }
    getBoundingClientRect() { return { left: 0, right: 100, top: 0, bottom: 100, width: 100, height: 100 }; }
    setPointerCapture() {}
    setAttribute(name, value) { this[name] = value; }
    querySelector() { return new Element(); }
    decode() { return Promise.resolve(); }
  }
  const elements = Object.fromEntries(['frame', 'status', 'release', 'up', 'down', 'left', 'right', 'research', 'critters', 'library', 'habitat', 'confirm', 'back'].map(name => [`#${name}`, new Element()]));
  const document = new EventTarget();
  document.hidden = false;
  document.querySelector = selector => elements[selector];
  document.querySelectorAll = () => Object.entries(elements).filter(([name]) => !['#frame', '#status', '#release'].includes(name)).map(([, element]) => element);
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
      if (input.event === 'confirm-down' && failDown) {
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
  const pointer = (type, pointerId = 1) => {
    const event = new Event(type, { cancelable: true });
    Object.assign(event, { button: 0, pointerId, clientX: 50, clientY: 50 });
    return event;
  };
  try {
    await import(`./bridge.mjs?scenario=${failDown ? 'failure' : 'overlap'}`);
    await until(() => requests.includes('ready'));
    await scenario({ elements, pointer, requests, until, heldInNative: () => heldInNative });
  } finally {
    for (const [key, value] of Object.entries(originals)) {
      if (value === undefined) delete globalThis[key]; else globalThis[key] = value;
    }
  }
}

test('lost down response cancels native hold and discards queued release', async () => {
  await withTransport(true, async ({ elements, pointer, requests, until, heldInNative }) => {
    elements['#confirm'].dispatchEvent(pointer('pointerdown'));
    elements['#confirm'].dispatchEvent(pointer('pointerup'));
    await until(() => requests.includes('cancel'));
    assert.equal(heldInNative(), false);
    assert(!requests.includes('confirm-up'), 'Queued release reached C after failed down response');
    const count = requests.length;
    elements['#confirm'].dispatchEvent(pointer('pointerdown'));
    elements['#confirm'].dispatchEvent(pointer('pointerup'));
    await new Promise(resolve => setImmediate(resolve));
    assert.equal(requests.length, count, 'Input resumed without an explicit reload after failure');
    assert.match(elements['#status'].textContent, /activation stopped/);
  });
});

test('overlapping panel presses cancel every gesture until all pointers release', async () => {
  await withTransport(false, async ({ elements, pointer, requests, until }) => {
    elements['#research'].dispatchEvent(pointer('pointerdown', 1));
    elements['#confirm'].dispatchEvent(pointer('pointerdown', 2));
    elements['#library'].dispatchEvent(pointer('pointerdown', 3));
    elements['#research'].dispatchEvent(pointer('pointerup', 1));
    elements['#confirm'].dispatchEvent(pointer('pointerup', 2));
    elements['#library'].dispatchEvent(pointer('pointerup', 3));
    await until(() => requests.includes('cancel'));
    await new Promise(resolve => setImmediate(resolve));
    assert(!requests.includes('research-up'));
    assert(!requests.includes('confirm-down'));
    assert(!requests.includes('confirm-up'));
    assert(!requests.includes('library-down'));
    assert(!requests.includes('library-up'));
    elements['#down'].dispatchEvent(pointer('pointerdown', 4));
    elements['#down'].dispatchEvent(pointer('pointerup', 4));
    await until(() => requests.includes('down-up'));
  });
});
