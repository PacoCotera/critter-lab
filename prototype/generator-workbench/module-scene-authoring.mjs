import { readFileSync, mkdirSync, writeFileSync } from "node:fs";
import { resolve } from "node:path";
import {
  authoringIdentity,
  creatureReferenceLine,
} from "./authoring-identity.mjs";
import {
  resolveAuthoring,
  digest,
  PACKET_SCHEMA,
} from "./authoring-adapter.mjs";
import { generateGenome, validateCatalogue, isRecord } from "./model.mjs";
import { GRAPH_COVERING_CATALOGUE } from "./graph-covering-catalogue.mjs";
import { bodyCoveringProofCases } from "./construct-covering-proof.mjs";
import { constructModuleScene, MODULE_SCENE_VERSION } from "./module-scene.mjs";
import {
  drawBodyCoveringScene,
  drawBodyCoveringComparison,
} from "./graph-covering-presentation.mjs";

export const SCENE_ART_VERSION = "module-scene-art/1";
export const SCENE_SEARCH_VERSION = "sequential-candidate-seed/1";
const template = JSON.parse(
  readFileSync(new URL("art-template.json", import.meta.url), "utf8"),
);
const rounded = (number) => Number(number.toFixed(1));
function rejected(code, message, extra = {}) {
  return {
    status: "rejected",
    errors: [{ code, path: "scene", message }],
    ...extra,
  };
}

export function moduleSceneCatalogue() {
  const examples = bodyCoveringProofCases().filter((item) =>
    ["single-skin", "axial-scales", "single-scales-extent"].includes(item.name),
  );
  return {
    catalogue: structuredClone(GRAPH_COVERING_CATALOGUE),
    defaultGeneration: { genome: structuredClone(examples[0].input.genome) },
    sceneExamples: examples.map((item) => ({
      name: item.name,
      ...structuredClone(item.input),
      relationship: item.relationship,
    })),
  };
}

function sceneReference(scene) {
  const camera = scene.body.bounds;
  const size = 512;
  const scale = Math.min(
    (size - 40) / (camera.maximumX - camera.minimumX),
    (size - 40) / (camera.maximumY - camera.minimumY),
  );
  const offsetX =
    (size - (camera.maximumX - camera.minimumX) * scale) / 2 -
    camera.minimumX * scale;
  const offsetY =
    (size - (camera.maximumY - camera.minimumY) * scale) / 2 -
    camera.minimumY * scale;
  const svg = drawBodyCoveringScene(scene.body, scene.ocular, scene.covering, {
    camera,
    size,
  });
  return {
    status: "constructed",
    profileVersion: MODULE_SCENE_VERSION,
    sceneDigest: scene.sceneDigest,
    svg,
    svgDigest: digest(svg),
    camera,
    size,
    mapping: { scale, offsetX, offsetY },
    view: "Orthographic XY static source construction; diagnostic ink is not inherited pigment.",
  };
}

