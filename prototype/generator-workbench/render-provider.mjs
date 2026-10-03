import { createHash } from "node:crypto";

export const sha256 = (bytes) => createHash("sha256").update(bytes).digest("hex");
const maximumOutputBytes = 4 * 1024 * 1024;

export function providerSettings(environment) {
  return {
    nanobanana: { model: environment.CRITTER_NANOBANANA_MODEL || "gemini-3.1-flash-image",
      key: environment.GEMINI_API_KEY || environment.GOOGLE_API_KEY || "" },
    openai: { model: environment.CRITTER_OPENAI_IMAGE_MODEL || "gpt-image-2.5-sunburst",
      key: environment.OPENAI_API_KEY || "" },
  };
}

export function imageBytes(base64, mime, maximum = maximumOutputBytes) {
  if (typeof base64 !== "string" || !base64.length || base64.length > Math.ceil(maximum / 3) * 4 ||
      !/^(?:[A-Za-z0-9+/]{4})*(?:[A-Za-z0-9+/]{2}==|[A-Za-z0-9+/]{3}=)?$/.test(base64)) {
    throw new Error("Provider image is missing or exceeds the bounded image limit");
  }
  const bytes = Buffer.from(base64, "base64");
  const png = bytes.subarray(0, 8).equals(Buffer.from([137, 80, 78, 71, 13, 10, 26, 10]));
  const jpeg = bytes[0] === 255 && bytes[1] === 216 && bytes[2] === 255;
  const webp = bytes.toString("ascii", 0, 4) === "RIFF" && bytes.toString("ascii", 8, 12) === "WEBP";
  if (!bytes.length || bytes.length > maximum || !({ "image/png": png, "image/jpeg": jpeg, "image/webp": webp }[mime])) {
    throw new Error("Provider image bytes do not match a supported bounded bitmap");
  }
  return bytes;
}

async function boundedJson(response) {
  // Base64 output plus finite provider metadata; never retain raw error bodies.
  const chunks = [];
  let size = 0;
  for await (const chunk of response.body) {
    size += chunk.byteLength;
    if (size > 7 * 1024 * 1024) throw new Error("Provider response exceeds the bounded response limit");
    chunks.push(Buffer.from(chunk));
  }
  return JSON.parse(Buffer.concat(chunks).toString("utf8"));
}

export async function renderProvider(provider, settings, sourcePng, prompt) {
  let endpoint, headers, body;
  if (provider === "nanobanana") {
    endpoint = "https://generativelanguage.googleapis.com/v1beta/interactions";
    headers = { "x-goog-api-key": settings.key, "Content-Type": "application/json" };
    body = JSON.stringify({ model: settings.model, store: false,
      input: [{ type: "text", text: prompt }, { type: "image", mime_type: "image/png", data: sourcePng.toString("base64") }],
      response_format: { type: "image", mime_type: "image/png" } });
  } else if (provider === "openai") {
    endpoint = "https://api.openai.com/v1/images/edits";
    headers = { Authorization: `Bearer ${settings.key}` };
    body = new FormData();
    body.set("model", settings.model);
    body.append("image[]", new Blob([sourcePng], { type: "image/png" }), "source.png");
    body.set("prompt", prompt);
    body.set("n", "1");
    body.set("output_format", "png");
  } else throw new Error("Unsupported provider");
  const response = await fetch(endpoint, { method: "POST", headers, body,
    signal: AbortSignal.timeout(180000), redirect: "error" });
  const requestId = response.headers.get("x-request-id") || response.headers.get("x-goog-request-id") || null;
  if (!response.ok) {
    await response.body?.cancel();
    throw Object.assign(new Error(`Provider returned HTTP${response.status}; no automatic retry. A charge may have occurred.`),
      { providerRequestId: requestId });
  }
  const result = await boundedJson(response);
  let base64, mime, revisedPrompt = null;
  if (provider === "openai") {
    base64 = result.data?.[0]?.b64_json;
    mime = "image/png";
    revisedPrompt = typeof result.data?.[0]?.revised_prompt === "string" ? result.data[0].revised_prompt.slice(0, 20000) : null;
  } else {
    const images = (result.steps ?? []).filter((step) => step.type === "model_output")
      .flatMap((step) => step.content ?? []).filter((content) => content.type === "image");
    const image = images.at(-1);
    base64 = image?.data;
    mime = image?.mime_type;
  }
  const bytes = imageBytes(base64, mime);
  const usage = result.usage ?? null;
  if (JSON.stringify(usage).length > 32768) throw new Error("Provider usage metadata exceeds its bound");
  return { bytes, mime, sha256: sha256(bytes), providerRequestId: requestId,
    interactionId: typeof result.id === "string" ? result.id.slice(0, 256) : null,
    usage, revisedPrompt };
}
