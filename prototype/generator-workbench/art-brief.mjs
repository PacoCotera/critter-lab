// Renderer prose is a presentation of verified expression, never an allele resolver.
const rounded = (value) => Number(value.toFixed(1));
const list = (values) => values.join(", ");
const pigmentLabel = (pigment) => {
  const names = {
    "#465459": "dark slate",
    "#ae674d": "russet",
    "#dfd2ae": "cream",
    "#718489": "blue-gray",
    "#f1eddc": "pale cream",
    "#273036": "charcoal",
  };
  return names[pigment] ? `${names[pigment]} ${pigment}` : pigment;
};

function regionName(volumes, id) {
  if (volumes.length === 1) return "body";
  const index = volumes.findIndex((node) => node.id === id);
  if (index === 0) return "leading region";
  if (index === volumes.length - 1) return "posterior region";
  return `body region ${index + 1}`;
}

function attachmentDescription(graph, volumes) {
  const nodes = new Map(graph.nodes.map((node) => [node.id, node]));
  const roots = graph.edges.filter((edge) =>
    volumes.some((volume) => volume.id === edge.from),
  );
  const chains = [];
  for (const edge of roots) {
    let node = nodes.get(edge.to);
    if (!["link", "contact-link"].includes(node?.role)) continue;
    const segments = [];
    while (node && ["link", "contact-link"].includes(node.role)) {
      segments.push(node);
      const next = graph.edges.find((candidate) => candidate.from === node.id);
      node = next ? nodes.get(next.to) : null;
    }
    const reach = segments.reduce(
      (sum, segment) => sum + segment.dimensions[0],
      0,
    );
    const proportions =
      segments.length === 2
        ? `, the first ${rounded((100 * segments[0].dimensions[0]) / reach)}% of that reach`
        : `, with segment shares ${segments.map((segment) => `${rounded((100 * segment.dimensions[0]) / reach)}%`).join(" and ")}`;
    chains.push(
      `${segments.length} segments with total reach ${rounded((100 * reach) / volumes[0].dimensions[1])}% of ${volumes.length === 1 ? "body" : "front-region"} width${proportions}, ${segments.at(-1).role === "contact-link" ? "ending in one contact tip" : "with no terminal contact"}, rooted on the ${regionName(volumes, edge.from)}`,
    );
  }
  const sentences = [];
  if (chains.length) {
    const groups = new Map();
    for (const chain of chains) groups.set(chain, (groups.get(chain) ?? 0) + 1);
    sentences.push(
      `There are ${chains.length} jointed appendages: ${list([...groups].map(([description, count]) => `${count} with ${description}`))}.`,
    );
  }
  for (const role of ["membrane", "fin"]) {
    const members = graph.nodes.filter((node) => node.role === role);
    if (!members.length) continue;
    const groups = new Map();
    for (const node of members) {
      const root = roots.find((edge) => edge.to === node.id);
      const description = `${root ? regionName(volumes, root.from) : "retained attachment"}, each spanning ${rounded((100 * node.dimensions[1]) / volumes[0].dimensions[1])}% of ${volumes.length === 1 ? "body" : "front-region"} width`;
      groups.set(description, (groups.get(description) ?? 0) + 1);
    }
    const shape =
      role === "fin" && graph.profile?.id === "continuous-pet/1"
        ? " rounded leaf-shaped"
        : "";
    sentences.push(
      `${members.length}${shape} ${role}${members.length === 1 ? "" : "s"}: ${list([...groups].map(([description, count]) => `${count} on the ${description}`))}.`,
    );
  }
  return sentences;
}

