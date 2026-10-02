import test from "node:test";
import assert from "node:assert/strict";
import { readFileSync } from "node:fs";
import { once } from "node:events";
import { bodyCoveringProofCases } from "./construct-covering-proof.mjs";
import { GRAPH_COVERING_CATALOGUE } from "./graph-covering-catalogue.mjs";
import {
  resolveAuthoring,
  replayAuthoring,
  digest,
  authoringCatalogue,
} from "./authoring-adapter.mjs";
import { constructModuleScene } from "./module-scene.mjs";
import { authoringIdentity } from "./authoring-identity.mjs";
import { makeServer } from "./server.mjs";
import {
  resolveModuleSceneAuthoring,
  replayModuleSceneAuthoring,
  compactSceneReplayEnvelope,
  projectModuleScenePrompt,
  generateModuleSceneAuthoring,
  scenePromptBoundsError,
  describeModuleScene,
} from "./module-scene-authoring.mjs";
import {
  sharedSceneCamera,
  scenePreviewMarkup,
  sceneCausalSummary,
  isResolvedAuthoringPacket,
  authoringRoute,
  copyableAuthoringExport,
} from "./authoring-ui.mjs";

const inputs = bodyCoveringProofCases();
const scenePacket = (name) =>
  resolveModuleSceneAuthoring(inputs.find((item) => item.name === name).input);
const retained = (path) =>
  JSON.parse(readFileSync(new URL(path, import.meta.url), "utf8"));

test("scene copy/export is a bounded importable replay envelope; old exports stay complete", () => {
  const packet = scenePacket("single-scales");
  const exported = copyableAuthoringExport(packet);
  const json = JSON.stringify(exported, null, 2);
  assert.ok(json.length < 1000000);
  assert.ok(Buffer.byteLength(JSON.stringify(exported)) <= 65536);
  assert.deepEqual(exported, compactSceneReplayEnvelope(packet));
  const parsed = JSON.parse(json);
  assert.deepEqual(copyableAuthoringExport(parsed), exported);
  const imported = replayModuleSceneAuthoring(copyableAuthoringExport(parsed));
  assert.equal(imported.status, "resolved");
  assert.equal(imported.resultDigest, packet.resultDigest);
  assert.equal(imported.scene.sceneDigest, packet.scene.sceneDigest);
  assert.equal(imported.prompt.text, packet.prompt.text);
  const oldPacket = retained("evidence/diversity-diagnosis/broad-seed-1.json");
  assert.strictEqual(copyableAuthoringExport(oldPacket), oldPacket);
});

test("reference identities separate inherited copies, expression and view/provenance", () => {
  const packet = scenePacket("single-scales");
  const identity = packet.identity;
  assert.equal(identity.inheritedDigest.length, 64);
  assert.equal(identity.expressionDigest.length, 64);
  assert.equal(identity.shortCodeFormat, "hex-prefix12/1");
  assert.equal(
    identity.genomeCode,
    `#G${identity.inheritedDigest.slice(0, 12).toUpperCase()}`,
  );
  assert.match(
    packet.prompt.text,
    new RegExp(
      `Creature reference: ${identity.genomeCode} ${identity.expressionCode}$`,
    ),
  );
  const changedView = structuredClone(packet);
  changedView.reference.svg = "a different display camera";
  changedView.prompt.text = "a different style";
  assert.deepEqual(authoringIdentity(changedView), identity);
  const changedOrigin = structuredClone(packet.input);
  changedOrigin.genome.origin.seed += 1;
  const provenance = resolveModuleSceneAuthoring(changedOrigin);
  assert.equal(provenance.identity.inheritedDigest, identity.inheritedDigest);
  assert.equal(provenance.identity.expressionDigest, identity.expressionDigest);
  assert.notEqual(provenance.recordId, packet.recordId);
  const expressionInput = structuredClone(packet.input);
  expressionInput.expressionSeed = 42;
  const expression = resolveModuleSceneAuthoring(expressionInput);
  assert.equal(expression.identity.inheritedDigest, identity.inheritedDigest);
  assert.notEqual(
    expression.identity.expressionDigest,
    identity.expressionDigest,
  );
  const contextInput = structuredClone(packet.input);
  contextInput.context.medium = "water";
  const context = resolveModuleSceneAuthoring(contextInput);
  assert.equal(context.identity.inheritedDigest, identity.inheritedDigest);
  assert.notEqual(context.identity.expressionDigest, identity.expressionDigest);
  const changedCopies = scenePacket("single-scales-extent");
  assert.notEqual(
    changedCopies.identity.inheritedDigest,
    identity.inheritedDigest,
  );
  const sceneWithoutDigest = structuredClone(packet.scene);
  delete sceneWithoutDigest.sceneDigest;
  assert.equal(digest(sceneWithoutDigest), packet.scene.sceneDigest);
  assert.doesNotMatch(
    JSON.stringify(sceneWithoutDigest),
    /<svg|inspectionInk|promptSections|referenceCamera/,
  );
});

