import { createServer } from "node:http";
import { readFile, readdir } from "node:fs/promises";
import { fileURLToPath } from "node:url";
import { resolve } from "node:path";
import { catalogue, evaluate } from "./evaluate.mjs";
import {
  authoringCatalogue,
  resolveAuthoring,
  replayAuthoring,
  validateDraft,
} from "./authoring-adapter.mjs";
import { AUTHORING_CATALOGUE } from "./catalogue.mjs";
import { generateGenome } from "./model.mjs";

const files = new Map([
  ["/legacy", ["legacy.html", "text/html; charset=utf-8"]],
  ["/app.mjs", ["app.mjs", "text/javascript; charset=utf-8"]],
  ["/style.css", ["style.css", "text/css; charset=utf-8"]],
]);
const maximumBodyBytes = 65536;

export function makeServer() {
  return createServer(async (request, response) => {
    const send = (status, type, content) => {
      response.writeHead(status, {
        "Content-Type": type,
        "Cache-Control": "no-store",
        "X-Content-Type-Options": "nosniff",
      });
      response.end(content);
    };
    const json = (status, body) =>
      send(status, "application/json; charset=utf-8", JSON.stringify(body));
    // This is a local developer tool; reject remote browser origins instead of exposing a service.
    const host = request.headers.host;
    if (!host || !/^127\.0\.0\.1:\d+$/.test(host))
      return json(403, { error: "Loopback Host required" });
    if (request.headers.origin && request.headers.origin !== `http://${host}`)
      return json(403, { error: "Same-origin request required" });
    try {
      if (request.method === "GET" && request.url === "/api/catalogue")
        return json(200, catalogue());
      if (
        request.method === "GET" &&
        request.url === "/api/authoring/catalogue"
      )
        return json(200, authoringCatalogue());
      const operations = [
        "/api/evaluate",
        "/api/authoring/evaluate",
        "/api/authoring/generate",
        "/api/authoring/validate",
        "/api/authoring/replay",
      ];
      if (request.method === "POST" && operations.includes(request.url)) {
        const chunks = [];
        let size = 0;
        for await (const chunk of request) {
          size += chunk.length;
          if (size > maximumBodyBytes)
            return json(413, { error: "Experiment exceeds 64 KiB" });
          chunks.push(chunk);
        }
        let input;
        try {
          input = JSON.parse(Buffer.concat(chunks).toString("utf8"));
        } catch {
          return json(400, { error: "Invalid JSON" });
        }
        let result;
        if (request.url === "/api/evaluate") result = evaluate(input);
        else if (request.url === "/api/authoring/evaluate")
          result = resolveAuthoring(input);
        else if (request.url === "/api/authoring/replay")
          result = replayAuthoring(input);
        else if (request.url === "/api/authoring/validate") {
          result = validateDraft(input);
          return json(result.valid ? 200 : 422, result);
        } else {
          if (
            !input ||
            typeof input !== "object" ||
            Object.keys(input).some(
              (key) => !["catalogue", "seed", "maxAttempts"].includes(key),
            )
          )
            return json(422, {
              status: "rejected",
              errors: [
                {
                  code: "generation-envelope",
                  path: "input",
                  message: "Only catalogue, seed and maxAttempts supported.",
                },
              ],
            });
          const source = Object.hasOwn(input, "catalogue")
            ? input.catalogue
            : AUTHORING_CATALOGUE;
          result = generateGenome(source, input.seed, {
            maxAttempts: input.maxAttempts ?? 128,
          });
          if (result.status === "generated")
            result = {
              ...resolveAuthoring({ catalogue: source, genome: result.genome }),
              generation: {
                seed: result.seed,
                attempts: result.attempts,
                algorithmVersion: result.algorithmVersion,
              },
            };
        }
        return json(result.status === "resolved" ? 200 : 422, result);
      }
      if (request.method === "GET" && request.url === "/") {
        try {
          return send(
            200,
            "text/html; charset=utf-8",
            await readFile(new URL("dist/index.html", import.meta.url)),
          );
        } catch {
          return send(
            503,
            "text/plain; charset=utf-8",
            "Run npm install and npm run build, then restart. Preserved Pip proof: /legacy",
          );
        }
      }
      if (
        request.method === "GET" &&
        /^\/assets\/[a-zA-Z0-9_.-]+\.(js|css)$/.test(request.url)
      ) {
        const name = request.url.slice(8);
        const assets = await readdir(new URL("dist/assets/", import.meta.url));
        if (assets.includes(name))
          return send(
            200,
            name.endsWith(".js")
              ? "text/javascript; charset=utf-8"
              : "text/css; charset=utf-8",
            await readFile(new URL(`dist/assets/${name}`, import.meta.url)),
          );
      }
      const file = files.get(request.url);
      if (request.method === "GET" && file) {
        return send(
          200,
          file[1],
          await readFile(new URL(file[0], import.meta.url)),
        );
      }
      return json(404, { error: "Not found" });
    } catch {
      return json(500, { error: "Evaluation failed; no result produced" });
    }
  });
}

if (
  process.argv[1] &&
  resolve(process.argv[1]) === fileURLToPath(import.meta.url)
) {
  const server = makeServer();
  server.listen(4381, "127.0.0.1", () =>
    console.log("Generator workbench: http://127.0.0.1:4381"),
  );
}