// The renderer reads macro form; exact coordinates and height remain in the scene artifact.
export function describeModuleScene(scene) {
  const stations = scene.body.bodyExteriors[0].stations;
  const unit = stations[0].dimensions[1];
  const regionName = (id) => {
    if (stations.length === 1) return "body";
    const index = stations.findIndex((station) => station.nodeId === id);
    return index === 0
      ? "front"
      : index === stations.length - 1
        ? "rear"
        : `middle region ${index + 1}`;
  };
  const widths = stations.map((station) => station.dimensions[1]);
  const lengths = stations.map((station) => station.dimensions[0]);
  const equalLengths = lengths.every(
    (length) => Math.abs(length - lengths[0]) < 1e-8,
  );
  const lengthDescription = equalLengths ? "equal-length" : "differently sized";
  const widthDescription = widths.every(
    (width) => Math.abs(width - widths[0]) < 1e-8,
  )
    ? "Their widths are equal."
    : widths.length === 3 && Math.abs(widths[0] - widths[2]) < 1e-8
      ? `The middle is ${rounded(Math.abs(widths[1] / widths[0] - 1) * 100)}% ${widths[1] > widths[0] ? "wider" : "narrower"} than the two ends.`
      : stations
          .slice(1)
          .map(
            (station) =>
              `The ${regionName(station.nodeId)} is ${rounded(Math.abs(station.dimensions[1] / unit - 1) * 100)}% ${station.dimensions[1] >= unit ? "wider" : "narrower"} than the front.`,
          )
          .join(" ");
  const outline = scene.body.bodyExteriors[0].outline;
  const bodyLength =
    Math.max(...outline.map((point) => point[0])) -
    Math.min(...outline.map((point) => point[0]));
  const phrases = [
    stations.length === 1
      ? `One continuous rounded body, ${rounded(stations[0].dimensions[0] / unit)} times as long as wide.`
      : `One continuous body has ${stations.length} ${lengthDescription} rounded regions joined through ${stations.length - 1} narrower waists. ${widthDescription} The whole body is about ${rounded(bodyLength / unit)} times the front region's width.`,
  ];
  const roots = scene.body.appendages.filter((item) =>
    stations.some((station) => station.nodeId === item.parentNodeId),
  );
  const groups = new Map();
  for (const root of roots) {
    let description;
    if (root.role === "fin") {
      description = `fins on the ${regionName(root.parentNodeId)}, each spanning ${rounded((100 * root.sourceDimensions[1]) / unit)}% of ${stations.length === 1 ? "body" : "front-region"} width`;
    } else {
      const chain = [root];
      let next = scene.body.appendages.find(
        (item) => item.parentNodeId === root.nodeId,
      );
      while (next) {
        chain.push(next);
        next = scene.body.appendages.find(
          (item) => item.parentNodeId === next.nodeId,
        );
      }
      const segmentSizes = chain
        .map((item) => `${rounded((100 * item.length) / unit)}%`)
        .join(" and ");
      description = `jointed appendages along the ${regionName(root.parentNodeId)}, ${chain.length} segments about ${segmentSizes} of ${stations.length === 1 ? "body" : "front-region"} width${chain.at(-1).role === "contact-link" ? ", each ending in one contact tip" : ""}`;
    }
    groups.set(description, (groups.get(description) ?? 0) + 1);
  }
  const pairedFins =
    roots.length === stations.length * 2 &&
    roots.every((root) => root.role === "fin") &&
    stations.every(
      (station) =>
        roots.filter((root) => root.parentNodeId === station.nodeId).length ===
        2,
    );
  const equalSpans =
    roots.length &&
    roots.every(
      (root) => root.sourceDimensions[1] === roots[0].sourceDimensions[1],
    );
  if (pairedFins && equalSpans) {
    phrases.push(
      `${roots.length} fins form ${stations.length} opposed pairs, ${stations.length === 1 ? "one pair on the body" : "one pair on each region"}; each fin spans ${rounded((100 * roots[0].sourceDimensions[1]) / unit)}% of ${stations.length === 1 ? "body" : "front-region"} width.`,
    );
  } else if (roots.length)
    phrases.push(
      `${[...groups].map(([description, count]) => `${count} ${description}, ${count / 2} per side`).join("; ")}.`,
    );
  else phrases.push("No appendages.");
  const eyes = scene.ocular.features;
  if (eyes.length) {
    const eye = eyes[0];
    const position = rounded(
      (100 * (eye.center[0] - stations[0].position[0])) /
        stations[0].dimensions[0],
    );
    const placement =
      position === 0
        ? `centered on the ${stations.length === 1 ? "body" : "front region"}`
        : `on the ${stations.length === 1 ? "body" : "front region"}, ${Math.abs(position)}% of its length ${position >= 0 ? "behind" : "ahead of"} its center`;
    phrases.push(
      `Two circular eyes ${placement}, diameter ${rounded((100 * 2 * eye.radius) / unit)}% of ${stations.length === 1 ? "body" : "front-region"} width, their centers separated by ${rounded((100 * Math.abs(eyes[0].center[1] - eyes[1].center[1])) / unit)}% of that width; ${pigmentName(eye.outerPalette)} surrounds ${pigmentName(eye.pupilPalette)} pupils (${rounded((eye.pupilRadius / eye.radius) * 100)}% radius). No mouth.`,
    );
  } else phrases.push("Faceless.");
  const fields = new Map();
  for (const surface of scene.body.surfaces) {
    const role = scene.body.appendages.find(
      (item) => item.id === surface.shapeId,
    )?.role;
    const label =
      surface.atlas.kind === "longitudinal-body-ownership"
        ? "body regions"
        : ({
            link: "jointed segments",
            "contact-link": "contact tips",
            fin: "fins",
          }[role] ?? "appendages");
    const colors = surface.palette.map(pigmentName).join(" then ");
    const mapping =
      surface.palette.length === 1
        ? colors
        : `equal ${label === "body regions" ? "front/rear" : "root/tip"} halves: ${colors}`;
    fields.set(
      `${label}: ${mapping}, ${surface.texture.replaceAll("-", " ")}`,
      true,
    );
  }
  phrases.push(`${[...fields.keys()].join("; ")}. Unmarked.`);
  if (scene.covering.kind === "skin") phrases.push("Bare skin.");
  else {
    const covering = scene.covering;
    phrases.push(
      `${covering.plates.length} overlapping rounded plates run rearward through the ${rounded(covering.field.uStart * 100)}–${rounded(covering.field.uEnd * 100)}% longitudinal strip; diameter ${rounded((100 * 2 * covering.plateProfile.halfWidth) / unit)}% of ${stations.length === 1 ? "body" : "front-region"} width. Pigment boundaries continue through plates; eyes and appendage roots stay clear.`,
    );
  }
  if (
    stations.length > 1 ||
    roots.length ||
    eyes.length ||
    scene.covering.kind === "scales"
  )
    phrases.push("View from above.");
  else phrases.push("View from above.");
  return phrases.join(" ");
}