test("optional wrapper preserves old packets and aggregates exact body/eye/material identities", () => {
  for (const file of [
    "evidence/diversity-diagnosis/broad-seed-1.json",
    "evidence/diversity-diagnosis/broad-seed-21.json",
    "evidence/pet-materials/pet-skin.json",
    "evidence/graph-ocular-proof/single-volume-contacts-ocular.packet.json",
  ]) {
    const saved = retained(file);
    const packet = saved.packet ?? saved;
    const resolved = resolveAuthoring(packet.input);
    assert.equal(resolved.resultDigest, packet.resultDigest);
    assert.equal(replayAuthoring(packet).resultDigest, packet.resultDigest);
    assert.equal(resolved.diagnostic, packet.diagnostic);
    const currentBrief = file.includes("broad-seed")
      ? `evidence/renderer-brief/current-prompts/${file.includes("21") ? "broad-seed-21" : "broad-seed-1"}.txt`
      : file.includes("pet-skin")
        ? "evidence/renderer-brief/current-prompts/pet-skin.txt"
        : null;
    if (currentBrief) {
      assert.equal(resolved.prompt.templateVersion, 3);
      assert.ok(
        readFileSync(new URL(currentBrief, import.meta.url), "utf8").length > 0,
      );
      assert.doesNotMatch(
        resolved.prompt.text,
        /HiBit|calm midtone|warm upper-left|cool shadow/,
      );
    }
  }
  assert.equal(authoringCatalogue().packages.length, 3);
  const packet = scenePacket("single-scales");
  assert.equal(packet.status, "resolved");
  assert.deepEqual(
    packet.scene,
    constructModuleScene(packet.result, { profileVersion: "module-scene/1" }),
  );
  assert.equal(
    packet.resultDigest,
    resolveAuthoring(packet.input).resultDigest,
  );
  assert.equal(packet.scene.sourceResultDigest, packet.resultDigest);
  assert.notEqual(packet.sceneProjection.recordId, packet.recordId);
  assert.equal(packet.reference.svgDigest, digest(packet.reference.svg));
});

test("new scene replay reconstructs artifacts and rejects identities, profiles and rehashed geometry", () => {
  const packet = scenePacket("axial-scales");
  const before = digest(packet);
  const envelope = compactSceneReplayEnvelope(packet);
  assert.ok(Buffer.byteLength(JSON.stringify(envelope)) < 65536);
  assert.ok(Buffer.byteLength(JSON.stringify(packet)) > 65536);
  const replayed = replayModuleSceneAuthoring(envelope);
  assert.deepEqual(replayed.scene, packet.scene);
  assert.equal(replayed.prompt.text, packet.prompt.text);
  assert.equal(replayed.replay.clientArtifactsTrusted, false);
  for (const key of [
    "sceneDigest",
    "promptDigest",
    "resultDigest",
    "inputDigest",
  ]) {
    assert.equal(
      replayModuleSceneAuthoring({ ...envelope, [key]: "0".repeat(64) }).status,
      "rejected",
    );
  }
  assert.equal(
    replayModuleSceneAuthoring({ ...envelope, geometry: packet.scene }).status,
    "rejected",
  );
  assert.equal(
    replayModuleSceneAuthoring({
      ...envelope,
      sceneProjectionVersion: "unknown",
    }).status,
    "rejected",
  );
  const forged = structuredClone(packet);
  forged.scene.ocular.features[0].radius *= 0.5;
  const { sceneDigest, ...scene } = forged.scene;
  forged.scene.sceneDigest = digest(scene);
  assert.throws(
    () => projectModuleScenePrompt(forged),
    /independently reconstructed/,
  );
  for (const options of [
    null,
    [],
    { profileVersion: "unknown" },
    { profileVersion: "module-scene/1", repair: true },
  ])
    assert.equal(
      constructModuleScene(packet.result, options).status,
      "rejected",
    );
  assert.equal(digest(packet), before);
});

