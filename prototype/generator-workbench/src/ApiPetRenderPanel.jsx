import React, { useEffect, useRef, useState } from "react";
import { Alert, Badge, Button, Group, Paper, PasswordInput, Select, Text, TextInput, Title } from "@mantine/core";
import { proposalBindingKey, retainPetProposal } from "../retained-pet-proposals.mjs";
import { sourcePng, pngRequest } from "../source-png.mjs";

async function renderingRequest(path, token, input, signal) {
  const response = await fetch(`${import.meta.env.BASE_URL}api/rendering/${path}`, {
    method: input === undefined ? "GET" : "POST", signal,
    headers: { ...(token ? { Authorization: `Bearer ${token}` } : {}),
      ...(input === undefined ? {} : { "Content-Type": "application/json" }) },
    ...(input === undefined ? {} : { body: JSON.stringify(input) }),
  });
  if (path.endsWith("/image") && response.ok) return response.blob();
  const result = await response.json();
  if (!response.ok) throw new Error(result.error || "Rendering request unavailable");
  return result;
}

export default function ApiPetRenderPanel({ binding, referenceSvg, revision, onRetained }) {
  const key = `${revision}:${proposalBindingKey(binding)}`;
  const currentKey = useRef(key);
  currentKey.current = key;
  const [config, setConfig] = useState(null);
  const [provider, setProvider] = useState("nanobanana");
  const [token, setToken] = useState("");
  const [job, setJob] = useState(null);
  const [jobId, setJobId] = useState("");
  const [busy, setBusy] = useState(false);
  const [notice, setNotice] = useState("");
  const [submission, setSubmission] = useState(null);
  const [foundJobs, setFoundJobs] = useState({ key: "", jobs: [] });
  const operation = useRef(null);
  const retainedIds = useRef(new Set());
  const selected = config?.providers.find((item) => item.id === provider);
  const matching = job && proposalBindingKey(job.sourceBinding) === proposalBindingKey(binding);

  useEffect(() => {
    const controller = new AbortController();
    renderingRequest("config", "", undefined, controller.signal).then(setConfig).catch((error) => {
      if (!controller.signal.aborted) setNotice(error.message);
    });
    return () => controller.abort();
  }, []);
  useEffect(() => {
    operation.current?.abort();
    setBusy(false);
    return () => operation.current?.abort();
  }, [key]);

  async function recoverCandidate(candidate, controller, startKey) {
    if (proposalBindingKey(candidate.sourceBinding) !== proposalBindingKey(binding)) {
      setNotice("This job belongs to another source or prompt. Reopen that exact source before retaining its candidate.");
      return;
    }
    if (candidate.state !== "completed" || retainedIds.current.has(candidate.jobId)) return;
    const blob = await renderingRequest(`jobs/${candidate.jobId}/image`, token, undefined, controller.signal);
    const hash = await crypto.subtle.digest("SHA-256", await blob.arrayBuffer());
    const actualHash = Array.from(new Uint8Array(hash), (byte) => byte.toString(16).padStart(2, "0")).join("");
    if (actualHash !== candidate.output.sha256) throw new Error("Recovered image digest differs");
    if (controller.signal.aborted || currentKey.current !== startKey) return;
    const entry = await retainPetProposal(blob, candidate.sourceBinding, `${candidate.provider}: ${candidate.model}`.slice(0, 80),
      controller.signal, candidate);
    if (controller.signal.aborted || currentKey.current !== startKey) return;
    retainedIds.current.add(candidate.jobId);
    onRetained(entry);
    setNotice("API candidate retained with exact source, prompt and job provenance. Art fidelity remains unaccepted.");
  }

  async function follow(initial, controller, startKey) {
    let candidate = initial;
    setJobId(candidate.jobId);
    while (!controller.signal.aborted && currentKey.current === startKey) {
      setJob(candidate);
      if (!["pending", "running"].includes(candidate.state)) {
        if (candidate.state === "completed") await recoverCandidate(candidate, controller, startKey);
        else setNotice(candidate.error || "Rendering stopped; no automatic retry.");
        return;
      }
      await new Promise((resolve) => {
        const finish = () => { clearTimeout(timer); controller.signal.removeEventListener("abort", finish); resolve(); };
        const timer = setTimeout(finish, 2000);
        controller.signal.addEventListener("abort", finish, { once: true });
      });
      if (!controller.signal.aborted) candidate = await renderingRequest(`jobs/${candidate.jobId}`, token, undefined, controller.signal);
    }
  }

  async function perform(action) {
    if (busy) return;
    const startKey = key;
    const controller = new AbortController();
    operation.current = controller;
    setBusy(true);
    setNotice("");
    try { await action(controller, startKey); }
    catch (error) {
      if (!controller.signal.aborted && currentKey.current === startKey) setNotice(error.message);
    } finally {
      if (operation.current === controller) operation.current = null;
      if (currentKey.current === startKey) setBusy(false);
    }
  }
  async function render(controller, startKey) {
    if (!binding || !referenceSvg || !selected?.available) throw new Error("Current source and configured provider required");
    const image = await pngRequest(await sourcePng(referenceSvg, controller.signal));
    if (controller.signal.aborted || currentKey.current !== startKey) return;
    setJob(null);
    setJobId("");
    const payload = { schemaVersion: "render-request/1", requestId: crypto.randomUUID(), provider,
      replay: binding.sourceReplayEnvelope, expectedBinding: binding, sourcePng: image };
    setSubmission({ key: startKey, payload });
    setNotice(`Submission ${payload.requestId}; keep the returned job ID for explicit recovery.`);
    const created = await renderingRequest("jobs", token, payload, controller.signal);
    await follow(created, controller, startKey);
  }

  return (
    <Paper withBorder p="sm" mt="sm">
      <Group justify="space-between"><Title order={5}>Render a pet via API</Title><Badge color="orange">Candidate only</Badge></Group>
      <Text size="sm" c="dimmed" mt="xs">Sends the current512px source PNG and exact brief shown above. One explicit provider request; no automatic retry, fallback or refinement. A failed or interrupted job may still incur a charge.</Text>
      <Select label="Provider" mt="sm" value={provider} disabled={busy}
        data={[{ value: "nanobanana", label: "NanoBanana (Google)" }, { value: "openai", label: "OpenAI Images" }]}
        onChange={(value) => { if (value) setProvider(value); }} />
      <Text size="xs" mt="xs">Model: {selected?.model || "Loading configuration…"} · {selected?.available ? "configured" : selected?.reason || "unavailable"}</Text>
      <PasswordInput label="Rendering operator token" mt="sm" value={token} autoComplete="off" disabled={busy}
        onChange={(event) => setToken(event.currentTarget.value)} description="Kept in this component only; this is not a provider API key." />
      <Group mt="sm">
        <Button size="xs" disabled={busy || !binding || !referenceSvg || !token || !selected?.available ||
          (submission?.key === key && (!job || ["pending", "running"].includes(job.state)))} onClick={() => perform(render)}>Render pet</Button>
        {submission?.key === key && !job && <Button size="xs" variant="light" disabled={busy || !token}
          onClick={() => perform(async (controller, startKey) => follow(await renderingRequest("jobs", token, submission.payload, controller.signal), controller, startKey))}>Recover submission</Button>}
        {busy && <Button size="xs" variant="subtle" onClick={() => {
          operation.current?.abort(); setBusy(false);
          setNotice("Stopped waiting in this browser. The server job continues; recover it explicitly without another provider request.");
        }}>Stop waiting</Button>}
      </Group>
      {submission?.key === key && <Text size="xs" c="dimmed" mt="xs">Request ID: {submission.payload.requestId}</Text>}
      <TextInput label="Known server job ID" value={jobId} mt="sm" disabled={busy}
        onChange={(event) => setJobId(event.currentTarget.value)} />
      <Button size="xs" variant="light" mt="xs" disabled={busy || !token || !jobId.trim()}
        onClick={() => perform(async (controller, startKey) => follow(await renderingRequest("recovery", token, { jobId: jobId.trim() }, controller.signal), controller, startKey))}>Recover known job</Button>
      <Button size="xs" variant="light" mt="xs" ml="xs" disabled={busy || !token || !binding}
        onClick={() => perform(async (controller, startKey) => {
          const retained = await renderingRequest("jobs", token, undefined, controller.signal);
          if (controller.signal.aborted || currentKey.current !== startKey) return;
          const matches = retained.jobs.filter((candidate) => proposalBindingKey(candidate.sourceBinding) === proposalBindingKey(binding));
          setFoundJobs({ key: startKey, jobs: matches });
          setNotice(matches.length ? `Found ${matches.length} retained job(s) for this exact source and prompt.` : "No retained jobs match this exact source and prompt.");
        })}>Find retained jobs for this source</Button>
      {foundJobs.key === key && foundJobs.jobs.map((candidate) => (
        <Group mt="xs" key={candidate.jobId}>
          <Text size="xs">{candidate.provider} · {candidate.state} · {candidate.jobId}</Text>
          <Button size="compact-xs" variant="subtle" disabled={busy || !token}
            onClick={() => perform(async (controller, startKey) => follow(await renderingRequest(`jobs/${candidate.jobId}`, token, undefined, controller.signal), controller, startKey))}>Recover this job</Button>
        </Group>
      ))}
      {job && <Text size="sm" mt="sm">Job {job.jobId}: {job.state}. {matching ? "Bound to this source." : "Original source differs; candidate cannot attach here."}</Text>}
      {notice && <Alert mt="sm">{notice}</Alert>}
      <Text size="xs" c="dimmed" mt="sm">Up to eight durable server jobs and eight separate browser proposals; capacity rejects without eviction. Browser retention failure leaves the completed server image recoverable.</Text>
    </Paper>
  );
}
