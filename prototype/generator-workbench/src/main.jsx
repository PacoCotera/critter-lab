import React, { useEffect, useRef, useState } from "react";
import { createRoot } from "react-dom/client";
import {
  Accordion,
  Alert,
  AppShell,
  Badge,
  Button,
  Code,
  Divider,
  Group,
  JsonInput,
  MantineProvider,
  Modal,
  NavLink,
  NumberInput,
  Paper,
  ScrollArea,
  Select,
  Stack,
  Table,
  Tabs,
  Text,
  TextInput,
  Textarea,
  Title,
} from "@mantine/core";
import "@mantine/core/styles.css";
import "./workbench.css";
import {
  drawAuthoringCreature,
  drawGenomeField,
  compareResults,
} from "../presentation.mjs";
import { SURFACE_DETAIL_PROJECTION_VERSION } from "../family-presentation.mjs";
import {
  scopedLoci,
  scopedSelection,
  copyLabels,
  outputText,
  causalSummary,
  sharedPreviewCamera,
  freshGenerationSeed,
  isResolvedAuthoringPacket,
  initialAuthoringInputs,
  authoringPackageLabel,
  unconsumedOutputNotice,
} from "../authoring-ui.mjs";

const clone = (value) => structuredClone(value);
const pretty = (value) => JSON.stringify(value, null, 2);
const familyLabel = (id) => id.replaceAll("-", " ");
const storageKey = "critter-authoring-records-v1";
const draftKey = "critter-authoring-draft-v1";

async function request(path, input) {
  const response = await fetch(
    path,
    input === undefined
      ? {}
      : {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify(input),
        },
  );
  const data = await response.json();
  if (!response.ok) {
    const error = new Error(
      data.errors
        ?.map((error) => `${error.path}: ${error.message}`)
        .join("\n") ??
        data.error ??
        "Request rejected.",
    );
    error.code = data.errors?.[0]?.code;
    throw error;
  }
  return data;
}

function SvgView({ markup, onSelect, compact = false }) {
  return (
    <div
      className={compact ? "svg-view creature-view" : "svg-view"}
      onClick={
        onSelect
          ? (event) => {
              const item = event.target.closest("[data-locus]");
              if (item) onSelect(item.getAttribute("data-locus"));
            }
          : undefined
      }
      dangerouslySetInnerHTML={{ __html: markup }}
    />
  );
}