function faceDescription(graph, volumes) {
  const oculars = graph.nodes.filter((node) => node.role === "ocular");
  const mouths = graph.nodes.filter((node) => node.role === "oral-aperture");
  if (!oculars.length && !mouths.length)
    return "The modeled subject is faceless.";
  const parts = [];
  if (oculars.length) {
    const eye = oculars[0];
    const pupil = eye.shape?.components?.find(
      (part) => part.kind === "pupil-circle",
    );
    const outer = eye.shape?.components?.find(
      (part) => part.kind === "outer-circle",
    );
    const radius = eye.shape?.radii?.[0];
    parts.push(
      `${oculars.length} ${outer ? `${pigmentLabel(outer.pigment)} ` : ""}circular eyes, diameter ${rounded((eye.dimensions[0] / volumes[0].dimensions[1]) * 100)}% of front width${pupil ? ` with ${pigmentLabel(pupil.pigment)} pupils at ${rounded((pupil.radius / radius) * 100)}% radius` : ""}`,
    );
    parts[0] += `, centered ${rounded(((eye.position[0] - volumes[0].position[0]) / volumes[0].dimensions[0]) * 100)}% of region length behind its center${oculars.length === 2 ? ` and separated by ${rounded((Math.abs(oculars[0].position[1] - oculars[1].position[1]) / volumes[0].dimensions[1]) * 100)}% of its width` : ""}`;
  }
  if (mouths.length) {
    const placement = oculars.length
      ? mouths[0].position[0] > oculars[0].position[0]
        ? "behind"
        : "ahead of"
      : "on";
    const pigment = graph.surfaces.find(
      (surface) => surface.nodeId === mouths[0].id,
    )?.palette[0];
    parts.push(
      `${mouths.length} ${pigment ? `${pigmentLabel(pigment)} ` : ""}small oval mouth opening${mouths.length === 1 ? "" : "s"} ${placement} ${oculars.length ? "the eyes" : "the front"}`,
    );
  }
  return `The ${volumes.length === 1 ? "body" : "leading region"} carries ${parts.join(" and ")}.`;
}

function surfaceDescription(graph) {
  const groups = new Map();
  for (const surface of graph.surfaces) {
    if (["ocular", "oral-aperture"].includes(surface.region)) continue;
    const key = JSON.stringify([
      surface.palette,
      surface.partition,
      surface.texture,
    ]);
    if (!groups.has(key)) groups.set(key, { surface, regions: new Set() });
    groups.get(key).regions.add(surface.region);
  }
  const names = {
    volume: "body regions",
    "tissue-join": "joins",
    link: "chain segments",
    "contact-link": "terminal contacts",
    membrane: "membranes",
    fin: "fins",
  };
  const sentences = [...groups.values()].map(({ surface, regions }) => {
    const areas = list([...regions].map((region) => names[region] ?? region));
    const pigment =
      surface.palette.length === 1
        ? `uniform ${pigmentLabel(surface.palette[0])}`
        : `${surface.partition === "two declared equal local masks" ? "two equal local pigment fields" : surface.partition} in palette order ${list(surface.palette.map(pigmentLabel))}`;
    return `The ${areas} have ${pigment}, with ${surface.texture.replaceAll("-", " ")} surfaces.`;
  });
  const marks = graph.surfaces.flatMap((surface) =>
    surface.markings.map((mark) => ({ ...mark, region: surface.region })),
  );
  if (!marks.length) sentences.push("There are no expressed markings.");
  else {
    const fields = new Set();
    for (const surface of graph.surfaces.filter(
      (item) => item.markings.length,
    )) {
      const area =
        surface.atlas?.sharedWith === "continuous-body"
          ? "the continuous body"
          : surface.region === "volume"
            ? `the ${regionName(
                graph.nodes.filter((node) => node.role === "volume"),
                surface.nodeId,
              )}`
            : (names[surface.region] ?? surface.region);
      const count = surface.markings.length;
      const layouts = new Set(
        surface.markings.map(
          (mark) =>
            `${mark.layout}, orientation ${rounded(mark.orientation)} radians, scale ${rounded(mark.scale)}, contrast ${rounded(mark.contrast)}`,
        ),
      );
      const countLabel =
        surface.region === "volume" ||
        surface.atlas?.sharedWith === "continuous-body"
          ? `${count} marks on ${area}`
          : `${count} marks per ${surface.region} surface field`;
      fields.add(`${countLabel} in ${list([...layouts])}`);
    }
    sentences.push(`Expressed marking fields contain ${list([...fields])}.`);
  }
  const covering = graph.covering;
  if (covering?.kind === "skin")
    sentences.push("The body covering is bare skin.");
  else if (covering) {
    const count =
      covering.kind === "scales"
        ? covering.plates.length
        : covering.elements.length;
    const unit =
      covering.kind === "fur"
        ? "rooted fur tufts"
        : covering.kind === "feathers"
          ? "rooted feathers"
          : "overlapping scale plates";
    const extent =
      covering.atlas?.startU !== undefined
        ? ` from ${rounded(covering.atlas.startU * 100)}% to ${rounded(covering.atlas.endU * 100)}% along the body`
        : " in the retained body field";
    const leadingWidth = graph.nodes.find((node) => node.role === "volume")
      .dimensions[1];
    const elementSize = covering.elementProfile?.size
      ? `element scale ${rounded(covering.elementProfile.size / leadingWidth)}`
      : covering.elementProfile?.halfWidth
        ? `plate half-width ${rounded(covering.elementProfile.halfWidth / leadingWidth)}`
        : "";
    sentences.push(
      `The body carries ${count} ${unit}${extent}, around the clear facial and fin-root areas${elementSize ? `, with ${elementSize} leading-width units` : ""}.`,
    );
    if (["fur", "feathers"].includes(covering.kind))
      sentences.push(
        "Each covering element keeps the pigment of its body-local root; the remaining body surface stays visible between elements.",
      );
    if (covering.elements?.length) {
      const contourCount = covering.elements.filter(
        (element) => element.contour,
      ).length;
      sentences.push(
        `Surface ${covering.kind} flows toward the posterior${contourCount ? `; ${contourCount} contour tufts fan outward` : ""}.`,
      );
    }
  }
  return sentences;
}