test("positive brief consumes continuous source roots, proportions, eyes and conditional material", () => {
  const skin = scenePacket("single-skin");
  const scales = scenePacket("single-scales");
  const changed = scenePacket("single-scales-extent");
  assert.match(skin.prompt.text, /One continuous rounded body/);
  assert.match(
    scales.prompt.text,
    new RegExp(
      `${scales.scene.covering.plates.length} overlapping rounded plates`,
    ),
  );
  assert.notEqual(scales.prompt.text, changed.prompt.text);
  assert.match(changed.prompt.text, /20–95%/);
  assert.match(scales.prompt.text, /6 jointed appendages along the body/);
  assert.match(
    scales.prompt.text,
    /segments about 33.3% and 22.2% of body width/,
  );
  assert.match(scales.prompt.text, /Two circular eyes centered on the body/);
  assert.doesNotMatch(scales.prompt.text, /HiBit|Critter Lab|calm midtone/);
  assert.equal(scales.prompt.promptDigest, digest(scales.prompt.text));
  assert.match(scales.prompt.bindings.sceneArtifactReference, /exported scene/);
  assert.doesNotMatch(
    scales.prompt.text,
    /inspection ink|constructionDigest|locusIds|body-covering\/1/,
  );
  const noEyes = structuredClone(inputs[0].input);
  noEyes.genome.loci["structure.ocular-pair"] = ["absent", "absent"];
  const absent = resolveModuleSceneAuthoring(noEyes);
  assert.equal(absent.status, "resolved");
  assert.match(absent.prompt.text, /Faceless/);
  assert.doesNotMatch(absent.prompt.text, /Two circular eyes/);
  assert.match(
    scenePromptBoundsError({}, { phenotypeDescription: "x".repeat(8193) }, ""),
    /binding/,
  );
  assert.match(
    scenePromptBoundsError({ large: "é".repeat(40000) }, {}, ""),
    /64 KiB/,
  );
  assert.match(
    scenePromptBoundsError({}, {}, "x".repeat(32769)),
    /prompt bound/,
  );
  assert.equal(scales.status, "resolved");
  assert.equal(scales.scene.status, "constructed");
  const swappedSegments = structuredClone(scales.scene);
  const chainRoot = swappedSegments.body.appendages.find(
    (item) => item.role === "link",
  );
  const chainTip = swappedSegments.body.appendages.find(
    (item) => item.parentNodeId === chainRoot.nodeId,
  );
  [chainRoot.length, chainTip.length] = [chainTip.length, chainRoot.length];
  assert.notEqual(
    describeModuleScene(swappedSegments),
    describeModuleScene(scales.scene),
  );
});

