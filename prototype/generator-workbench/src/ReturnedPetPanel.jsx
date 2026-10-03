import React, { useEffect, useMemo, useRef, useState } from "react";
import { Alert, Badge, Button, FileInput, Group, Paper, Stack, Text, TextInput, Title } from "@mantine/core";
import { isResolvedAuthoringPacket, sceneReplayEnvelope } from "../authoring-ui.mjs";
import {
  downloadProposalBlob,
  proposalBindingKey,
  proposalSourceBinding,
  readPetProposals,
  retainPetProposal,
} from "../retained-pet-proposals.mjs";

function ProposalImage({ entry }) {
  const [image, setImage] = useState(null);
  useEffect(() => {
    const url = URL.createObjectURL(entry.imageBlob);
    setImage({ proposalId: entry.proposalId, url });
    return () => URL.revokeObjectURL(url);
  }, [entry]);
  return image?.proposalId === entry.proposalId ? (
    <img src={image.url} alt="Returned pet proposal; inherited fidelity is unaccepted"
      style={{ display: "block", maxWidth: "100%", maxHeight: 420, objectFit: "contain" }} />
  ) : null;
}

export default function ReturnedPetPanel({ packet, handoff, revision, sourceBusy }) {
  const source = useMemo(() => {
    try {
      if (sourceBusy || !isResolvedAuthoringPacket(packet) || handoff.status !== "ready") {
        return { binding: null, reason: "Resolve a current source and prompt before retaining returned art." };
      }
      return { binding: proposalSourceBinding(packet, handoff.text, sceneReplayEnvelope(packet)) };
    } catch (error) {
      return { binding: null, reason: error.message };
    }
  }, [packet, handoff.text, handoff.status, sourceBusy]);
  const bindingKey = proposalBindingKey(source.binding);
  // Revision also invalidates a selection if identical source bytes are re-resolved.
  const activeKey = `${revision}:${bindingKey}`;
  const currentKey = useRef(activeKey);
  currentKey.current = activeKey;
  const operation = useRef(null);
  const [selection, setSelection] = useState(null);
  const [providerLabel, setProviderLabel] = useState("Gemini (manual return)");
  const [retained, setRetained] = useState({ key: "", entries: [] });
  const [notice, setNotice] = useState({ key: "", text: "", error: false });
  const [savingKey, setSavingKey] = useState("");
  const saving = savingKey === activeKey;
  const selectedFile = selection?.key === activeKey ? selection.file : null;
  const entries = retained.key === activeKey ? retained.entries : [];

  useEffect(() => {
    const controller = new AbortController();
    const key = activeKey;
    operation.current?.abort();
    operation.current = null;
    setSelection(null);
    setSavingKey("");
    setNotice({ key, text: "", error: false });
    setRetained({ key, entries: [] });
    if (source.binding) {
      readPetProposals(source.binding, controller.signal).then((loaded) => {
        if (!controller.signal.aborted && currentKey.current === key) setRetained((previous) => {
          const merged = new Map(loaded.map((entry) => [entry.proposalId, entry]));
          if (previous.key === key) for (const entry of previous.entries) merged.set(entry.proposalId, entry);
          return { key, entries: Array.from(merged.values()) };
        });
      }).catch((error) => {
        if (error.name !== "AbortError" && currentKey.current === key) {
          setNotice({ key, text: error.message, error: true });
        }
      });
    }
    return () => {
      controller.abort();
      operation.current?.abort();
    };
  }, [activeKey]);

  async function retainSelected() {
    if (!selectedFile || !source.binding || saving) return;
    const key = activeKey;
    const controller = new AbortController();
    operation.current = controller;
    setSavingKey(key);
    setNotice({ key, text: "", error: false });
    try {
      const entry = await retainPetProposal(selectedFile, source.binding, providerLabel, controller.signal);
      if (controller.signal.aborted || currentKey.current !== key) return;
      setRetained((previous) => ({ key, entries: [
        ...(previous.key === key ? previous.entries.filter((item) => item.proposalId !== entry.proposalId) : []), entry,
      ] }));
      setSelection(null);
      setNotice({ key, text: "Pet proposal retained in this browser, linked to this exact source and prompt.", error: false });
    } catch (error) {
      if (error.name !== "AbortError" && currentKey.current === key) setNotice({ key, text: error.message, error: true });
    } finally {
      if (operation.current === controller) operation.current = null;
      if (currentKey.current === key) setSavingKey("");
    }
  }

  function download(entry, metadataOnly) {
    try {
      const extension = { "image/png": "png", "image/jpeg": "jpg", "image/webp": "webp" }[entry.metadata.image.mime];
      const blob = metadataOnly ? new Blob([JSON.stringify(entry.metadata, null, 2)], { type: "application/json" }) : entry.imageBlob;
      downloadProposalBlob(blob, `${entry.proposalId}.${metadataOnly ? "json" : extension}`);
    } catch (error) {
      setNotice({ key: activeKey, text: error.message, error: true });
    }
  }

  async function copyMetadata(entry) {
    const key = activeKey;
    try {
      if (!navigator.clipboard?.writeText) throw new Error("Clipboard access is unavailable in this browser.");
      await navigator.clipboard.writeText(JSON.stringify(entry.metadata, null, 2));
      if (currentKey.current === key) setNotice({ key, text: "Linked metadata copied, including the exact source replay recipe.", error: false });
    } catch (error) {
      if (currentKey.current === key) setNotice({ key, text: `Could not copy linked metadata: ${error.message}`, error: true });
    }
  }

  return (
    <Paper withBorder p="md" mt="md">
      <Group justify="space-between"><Title order={4}>Returned pet art</Title><Badge color="orange">Proposal only</Badge></Group>
      <Text size="sm" c="dimmed" mt="xs">
        Attach the returned bitmap and retain it explicitly. It stays linked to the shown source and prompt;
        model changes are unaccepted proposals and never change inherited traits.
      </Text>
      {!source.binding ? <Text size="sm" mt="sm">{source.reason}</Text> : (
        <Stack gap="sm" mt="sm">
          <Text size="xs" c="dimmed">Source: {source.binding.sourceRecordId}</Text>
          <FileInput key={activeKey} label="Returned bitmap" accept="image/png,image/jpeg,image/webp"
            placeholder="Choose PNG, JPEG or WebP (at most4MiB)" value={selectedFile}
            disabled={saving} clearable onChange={(file) => setSelection(file ? { key: activeKey, file } : null)} />
          <TextInput label="Provider label" value={providerLabel} maxLength={80} disabled={saving}
            onChange={(event) => setProviderLabel(event.currentTarget.value)} />
          <Group>
            <Button size="xs" disabled={!selectedFile || saving || !providerLabel.trim()} onClick={retainSelected}>
              {saving ? "Retaining…" : "Retain pet proposal"}
            </Button>
            {saving && <Button size="xs" variant="subtle" onClick={() => {
              operation.current?.abort();
              setNotice({ key: activeKey, text: "Cancellation requested. Reopen the source to see any retention that already completed.", error: false });
            }}>Cancel</Button>}
          </Group>
          <Text size="xs" c="dimmed">This separate browser store holds up to eight proposals. It never evicts older art or genome saves.</Text>
          {entries.map((entry) => (
            <Paper withBorder p="sm" key={entry.proposalId}>
              <ProposalImage entry={entry} />
              <Text size="sm" mt="xs">{entry.metadata.providerLabel} · proposal · {entry.metadata.image.width}×{entry.metadata.image.height}</Text>
              <Text size="xs" c="dimmed">{entry.metadata.sourceBinding.sourceRecordId} · {entry.proposalId}</Text>
              <Group mt="xs">
                <Button size="xs" variant="light" onClick={() => download(entry, false)}>Download returned bitmap</Button>
                <Button size="xs" variant="light" onClick={() => download(entry, true)}>Download linked metadata</Button>
                <Button size="xs" variant="light" onClick={() => copyMetadata(entry)}>Copy linked metadata</Button>
              </Group>
            </Paper>
          ))}
        </Stack>
      )}
      {notice.key === activeKey && notice.text && <Alert mt="sm" color={notice.error ? "red" : "sage"}>{notice.text}</Alert>}
    </Paper>
  );
}
