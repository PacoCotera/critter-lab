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
  if (!response.ok)
    throw new Error(
      data.errors
        ?.map((error) => `${error.path}: ${error.message}`)
        .join("\n") ??
        data.error ??
        "Request rejected.",
    );
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
  const [view, setView] = useState("compendium");
  const [family, setFamily] = useState("all");
  const [search, setSearch] = useState("");
  const [selected, setSelected] = useState("development.axial-repeat");
  const [edited, setEdited] = useState("");
  const [seed, setSeed] = useState(1);
  const [expressionSeed, setExpressionSeed] = useState(7);
  const [message, setMessage] = useState("Loading pinned content…");
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
        const initial =
          data.packages?.find(
            (item) => item.catalogue.ruleVersion === "continuous-pet/1",
          ) ?? data;
        const example = data.petExamples?.[0];
        setCatalogue(initial.catalogue);
        setDraft(clone(initial.catalogue));
        setGenome(example?.genome ?? initial.defaultGeneration.genome);
        setContext(example?.context ?? data.referenceContext);
        setEdited(pretty(initial.catalogue.loci[0]));
        setMessage(
          `Pinned ${initial.catalogue.id} · ${initial.catalogue.loci.length} records / ${initial.catalogue.loci.filter((item) => item.status === "validated").length} executable. Resolve or generate an experiment.`,
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
    setSelected(id);
    const locus = draft?.loci.find((item) => item.id === id);
    if (locus) setEdited(pretty(locus));
  }
  function invalidate() {
    inputRevision.current += 1;
    setPacket(null);
    setFailed(false);
    setMessage(
      "Inputs changed · current output/export cleared. Pinned comparison is retained.",
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
    setEdited(pretty(item.catalogue.loci[0]));
    setBatch([]);
  }
  function loadFamilyExample(name) {
    const example = [...familyExamples, ...petExamples].find(
      (item) => item.name === name,
    );
    if (!example || busy) return;
    invalidate();
    setGenome(clone(example.genome));
    setContext(clone(example.context));
    setMessage(
      `${name}: controlled authoring input; ${example.relationship.kind}. Resolve to inspect.`,
    );
  }
  async function run(operation) {
    if (processing.current) return;
    processing.current = true;
    setBusy(true);
    setFailed(false);
    try {
      await operation();
    } catch (error) {
      setPacket(null);
      setMessage(error.message);
      setFailed(true);
    } finally {
      processing.current = false;
      setBusy(false);
    }
  }
  async function resolve(expression = null) {
    const revision = inputRevision.current;
    const data = await request("/api/authoring/evaluate", {
      catalogue,
      genome,
      context,
      expressionSeed: expression,
    });
    if (revision !== inputRevision.current) return;
    setPacket(data);
    setMessage(
      `Resolved ${data.recordId} · complete diagnostic, not game creation.`,
    );
  }
  async function generate() {
    const revision = inputRevision.current;
    const data = await request("/api/authoring/generate", {
      catalogue,
      seed: Number(seed),
    });
    if (revision !== inputRevision.current) return;
    setGenome(data.input.genome);
    setContext(data.input.context);
    setPacket(data);
    setMessage(
      `Accepted unmodified draw ${data.generation.attempts}, seed ${seed}. No class/template was sampled.`,
    );
  }
  function exportJson(kind, value) {
    setJson(kind);
    setJsonText(pretty(value));
  }
  function editCopy(locusId, index, value) {
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
    setEdited(pretty(item));
    setMessage(
      `Saved validated-schema draft v${next.version}; pinned package/experiment unchanged. This is not product approval.`,
    );
  }
  async function useDraft() {
    await request("/api/authoring/validate", draft);
    const next = clone(genome);
    next.contentId = draft.id;
    next.contentVersion = draft.version;
    // Content edits are explicit. Existing copies are never repaired to make a new package pass.
    setCatalogue(clone(draft));
    setGenome(next);
    invalidate();
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
    if (jsonText.length > 1000000)
      throw new Error("Import exceeds the one-megabyte local record limit.");
    const imported = JSON.parse(jsonText);
    if (imported.schemaVersion === "critter-catalogue/1") {
      await request("/api/authoring/validate", imported);
      setDraft(clone(imported));
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
      setCatalogue(data.input.catalogue);
      setDraft(clone(data.input.catalogue));
      setGenome(data.input.genome);
      setContext(data.input.context);
      setPacket(data);
      const retainedSelected =
        data.input.catalogue.loci.find((item) => item.id === selected) ??
        data.input.catalogue.loci[0];
      setSelected(retainedSelected.id);
      setEdited(pretty(retainedSelected));
      setView("experiment");
      setMessage(
        "Digest replay verified from inputs. Embedded output/SVG/prompt was not trusted.",
      );
    }
    setJson(null);
  }
  async function runBatch() {
    const items = [];
    for (let index = 0; index < 6; index++)
      items.push(
        await request("/api/authoring/generate", {
          catalogue,
          seed: Number(seed) + index,
        }),
      );
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
  const filtered = draft.loci.filter(
    (item) =>
      (family === "all" || item.family === family) &&
      `${item.id} ${item.label} ${item.aliases.join(" ")}`
        .toLowerCase()
        .includes(search.toLowerCase()),
  );
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
      navbar={{ width: 220, breakpoint: "sm" }}
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
                label: `${item.catalogue.id} v${item.catalogue.version}`,
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
            ["compendium", "Compendium"],
            ["experiment", "Genome experiment"],
            ["compare", "Pinned comparison"],
            ["records", "Saved records"],
            ["batch", "Generation batch"],
          ].map(([id, label]) => (
            <NavLink
              key={id}
              label={label}
              active={view === id}
              onClick={() => setView(id)}
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
            draft. Catalogue validation means engine/schema support, not
            canonical biology.
          </Text>
          <Text size="xs" c="dimmed">
            Classes describe generated outcomes; no species or body-preset
            selector.
          </Text>
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
                    onChange={(event) => setSearch(event.target.value)}
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
                    onChange={setFamily}
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
                    onChange={(event) => setField("label", event.target.value)}
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
                  <Button disabled={busy} onClick={() => run(saveDraftRecord)}>
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
                  Operator/range changes outside implemented semantics reject.
                  Changing status alone cannot promote a candidate.
                </Text>
              </Paper>
            </div>
          </>
        )}
        {view === "experiment" && (
          <>
            <Group justify="space-between" mb="md">
              <div>
                <Title order={2}>Genome → expression → construction</Title>
                {["continuous-static/1", "continuous-pet/1"].includes(
                  catalogue.ruleVersion,
                ) && (
                  <Group mt="sm">
                    {(catalogue.ruleVersion === "continuous-pet/1"
                      ? petExamples
                      : familyExamples
                    ).map((example) => (
                      <Button
                        key={example.name}
                        size="xs"
                        variant="light"
                        disabled={busy}
                        onClick={() => loadFamilyExample(example.name)}
                      >
                        {example.name.replaceAll("-", " ")}
                      </Button>
                    ))}
                  </Group>
                )}
                <Text c="dimmed">
                  Complete executable copies, including inactive and suppressed
                  contributors.
                </Text>
              </div>
              <Group>
                <NumberInput
                  label="Genome seed"
                  min={0}
                  max={4294967295}
                  value={seed}
                  onChange={setSeed}
                  w={130}
                />
                <Button disabled={busy} onClick={() => run(generate)}>
                  Generate genome
                </Button>
                <Button
                  disabled={busy}
                  onClick={() => run(() => resolve(null))}
                >
                  Resolve edited genome
                </Button>
              </Group>
            </Group>
            <div className="experiment-layout">
              <Paper withBorder p="md">
                <Select
                  disabled={busy}
                  label="Reference medium"
                  data={["ground", "air", "water"]}
                  value={context.medium}
                  onChange={(medium) => {
                    setContext({ ...context, medium });
                    invalidate();
                  }}
                />
                <Text size="xs" c="dimmed" my="sm">
                  Adult/rested/reference only. Medium does not add a capability.
                </Text>
                <ScrollArea h={590}>
                  <Stack gap="xs">
                    {catalogue.loci
                      .filter((item) => item.status === "validated")
                      .map((item) => (
                        <div
                          key={item.id}
                          className={
                            selected === item.id
                              ? "copy-row selected"
                              : "copy-row"
                          }
                        >
                          <Button
                            variant="subtle"
                            size="compact-xs"
                            onClick={() => selectLocus(item.id)}
                          >
                            {item.label}
                          </Button>
                          <div className="copy-controls">
                            {[0, 1].map((index) => (
                              <Select
                                disabled={busy}
                                key={index}
                                size="xs"
                                aria-label={`${item.label} copy ${index + 1}`}
                                data={item.alleles.map((allele) => ({
                                  value: allele.id,
                                  label: allele.label,
                                }))}
                                value={genome.loci[item.id]?.[index] ?? null}
                                onChange={(value) =>
                                  editCopy(item.id, index, value)
                                }
                              />
                            ))}
                          </div>
                        </div>
                      ))}
                  </Stack>
                </ScrollArea>
              </Paper>
              <div>
                <Paper withBorder p="md">
                  <Group justify="space-between">
                    <Text fw={700}>Resolved diagnostic</Text>
                    <Badge color="gray">
                      Static analytic concept · no animation
                    </Badge>
                    {selectedFact && (
                      <Text size="sm" mt="sm">
                        {selectedFact.id}:{" "}
                        {typeof selectedFact.value === "object"
                          ? pretty(selectedFact.value)
                          : String(selectedFact.value)}{" "}
                        {selectedFact.unit} ·{" "}
                        {selectedFact.prerequisites.length} declared
                        prerequisite(s)
                      </Text>
                    )}
                    {packet?.ruleVersion === "continuous-pet/1" && (
                      <Text size="sm" mt="sm">
                        {packet.result.graph.covering.kind} ·{" "}
                        {packet.result.graph.covering.elements?.length ??
                          packet.result.graph.covering.plates.length}{" "}
                        material elements ·{" "}
                        {
                          packet.result.graph.nodes.filter((node) =>
                            node.sources.includes(selected),
                          ).length
                        }{" "}
                        sourced body/feature targets
                      </Text>
                    )}
                  </Group>
                  {packet ? (
                    <SvgView
                      compact
                      onSelect={selectLocus}
                      markup={drawAuthoringCreature(packet.result, selected)}
                    />
                  ) : (
                    <div className="empty-result">
                      <Title order={3}>Current output cleared</Title>
                      <Text>
                        Resolve the complete genome to inspect its actual
                        construction.
                      </Text>
                    </div>
                  )}
                  {packet && (
                    <>
                      <Group mt="sm">
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
                          Save exact record
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
                      <Group mt="md" align="end">
                        <NumberInput
                          label="Expression seed · marking placement only"
                          value={expressionSeed}
                          min={0}
                          max={4294967295}
                          onChange={setExpressionSeed}
                          w={240}
                        />
                        <Button
                          size="sm"
                          disabled={busy}
                          onClick={() =>
                            run(() => resolve(Number(expressionSeed)))
                          }
                        >
                          Sample expression
                        </Button>
                      </Group>
                      <Text size="xs" c="dimmed" mt="xs">
                        Same inherited genome. Marking enable/layout/amount stay
                        fixed; all other expression deterministic.
                      </Text>
                    </>
                  )}
                </Paper>
                {packet && (
                  <Paper withBorder p="md" mt="md">
                    <Tabs defaultValue="inherited">
                      <Tabs.List>
                        <Tabs.Tab value="baseline">Baseline</Tabs.Tab>
                        <Tabs.Tab value="inherited">Inherited copies</Tabs.Tab>
                        <Tabs.Tab value="expression">Expression field</Tabs.Tab>
                      </Tabs.List>
                      {["baseline", "inherited", "expression"].map((kind) => (
                        <Tabs.Panel key={kind} value={kind} pt="md">
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
                          <Accordion>
                            <Accordion.Item value="sequence">
                              <Accordion.Control>
                                Complete {kind} sequence · fictional encoding
                              </Accordion.Control>
                              <Accordion.Panel>
                                <Code block className="sequence">
                                  {packet.representations[kind]}
                                </Code>
                              </Accordion.Panel>
                            </Accordion.Item>
                          </Accordion>
                        </Tabs.Panel>
                      ))}
                    </Tabs>
                    <Text size="xs" c="dimmed">
                      Glyphs map exact records/copies/states. Grouping is not
                      chromosome order. SHA digests are separate.
                    </Text>
                  </Paper>
                )}
              </div>
              <Paper withBorder p="md">
                <Title order={4}>Selected contributor</Title>
                <Text fw={700} mt="sm">
                  {locus?.label}
                </Text>
                <Code>{selected}</Code>
                <Text size="sm" mt="sm">
                  {locus?.purpose}
                </Text>
                {selectedFact && (
                  <>
                    <Badge
                      mt="sm"
                      color={
                        selectedFact.state === "expressed" ? "sage" : "gray"
                      }
                    >
                      {selectedFact.state}
                    </Badge>
                    <Code block mt="sm">
                      {pretty(selectedFact)}
                    </Code>
                  </>
                )}
                {packet && (
                  <>
                    <Divider my="md" />
                    <Title order={5}>Supported analytic mechanisms</Title>
                    {packet.result.motion.map((item) => (
                      <div key={item.id} className="motion">
                        <Text size="sm" fw={700}>
                          {item.medium} · {item.status}
                        </Text>
                        <Text size="xs">{item.reasons.join(" ")}</Text>
                      </div>
                    ))}
                    <Text size="xs" c="dimmed" mt="md">
                      {packet.result.classification.labels.join(" · ")} ·
                      descriptive output only
                    </Text>
                  </>
                )}
              </Paper>
            </div>
            {packet && (
              <>
                <Coverage catalogue={catalogue} packet={packet} />
                <Accordion mt="md">
                  <Accordion.Item value="geometry">
                    <Accordion.Control>
                      Projected geometry and pigment-mask reference
                    </Accordion.Control>
                    <Accordion.Panel>
                      {packet.geometryReference?.status === "available" ? (
                        <>
                          <Text size="sm" mb="sm">
                            {[
                              "continuous-static/1",
                              "continuous-pet/1",
                            ].includes(packet.ruleVersion)
                              ? "Solved continuous exterior, rooted fin outlines and sourced feature/material geometry. One profile camera preserves relative proportions; no internal station lines in ordinary view."
                              : "Exact XY footprints; all nodes shown by inspection paint order. Neutral edges are graph annotations, not tissue. Texture is retained in the trace but not rendered."}
                          </Text>
                          <div
                            className="svg-view"
                            dangerouslySetInnerHTML={{
                              __html: packet.geometryReference.svg,
                            }}
                          />
                          <Text size="sm" mt="sm">
                            {packet.geometryReference.manifest.counts.volumes}{" "}
                            body volumes ·{" "}
                            {packet.geometryReference.manifest.counts.fins}{" "}
                            fins. Static diagnostic only; no physical z-order or
                            whole 3D body claim.
                          </Text>
                          <Code block mt="sm">
                            {pretty(packet.geometryReference.manifest)}
                          </Code>
                        </>
                      ) : (
                        <Text size="sm">
                          Reference unavailable:{" "}
                          {packet.geometryReference?.error ??
                            "Not retained in this record; resolve to produce the bounded reference."}
                        </Text>
                      )}
                    </Accordion.Panel>
                  </Accordion.Item>
                  <Accordion.Item value="prompt">
                    <Accordion.Control>
                      Fact description and reusable art template · no provider
                      call
                    </Accordion.Control>
                    <Accordion.Panel>
                      <Text size="sm">{packet.description}</Text>
                      {packet.prompt.status === "rejected" && (
                        <Alert color="orange" mb="md">
                          Art projection unavailable: {packet.prompt.error}.
                          Genetic result is retained; no facts were truncated.
                        </Alert>
                      )}
                      <Textarea
                        label="Resolved fact-derived calibration prompt"
                        readOnly
                        value={packet.prompt.text}
                        autosize
                        minRows={7}
                        maxRows={20}
                      />
                    </Accordion.Panel>
                  </Accordion.Item>
                </Accordion>
              </>
            )}
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
                  <SvgView markup={pinned.diagnostic} />
                </div>
                {packet && (
                  <div>
                    <Text size="sm">Current · {packet.ruleVersion}</Text>
                    <SvgView markup={packet.diagnostic} />
                  </div>
                )}
              </div>
            )}
            {!pinned && <Text>Pin a resolved experiment first.</Text>}
            {pinned && packet && pinned.ruleVersion !== packet.ruleVersion && (
              <Alert color="orange" mt="sm">
                Different construction profiles; these are separate inspections,
                not compatible family or inherited-output comparison.
              </Alert>
            )}
            {pinned && packet && pinned.ruleVersion === packet.ruleVersion && (
              <Table striped>
                <Table.Thead>
                  <Table.Tr>
                    <Table.Th>Output</Table.Th>
                    <Table.Th>Pinned</Table.Th>
                    <Table.Th>Current</Table.Th>
                  </Table.Tr>
                </Table.Thead>
                <Table.Tbody>
                  {differences.map((item) => (
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
