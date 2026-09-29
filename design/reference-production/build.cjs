const fs = require('fs');
const path = require('path');
const crypto = require('crypto');
const sharp = require('sharp');

const root = __dirname;
const source = path.join(root, 'src');
const output = path.join(root, 'exports');
let textRuns = [];

function text(x, y, value, size = 20, color = '#d5e0e3', bold = false) {
  textRuns.push({ x, y, value, size, color, bold });
}

async function renderedText() {
  const layers = [];
  for (const run of textRuns) {
    // Explicit font files prevent host font substitution. Coordinates use a
    // nominal baseline; all final placements are inspected in the offline proof.
    const input = await sharp({
      text: {
        text: `<span foreground="${run.color}">${run.value}</span>`,
        font: `Bitstream Vera Sans ${run.bold ? 'Bold ' : ''}${run.size}`,
        fontfile: path.join(root, '../../native/shared/fonts', run.bold ? 'VeraBd.ttf' : 'Vera.ttf'),
        rgba: true,
        dpi: 72,
      },
    }).png().toBuffer();
    layers.push({ input, left: run.x, top: run.y - run.size });
  }
  textRuns = [];
  return layers;
}

function place(name, left, top) {
  return { input: path.join(output, `${name}.png`), left, top };
}

async function saveComposition(filename, width, height, background, layers) {
  await sharp({ create: { width, height, channels: 4, background } })
    .composite(layers).png().toFile(path.join(output, filename));
}

