import _Math from "./Math";
import Quaternion from "./Quaternion";

class Vector3 {
  x: number;
  y: number;
  z: number;
  isVector3: boolean;

  constructor(x = 0, y = 0, z = 0) {
    this.x = x;
    this.y = y;
    this.z = z;
    this.isVector3 = true;
  }

  set(x: number, y: number, z: number) {
    this.x = x;
    this.y = y;
    this.z = z;

    return this;
  }

  setScalar(scalar: number) {
    this.x = scalar;
    this.y = scalar;
    this.z = scalar;

    return this;
  }

  setX(x: number) {
    this.x = x;

    return this;
  }

  setY(y: number) {
    this.y = y;

    return this;
  }

  setZ(z: number) {
    this.z = z;

    return this;
  }

  setComponent(index: number, value: number) {
    switch (index) {
      case 0:
        this.x = value;
        break;
      case 1:
        this.y = value;
        break;
      case 2:
        this.z = value;
        break;
      default:
        throw new Error("index is out of range: " + index);
    }

    return this;
  }

  getComponent(index: number) {
    switch (index) {
      case 0:
        return this.x;
      case 1:
        return this.y;
      case 2:
        return this.z;
      default:
        throw new Error("index is out of range: " + index);
    }
  }

  clone() {
    return new Vector3(this.x, this.y, this.z);
  }

  copy(v: Vector3) {
    this.x = v.x;
    this.y = v.y;
    this.z = v.z;

    return this;
  }

  copyArray(a: number[]) {
    this.x = a[0];
    this.y = a[1];
    this.z = a[2];

    return this;
  }

  add(v: Vector3, w?: Vector3) {
    if (w !== undefined) {
      return this.addVectors(v, w);
    }

    this.x += v.x;
    this.y += v.y;
    this.z += v.z;

    return this;
  }

  addScalar(s: number) {
    this.x += s;
    this.y += s;
    this.z += s;

    return this;
  }

  addVectors(a: Vector3, b: Vector3) {
    this.x = a.x + b.x;
    this.y = a.y + b.y;
    this.z = a.z + b.z;

    return this;
  }

  addScaledVector(v: Vector3, s: number) {
    this.x += v.x * s;
    this.y += v.y * s;
    this.z += v.z * s;

    return this;
  }

  sub(v: Vector3, w?: Vector3) {
    if (w !== undefined) {
      return this.subVectors(v, w);
    }

    this.x -= v.x;
    this.y -= v.y;
    this.z -= v.z;

    return this;
  }

  subScalar(s: number) {
    this.x -= s;
    this.y -= s;
    this.z -= s;

    return this;
  }

  subVectors(a: Vector3, b: Vector3) {
    this.x = a.x - b.x;
    this.y = a.y - b.y;
    this.z = a.z - b.z;

    return this;
  }

  multiply(v: Vector3, w?: Vector3) {
    if (w !== undefined) {
      return this.multiplyVectors(v, w);
    }

    this.x *= v.x;
    this.y *= v.y;
    this.z *= v.z;

    return this;
  }

  multiplyScalar(scalar: number) {
    this.x *= scalar;
    this.y *= scalar;
    this.z *= scalar;

    return this;
  }

  multiplyVectors(a: Vector3, b: Vector3) {
    this.x = a.x * b.x;
    this.y = a.y * b.y;
    this.z = a.z * b.z;

    return this;
  }

  // applyEuler(euler) {
  //   return this.applyQuaternion(_quaternion.setFromEuler(euler));
  // }

  // applyAxisAngle(axis, angle) {
  //   return this.applyQuaternion(_quaternion.setFromAxisAngle(axis, angle));
  // }

  applyMatrix3(m: any) {
    const x = this.x;
    const y = this.y;
    const z = this.z;
    const e = m.elements;

    this.x = e[0] * x + e[3] * y + e[6] * z;
    this.y = e[1] * x + e[4] * y + e[7] * z;
    this.z = e[2] * x + e[5] * y + e[8] * z;

    return this;
  }

  applyNormalMatrix(m: any) {
    return this.applyMatrix3(m).normalize();
  }

  applyMatrix4(m: any) {
    const x = this.x;
    const y = this.y;
    const z = this.z;
    const e = m.elements;

    const w = 1 / (e[3] * x + e[7] * y + e[11] * z + e[15]);

    this.x = (e[0] * x + e[4] * y + e[8] * z + e[12]) * w;
    this.y = (e[1] * x + e[5] * y + e[9] * z + e[13]) * w;
    this.z = (e[2] * x + e[6] * y + e[10] * z + e[14]) * w;

    return this;
  }