function pigmentName(pigment) {
  const names = {
    "#f1eddc": "pale cream",
    "#273036": "charcoal",
    "#465459": "dark slate",
    "#dfd2ae": "cream",
    "#ae674d": "russet",
    "#718489": "blue-gray",
  };
  return names[pigment] ? `${names[pigment]} ${pigment}` : pigment;
}

export const MODULE_SCENE_PIXEL_DIRECTION = template.promptSections
  .filter((section) => section.id !== "phenotype")
  .map((section) => section.text)
  .join("\n\n");

function verifiedSource(packet) {
  if (
    packet?.status !== "resolved" ||
    packet.schemaVersion !== PACKET_SCHEMA ||
    packet.sceneProjectionVersion !== MODULE_SCENE_VERSION
  )
    throw new Error("A current resolved module scene packet is required.");
  const source = resolveAuthoring(packet.input);
  if (
    source.status !== "resolved" ||
    source.inputDigest !== packet.inputDigest ||
    source.resultDigest !== packet.resultDigest ||
    digest(packet.result) !== packet.resultDigest ||
    source.recordId !== packet.recordId ||
    source.contentId !== packet.contentId ||
    source.contentVersion !== packet.contentVersion ||
    source.ruleVersion !== packet.ruleVersion
  )
    throw new Error(
      "Scene inputs and metadata must match independent genetic replay.",
    );
  const scene = constructModuleScene(source.result, {
    profileVersion: MODULE_SCENE_VERSION,
  });
  const projectionId = `scene-${digest({ inputDigest: source.inputDigest, resultDigest: source.resultDigest, sceneDigest: scene.sceneDigest, profileVersion: MODULE_SCENE_VERSION, artVersion: SCENE_ART_VERSION }).slice(0, 20)}`;
  if (
    scene.status !== "constructed" ||
    digest(scene) !== digest(packet.scene) ||
    packet.sceneProjection?.recordId !== projectionId ||
    packet.sceneProjection?.profileVersion !== MODULE_SCENE_VERSION ||
    packet.sceneProjection?.artVersion !== SCENE_ART_VERSION
  )
    throw new Error(
      "Scene artifact/profile must match independently reconstructed consumers.",
    );
  return scene;
}

