import Vector3 from "./Vector3";

export function compareVector3(a: Vector3, b: Vector3) {
  return a.x === b.x && a.y === b.y && a.z === b.z;
}