  applyQuaternion(q: Quaternion) {
    const x = this.x;
    const y = this.y;
    const z = this.z;
    const qx = q.x;
    const qy = q.y;
    const qz = q.z;
    const qw = q.w;

    // calculate quat * vector
    const ix = qw * x + qy * z - qz * y;
    const iy = qw * y + qz * x - qx * z;
    const iz = qw * z + qx * y - qy * x;
    const iw = -qx * x - qy * y - qz * z;

    // calculate result * inverse quat
    this.x = ix * qw + iw * -qx + iy * -qz - iz * -qy;
    this.y = iy * qw + iw * -qy + iz * -qx - ix * -qz;
    this.z = iz * qw + iw * -qz + ix * -qy - iy * -qx;

    return this;
  }

  project(camera: any) {
    return this.applyMatrix4(camera.matrixWorldInverse).applyMatrix4(
      camera.projectionMatrix
    );
  }

  unproject(camera: any) {
    return this.applyMatrix4(camera.projectionMatrixInverse).applyMatrix4(
      camera.matrixWorld
    );
  }

  transformDirection(m: any) {
    // input: THREE.Matrix4 affine matrix
    // vector interpreted as a direction

    const x = this.x;
    const y = this.y;
    const z = this.z;
    const e = m.elements;

    this.x = e[0] * x + e[4] * y + e[8] * z;
    this.y = e[1] * x + e[5] * y + e[9] * z;
    this.z = e[2] * x + e[6] * y + e[10] * z;

    return this.normalize();
  }

  divide(v: Vector3) {
    this.x /= v.x;
    this.y /= v.y;
    this.z /= v.z;

    return this;
  }

  divideScalar(scalar: number) {
    return this.multiplyScalar(1 / scalar);
  }

  min(v: Vector3) {
    this.x = Math.min(this.x, v.x);
    this.y = Math.min(this.y, v.y);
    this.z = Math.min(this.z, v.z);

    return this;
  }

  max(v: Vector3) {
    this.x = Math.max(this.x, v.x);
    this.y = Math.max(this.y, v.y);
    this.z = Math.max(this.z, v.z);

    return this;
  }

  clamp(min: Vector3, max: Vector3) {
    // assumes min < max, componentwise
    this.x = Math.max(min.x, Math.min(max.x, this.x));
    this.y = Math.max(min.y, Math.min(max.y, this.y));
    this.z = Math.max(min.z, Math.min(max.z, this.z));

    return this;
  }

  clampScalar(minVal: number, maxVal: number) {
    this.x = Math.max(minVal, Math.min(maxVal, this.x));
    this.y = Math.max(minVal, Math.min(maxVal, this.y));
    this.z = Math.max(minVal, Math.min(maxVal, this.z));

    return this;
  }

  clampLength(min: number, max: number) {
    const length = this.length();

    return this.divideScalar(length || 1).multiplyScalar(
      Math.max(min, Math.min(max, length))
    );
  }

  floor() {
    this.x = Math.floor(this.x);
    this.y = Math.floor(this.y);
    this.z = Math.floor(this.z);

    return this;
  }

  ceil() {
    this.x = Math.ceil(this.x);
    this.y = Math.ceil(this.y);
    this.z = Math.ceil(this.z);

    return this;
  }

  round() {
    this.x = Math.round(this.x);
    this.y = Math.round(this.y);
    this.z = Math.round(this.z);

    return this;
  }

  roundToZero() {
    this.x = this.x < 0 ? Math.ceil(this.x) : Math.floor(this.x);
    this.y = this.y < 0 ? Math.ceil(this.y) : Math.floor(this.y);
    this.z = this.z < 0 ? Math.ceil(this.z) : Math.floor(this.z);

    return this;
  }

  negate() {
    this.x = -this.x;
    this.y = -this.y;
    this.z = -this.z;

    return this;
  }

  dot(v: Vector3) {
    return this.x * v.x + this.y * v.y + this.z * v.z;
  }

  // TODO lengthSquared?

  lengthSq() {
    return this.x * this.x + this.y * this.y + this.z * this.z;
  }

  length() {
    return Math.sqrt(this.x * this.x + this.y * this.y + this.z * this.z);
  }

  manhattanLength() {
    return Math.abs(this.x) + Math.abs(this.y) + Math.abs(this.z);
  }

  normalize() {
    return this.divideScalar(this.length() || 1);
  }

  setLength(length: number) {
    return this.normalize().multiplyScalar(length);
  }

