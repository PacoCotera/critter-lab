import { compileCompositionalDraft } from "./compositional-draft-package.mjs";
import { COMPOSITIONAL_DRAFT_PACKET } from "./compositional-draft-format.mjs";
import { resolveCompositionalVocabulary, generateCompositionalVocabulary } from "./compositional-vocabulary-adapter.mjs";

function rejected(error) {
  return { status: "rejected", errors: [{ code: "compositional-authoring", path: "input", message: error.message }] };
}
export function validateCompositionalDraft(recipe) {
  try {
    return { valid: true, ...compileCompositionalDraft(recipe) };
  } catch (error) {
    return { valid: false, ...rejected(error) };
  }
}
function draftOperation(input, generate) {
  try {
    if (!input?.catalogue?.definitionPin) throw new Error("A validated authored definition pin is required.");
    const descriptor = compileCompositionalDraft(input.catalogue);
    const request = { ...input, catalogue: descriptor.foundation };
    return generate ? generateCompositionalVocabulary(request, descriptor) :
      resolveCompositionalVocabulary(request, "compositional-source/4", descriptor);
  } catch (error) {
    return rejected(error);
  }
}
export const resolveCompositionalDraft = (input) => draftOperation(input, false);
export const generateCompositionalDraft = (input) => draftOperation(input, true);

export function replayCompositionalDraft(record) {
  try {
    const keys = ["schemaVersion", "sceneProjectionVersion", "materialProfileVersion", "sceneRecordId", "input", "inputDigest", "resultDigest", "sceneDigest"];
    if (!record || Object.keys(record).some((key) => !keys.includes(key)) ||
        record.schemaVersion !== COMPOSITIONAL_DRAFT_PACKET || record.sceneProjectionVersion !== "compositional-source/4" ||
        record.materialProfileVersion !== "compositional-surface-fields/2") throw new Error("Unsupported authored replay identity.");
    const packet = resolveCompositionalDraft(record.input);
    if (packet.status !== "resolved") return packet;
    if (["inputDigest", "resultDigest", "sceneDigest", "sceneRecordId"].some((key) => record[key] !== packet[key])) {
      throw new Error("Authored recipe or source digest differs.");
    }
    return { ...packet, replay: { status: "verified", profileVersion: record.sceneProjectionVersion } };
  } catch (error) {
    return rejected(error);
  }
}
