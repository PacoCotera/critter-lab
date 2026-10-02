const escape = (value) =>
  String(value)
    .replaceAll("&", "&amp;")
    .replaceAll("<", "&lt;")
    .replaceAll('"', "&quot;");
const rounded = (value) => Number(value.toFixed(6));
function namespace(graph) {
  let hash = 2166136261;
  for (const character of JSON.stringify(graph))
    hash = Math.imul(hash ^ character.charCodeAt(0), 16777619);
  return `continuous-${(hash >>> 0).toString(16)}`;
}

export function drawContinuousFamily(result, selected = null) {
  const graph = result.graph;
  if (
    result.status !== "resolved" ||
    !graph.exterior?.points ||
    graph.profile?.id !== "continuous-static/1"
  )
    throw new Error("Resolved continuous-static geometry required.");
  // One versioned world camera preserves proportion differences across relatives.
  const [minX, maxX, minY, maxY] = graph.profile.referenceCamera;
  const scale = Math.min(880 / (maxX - minX), 456 / (maxY - minY));
  const offsetX = (1024 - scale * (minX + maxX)) / 2;
  const offsetY = (600 - scale * (minY + maxY)) / 2;
  const point = ([x, y]) => [
    rounded(offsetX + x * scale),
    rounded(offsetY + y * scale),
  ];
  const polygon = (points) =>
    `<path d="${points.map((position, index) => `${index ? "L" : "M"}${point(position).join(",")}`).join(" ")}Z"/>`;
  const prefix = namespace(graph);
  let definitions = "";
  let shapes = "";
  const bodySurface = graph.surfaces.find(
    (surface) => surface.nodeId === "volume-0",
  );
  function paint(id, outline, surface, sources) {
    const clip = `${prefix}-${id}`;
    definitions += `<clipPath id="${clip}">${outline}</clipPath>`;
    const { bounds } = surface.atlas;
    const [x, y] = point([bounds.minimumX, bounds.minimumY]);
    const width = (bounds.maximumX - bounds.minimumX) * scale;
    const height = (bounds.maximumY - bounds.minimumY) * scale;
    const highlighted = selected && sources.includes(selected);
    const primary = sources[0];
    shapes += `<g data-region="${id}" data-locus="${escape(primary)}"><g clip-path="url(#${clip})"><rect x="${x}" y="${y}" width="${width}" height="${height}" fill="${surface.palette[0]}"/>`;
    if (surface.palette.length === 2)
      shapes += `<rect x="${x + width / 2}" y="${y}" width="${width / 2}" height="${height}" fill="${surface.palette[1]}"/>`;
    if (id === "continuous-body" && graph.covering.kind === "scales") {
      const selectedCovering =
        selected && graph.covering.sources.includes(selected);
      for (const plate of graph.covering.plates) {
        const plateClip = `${prefix}-${plate.id}`;
        const plateOutline = polygon(plate.points);
        definitions += `<clipPath id="${plateClip}">${plateOutline}</clipPath>`;
        shapes += `<g data-plate="${plate.id}" data-locus="${escape(graph.covering.sources[0])}"><g clip-path="url(#${plateClip})"><rect x="${x}" y="${y}" width="${width}" height="${height}" fill="${surface.palette[0]}"/>`;
        if (surface.palette.length === 2)
          shapes += `<rect data-mask="body-high-u" x="${x + width / 2}" y="${y}" width="${width / 2}" height="${height}" fill="${surface.palette[1]}"/>`;
        shapes += `</g><g fill="none" stroke="${selectedCovering ? "#d78932" : "#52625c"}" stroke-width="${selectedCovering ? 2.5 : 1}">${plateOutline}</g></g>`;
      }
    }
    for (const [index, mark] of surface.markings.entries()) {
      const mx = x + mark.u * width;
      const my = y + mark.v * height;
      const size = mark.scale * width;
      const pigment = graph.profile.markingPigment;
      if (
        mark.layout === "bands" ||
        (mark.layout === "bands-and-patches" && index % 2 === 0)
      )
        shapes += `<rect x="${mx - size / 2}" y="${my - height / 2}" width="${size}" height="${height}" fill="${pigment}" opacity="${mark.contrast}" transform="rotate(${(mark.orientation * 180) / Math.PI},${mx},${my})"/>`;
      else
        shapes += `<ellipse cx="${mx}" cy="${my}" rx="${size}" ry="${size * 0.6}" fill="${pigment}" opacity="${mark.contrast}"/>`;
    }
    shapes += `</g><g fill="none" stroke="${highlighted ? "#d78932" : "#384d4c"}" stroke-width="${highlighted ? 4 : 1.5}">${outline}</g></g>`;
  }
  // Rooted polygons share the solved body boundary; ordinary view has no graph lines.
  paint("continuous-body", polygon(graph.exterior.points), bodySurface, [
    ...graph.exterior.sources,
    ...bodySurface.sources,
  ]);
  for (const node of graph.nodes.filter((node) => node.shape)) {
    const surface = graph.surfaces.find((item) => item.nodeId === node.id);
    let outline;
    if (node.shape.kind === "polygon") outline = polygon(node.shape.points);
    else {
      const [x, y] = point(node.shape.center);
      outline = `<ellipse cx="${x}" cy="${y}" rx="${node.shape.radii[0] * scale}" ry="${node.shape.radii[1] * scale}"/>`;
    }
    paint(node.id, outline, surface, [...node.sources, ...surface.sources]);
    if (node.role === "ocular") {
      const [x, y] = point(node.shape.center);
      const pupil = node.shape.components.find(
        (component) => component.kind === "pupil-circle",
      );
      shapes += `<circle data-component="${node.id}-pupil" cx="${x}" cy="${y}" r="${pupil.radius * scale}" fill="${pupil.pigment}"/>`;
    }
  }
  if (selected) {
    for (const anchor of graph.rootAnchors.filter((anchor) =>
      anchor.sources.includes(selected),
    )) {
      const [x, y] = point(anchor.position);
      shapes += `<circle cx="${x}" cy="${y}" r="5" fill="none" stroke="#d78932" stroke-width="2"/>`;
    }
    for (const node of graph.nodes.filter(
      (node) => node.role === "tissue-join" && node.sources.includes(selected),
    )) {
      const [x, y] = point(node.position);
      shapes += `<line x1="${x}" x2="${x}" y1="${y - (node.dimensions[1] * scale) / 2}" y2="${y + (node.dimensions[1] * scale) / 2}" stroke="#d78932" stroke-width="2"/>`;
    }
  }
  return `<svg xmlns="http://www.w3.org/2000/svg" width="1024" height="600" viewBox="0 0 1024 600" role="img" aria-label="Genome-derived continuous static creature"><defs>${definitions}</defs><rect width="1024" height="600" fill="#ffffff"/>${shapes}</svg>`;
}