  lerp(v: Vector3, alpha: number) {
    this.x += (v.x - this.x) * alpha;
    this.y += (v.y - this.y) * alpha;
    this.z += (v.z - this.z) * alpha;

    return this;
  }

  lerpVectors(v1: Vector3, v2: Vector3, alpha: number) {
    return this.subVectors(v2, v1).multiplyScalar(alpha).add(v1);
  }

  cross(v: Vector3, w?: Vector3) {
    if (w !== undefined) {
      return this.crossVectors(v, w);
    }

    return this.crossVectors(this, v);
  }

  crossVectors(a: Vector3, b: Vector3) {
    const ax = a.x;
    const ay = a.y;
    const az = a.z;
    const bx = b.x;
    const by = b.y;
    const bz = b.z;

    this.x = ay * bz - az * by;
    this.y = az * bx - ax * bz;
    this.z = ax * by - ay * bx;

    return this;
  }

  projectOnVector(v: Vector3) {
    // v cannot be the zero v
    const scalar = v.dot(this) / v.lengthSq();

    return this.copy(v).multiplyScalar(scalar);
  }

  projectOnPlane(planeNormal: Vector3) {
    const vector = this.clone();
    vector.projectOnVector(planeNormal);

    return this.sub(vector);
  }

  reflect(normal: Vector3) {
    // reflect incident vector off plane orthogonal to normal
    // normal is assumed to have unit length
    const vector = normal.clone();
    return this.sub(vector.multiplyScalar(2 * this.dot(normal)));
  }

  angleTo(v: Vector3) {
    const denominator = Math.sqrt(this.lengthSq() * v.lengthSq());

    const theta = this.dot(v) / denominator;

    // clamp, to handle numerical problems
    return Math.acos(_Math.clamp(theta, -1, 1));
  }

  distanceTo(v: Vector3) {
    return Math.sqrt(this.distanceToSquared(v));
  }

  distanceToSquared(v: Vector3) {
    var dx = this.x - v.x,
      dy = this.y - v.y,
      dz = this.z - v.z;

    return dx * dx + dy * dy + dz * dz;
  }

  manhattanDistanceTo(v: Vector3) {
    return (
      Math.abs(this.x - v.x) + Math.abs(this.y - v.y) + Math.abs(this.z - v.z)
    );
  }

  setFromSpherical(s: any) {
    return this.setFromSphericalCoords(s.radius, s.phi, s.theta);
  }

  setFromSphericalCoords(radius: number, phi: number, theta: number) {
    const sinPhiRadius = Math.sin(phi) * radius;

    this.x = sinPhiRadius * Math.sin(theta);
    this.y = Math.cos(phi) * radius;
    this.z = sinPhiRadius * Math.cos(theta);

    return this;
  }

  setFromCylindrical(c: any) {
    return this.setFromCylindricalCoords(c.radius, c.theta, c.y);
  }

  setFromCylindricalCoords(radius: number, theta: number, y: number) {
    this.x = radius * Math.sin(theta);
    this.y = y;
    this.z = radius * Math.cos(theta);

    return this;
  }

  setFromMatrixPosition(m) {
    const e = m.elements;

    this.x = e[12];
    this.y = e[13];
    this.z = e[14];

    return this;
  }

  setFromMatrixScale(m: any) {
    const sx = this.setFromMatrixColumn(m, 0).length();
    const sy = this.setFromMatrixColumn(m, 1).length();
    const sz = this.setFromMatrixColumn(m, 2).length();

    this.x = sx;
    this.y = sy;
    this.z = sz;

    return this;
  }

  setFromMatrixColumn(m, index: number) {
    return this.fromArray(m.elements, index * 4);
  }

  equals(v: Vector3) {
    return v.x === this.x && v.y === this.y && v.z === this.z;
  }

  equalsArray(a: number[]) {
    return a[0] === this.x && a[1] === this.y && a[2] === this.z;
  }

  fromArray(array: Array<number>, offset: number) {
    if (offset === undefined) offset = 0;

    this.x = array[offset];
    this.y = array[offset + 1];
    this.z = array[offset + 2];

    return this;
  }

  toArray(array?: Array<number>, offset?: number) {
    if (array === undefined) array = [];
    if (offset === undefined) offset = 0;

    array[offset] = this.x;
    array[offset + 1] = this.y;
    array[offset + 2] = this.z;

    return array;
  }

  // fromBufferAttribute(attribute, index) {
  //   this.x = attribute.getX(index);
  //   this.y = attribute.getY(index);
  //   this.z = attribute.getZ(index);

  //   return this;
  // }
}

export default Vector3;