test("scene eligibility search counts total unmodified draws with deterministic seeds and exhaustion", () => {
  for (const [seed, attempts] of [
    [1, 79],
    [1000, 260],
    [2000, 164],
  ]) {
    const packet = generateModuleSceneAuthoring(GRAPH_COVERING_CATALOGUE, seed);
    assert.equal(packet.status, "resolved");
    assert.equal(packet.generation.attempts, attempts);
    assert.equal(packet.generation.winningSeed, (seed + attempts - 1) >>> 0);
    assert.equal(
      Object.values(packet.generation.rejected).reduce(
        (sum, count) => sum + count,
        0,
      ),
      attempts - 1,
    );
    assert.equal(packet.generation.seedSequence.length, attempts);
    assert.equal(
      packet.input.genome.origin.seed,
      packet.generation.winningSeed,
    );
    const repeat = generateModuleSceneAuthoring(GRAPH_COVERING_CATALOGUE, seed);
    assert.equal(repeat.resultDigest, packet.resultDigest);
    assert.equal(repeat.scene.sceneDigest, packet.scene.sceneDigest);
  }
  const exhausted = generateModuleSceneAuthoring(GRAPH_COVERING_CATALOGUE, 1, {
    maxAttempts: 1,
  });
  assert.equal(exhausted.status, "rejected");
  assert.equal(exhausted.errors[0].code, "generation-exhausted");
  assert.equal(exhausted.generation.attempts, 1);
  assert.equal(
    Object.values(exhausted.generation.rejected).reduce(
      (sum, count) => sum + count,
      0,
    ),
    1,
  );
  const wrapped = generateModuleSceneAuthoring(
    GRAPH_COVERING_CATALOGUE,
    0xffffffff,
    { maxAttempts: 2 },
  );
  assert.deepEqual(wrapped.generation.seedSequence, [0xffffffff, 0]);
});

test("stage rejection retains verified genetic packet but no ready scene", () => {
  const input = structuredClone(inputs[0].input);
  input.genome.loci["development.symmetry"] = ["radial", "radial"];
  const rejected = resolveModuleSceneAuthoring(input);
  assert.equal(rejected.status, "rejected");
  assert.equal(rejected.stage, "body");
  assert.equal(rejected.sourcePacket.status, "resolved");
  assert.equal(isResolvedAuthoringPacket(rejected), false);
  assert.equal(
    isResolvedAuthoringPacket(resolveAuthoring(inputs[0].input)),
    false,
  );
  assert.equal(
    authoringRoute(input.catalogue, "evaluate"),
    "/api/module-scene/evaluate",
  );
});

test("browser scene comparison reframes server SVGs to equal world scale without changing inner geometry", () => {
  const packets = [scenePacket("single-skin"), scenePacket("axial-scales")];
  const camera = sharedSceneCamera(packets);
  const mappedWorldScales = packets.map((packet) => {
    const markup = scenePreviewMarkup(packet, camera);
    assert.equal(
      markup.replace(/viewBox="[^"]*"/, ""),
      packet.reference.svg.replace(/viewBox="[^"]*"/, ""),
    );
    const viewBox = markup
      .match(/viewBox="([^"]*)"/)[1]
      .split(" ")
      .map(Number);
    return (512 / viewBox[2]) * packet.reference.mapping.scale;
  });
  assert.ok(Math.abs(mappedWorldScales[0] - mappedWorldScales[1]) < 1e-10);
  assert.equal(
    sceneCausalSummary(packets[1].scene, "appearance.covering-kind").covering,
    true,
  );
  assert.equal(
    sceneCausalSummary(packets[0].scene, "structure.ocular-pair").ocular,
    true,
  );
});

test("actual compact replay HTTP fits the unchanged body guard; full artifacts cannot bypass it", async () => {
  const server = makeServer();
  server.listen(0, "127.0.0.1");
  await once(server, "listening");
  try {
    const url = `http://127.0.0.1:${server.address().port}`;
    const packet = scenePacket("single-scales");
    const compact = await fetch(`${url}/api/module-scene/replay`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(compactSceneReplayEnvelope(packet)),
    });
    assert.equal(compact.status, 200);
    assert.equal(
      (await compact.json()).scene.sceneDigest,
      packet.scene.sceneDigest,
    );
    const large = await fetch(`${url}/api/module-scene/replay`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(packet),
    });
    assert.equal(large.status, 413);
    assert.deepEqual(await large.json(), {
      error: "Experiment exceeds 64 KiB",
    });
    const afterOversize = await fetch(`${url}/api/module-scene/replay`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(compactSceneReplayEnvelope(packet)),
    });
    assert.equal(afterOversize.status, 200);
    const old = await fetch(`${url}/api/authoring/catalogue`);
    assert.equal((await old.json()).packages.length, 3);
    const optional = await fetch(`${url}/api/module-scene/catalogue`);
    assert.equal((await optional.json()).catalogue.loci.length, 54);
  } finally {
    const closed = once(server, "close");
    server.close();
    server.closeIdleConnections();
    await closed;
  }
});