async function main() {
  fs.mkdirSync(output, { recursive: true });
  const assets = [];
  const files = fs.readdirSync(source).filter(file => file.endsWith('.svg')).sort();
  for (const file of files) {
    const name = path.basename(file, '.svg');
    const master = fs.readFileSync(path.join(source, file));
    const result = await sharp(master).png().toFile(path.join(output, `${name}.png`));
    const exportedPng = fs.readFileSync(path.join(output, `${name}.png`));
    const metadata = await sharp(exportedPng).metadata();
    const { data: pixels, info } = await sharp(exportedPng).raw().toBuffer({ resolveWithObject: true });
    let left = info.width;
    let top = info.height;
    let right = -1;
    let bottom = -1;
    for (let y = 0; y < info.height; y++) {
      for (let x = 0; x < info.width; x++) {
        if (pixels[(y * info.width + x) * info.channels + 3] < 128) continue;
        left = Math.min(left, x);
        top = Math.min(top, y);
        right = Math.max(right, x);
        bottom = Math.max(bottom, y);
      }
    }
    assets.push({
      name,
      source: `src/${name}.svg`,
      export: `exports/${name}.png`,
      width: result.width,
      height: result.height,
      drawScale: 1,
      alpha: metadata.hasAlpha,
      occupiedBoundsAtHalfAlpha: { x: left, y: top, width: right - left + 1, height: bottom - top + 1 },
      sha256: crypto.createHash('sha256').update(master).digest('hex'),
      exportSha256: crypto.createHash('sha256').update(exportedPng).digest('hex'),
    });
  }

  // Fixed 24px outer inset and 16px gutters; read-only summaries share one field.
  const layers = [
    place('header', 24, 24),
    place('navigation-frame', 24, 140),
    place('overview-frame', 248, 140),
    place('nav-focus', 36, 176),
    ...[0, 1, 2, 3].map(index => place('nav-quiet', 36, 244 + index * 68)),
    place('data', 404, 40),
    place('energy', 602, 40),
    place('essence', 800, 40),
    place('explore-topic', 272, 213),
    place('research-topic', 642, 213),
    place('incubator-topic', 272, 377),
    place('habitat-topic', 642, 377),
  ];
  const dividers = '<svg xmlns="http://www.w3.org/2000/svg" width="752" height="416"><path d="M24 65 H728 M380 92 V346 M24 220 H728" fill="none" stroke="#34617a" stroke-width="1"/><path d="M24 65 H112 M672 65 H728" stroke="#3b8bbc"/></svg>';
  layers.splice(3, 0, { input: Buffer.from(dividers), left: 248, top: 140 });
  text(48, 71, 'BEECHO LAB', 29, '#e1eaec', true);
  text(49, 100, 'LAB STOCK', 17, '#acbec7');
  for (const [label, x] of [['Data', 470], ['Energy', 668], ['Essence', 866]]) {
    text(x, 61, label, 18, '#b7c8cf');
    text(x, 87, '0', 28, '#e5ebeb', true);
    text(x, 104, 'Next unit 0%', 14, '#acbec7');
  }
  ['Overview', 'Explore', 'Research', 'Incubator', 'Habitat'].forEach((label, index) => {
    text(60, 214 + index * 68, label, 21, index === 0 ? '#f6e4ad' : '#d1dce0');
  });
  text(272, 190, 'Overview - Lab', 30, '#e3e9e9', true);
  text(422, 265, 'Explore', 23, '#e1e9eb', true);
  text(422, 299, 'No expedition', 18);
  text(790, 265, 'Research', 23, '#e1e9eb', true);
  text(790, 299, '0 samples', 23, '#e3ebee');
  text(790, 328, '0 findings', 18, '#adc2ce');
  text(422, 425, 'Incubator', 23, '#e1e9eb', true);
  text(422, 459, 'No incubation', 18);
  text(790, 425, 'Habitat', 23, '#e1e9eb', true);
  text(790, 459, '0 revealed', 23, '#e3ebee');
  text(790, 487, 'residents', 18, '#adc2ce');
  text(422, 328, 'Bring a sample home', 16, '#adc2ce');
  text(28, 584, 'Up/down: preview workspaces', 17, '#b0c1c9');
  layers.push(...await renderedText());
  await saveComposition('overview-offline.png', 1024, 600, '#172129', layers);

  const sheetWidth = 1024;
  const sheetHeight = 1080;
  const sheetLayers = [
    place('header', 24, 28),
    place('data', 24, 157), place('energy', 100, 157), place('essence', 176, 157),
    place('explore-topic', 280, 140), place('research-topic', 440, 140),
    place('incubator-topic', 600, 140), place('habitat-topic', 760, 140),
    place('navigation-frame', 24, 300), place('overview-frame', 248, 300),
    place('nav-quiet', 24, 738), place('nav-focus', 232, 738),
    place('sample-capsule', 446, 888),
    place('information-wide', 24, 888), place('information-narrow', 560, 888),
  ];
  text(24, 20, 'CURRENT NATIVE MASTERS · 1×', 15);
  text(24, 147, 'Header resource family', 14);
  text(24, 727, 'Quiet navigation', 14);
  text(232, 727, 'Console focus', 14);
  text(24, 873, 'Retained prior variants · not used in this composition', 15);
  sheetLayers.push(...await renderedText());
  await saveComposition('sheet-native.png', sheetWidth, sheetHeight, '#151d24', sheetLayers);
  await sharp(path.join(output, 'sheet-native.png'))
    .resize(sheetWidth * 3, sheetHeight * 3, { kernel: 'nearest' })
    .png().toFile(path.join(output, 'sheet-3x.png'));

  const manifest = {
    status: 'Offline art proof; not runtime integration or owner approval',
    reference: '../game-art-proposals/37-lab-extracted-kit/screen-reference.png',
    referenceSize: [752, 421],
    font: 'Explicit bundled Bitstream Vera Sans/Vera Bold font files; C18 type identity unresolved',
    paletteRoles: {
      surface: '#222d35', frame: '#2385c8', focus: '#f5d77f',
      data: '#2568a6', energy: '#f7bc42', essence: '#a4e566',
    },
    composition: { outerInset: 24, gutter: 16, header: [976, 100], navigation: [208, 416], workField: [752, 416], workFieldTextInset: 24, topicFootprint: [136, 144], resourceFootprint: [56, 68], statusAreasReadOnly: true },
    assets,
  };
  fs.writeFileSync(path.join(root, 'manifest.json'), `${JSON.stringify(manifest, null, 2)}\n`);
}

main().catch(error => {
  console.error(error);
  process.exitCode = 1;
});