export function projectModuleScenePrompt(packet) {
  const scene = verifiedSource(packet);
  const phenotypeDescription = describeModuleScene(scene);
  const bindings = {
    phenotypeDescription,
    resultIdentity: JSON.stringify({
      sourceRecordId: packet.recordId,
      inputDigest: packet.inputDigest,
      resultDigest: packet.resultDigest,
      sceneRecordId: packet.sceneProjection.recordId,
      sceneDigest: scene.sceneDigest,
    }),
    sceneArtifactReference: JSON.stringify({
      path: "scene",
      profileVersion: MODULE_SCENE_VERSION,
      body: scene.body.constructionDigest,
      ocular: scene.ocular.moduleDigest,
      covering: scene.covering.coveringDigest,
      policy:
        "Complete lossless geometry, materials and traces remain in the exported scene; this binding references that artifact rather than containing every coordinate.",
    }),
  };
  const subject = {
    projectionVersion: SCENE_ART_VERSION,
    context: packet.input.context,
    bindings,
  };
  const text =
    MODULE_SCENE_PIXEL_DIRECTION +
    "\n\n" +
    phenotypeDescription +
    "\n\n" +
    creatureReferenceLine(authoringIdentity(packet));
  const overflow = scenePromptBoundsError(subject, bindings, text);
  if (overflow) throw new Error(overflow);
  return {
    status: "source-derived concept brief; no provider called",
    projectionVersion: SCENE_ART_VERSION,
    templateId: template.id,
    templateVersion: template.version,
    bindings,
    text,
    promptDigest: digest(text),
    limitations:
      "Reference-backed macro description; full source artifact is retained separately. This does not establish illustration fidelity.",
  };
}

export function resolveModuleSceneAuthoring(input) {
  const source = resolveAuthoring(input);
  if (source.status !== "resolved") return { ...source, stage: "genetic" };
  const scene = constructModuleScene(source.result, {
    profileVersion: MODULE_SCENE_VERSION,
  });
  if (scene.status !== "constructed")
    return {
      ...scene,
      sourcePacket: source,
      geneticResultRetained: true,
      inputDigest: source.inputDigest,
      resultDigest: source.resultDigest,
    };
  const packet = {
    ...source,
    sceneProjectionVersion: MODULE_SCENE_VERSION,
    scene,
    sceneProjection: {
      recordId: `scene-${digest({ inputDigest: source.inputDigest, resultDigest: source.resultDigest, sceneDigest: scene.sceneDigest, profileVersion: MODULE_SCENE_VERSION, artVersion: SCENE_ART_VERSION }).slice(0, 20)}`,
      profileVersion: MODULE_SCENE_VERSION,
      artVersion: SCENE_ART_VERSION,
    },
    presentation: {
      status: "constructed",
      profileVersion: MODULE_SCENE_VERSION,
      view: "Orthographic XY source inspection, not finished game art.",
    },
    reference: sceneReference(scene),
  };
  packet.diagnostic = packet.reference.svg;
  packet.geometryReference = packet.reference;
  packet.description = describeModuleScene(scene);
  packet.identity = authoringIdentity(packet);
  try {
    packet.prompt = projectModuleScenePrompt(packet);
  } catch (error) {
    packet.prompt = {
      status: "rejected",
      error: error.message,
      text: "",
      promptDigest: digest(""),
      limitations:
        "Valid genetic result and full scene retained; no content truncated.",
    };
  }
  return packet;
}