export function continuousReference(result, identity) {
  return {
    status: "available",
    svg: drawContinuousFamily(result),
    manifest: {
      schemaVersion: "critter-continuous-reference/1",
      identity: structuredClone(identity),
      counts: {
        volumes: result.graph.nodes.filter((node) => node.role === "volume")
          .length,
        fins: result.graph.rootAnchors.length,
        edges: result.graph.edges.length,
      },
      projection: {
        type: "orthographic XY",
        width: 1024,
        height: 600,
        referenceCamera: result.graph.profile.referenceCamera,
        sharedAcrossProfile: true,
      },
      profile: structuredClone(result.graph.profile),
      exterior: structuredClone(result.graph.exterior),
      nodes: structuredClone(result.graph.nodes),
      edges: structuredClone(result.graph.edges),
      rootAnchors: structuredClone(result.graph.rootAnchors),
      surfaces: structuredClone(result.graph.surfaces),
      covering: structuredClone(result.graph.covering),
      masks:
        "palette0 local-u<0.5; palette1 local-u>=0.5 clipped to solved outlines; coordinate halves need not have equal physical area",
      limitations: [
        ...result.limitations,
        "Unshaded flat pigment reference; highlight overlays are inspection only.",
      ],
    },
  };
}

export function describeContinuousFamily(result) {
  const graph = result.graph;
  const count = (role) =>
    graph.nodes.filter((node) => node.role === role).length;
  const bodyPalette = graph.surfaces.find(
    (surface) => surface.nodeId === "volume-0",
  ).palette;
  const finPalette = graph.surfaces.find(
    (surface) => surface.region === "fin",
  ).palette;
  return `A continuous bilateral body joins ${count("volume")} cross-section stations through ${count("tissue-join")} inherited necks. ${count("fin")} tapered fins root on its solved exterior; ${count("ocular")} ocular features and ${count("oral-aperture")} oral aperture(s) are present. Body pigments are ${bodyPalette.join("/")}; fin pigments are ${finPalette.join("/")}, with ${result.realization.markings.length} retained marking(s) and ${graph.covering.kind === "scales" ? `${graph.covering.plates.length} actual overlapping scale plates` : "skin covering"}. Supported fictional analytic media: ${
    result.motion
      .filter((motion) => motion.status === "supported")
      .map((motion) => motion.medium)
      .join(", ") || "none"
  }. Static geometry does not establish sensing, nutrition, physical motion or pet behavior.`;
}
