export const COMPOSITIONAL_DRAFT_SCHEMA = "compositional-authoring-delta/1";
export const COMPOSITIONAL_DRAFT_ID = "genomic-compositional-source-draft";
export const COMPOSITIONAL_DRAFT_PACKET = "compositional-authored-record/1";

export function isCompositionalDraft(catalogue) {
  return catalogue?.id === `${COMPOSITIONAL_DRAFT_ID}-${catalogue.authoredRecipe?.forkId}` &&
    catalogue.authoredRecipe?.schemaVersion === COMPOSITIONAL_DRAFT_SCHEMA &&
    catalogue.foundationPin?.profile === "compositional-authored-foundation/1" &&
    catalogue.foundationPin.id === catalogue.id && catalogue.foundationPin.version === catalogue.version &&
    catalogue.authoredRecipe.definitionPin?.profile === catalogue.foundationPin.profile &&
    catalogue.authoredRecipe.definitionPin.id === catalogue.id && catalogue.authoredRecipe.definitionPin.version === catalogue.version &&
    catalogue.foundationPin.digest === catalogue.authoredRecipe.definitionPin?.digest;
}

// Use the exact published parent supplied by the catalogue endpoint. Never
// replace missing inherited copies when constructing an authoring recipe.
export function compositionalDraftRecipe(catalogue, parent, startingCopies, baselineMetadata = {}) {
  const previous = catalogue.authoredRecipe;
  return {
    schemaVersion: COMPOSITIONAL_DRAFT_SCHEMA,
    parent: parent.foundationPin,
    forkId: previous?.forkId ?? crypto.randomUUID().replaceAll("-", "").slice(0, 16),
    revision: (previous?.revision ?? 0) + 1,
    records: catalogue.loci.filter((record) => {
      const original = parent.loci.find((locus) => locus.id === record.id);
      return JSON.stringify(record) !== JSON.stringify(original);
    }),
    startingCopies,
    baselineMetadata
  };
}
