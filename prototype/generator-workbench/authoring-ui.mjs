export function scopedLoci(catalogue, dimension = "all", query = "") {
  const text = query.trim().toLowerCase();
  return (catalogue?.loci ?? []).filter((locus) => {
    const inDimension = dimension === "all" || locus.family === dimension;
    const searchable = [locus.id, locus.label, ...(locus.aliases ?? [])]
      .join(" ")
      .toLowerCase();
    return inDimension && searchable.includes(text);
  });
}
export function initialAuthoringInputs(data) {
  return {
    catalogue: data.catalogue,
    genome: data.defaultGeneration.genome,
    context: data.referenceContext,
  };
}
export function authoringPackageLabel(catalogue) {
  if (catalogue.ruleVersion === "continuous-pet/1")
    return "Narrow face/material calibration";
  if (catalogue.ruleVersion === "continuous-static/1")
    return "Narrow continuous-body calibration";
  return "Anatomy-diversity diagnostic";
}
export function unconsumedOutputNotice(catalogue, locusId) {
  if (
    locusId === "energy.reserve-capacity" ||
    (["continuous-pet/1", "continuous-static/1"].includes(
      catalogue.ruleVersion,
    ) &&
      locusId === "movement.turn-control")
  )
    return "Retained resolved value; no implemented behavior or construction consumer in this profile.";
  return "";
}
export function freshGenerationSeed(previousSeed, randomSeed) {
  if (
    !Number.isInteger(randomSeed) ||
    randomSeed < 0 ||
    randomSeed > 4294967295
  )
    throw new Error("Generation seed must be a uint32 value.");
  return randomSeed === previousSeed ? (randomSeed + 1) >>> 0 : randomSeed;
}
export function isResolvedAuthoringPacket(packet) {
  return packet?.status === "resolved" && packet?.result?.status === "resolved";
}
export function scopedSelection(loci, current) {
  return loci.some((locus) => locus.id === current)
    ? current
    : (loci[0]?.id ?? null);
}
export function copyLabels(locus, genome) {
  return (genome?.loci?.[locus.id] ?? []).map(
    (id) => locus.alleles.find((allele) => allele.id === id)?.label ?? id,
  );
}
export function outputText(fact) {
  if (!fact) return "Not resolved";
  let value;
  if (typeof fact.value === "boolean")
    value = fact.value ? "present" : "absent";
  else if (Array.isArray(fact.value)) value = fact.value.join(" / ");
  else if (typeof fact.value === "object") value = JSON.stringify(fact.value);
  else value = String(fact.value);
  return `${value}${fact.unit ? ` · ${fact.unit}` : ""}`;
}
export function causalSummary(catalogue, result, id) {
  const locus = catalogue?.loci.find((item) => item.id === id);
  if (!locus) return null;
  const label = (key) =>
    catalogue.loci.find((item) => item.id === key)?.label ?? key;
  return {
    locus,
    fact: result?.facts.find((item) => item.locusId === id),
    prerequisites: (locus.requires ?? []).map((key) => ({
      id: key,
      label: label(key),
    })),
    dependents: (result?.facts ?? [])
      .filter((item) => item.locusId !== id && item.prerequisites?.includes(id))
      .map((item) => label(item.locusId)),
    targets: (result?.graph.nodes ?? [])
      .filter((node) => node.sources.includes(id))
      .map((node) => ({ id: node.id, role: node.role })),
    coveringInvolvement: Boolean(result?.graph.covering?.sources.includes(id)),
    coveringContext: Boolean(
      result?.graph.covering?.sources.includes(id) &&
        !locus.outputs.some((output) => output.startsWith("covering")),
    ),
  };
}
// A display camera consumes retained geometry; it never repairs anatomy.
export function geometryBounds(result) {
  if (result?.status !== "resolved" || !result.graph?.nodes?.length)
    return null;
  const graph = result.graph;
  const points = [];
  let invalid = false;
  function addCoordinates(value) {
    if (!Array.isArray(value)) return;
    if (
      value.length === 2 &&
      value.every((entry) => typeof entry === "number")
    ) {
      if (value.every(Number.isFinite)) points.push(value);
      else invalid = true;
    } else value.forEach(addCoordinates);
  }
  function addEllipse(center, radii) {
    if (
      !Array.isArray(center) ||
      !Array.isArray(radii) ||
      ![...center, ...radii].every(Number.isFinite) ||
      radii.some((radius) => radius < 0)
    ) {
      invalid = true;
      return;
    }
    const [x, y] = center;
    const [radiusX, radiusY] = radii;
    addCoordinates([
      [x - radiusX, y - radiusY],
      [x + radiusX, y + radiusY],
    ]);
  }
  if (graph.exterior) {
    addCoordinates(graph.exterior.points);
    // Control-point hulls also contain declared Bezier segments.
    addCoordinates(graph.exterior.controls);
    for (const node of graph.nodes) {
      addCoordinates(node.shape?.points);
      addCoordinates(node.shape?.controls);
      if (node.shape?.center && node.shape?.radii) {
        addEllipse(node.shape.center, node.shape.radii);
        for (const component of node.shape.components ?? []) {
          const offset = component.offset ?? [0, 0];
          const center = node.shape.center.map(
            (value, index) => value + offset[index],
          );
          if (component.radius !== undefined)
            addEllipse(center, [component.radius, component.radius]);
        }
      }
      if (node.role === "tissue-join") {
        const [x, y] = node.position;
        addCoordinates([
          [x, y - node.dimensions[1] / 2],
          [x, y + node.dimensions[1] / 2],
        ]);
      }
    }
    for (const anchor of graph.rootAnchors ?? [])
      addCoordinates(anchor.position);
    for (const plate of graph.covering?.plates ?? []) {
      addCoordinates(plate.points);
      addCoordinates(plate.controls);
    }
    for (const element of graph.covering?.elements ?? []) {
      for (const coordinates of Object.values(element.geometry))
        addCoordinates(coordinates);
    }
  } else {
    for (const node of graph.nodes) {
      const [x, y, z] = node.position;
      const width = node.dimensions[0];
      const height = Math.max(node.dimensions[1], node.dimensions[2]);
      addCoordinates([
        [x - width / 2, y + z * 0.3 - height / 2],
        [x + width / 2, y + z * 0.3 + height / 2],
      ]);
    }
  }
  if (invalid || !points.length) return null;
  return [
    Math.min(...points.map((point) => point[0])),
    Math.max(...points.map((point) => point[0])),
    Math.min(...points.map((point) => point[1])),
    Math.max(...points.map((point) => point[1])),
  ];
}
export function sharedPreviewCamera(results) {
  const bounds = results.map(geometryBounds);
  if (!bounds.length || bounds.some((value) => !value)) return null;
  const union = [
    Math.min(...bounds.map((value) => value[0])),
    Math.max(...bounds.map((value) => value[1])),
    Math.min(...bounds.map((value) => value[2])),
    Math.max(...bounds.map((value) => value[3])),
  ];
  // Margin also contains fixed-pixel selection rings and ordinary strokes.
  const padding =
    Math.max(union[1] - union[0], union[3] - union[2], 0.001) * 0.09;
  return [
    union[0] - padding,
    union[1] + padding,
    union[2] - padding,
    union[3] + padding,
  ];
}
