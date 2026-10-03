import { regionalSceneWorkbenchPackage } from "./regional-scene-workbench-package.mjs";
import { anatomicalSourcePackage } from "./anatomical-source-package.mjs";
import { compositionalSourcePackage } from "./compositional-source-package.mjs";
import { resolveCompositionalSource, generateCompositionalSource, replayCompositionalSource } from "./compositional-source-adapter.mjs";
import { compositionalVocabularyPackage } from "./compositional-vocabulary-package.mjs";
import { resolveCompositionalVocabulary, generateCompositionalVocabulary, replayCompositionalVocabulary } from "./compositional-vocabulary-adapter.mjs";
import { resolveAnatomicalSource, generateAnatomicalSource, replayAnatomicalSource } from "./anatomical-source-adapter.mjs";
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
import { pigmentWorkbenchPackage } from "./pigment-workbench-package.mjs";
import {
  moduleSceneCatalogue,
  resolveModuleSceneAuthoring,
  generateModuleSceneAuthoring,
  replayModuleSceneAuthoring,
} from "./module-scene-authoring.mjs";

const files = new Map([
  ["/legacy", ["legacy.html", "text/html; charset=utf-8"]],
  ["/app.mjs", ["app.mjs", "text/javascript; charset=utf-8"]],
  ["/style.css", ["style.css", "text/css; charset=utf-8"]],
]);
const maximumBodyBytes = 65536;
function compositionOperations(input, replay = false) {
  const foundation = replay ? input?.input?.catalogue : input?.catalogue;
  const vocabulary = foundation?.id === "genomic-compositional-source-experiment" && foundation?.version === 2;
  return vocabulary
    ? { evaluate: resolveCompositionalVocabulary, generate: generateCompositionalVocabulary, replay: replayCompositionalVocabulary }
    : { evaluate: resolveCompositionalSource, generate: generateCompositionalSource, replay: replayCompositionalSource };
}

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
      if (request.method === "GET" && request.url === "/api/anatomical-source/catalogue")
        return json(200, anatomicalSourcePackage());
      if (request.method === "GET" && request.url === "/api/compositional-source/catalogue")
        return json(200, { ...compositionalVocabularyPackage(), retainedPackages: [compositionalSourcePackage()] });
      if (
        request.method === "GET" &&
        request.url === "/api/authoring/catalogue"
      )
        return json(200, authoringCatalogue());
      if (
        request.method === "GET" &&
        request.url === "/api/module-scene/catalogue"
      )
        return json(200, {
          ...moduleSceneCatalogue(),
          candidatePackage: pigmentWorkbenchPackage(),
          regionalPackage: regionalSceneWorkbenchPackage(),
        });
      const operations = [
        "/api/evaluate",
        "/api/authoring/evaluate",
        "/api/authoring/generate",
        "/api/authoring/validate",
        "/api/authoring/replay",
        "/api/module-scene/evaluate",
        "/api/module-scene/generate",
        "/api/module-scene/replay",
        "/api/anatomical-source/evaluate",
        "/api/anatomical-source/generate",
        "/api/anatomical-source/replay",
        "/api/compositional-source/evaluate",
        "/api/compositional-source/generate",
        "/api/compositional-source/replay",
      ];
      if (request.method === "POST" && operations.includes(request.url)) {
        const chunks = [];
        let size = 0;
        let exceeded = false;
        for await (const chunk of request.iterator({
          destroyOnReturn: false,
        })) {
          size += chunk.length;
          if (size > maximumBodyBytes) {
            exceeded = true;
            break;
          }
          chunks.push(chunk);
        }
        if (exceeded) {
          chunks.length = 0;
          request.resume();
          return json(413, { error: "Experiment exceeds 64 KiB" });
        }
        let input;
        try {
          input = JSON.parse(Buffer.concat(chunks).toString("utf8"));
        } catch {
          return json(400, { error: "Invalid JSON" });
        }
        let result;
        if (request.url === "/api/evaluate") result = evaluate(input);
        else if (request.url === "/api/compositional-source/evaluate")
          result = compositionOperations(input).evaluate(input);
        else if (request.url === "/api/compositional-source/generate")
          result = compositionOperations(input).generate(input);
        else if (request.url === "/api/compositional-source/replay")
          result = compositionOperations(input, true).replay(input);
        else if (request.url === "/api/anatomical-source/evaluate")
          result = resolveAnatomicalSource(input);
        else if (request.url === "/api/anatomical-source/generate")
          result = generateAnatomicalSource(input);
        else if (request.url === "/api/anatomical-source/replay")
          result = replayAnatomicalSource(input);
        else if (request.url === "/api/module-scene/evaluate")
          result = resolveModuleSceneAuthoring(input);
        else if (request.url === "/api/module-scene/replay")
          result = replayModuleSceneAuthoring(input);
        else if (request.url === "/api/module-scene/generate") {
          if (
            !input ||
            typeof input !== "object" ||
            Array.isArray(input) ||
            Object.keys(input).some(
              (key) => !["catalogue", "seed", "maxAttempts"].includes(key),
            )
          )
            return json(422, {
              status: "rejected",
              errors: [
                {
                  code: "scene-generation-envelope",
                  path: "input",
                  message:
                    "Only catalogue, seed and maxAttempts are supported.",
                },
              ],
            });
          result = generateModuleSceneAuthoring(input.catalogue, input.seed, {
            maxAttempts: input.maxAttempts ?? 1024,
          });
        } else if (request.url === "/api/authoring/evaluate")
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
