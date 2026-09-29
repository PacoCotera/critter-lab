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
    assets.push({
      name,
      source: `src/${name}.svg`,
      export: `exports/${name}.png`,
      width: result.width,
      height: result.height,
      drawScale: 1,
      alpha: metadata.hasAlpha,
      sha256: crypto.createHash('sha256').update(master).digest('hex'),
      exportSha256: crypto.createHash('sha256').update(exportedPng).digest('hex'),
    });
  }

  // The proof consumes the same PNG exports supplied for future integration.
  const layers = [
    place('header', 20, 16),
    place('navigation-frame', 20, 132),
    place('overview-frame', 280, 132),
    place('information-wide', 300, 225),
    place('information-wide', 300, 379),
    place('information-narrow', 704, 225),
    place('information-narrow', 704, 379),
    place('nav-focus', 33, 178),
    ...[0, 1, 2, 3].map(index => place('nav-quiet', 33, 246 + index * 67)),
    place('data', 420, 37),
    place('energy', 603, 37),
    place('essence', 785, 37),
    place('sample-capsule', 591, 274),
  ];
  text(47, 62, 'BEECHO LAB', 29, '#e1eaec', true);
  text(48, 93, 'LAB STOCK', 17, '#acbec7');
  for (const [label, x] of [['DATA', 473], ['ENERGY', 653], ['ESSENCE', 835]]) {
    text(x, 48, label, 16, '#acbec7');
    text(x, 74, '0', 25, '#e5ebeb', true);
    text(x, 95, 'Next unit 0%', 15, '#acbec7');
  }
  ['Overview', 'Explore', 'Research', 'Incubator', 'Habitat'].forEach((label, index) => {
    text(58, 215 + index * 67, label, 21, index === 0 ? '#f6e4ad' : '#d1dce0');
  });
  text(307, 181, 'Overview - Lab', 29, '#e3e9e9', true);
  text(308, 207, 'Explore to bring your first sample home', 18, '#b8cbd2');
  text(323, 269, 'RESEARCH', 22, '#dbe8ee', true);
  text(323, 299, '0 SAMPLES', 27, '#e0e9eb', true);
  text(323, 324, '0 findings recorded', 18);
  text(323, 346, 'No sample retained', 18, '#a9bdc8');
  text(727, 269, 'EXPLORE', 22, '#dbe8ee', true);
  text(727, 301, 'No expedition active', 17);
  text(323, 423, 'INCUBATOR', 22, '#dbe8ee', true);
  text(323, 457, 'No incubation', 21);
  text(727, 423, 'HABITAT', 22, '#dbe8ee', true);
  text(727, 457, '0 revealed residents', 17);
  text(727, 493, 'No Beecho revealed', 17, '#a9bdc8');
  text(30, 576, 'Up/down: preview workspaces', 17, '#b0c1c9');
  layers.push(...await renderedText());
  await saveComposition('overview-offline.png', 1024, 600, '#172129', layers);

  const sheetWidth = 1024;
  const sheetHeight = 850;
  const sheetLayers = [
    place('header', 20, 26),
    place('nav-quiet', 22, 162),
    place('nav-focus', 264, 162),
    place('data', 536, 162),
    place('energy', 604, 162),
    place('essence', 672, 162),
    place('sample-capsule', 748, 151),
    place('navigation-frame', 20, 237),
    place('overview-frame', 284, 237),
    place('information-wide', 20, 677),
    place('information-narrow', 430, 677),
  ];
  text(22, 19, 'NATIVE MASTERS · 1× · transparent PNG assets', 15);
  text(22, 153, 'Quiet navigation', 14);
  text(264, 153, 'Console focus', 14);
  text(535, 153, 'Data       Energy      Essence', 14);
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
      surface: '#222d35', frame: '#2877aa', focus: '#f5d77f',
      data: '#2568a6', energy: '#f7bc42', essence: '#a4e566',
    },
    assets,
  };
  fs.writeFileSync(path.join(root, 'manifest.json'), `${JSON.stringify(manifest, null, 2)}\n`);
}

main().catch(error => {
  console.error(error);
  process.exitCode = 1;
});