export function compactSceneReplayEnvelope(packet) {
  return {
    schemaVersion: packet.schemaVersion,
    sceneProjectionVersion: packet.sceneProjectionVersion,
    sceneRecordId: packet.sceneProjection?.recordId,
    input: packet.input,
    inputDigest: packet.inputDigest,
    resultDigest: packet.resultDigest,
    sceneDigest: packet.scene?.sceneDigest,
    promptDigest: packet.prompt?.promptDigest,
  };
}

export function replayModuleSceneAuthoring(envelope) {
  const allowed = [
    "schemaVersion",
    "sceneProjectionVersion",
    "sceneRecordId",
    "input",
    "inputDigest",
    "resultDigest",
    "sceneDigest",
    "promptDigest",
  ];
  if (
    !isRecord(envelope) ||
    Object.keys(envelope).some((key) => !allowed.includes(key)) ||
    allowed.some((key) => !Object.hasOwn(envelope, key)) ||
    envelope.schemaVersion !== PACKET_SCHEMA ||
    envelope.sceneProjectionVersion !== MODULE_SCENE_VERSION ||
    allowed
      .filter((key) => key.endsWith("Digest"))
      .some((key) => !/^[a-f0-9]{64}$/.test(envelope[key]))
  )
    return rejected(
      "scene-replay-envelope",
      "A closed compact scene replay envelope with declared identities is required.",
    );
  if (Buffer.byteLength(JSON.stringify(envelope), "utf8") > 65536)
    return rejected(
      "scene-replay-transport",
      "Compact scene replay exceeds the unchanged 64 KiB transport bound.",
    );
  const packet = resolveModuleSceneAuthoring(envelope.input);
  if (packet.status !== "resolved") return packet;
  const actual = compactSceneReplayEnvelope(packet);
  if (
    allowed
      .filter((key) => key !== "input")
      .some((key) => actual[key] !== envelope[key])
  )
    return rejected(
      "scene-replay-mismatch",
      "Source/result/scene/prompt identity differs from independently reconstructed inputs.",
    );
  return {
    ...packet,
    replay: {
      verified: true,
      clientArtifactsTrusted: false,
      artifactsReconstructed: true,
    },
  };
}

export function generateModuleSceneAuthoring(catalogue, seed, options = {}) {
  if (
    !isRecord(options) ||
    Object.keys(options).some((key) => key !== "maxAttempts")
  )
    return rejected(
      "scene-generation-options",
      "Only maxAttempts is supported.",
    );
  const { maxAttempts = 1024 } = options;
  const checked = validateCatalogue(catalogue);
  if (!checked.valid) return { status: "rejected", errors: checked.errors };
  if (
    catalogue.ruleVersion !== "developmental-covering/1" ||
    !Number.isInteger(seed) ||
    seed < 0 ||
    seed > 0xffffffff ||
    !Number.isInteger(maxAttempts) ||
    maxAttempts < 1 ||
    maxAttempts > 1024
  )
    return rejected(
      "scene-generation-options",
      "Covering source catalogue, uint32 seed and total bound 1–1024 required.",
    );
  const accounting = {
    algorithmVersion: SCENE_SEARCH_VERSION,
    requestedSeed: seed,
    maxAttempts,
    attempts: 0,
    rejected: { genetic: 0, body: 0, ocular: 0, covering: 0, source: 0 },
    stageErrorCounts: {},
    seedSequence: [],
  };
  for (let draw = 1; draw <= maxAttempts; draw++) {
    const candidateSeed = (seed + draw - 1) >>> 0;
    accounting.attempts = draw;
    accounting.seedSequence.push(candidateSeed);
    const generated = generateGenome(catalogue, candidateSeed, {
      maxAttempts: 1,
    });
    const candidate =
      generated.status === "generated"
        ? resolveModuleSceneAuthoring({ catalogue, genome: generated.genome })
        : { ...generated, stage: "genetic" };
    if (candidate.status === "resolved")
      return {
        ...candidate,
        generation: {
          ...accounting,
          seed: candidateSeed,
          winningSeed: candidateSeed,
          winningDraw: draw,
        },
      };
    const stage = candidate.stage ?? "genetic";
    accounting.rejected[stage]++;
    for (const error of candidate.errors ?? []) {
      const key = `${stage}:${error.code}`;
      accounting.stageErrorCounts[key] =
        (accounting.stageErrorCounts[key] ?? 0) + 1;
    }
  }
  return rejected(
    "generation-exhausted",
    "No scene-eligible creature found in this bounded unmodified search.",
    { generation: accounting },
  );
}