function Workbench() {
  const [catalogue, setCatalogue] = useState(null);
  const [packages, setPackages] = useState([]);
  const [familyExamples, setFamilyExamples] = useState([]);
  const [petExamples, setPetExamples] = useState([]);
  const [draft, setDraft] = useState(null);
  const [genome, setGenome] = useState(null);
  const [context, setContext] = useState(null);
  const [packet, setPacket] = useState(null);
  const [pinned, setPinned] = useState(null);
  const [records, setRecords] = useState([]);
  const [view, setView] = useState("experiment");
  const [family, setFamily] = useState("all");
  const [search, setSearch] = useState("");
  const [selected, setSelected] = useState("development.axial-repeat");
  const [edited, setEdited] = useState("");
  const [seed, setSeed] = useState(1);
  const [expressionSeed, setExpressionSeed] = useState(7);
  const [message, setMessage] = useState("Loading pinned content…");
  const [errorDetails, setErrorDetails] = useState("");
  const [failed, setFailed] = useState(false);
  const [busy, setBusy] = useState(false);
  const [json, setJson] = useState(null);
  const [jsonText, setJsonText] = useState("");
  const [batch, setBatch] = useState([]);
  const inputRevision = useRef(0);
  const processing = useRef(false);

  useEffect(() => {
    request("/api/authoring/catalogue")
      .then((data) => {
        setPackages(data.packages ?? []);
        setFamilyExamples(data.familyExamples ?? []);
        setPetExamples(data.petExamples ?? []);
        const initial = initialAuthoringInputs(data);
        setCatalogue(initial.catalogue);
        setDraft(clone(initial.catalogue));
        setGenome(initial.genome);
        setContext(initial.context);
        setEdited(pretty(initial.catalogue.loci[0]));
        setMessage(
          "Ready to generate a creature. Known examples are also available.",
        );
        try {
          setRecords(JSON.parse(localStorage.getItem(storageKey) ?? "[]"));
        } catch {
          setMessage(
            "Local records could not be read; current catalogue remains available.",
          );
        }
      })
      .catch((error) => {
        setMessage(error.message);
        setFailed(true);
      });
  }, []);

  function selectLocus(id) {
    const current = (view === "compendium" ? draft : catalogue)?.loci.find(
      (item) => item.id === id,
    );
    if (!current) return;
    if (family !== "all" && family !== current.family)
      setFamily(current.family);
    if (!scopedLoci({ loci: [current] }, "all", search).length) setSearch("");
    setSelected(id);
    const locus = draft?.loci.find((item) => item.id === id);
    if (locus) setEdited(pretty(locus));
  }
  function changeScope(dimension, query = search) {
    setFamily(dimension);
    setSearch(query);
    const next = scopedSelection(
      scopedLoci(view === "compendium" ? draft : catalogue, dimension, query),
      selected,
    );
    setSelected(next);
    const record = (view === "compendium" ? draft : catalogue)?.loci.find(
      (item) => item.id === next,
    );
    if (record) setEdited(pretty(record));
  }
  function changeWorkspace(nextView) {
    if (["experiment", "compendium"].includes(nextView)) {
      const content = nextView === "compendium" ? draft : catalogue;
      let options = scopedLoci(content, family, search);
      if (!options.length) {
        setFamily("all");
        setSearch("");
        options = content.loci;
      }
      const nextSelected = scopedSelection(options, selected);
      setSelected(nextSelected);
      const record = draft.loci.find((item) => item.id === nextSelected);
      if (record) setEdited(pretty(record));
    }
    setView(nextView);
  }
  function invalidate() {
    inputRevision.current += 1;
    setPacket(null);
    setFailed(false);
    setErrorDetails("");
    setMessage(
      "Changes ready to preview. Resolve to update the current output.",
    );
  }
  function switchPackage(id) {
    const item = packages.find((item) => item.catalogue.id === id);
    if (!item || busy) return;
    invalidate();
    setCatalogue(clone(item.catalogue));
    setDraft(clone(item.catalogue));
    const example =
      item.catalogue.ruleVersion === "continuous-pet/1"
        ? petExamples[0]
        : item.catalogue.ruleVersion === "continuous-static/1"
          ? familyExamples[0]
          : null;
    setGenome(clone(example?.genome ?? item.defaultGeneration.genome));
    setContext(
      example?.context ?? {
        stage: "adult",
        condition: "rested",
        environment: "reference",
        medium: "ground",
      },
    );
    setSelected(item.catalogue.loci[0].id);
    setFamily("all");
    setSearch("");
    setEdited(pretty(item.catalogue.loci[0]));
    setBatch([]);
  }
  function loadFamilyExample(name) {
    const example = [...familyExamples, ...petExamples].find(
      (item) => item.name === name,
    );
    if (!example || processing.current) return;
    invalidate();
    const revision = inputRevision.current;
    const exactInput = {
      catalogue: clone(catalogue),
      genome: clone(example.genome),
      context: clone(example.context),
      expressionSeed: null,
    };
    setGenome(exactInput.genome);
    setContext(exactInput.context);
    return run(async () => {
      const data = await request("/api/authoring/evaluate", exactInput);
      if (revision !== inputRevision.current) return;
      setPacket(data);
      setMessage("Example loaded. Preview ready.");
    });
  }
  async function run(operation) {
    if (processing.current) return;
    processing.current = true;
    setBusy(true);
    setFailed(false);
    setErrorDetails("");
    const revision = inputRevision.current;
    try {
      await operation();
    } catch (error) {
      if (revision === inputRevision.current) {
        setPacket(null);
        setMessage(
          error.code === "generation-exhausted"
            ? "No valid creature found in this bounded search. Generate again for a new seed."
            : error.message,
        );
        setErrorDetails(error.message);
        setFailed(true);
      }
    } finally {
      processing.current = false;
      setBusy(false);
    }
  }
  async function resolve(expression = null) {
    const revision = inputRevision.current;
    setPacket(null);
    const data = await request("/api/authoring/evaluate", {
      catalogue,
      genome,
      context,
      expressionSeed: expression,
    });
    if (revision !== inputRevision.current) return;
    setPacket(data);
    setMessage("Preview updated.");
  }
  function generateFresh() {
    if (processing.current) return;
    const randomSeed = crypto.getRandomValues(new Uint32Array(1))[0];
    const generationSeed = freshGenerationSeed(Number(seed), randomSeed);
    setSeed(generationSeed);
    return run(() => generate(generationSeed));
  }
  async function generate(generationSeed = Number(seed)) {
    const revision = inputRevision.current;
    setPacket(null);
    const data = await request("/api/authoring/generate", {
      catalogue,
      seed: generationSeed,
      maxAttempts: 1024,
    });
    if (revision !== inputRevision.current) return;
    if (!isResolvedAuthoringPacket(data))
      throw new Error(
        "Generation returned no resolved creature. Try a new seed.",
      );
    setGenome(data.input.genome);
    setContext(data.input.context);
    setPacket(data);
    setMessage("New genome generated. Preview ready.");
  }
  function exportJson(kind, value) {
    setJson(kind);
    setJsonText(pretty(value));
  }
  function editCopy(locusId, index, value) {
    if (!genome?.loci?.[locusId]) return;
    const next = clone(genome);
    next.loci[locusId][index] = value;
    setGenome(next);
    invalidate();
  }
  async function saveDraftRecord() {
    const item = JSON.parse(edited);
    const next = clone(draft);
    next.version = Math.max(catalogue.version + 1, next.version);
    const index = next.loci.findIndex((locus) => locus.id === selected);
    if (item.id !== selected && next.loci.some((locus) => locus.id === item.id))
      throw new Error(
        "Duplicate ID. Display names and aliases can change without changing IDs.",
      );
    next.loci[index] = item;
    await request("/api/authoring/validate", next);
    localStorage.setItem(draftKey, pretty(next));
    setDraft(next);
    setSelected(item.id);
    setFamily("all");
    setSearch("");
    setEdited(pretty(item));
    setMessage(
      `Saved validated-schema draft v${next.version}; pinned package/experiment unchanged. This is not product approval.`,
    );
  }
  async function useDraft() {
    const revision = inputRevision.current;
    await request("/api/authoring/validate", draft);
    if (revision !== inputRevision.current) return;
    const next = clone(genome);
    next.contentId = draft.id;
    next.contentVersion = draft.version;
    // Content edits are explicit. Existing copies are never repaired to make a new package pass.
    setCatalogue(clone(draft));
    setGenome(next);
    invalidate();
    setFamily("all");
    setSearch("");
    setSelected(scopedSelection(draft.loci, selected));
    setView("experiment");
    setMessage(
      "Explicit draft package selected; resolve existing copies or generate a new genome. Retained records remain unchanged.",
    );
  }
  function saveRecord() {
    const next = [
      clone(packet),
      ...records.filter((item) => item.recordId !== packet.recordId),
    ].slice(0, 8);
    localStorage.setItem(storageKey, JSON.stringify(next));
    setRecords(next);
    setMessage(
      "Saved exact record locally. Eight most recent records retained; export important experiments.",
    );
  }
  async function importRecord() {
    const revision = inputRevision.current;
    if (jsonText.length > 1000000)
      throw new Error("Import exceeds the one-megabyte local record limit.");
    const imported = JSON.parse(jsonText);
    if (imported.schemaVersion === "critter-catalogue/1") {
      await request("/api/authoring/validate", imported);
      if (revision !== inputRevision.current) return;
      setDraft(clone(imported));
      setFamily("all");
      setSearch("");
      const nextSelected = scopedSelection(imported.loci, selected);
      setSelected(nextSelected);
      setEdited(pretty(imported.loci.find((item) => item.id === nextSelected)));
      localStorage.setItem(draftKey, pretty(imported));
      setMessage(
        "Imported catalogue as separate draft; current experiment unchanged.",
      );
    } else {
      const data = await request("/api/authoring/replay", {
        schemaVersion: imported.schemaVersion,
        input: imported.input,
        inputDigest: imported.inputDigest,
        resultDigest: imported.resultDigest,
      });
      if (revision !== inputRevision.current) return;
      setCatalogue(data.input.catalogue);
      setDraft(clone(data.input.catalogue));
      setGenome(data.input.genome);
      setContext(data.input.context);
      setPacket(data);
      const retainedSelected =
        data.input.catalogue.loci.find((item) => item.id === selected) ??
        data.input.catalogue.loci[0];
      setSelected(retainedSelected.id);
      setFamily("all");
      setSearch("");
      setEdited(pretty(retainedSelected));
      setView("experiment");
      setMessage(
        "Digest replay verified from inputs. Embedded output/SVG/prompt was not trusted.",
      );
    }
    setJson(null);
  }
  async function runBatch() {
    const revision = inputRevision.current;
    const items = [];
    for (let index = 0; index < 6; index++)
      items.push(
        await request("/api/authoring/generate", {
          catalogue,
          seed: Number(seed) + index,
        }),
      );
    if (revision !== inputRevision.current) return;
    setBatch(items);
    setMessage(
      "Six retained generated experiments; classifications describe outputs, not selected input classes.",
    );
  }

  if (!catalogue)
    return (
      <Paper p="xl">
        <Title order={2}>Genome authoring workbench</Title>
        <Alert color={failed ? "red" : "sage"}>{message}</Alert>
      </Paper>
    );
  const locus = catalogue.loci.find((item) => item.id === selected);
  const draftLocus = draft.loci.find((item) => item.id === selected);
  const selectedFact = packet?.result.facts.find(
    (fact) => fact.locusId === selected,
  );
  const filtered = scopedLoci(draft, family, search);
  const genomeLoci = scopedLoci(catalogue, family, search);
  const cause = causalSummary(catalogue, packet?.result, selected);
  const consumerNotice = unconsumedOutputNotice(catalogue, selected);
  const previewCamera = sharedPreviewCamera([packet?.result].filter(Boolean));
  const diagnosticViewOptions = (camera) => ({
    projectionVersion: SURFACE_DETAIL_PROJECTION_VERSION,
    ...(camera ? { camera } : {}),
  });
  const compatibleComparison =
    pinned &&
    packet &&
    pinned.ruleVersion === packet.ruleVersion &&
    Boolean(packet.result.graph.exterior);
  const comparisonCamera = compatibleComparison
    ? sharedPreviewCamera([pinned.result, packet.result])
    : null;
  const differences = compareResults(pinned?.result, packet?.result);
  const setField = (key, value) => {
    try {
      const next = JSON.parse(edited);
      next[key] = value;
      setEdited(pretty(next));
    } catch {
      setMessage("Restore valid record JSON before using structured fields.");
    }
  };
  let formRecord;
  try {
    formRecord = JSON.parse(edited);
  } catch {
    formRecord = draftLocus;
  }

  return (
    <AppShell
      header={{ height: 74 }}
      navbar={{ width: 190, breakpoint: "sm" }}
      padding="lg"
    >
      <AppShell.Header px="lg">
        <Group justify="space-between" h="100%">
          <div>
            <Title order={3}>Critter Lab · Genome authoring</Title>
            <Text size="xs" c="dimmed">
              Adult developer tool · provisional shared engine · game/device UI
              unchanged
            </Text>
          </div>
          <Group>
            <Select
              size="xs"
              aria-label="Content package"
              disabled={busy}
              value={catalogue.id}
              data={packages.map((item) => ({
                value: item.catalogue.id,
                label: `${authoringPackageLabel(item.catalogue)} · v${item.catalogue.version}`,
              }))}
              onChange={switchPackage}
            />
            <Badge variant="light">
              {catalogue.id} v{catalogue.version}
            </Badge>
            <Button
              component="a"
              href="/legacy"
              target="_blank"
              variant="subtle"
              size="xs"
            >
              Preserved Pip proof
            </Button>
          </Group>
        </Group>
      </AppShell.Header>
      <AppShell.Navbar p="md">
        <Stack gap="xs">
          <Text size="xs" fw={700} c="dimmed">
            AUTHOR & INVESTIGATE
          </Text>
          {[
            ["experiment", "Genome"],
            ["compendium", "Compendium"],
            ["compare", "Pinned comparison"],
            ["records", "Saved records"],
            ["batch", "Generation batch"],
          ].map(([id, label]) => (
            <NavLink
              key={id}
              label={label}
              active={view === id}
              onClick={() => changeWorkspace(id)}
            />
          ))}
          <Divider />
          <Text size="xs" c="dimmed">
            {catalogue.loci.length} proposed records ·{" "}
            {
              catalogue.loci.filter((item) => item.status === "validated")
                .length
            }{" "}
            executable ·{" "}
            {catalogue.loci.filter((item) => item.status === "draft").length}{" "}
            draft candidates
          </Text>
          <Button
            component="a"
            href="https://github.com/PacoCotera/critter-lab/blob/codex/genome-art-reset/prototype/generator-workbench/evidence/art-reset/README.md"
            target="_blank"
            rel="noopener noreferrer"
            variant="subtle"
            size="xs"
          >
            Character art studies
          </Button>
        </Stack>
      </AppShell.Navbar>
      <AppShell.Main>
        <Alert
          mb="md"
          color={failed ? "red" : "sage"}
          role="status"
          className="status"
        >
          {message}
        </Alert>
        {failed && errorDetails !== message && (
          <Accordion mb="md">
            <Accordion.Item value="request-error">
              <Accordion.Control>Search details</Accordion.Control>
              <Accordion.Panel>
                <Code block className="sequence">
                  {errorDetails}
                </Code>
              </Accordion.Panel>
            </Accordion.Item>
          </Accordion>
        )}
        {view === "compendium" && (
          <>
            <Group justify="space-between" mb="md">
              <div>
                <Title order={2}>Locus compendium</Title>
                <Text c="dimmed">
                  Names, taxonomy, alleles, operators and dependencies · edits
                  are separate versioned drafts.
                </Text>
              </div>
              <Group>
                <Button
                  variant="light"
                  onClick={() => exportJson("Catalogue draft", draft)}
                >
                  Export draft
                </Button>
                <Button disabled={busy} onClick={() => run(useDraft)}>
                  Use draft in experiment
                </Button>
              </Group>
            </Group>
            <div className="catalogue-layout">
              <Paper withBorder p="md">
                <Stack>
                  <TextInput
                    label="Search loci / names / aliases"
                    value={search}
                    onChange={(event) =>
                      changeScope(family, event.target.value)
                    }
                  />
                  <Select
                    label="Dimension family"
                    data={[
                      { value: "all", label: "All eleven families" },
                      ...catalogue.families.map((item) => ({
                        value: item.id,
                        label: familyLabel(item.id),
                      })),
                    ]}
                    value={family}
                    onChange={(dimension) => changeScope(dimension)}
                  />
                  <Text size="xs">
                    {filtered.length} records · full inventory, including drafts
                  </Text>
                  <ScrollArea h={540}>
                    {filtered.map((item) => (
                      <NavLink
                        key={item.id}
                        active={item.id === selected}
                        label={item.label}
                        description={`${item.id} · ${item.status}`}
                        onClick={() => selectLocus(item.id)}
                      />
                    ))}
                    {filtered.length === 0 && (
                      <Text size="sm">
                        No locus records in this family. Its modeling gaps
                        remain in coverage.
                      </Text>
                    )}
                  </ScrollArea>
                </Stack>
              </Paper>
              <Paper withBorder p="lg">
                {draftLocus ? (
                  <>
                    <Group justify="space-between">
                      <Title order={3}>{draftLocus?.label ?? "Record"}</Title>
                      <Badge
                        color={draftLocus?.status === "draft" ? "gray" : "sage"}
                      >
                        {draftLocus?.status}
                      </Badge>
                    </Group>
                    <Text size="sm" c="dimmed" mb="lg">
                      Stable ID and versions preserve references. A label rename
                      does not change inherited copies.
                    </Text>
                    <div className="form-grid">
                      <TextInput
                        label="Display name"
                        value={formRecord?.label ?? ""}
                        onChange={(event) =>
                          setField("label", event.target.value)
                        }
                      />
                      <TextInput
                        label="Aliases (comma separated)"
                        value={formRecord?.aliases?.join(", ") ?? ""}
                        onChange={(event) =>
                          setField(
                            "aliases",
                            event.target.value
                              .split(",")
                              .map((item) => item.trim())
                              .filter(Boolean),
                          )
                        }
                      />
                      <Select
                        label="Primary family"
                        data={catalogue.families.map((item) => ({
                          value: item.id,
                          label: familyLabel(item.id),
                        }))}
                        value={formRecord?.family}
                        onChange={(value) => setField("family", value)}
                      />
                      <Select
                        label="Draft / engine-validated"
                        data={["draft", "validated"]}
                        value={formRecord?.status}
                        onChange={(value) => setField("status", value)}
                      />
                    </div>
                    <Text size="sm" mt="md">
                      {draftLocus?.purpose}
                    </Text>
                    <Text size="xs" c="dimmed">
                      Where used:{" "}
                      {draft.loci
                        .filter((item) => item.requires.includes(selected))
                        .map((item) => item.id)
                        .join(", ") ||
                        "No direct prerequisites reference this locus."}
                    </Text>
                    <JsonInput
                      mt="md"
                      label="Complete record · copies, allele values/maps, applicability, bounds, examples and visual binding"
                      value={edited}
                      onChange={setEdited}
                      autosize
                      minRows={12}
                      maxRows={24}
                      formatOnBlur
                      validationError="Invalid JSON"
                    />
                    <Group mt="md">
                      <Button
                        disabled={busy}
                        onClick={() => run(saveDraftRecord)}
                      >
                        Validate & save record to draft
                      </Button>
                      <Button
                        variant="subtle"
                        onClick={() => setEdited(pretty(draftLocus))}
                      >
                        Discard unsaved record edit
                      </Button>
                    </Group>
                    <Text size="xs" c="dimmed" mt="sm">
                      Operator/range changes outside implemented semantics
                      reject. Changing status alone cannot promote a candidate.
                    </Text>
                  </>
                ) : (
                  <Text>Choose a record from All or change the search.</Text>
                )}
              </Paper>
            </div>
          </>
        )}
        {view === "experiment" && (
          <>
            <Group justify="space-between" mb="md" align="end">
              <div>
                <Title order={2}>Genome</Title>
                <Text c="dimmed">
                  Generate a creature, then explore its inherited copies and
                  their effects.
                </Text>
              </div>
              <Group>
                <Button disabled={busy} onClick={generateFresh}>
                  Generate creature
                </Button>
                <Button
                  disabled={busy || !genome}
                  variant="light"
                  onClick={() => run(() => resolve(null))}
                >
                  Resolve genome
                </Button>
              </Group>
            </Group>
            <div className="genome-summaries">
              <Paper withBorder p="sm">
                <Text size="xs" c="dimmed">
                  BASELINE FOUNDATION
                </Text>
                <Text size="sm" fw={600}>
                  {catalogue.id} · v{catalogue.version}
                </Text>
                <Text size="xs">
                  {
                    catalogue.loci.filter((item) => item.status === "validated")
                      .length
                  }{" "}
                  supported contributor definitions
                </Text>
              </Paper>
              <Paper withBorder p="sm">
                <Text size="xs" c="dimmed">
                  INHERITED COPIES
                </Text>
                <Text size="sm" fw={600}>
                  {Object.keys(genome?.loci ?? {}).length} recorded loci
                </Text>
                <Text size="xs">
                  {Object.values(genome?.loci ?? {}).flat().length} retained
                  copies, including inactive ones
                </Text>
              </Paper>
              <Paper withBorder p="sm">
                <Text size="xs" c="dimmed">
                  RESOLVED EXPRESSION
                </Text>
                <Text size="sm" fw={600}>
                  {packet
                    ? `${packet.result.facts.filter((item) => item.state === "expressed").length} expressed outputs`
                    : "Not resolved"}
                </Text>
                <Text size="xs">
                  {packet
                    ? `${packet.result.facts.filter((item) => item.state !== "expressed").length} inactive or suppressed outputs`
                    : "Resolve to inspect the current inputs"}
                </Text>
              </Paper>
            </div>
            <div className="dimension-index" aria-label="Genome dimensions">
              {[
                { id: "all", label: "All" },
                ...catalogue.families.map((item) => ({
                  id: item.id,
                  label: familyLabel(item.id),
                })),
              ].map((item) => (
                <Button
                  key={item.id}
                  size="xs"
                  variant={family === item.id ? "filled" : "light"}
                  onClick={() => changeScope(item.id)}
                >
                  {item.label}
                  <span className="dimension-count">
                    {scopedLoci(catalogue, item.id).length}
                  </span>
                </Button>
              ))}
            </div>
            <div className="genome-layout">
              <Paper withBorder p="md" className="locus-browser">
                <TextInput
                  aria-label="Search genome loci"
                  placeholder="Search names or loci"
                  value={search}
                  onChange={(event) => changeScope(family, event.target.value)}
                />
                <Group justify="space-between" my="sm">
                  <Text size="sm" fw={600}>
                    {family === "all" ? "Whole genome" : familyLabel(family)}
                  </Text>
                  <Text size="xs" c="dimmed">
                    {genomeLoci.length} records
                  </Text>
                </Group>
                <ScrollArea h={470}>
                  <Stack gap="xs">
                    {genomeLoci.map((item) => {
                      const fact = packet?.result.facts.find(
                        (entry) => entry.locusId === item.id,
                      );
                      const copies = copyLabels(item, genome);
                      return (
                        <button
                          type="button"
                          key={item.id}
                          className={`locus-choice ${selected === item.id ? "selected" : ""}`}
                          onClick={() => selectLocus(item.id)}
                        >
                          <span className="locus-title">{item.label}</span>
                          <span className="locus-copy-labels">
                            {copies.length
                              ? copies.join(" / ")
                              : "No executable copies"}
                          </span>
                          <span className="locus-state">
                            {item.status === "draft"
                              ? "Candidate"
                              : (fact?.state ?? "Not resolved")}
                          </span>
                        </button>
                      );
                    })}
                    {!genomeLoci.length && (
                      <Text size="sm" c="dimmed">
                        No records in this scope. Choose All or clear the
                        search.
                      </Text>
                    )}
                  </Stack>
                </ScrollArea>
              </Paper>
              <div className="selected-preview">
                <Paper withBorder p="md" className="selected-editor">
                  {locus ? (
                    <>
                      <Text size="xs" c="dimmed">
                        SELECTED LOCUS · {familyLabel(locus.family)}
                      </Text>
                      <Title order={3} mt="xs">
                        {locus.label}
                      </Title>
                      <Text size="sm" mt="sm">
                        {locus.purpose}
                      </Text>
                      {locus.status === "validated" ? (
                        <div className="allele-editor">
                          {Array.from(
                            { length: locus.copyCount ?? 2 },
                            (_, index) => (
                              <div key={index} className="allele-choice-row">
                                <Text size="sm" fw={600}>
                                  Copy {index + 1}
                                </Text>
                                <Group gap={4}>
                                  {locus.alleles.map((allele) => (
                                    <Button
                                      key={allele.id}
                                      size="xs"
                                      px="xs"
                                      variant={
                                        genome?.loci[locus.id]?.[index] ===
                                        allele.id
                                          ? "filled"
                                          : "light"
                                      }
                                      disabled={busy || !genome?.loci[locus.id]}
                                      aria-pressed={
                                        genome?.loci[locus.id]?.[index] ===
                                        allele.id
                                      }
                                      onClick={() =>
                                        editCopy(locus.id, index, allele.id)
                                      }
                                    >
                                      {allele.label}
                                    </Button>
                                  ))}
                                </Group>
                              </div>
                            ),
                          )}
                          {!genome?.loci[locus.id] && (
                            <Text size="sm" c="orange">
                              Copies are missing from this input. Generate a
                              genome for this package.
                            </Text>
                          )}
                        </div>
                      ) : (
                        <Alert color="gray" mt="sm">
                          Candidate record · no executable copy or expressed
                          output.
                        </Alert>
                      )}
                      <Divider my="md" />
                      <Text size="xs" fw={700} c="dimmed">
                        DIRECT OUTPUT
                      </Text>
                      <Text fw={600} mt="xs">
                        {outputText(selectedFact)}
                      </Text>
                      <Badge
                        mt="xs"
                        color={
                          selectedFact?.state === "expressed" ? "sage" : "gray"
                        }
                      >
                        {selectedFact?.state ??
                          (locus.status === "draft"
                            ? "unsupported"
                            : "awaiting resolve")}
                      </Badge>
                      {selectedFact && consumerNotice && (
                        <Text size="sm" c="orange" mt="sm">
                          {consumerNotice}
                        </Text>
                      )}
                      {!!cause?.prerequisites.length && (
                        <>
                          <Text size="xs" fw={700} c="dimmed" mt="lg">
                            PREREQUISITES THIS OUTPUT USES
                          </Text>
                          <Group gap="xs" mt="xs">
                            {cause.prerequisites.map((item) => (
                              <Button
                                size="compact-xs"
                                variant="subtle"
                                key={item.id}
                                onClick={() => selectLocus(item.id)}
                              >
                                {item.label}
                              </Button>
                            ))}
                          </Group>
                        </>
                      )}
                      {packet && (
                        <>
                          <Text size="xs" fw={700} c="dimmed" mt="lg">
                            DOWNSTREAM INVOLVEMENT
                          </Text>
                          <Text size="sm" mt="xs">
                            {cause?.targets.length ?? 0} body/feature nodes
                            include this locus in their trace.
                          </Text>
                          {cause?.coveringInvolvement && (
                            <Text size="sm" mt="xs">
                              {cause.coveringContext
                                ? "Covering uses this locus as an exclusion or geometry dependency."
                                : `The ${packet.result.graph.covering.kind} covering includes this locus in its material trace.`}
                            </Text>
                          )}
                          {!!cause?.dependents.length && (
                            <Text size="xs" c="dimmed" mt="xs">
                              Other dependent outputs:{" "}
                              {cause.dependents.slice(0, 5).join(", ")}
                              {cause.dependents.length > 5
                                ? ` +${cause.dependents.length - 5} more in advanced inspection`
                                : ""}
                            </Text>
                          )}
                        </>
                      )}
                    </>
                  ) : (
                    <>
                      <Title order={3}>Choose a locus</Title>
                      <Text size="sm" mt="sm">
                        This dimension has no matching records. All keeps the
                        complete catalogue reachable.
                      </Text>
                    </>
                  )}
                </Paper>
                <Paper withBorder p="md" className="structural-preview">
                  <Group justify="space-between">
                    <Title order={4}>Structural preview</Title>
                    <Badge color="gray">Diagnostic geometry</Badge>
                  </Group>
                  {packet ? (
                    <SvgView
                      compact
                      onSelect={selectLocus}
                      markup={drawAuthoringCreature(
                        packet.result,
                        selected,
                        diagnosticViewOptions(
                          packet.result.graph.exterior ? previewCamera : null,
                        ),
                      )}
                    />
                  ) : (
                    <div className="empty-result">
                      <Title order={3}>No current preview</Title>
                      <Text size="sm">
                        Generate a creature or load a known example.
                      </Text>
                    </div>
                  )}
                  <Text size="xs" c="dimmed">
                    Amber shows direct and dependency involvement in the
                    retained construction.
                  </Text>
                  <Text size="xs" c="dimmed" mt="xs">
                    This structural diagram is not generated game art.
                  </Text>
                  <Text size="xs" c="dimmed" mt="xs">
                    Display: {SURFACE_DETAIL_PROJECTION_VERSION}.{" "}
                    {catalogue.ruleVersion === "developmental-analytic/1"
                      ? "Broad graph assembly; face and covering modules are not modeled in this package."
                      : "Narrow continuous-body calibration; this is not broad anatomy generation."}
                  </Text>
                  {packet && (
                    <Group mt="md" gap="xs">
                      <Button
                        size="xs"
                        variant="light"
                        onClick={() => setPinned(clone(packet))}
                      >
                        Pin comparison
                      </Button>
                      <Button
                        size="xs"
                        variant="light"
                        onClick={() => run(async () => saveRecord())}
                      >
                        Save record
                      </Button>
                      <Button
                        size="xs"
                        variant="light"
                        onClick={() =>
                          exportJson("Retained experiment", packet)
                        }
                      >
                        Export record
                      </Button>
                    </Group>
                  )}
                  {pinned && (
                    <Button
                      variant="subtle"
                      size="xs"
                      mt="sm"
                      onClick={() => setView("compare")}
                    >
                      Open retained comparison
                    </Button>
                  )}
                </Paper>
              </div>
            </div>
            <Accordion mt="md" variant="separated">
              <Accordion.Item value="examples">
                <Accordion.Control>
                  Known examples · load and resolve
                </Accordion.Control>
                <Accordion.Panel>
                  <Group>
                    {(catalogue.ruleVersion === "continuous-pet/1"
                      ? petExamples
                      : catalogue.ruleVersion === "continuous-static/1"
                        ? familyExamples
                        : []
                    ).map((example) => (
                      <Button
                        key={example.name}
                        size="sm"
                        variant="light"
                        disabled={busy}
                        onClick={() => loadFamilyExample(example.name)}
                      >
                        {example.name.replaceAll("-", " ")}
                      </Button>
                    ))}
                  </Group>
                  <Text size="xs" c="dimmed" mt="sm">
                    Controlled authoring inputs and copy edits; no species
                    selector.
                  </Text>
                </Accordion.Panel>
              </Accordion.Item>
              <Accordion.Item value="context">
                <Accordion.Control>Context and sampling</Accordion.Control>
                <Accordion.Panel>
                  <Group align="end">
                    <Select
                      label="Reference medium"
                      disabled={busy}
                      data={["ground", "air", "water"]}
                      value={context.medium}
                      onChange={(medium) => {
                        setContext({ ...context, medium });
                        invalidate();
                      }}
                    />
                    <NumberInput
                      label="Replay generation seed"
                      value={seed}
                      min={0}
                      max={4294967295}
                      onChange={setSeed}
                    />
                    <Button
                      disabled={busy}
                      variant="light"
                      onClick={() => run(() => generate(Number(seed)))}
                    >
                      Generate with this seed
                    </Button>
                    <NumberInput
                      label="Expression seed"
                      disabled={busy}
                      value={expressionSeed}
                      min={0}
                      max={4294967295}
                      onChange={(value) => {
                        setExpressionSeed(value);
                        invalidate();
                      }}
                    />
                    <Button
                      disabled={busy || !genome}
                      variant="light"
                      onClick={() => run(() => resolve(Number(expressionSeed)))}
                    >
                      Sample marking placement
                    </Button>
                  </Group>
                  <Text size="sm" c="dimmed" mt="sm">
                    Genome generation samples inherited copies. Expression
                    sampling changes permitted marking placement only; inherited
                    copies and other expression stay fixed.
                  </Text>
                  {packet?.generation && (
                    <Text size="xs" c="dimmed" mt="sm">
                      Accepted seed {packet.generation.seed} ·{" "}
                      {packet.generation.attempts} unmodified draws. Exact
                      copies and result are retained for replay.
                    </Text>
                  )}
                </Accordion.Panel>
              </Accordion.Item>
              <Accordion.Item value="whole">
                <Accordion.Control>
                  Whole genome · baseline, inherited copies and expression
                </Accordion.Control>
                <Accordion.Panel>
                  <Tabs defaultValue="inherited">
                    <Tabs.List>
                      <Tabs.Tab value="baseline">Baseline foundation</Tabs.Tab>
                      <Tabs.Tab value="inherited">Inherited copies</Tabs.Tab>
                      <Tabs.Tab value="expression">
                        Resolved expression
                      </Tabs.Tab>
                    </Tabs.List>
                    {["baseline", "inherited", "expression"].map((kind) => (
                      <Tabs.Panel key={kind} value={kind} pt="md">
                        {packet ? (
                          <>
                            <Accordion>
                              <Accordion.Item value="map">
                                <Accordion.Control>
                                  Inspect complete {kind} field
                                </Accordion.Control>
                                <Accordion.Panel>
                                  <SvgView
                                    markup={drawGenomeField(
                                      catalogue,
                                      genome,
                                      packet.result,
                                      kind,
                                      selected,
                                    )}
                                    onSelect={selectLocus}
                                  />
                                </Accordion.Panel>
                              </Accordion.Item>
                            </Accordion>
                            <Code block className="sequence" mt="sm">
                              {packet.representations[kind]}
                            </Code>
                          </>
                        ) : (
                          <Text size="sm">
                            Resolve to retain complete encodings. All
                            dimension/copy records remain accessible above.
                          </Text>
                        )}
                      </Tabs.Panel>
                    ))}
                  </Tabs>
                </Accordion.Panel>
              </Accordion.Item>
              <Accordion.Item value="advanced">
                <Accordion.Control>
                  Advanced inspection · traces, coverage, geometry and art
                  projection
                </Accordion.Control>
                <Accordion.Panel>
                  {packet ? (
                    <>
                      <Coverage catalogue={catalogue} packet={packet} />
                      <Accordion mt="md">
                        <Accordion.Item value="fact">
                          <Accordion.Control>
                            Selected output and dependencies
                          </Accordion.Control>
                          <Accordion.Panel>
                            <Code block className="sequence">
                              {pretty({
                                fact: selectedFact,
                                dependents: cause?.dependents,
                                targets: cause?.targets,
                              })}
                            </Code>
                          </Accordion.Panel>
                        </Accordion.Item>
                        <Accordion.Item value="geometry">
                          <Accordion.Control>
                            Complete geometry manifest
                          </Accordion.Control>
                          <Accordion.Panel>
                            <Code block className="sequence">
                              {pretty(
                                packet.geometryReference?.manifest ??
                                  packet.geometryReference,
                              )}
                            </Code>
                          </Accordion.Panel>
                        </Accordion.Item>
                        <Accordion.Item value="description">
                          <Accordion.Control>
                            Fact-derived description and supported motion
                          </Accordion.Control>
                          <Accordion.Panel>
                            <Text size="sm">{packet.description}</Text>
                            {packet.result.motion.map((item) => (
                              <Text key={item.id} size="sm" mt="xs">
                                {item.medium}: {item.status} ·{" "}
                                {item.reasons.join(" ")}
                              </Text>
                            ))}
                          </Accordion.Panel>
                        </Accordion.Item>
                        <Accordion.Item value="prompt">
                          <Accordion.Control>
                            Preset art projection
                          </Accordion.Control>
                          <Accordion.Panel>
                            {packet.prompt?.error && (
                              <Alert color="orange">
                                {packet.prompt.error}
                              </Alert>
                            )}
                            <Textarea
                              readOnly
                              autosize
                              minRows={6}
                              maxRows={16}
                              value={packet.prompt?.text ?? ""}
                            />
                          </Accordion.Panel>
                        </Accordion.Item>
                      </Accordion>
                    </>
                  ) : (
                    <Text size="sm">
                      No current resolved output. Inputs remain available to
                      edit and resolve.
                    </Text>
                  )}
                </Accordion.Panel>
              </Accordion.Item>
            </Accordion>
          </>
        )}

        {view === "compare" && (
          <Paper withBorder p="lg">
            <Title order={2}>Pinned comparison</Title>
            <Text c="dimmed">
              The retained pin survives edits, rejection and catalogue drafts.
            </Text>
            {pinned && (
              <div className="batch-grid">
                <div>
                  <Text size="sm">Pinned · {pinned.ruleVersion}</Text>
                  <SvgView
                    markup={drawAuthoringCreature(
                      pinned.result,
                      null,
                      diagnosticViewOptions(comparisonCamera),
                    )}
                    compact
                  />
                </div>
                {packet && (
                  <div>
                    <Text size="sm">Current · {packet.ruleVersion}</Text>
                    <SvgView
                      markup={drawAuthoringCreature(
                        packet.result,
                        null,
                        diagnosticViewOptions(comparisonCamera),
                      )}
                      compact
                    />
                  </div>
                )}
              </div>
            )}
            {!pinned && <Text>Pin a resolved experiment first.</Text>}
            {pinned && packet && (
              <Text size="sm" c="dimmed" mt="sm">
                {compatibleComparison
                  ? "Both subjects share one camera and world scale."
                  : "Separate preview framing; these images are not to a common world scale."}
              </Text>
            )}
            {pinned && packet && pinned.ruleVersion !== packet.ruleVersion && (
              <Alert color="orange" mt="sm">
                Different construction profiles; these are separate inspections,
                not compatible family or inherited-output comparison.
              </Alert>
            )}
            {pinned && packet && pinned.ruleVersion === packet.ruleVersion && (
              <>
                <Text size="sm" fw={600} mt="md">
                  {differences.filter((item) => item.changed).length} changed
                  outputs
                </Text>
                <Table striped>
                  <Table.Thead>
                    <Table.Tr>
                      <Table.Th>Output</Table.Th>
                      <Table.Th>Pinned</Table.Th>
                      <Table.Th>Current</Table.Th>
                    </Table.Tr>
                  </Table.Thead>
                  <Table.Tbody>
                    {differences
                      .filter((item) => item.changed)
                      .map((item) => (
                        <Table.Tr key={item.id}>
                          <Table.Td>
                            {item.id}{" "}
                            {item.changed && (
                              <Badge size="xs">
                                {item.valueChanged
                                  ? "value changed"
                                  : "state changed"}
                              </Badge>
                            )}
                          </Table.Td>
                          <Table.Td>
                            {JSON.stringify(item.before)} · {item.beforeState}
                          </Table.Td>
                          <Table.Td>
                            {JSON.stringify(item.after)} · {item.afterState}
                          </Table.Td>
                        </Table.Tr>
                      ))}
                  </Table.Tbody>
                </Table>
                <Accordion mt="md">
                  <Accordion.Item value="all-differences">
                    <Accordion.Control>
                      Inspect all output values and states
                    </Accordion.Control>
                    <Accordion.Panel>
                      <Code block className="sequence">
                        {pretty(differences)}
                      </Code>
                    </Accordion.Panel>
                  </Accordion.Item>
                </Accordion>
              </>
            )}
          </Paper>
        )}
        {view === "records" && (
          <Paper withBorder p="lg">
            <Group justify="space-between">
              <Title order={2}>Saved experiments</Title>
              <Button
                onClick={() => {
                  setJson("Import experiment or draft");
                  setJsonText("");
                }}
              >
                Import JSON / verified replay
              </Button>
            </Group>
            <Text c="dimmed">
              Local browser retention plus copyable exports. Imported result
              fields never establish genetic truth.
            </Text>
            {records.map((item) => (
              <Group
                key={item.recordId}
                justify="space-between"
                className="saved-record"
              >
                <div>
                  <Text fw={700}>{item.recordId}</Text>
                  <Text size="xs">
                    {item.result.classification.labels.join(" · ")}
                  </Text>
                </div>
                <Group>
                  <Button
                    size="xs"
                    variant="light"
                    onClick={() => exportJson("Saved experiment", item)}
                  >
                    Export
                  </Button>
                  <Button
                    size="xs"
                    onClick={() => {
                      setJsonText(pretty(item));
                      setJson("Replay saved experiment");
                    }}
                  >
                    Replay
                  </Button>
                </Group>
              </Group>
            ))}
            {!records.length && <Text>No saved experiments yet.</Text>}
            <Divider my="md" />
            <Button
              variant="light"
              onClick={() =>
                run(async () => {
                  const stored = JSON.parse(
                    localStorage.getItem(draftKey) ?? "null",
                  );
                  if (!stored) throw new Error("No saved draft.");
                  await request("/api/authoring/validate", stored);
                  setDraft(stored);
                  setView("compendium");
                  setMessage(
                    "Restored separate catalogue draft; experiment unchanged.",
                  );
                })
              }
            >
              Restore saved catalogue draft
            </Button>
          </Paper>
        )}
        {view === "batch" && (
          <>
            <Group justify="space-between" mb="md">
              <Title order={2}>Contrasting generation batch</Title>
              <Group>
                <NumberInput
                  label="First seed"
                  value={seed}
                  min={0}
                  max={4294967289}
                  onChange={setSeed}
                  w={130}
                />
                <Button disabled={busy} onClick={() => run(runBatch)}>
                  Generate six experiments
                </Button>
              </Group>
            </Group>
            <Text mb="md" c="dimmed">
              Seeded valid draws, no chosen output classes. Static diagnostics;
              medium capability is an analytic rule.
            </Text>
            <div className="batch-grid">
              {batch.map((item) => (
                <Paper key={item.recordId} withBorder p="md">
                  <SvgView markup={item.diagnostic} />
                  <Text size="sm" fw={700}>
                    {item.result.classification.labels.join(" · ")}
                  </Text>
                  <Text size="xs">
                    seed {item.input.genome.origin.seed} ·{" "}
                    {item.result.graph.nodes.length} nodes
                  </Text>
                  <Button
                    size="xs"
                    mt="sm"
                    variant="light"
                    onClick={() => exportJson("Batch experiment", item)}
                  >
                    Export retained packet
                  </Button>
                </Paper>
              ))}
            </div>
          </>
        )}
        <Text size="xs" c="dimmed" mt="xl">
          Provisional authoring content; no production sprite/rig/animation,
          sample permission, cloud, game-save or hardware claim.
        </Text>
        <Modal
          opened={json !== null}
          onClose={() => setJson(null)}
          title={json}
          size="xl"
        >
          <Textarea
            aria-label="Copyable authoring JSON"
            value={jsonText}
            onChange={(event) => setJsonText(event.target.value)}
            autosize
            minRows={12}
            maxRows={24}
          />
          <Group mt="md">
            <Button disabled={busy} onClick={() => run(importRecord)}>
              Validate inputs & replay / import draft
            </Button>
            <Text size="xs" c="dimmed">
              Copy this JSON to retain it outside this browser.
            </Text>
          </Group>
        </Modal>
      </AppShell.Main>
    </AppShell>
  );
}