export function describeRendererSubject(result, context) {
  if (result?.status !== "resolved")
    throw new Error("Resolved expression required for renderer prose.");
  const graph = result.graph;
  const volumes = graph.nodes
    .filter((node) => node.role === "volume")
    .sort((a, b) => a.position[0] - b.position[0]);
  const symmetry = result.facts.find((fact) => fact.id === "symmetry")?.value;
  const sentences = [
    `${symmetry ?? "The retained"} organization: ${graph.exterior ? `one continuous body with ${volumes.length} proportion regions` : volumes.length === 1 ? "one distinct body mass" : `${volumes.length} distinct body masses linked along an axis`}.`,
  ];
  const minimumX = Math.min(
    ...volumes.map((node) => node.position[0] - node.dimensions[0] / 2),
  );
  const maximumX = Math.max(
    ...volumes.map((node) => node.position[0] + node.dimensions[0] / 2),
  );
  const maximumWidth = Math.max(...volumes.map((node) => node.dimensions[1]));
  sentences.push(
    `The whole body is about ${rounded((maximumX - minimumX) / maximumWidth)} times as long as its widest region.`,
  );
  if (volumes.length > 1) {
    const equalLengths = volumes.every(
      (node) => node.dimensions[0] === volumes[0].dimensions[0],
    );
    if (equalLengths) sentences.push("The body regions have equal lengths.");
    else
      sentences.push(
        volumes
          .slice(1)
          .map(
            (node) =>
              `The ${regionName(volumes, node.id)} is ${rounded(Math.abs(node.dimensions[0] / volumes[0].dimensions[0] - 1) * 100)}% ${node.dimensions[0] >= volumes[0].dimensions[0] ? "longer" : "shorter"} than the leading region.`,
          )
          .join(" "),
      );
    if (
      volumes.every((node) => node.dimensions[1] === volumes[0].dimensions[1])
    )
      sentences.push("Their widths are equal.");
    else
      sentences.push(
        volumes
          .slice(1)
          .map((node) =>
            node.dimensions[1] === volumes[0].dimensions[1]
              ? `The ${regionName(volumes, node.id)} has the same width as the leading region.`
              : `The ${regionName(volumes, node.id)} is ${rounded(Math.abs(node.dimensions[1] / volumes[0].dimensions[1] - 1) * 100)}% ${node.dimensions[1] >= volumes[0].dimensions[1] ? "wider" : "narrower"} than the leading region.`,
          )
          .join(" "),
      );
  }
  sentences.push(
    ...attachmentDescription(graph, volumes),
    faceDescription(graph, volumes),
    ...surfaceDescription(graph),
  );
  sentences.push(
    `${context.stage}, ${context.condition}; quiet orthographic still${graph.profile?.id === "continuous-pet/1" ? ", front at the top" : ""}.`,
  );
  return sentences.join(" ");
}