// Bounds are checked before presentation; rejection never truncates the retained subject.
export function scenePromptBoundsError(subject, bindings, text) {
  if (
    Buffer.byteLength(JSON.stringify(subject), "utf8") >
    template.limits.maxSubjectPacketBytes
  )
    return "Scene art subject exceeds 64 KiB; full scene retained separately.";
  for (const [name, value] of Object.entries(bindings))
    if (value.length > template.limits.maxBindingCharacters)
      return `Scene art binding ${name} exceeds ${template.limits.maxBindingCharacters} characters.`;
  if (text.length > template.limits.maxPromptCharacters)
    return "Scene art prompt exceeds the existing prompt bound.";
  return null;
}

export function writeModuleSceneProof(directory) {
  const cases = bodyCoveringProofCases()
    .filter((item) =>
      ["single-scales", "axial-scales", "single-scales-extent"].includes(
        item.name,
      ),
    )
    .map((item) => {
      const packet = resolveModuleSceneAuthoring(item.input);
      if (packet.status !== "resolved" || packet.prompt.error)
        throw new Error(`${item.name}: scene or prompt rejected`);
      return { ...item, packet };
    });
  mkdirSync(directory, { recursive: true });
  const manifest = cases.map((item) => {
    writeFileSync(
      resolve(directory, `${item.name}.packet.json`),
      JSON.stringify(item.packet, null, 2) + "\n",
    );
    writeFileSync(
      resolve(directory, `${item.name}.svg`),
      item.packet.reference.svg,
    );
    writeFileSync(
      resolve(directory, `${item.name}.prompt.txt`),
      item.packet.prompt.text,
    );
    return {
      name: item.name,
      relationship: item.relationship,
      recordId: item.packet.recordId,
      sceneRecordId: item.packet.sceneProjection.recordId,
      inputDigest: item.packet.inputDigest,
      resultDigest: item.packet.resultDigest,
      sceneDigest: item.packet.scene.sceneDigest,
      promptDigest: item.packet.prompt.promptDigest,
      referenceDigest: item.packet.reference.svgDigest,
    };
  });
  writeFileSync(
    resolve(directory, "comparison.svg"),
    drawBodyCoveringComparison(
      cases.map((item) => ({
        construction: item.packet.scene.body,
        ocular: item.packet.scene.ocular,
        covering: item.packet.scene.covering,
      })),
    ),
  );
  const searchMeasurements = [1, 1000, 2000].map((seed) => {
    const packet = generateModuleSceneAuthoring(GRAPH_COVERING_CATALOGUE, seed);
    return {
      status: packet.status,
      generation: packet.generation,
      inputDigest: packet.inputDigest,
      resultDigest: packet.resultDigest,
      sceneDigest: packet.scene?.sceneDigest,
    };
  });
  writeFileSync(
    resolve(directory, "manifest.json"),
    JSON.stringify(
      {
        profileVersion: MODULE_SCENE_VERSION,
        artProjectionVersion: SCENE_ART_VERSION,
        cases: manifest,
        searchMeasurements,
      },
      null,
      2,
    ) + "\n",
  );
  return manifest;
}