function Coverage({ catalogue, packet }) {
  return (
    <Paper withBorder p="md" mt="md">
      <Title order={3}>Eleven-family coverage and gaps</Title>
      <Table mt="md" striped>
        <Table.Thead>
          <Table.Tr>
            <Table.Th>Family</Table.Th>
            <Table.Th>Active / inactive / draft</Table.Th>
            <Table.Th>Boundary</Table.Th>
          </Table.Tr>
        </Table.Thead>
        <Table.Tbody>
          {packet.result.coverage.map((family) => (
            <Table.Tr key={family.id}>
              <Table.Td>{familyLabel(family.id)}</Table.Td>
              <Table.Td>
                {family.activeContributors.length} /{" "}
                {family.inactiveContributors.length} /{" "}
                {family.draftRecords.length}
              </Table.Td>
              <Table.Td>{family.gaps}</Table.Td>
            </Table.Tr>
          ))}
        </Table.Tbody>
      </Table>
      <Text size="xs" c="dimmed" mt="sm">
        {catalogue.reproductionContract.mechanism} Authoring-only contract;
        taxonomy never authorizes reproduction.
      </Text>
    </Paper>
  );
}

const theme = {
  primaryColor: "sage",
  colors: {
    sage: [
      "#edf5ef",
      "#d9e8dd",
      "#bfd4c5",
      "#a3c0ad",
      "#89ae97",
      "#729d84",
      "#5f8970",
      "#4b745c",
      "#395e48",
      "#274735",
    ],
  },
  fontFamily: "Inter, system-ui, sans-serif",
};
createRoot(document.getElementById("root")).render(
  <MantineProvider theme={theme} defaultColorScheme="dark">
    <Workbench />
  </MantineProvider>,
);
