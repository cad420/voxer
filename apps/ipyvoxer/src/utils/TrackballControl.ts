import Vector3 from "./Vector3.js";
import Vector2 from "./Vector2.js";
import Quaternion from "./Quaternion.js";

enum STATE {
  NONE,
  ROTATE,
  ZOOM,
  PAN,
  TOUCH_ROTATE,
  TOUCH_ZOOM_PAN,
}

enum CONTROL {
  ROTATE, // left button
  ZOOM, // usually the wheel button or the middle button
  PAN, // usually the right button
}

enum MouseButtons {
  LEFT = CONTROL.ROTATE,
  MIDDLE = CONTROL.ZOOM,
  RIGHT = CONTROL.PAN,
}

const EPS = 0.000001;

class TrackballControl {
  el: HTMLElement;
  screen: { left: number; top: number; width: number; height: number };
  enabled: boolean;
  rotateSpeed: number;
  panSpeed: number;
  zoomSpeed: number;
  noRotate: boolean;
  noZoom: boolean;
  noPan: boolean;

  staticMoving: boolean;
  dynamicDampingFactor: number;

  minDistance: number;
  maxDistance: number;

  position: Vector3;
  target: Vector3;
  up: Vector3;
  zoom: number;

  lastPosition: Vector3;
  lastZoom: number;

  _state: STATE;
  _keyState: STATE;
  _eye: Vector3;
  _movePrev: Vector2;
  _moveCurr: Vector2;
  _lastAxis: Vector3;
  _lastAngle: number;
  _zoomStart: Vector2;
  _zoomEnd: Vector2;
  // _touchZoomDistanceStart: number;
  // _touchZoomDistanceEnd: number;
  _panStart: Vector2;
  _panEnd: Vector2;

  constructor(el: HTMLElement) {
    this.el = el;
    this.screen = { left: 0, top: 0, width: 0, height: 0 };

    this.handleResize();

    this.enabled = true;

    this.rotateSpeed = 1.0;
    this.zoomSpeed = 1.2;
    this.panSpeed = 0.3;

    this.noRotate = false;
    this.noZoom = false;
    this.noPan = false;

    this.staticMoving = false;
    this.dynamicDampingFactor = 0.2;

    this.minDistance = 0;
    this.maxDistance = Infinity;

    this.position = new Vector3(0, 0, 1);
    this.target = new Vector3(0, 0, -1);
    this.up = new Vector3(0, 1, 0);
    this.zoom = 1;

    this.lastPosition = new Vector3();
    this.lastZoom = 1;

    this._state = STATE.NONE;
    this._keyState = STATE.NONE;
    this._eye = new Vector3();
    this._movePrev = new Vector2();
    this._moveCurr = new Vector2();
    this._lastAxis = new Vector3();
    this._lastAngle = 0;
    this._zoomStart = new Vector2();
    this._zoomEnd = new Vector2();
    // this._touchZoomDistanceStart = 0;
    // this._touchZoomDistanceEnd = 0;
    this._panStart = new Vector2();
    this._panEnd = new Vector2();

    this.addEventListener();
  }

  addEventListener() {
    this.el.addEventListener("contextmenu", this.contextmenu, false);
    this.el.addEventListener("mousedown", this.mousedown, false);
    this.el.addEventListener("wheel", this.mousewheel, false);

    // this.el.addEventListener("touchstart", this.touchstart, false);
    // this.el.addEventListener("touchend", this.touchend, false);
    // this.el.addEventListener("touchmove", this.touchmove, false);

    // window.addEventListener("keydown", this.keydown, false);
    // window.addEventListener("keyup", this.keyup, false);

    this.update();
  }

  handleResize() {
    const box = this.el.getBoundingClientRect();
    // adjustments come from similar code in the jquery offset() function
    const d = this.el.ownerDocument.documentElement;
    this.screen.left = box.left + window.pageXOffset - d.clientLeft;
    this.screen.top = box.top + window.pageYOffset - d.clientTop;
    this.screen.width = box.width;
    this.screen.height = box.height;
  }

  getMouseOnScreen(pageX: number, pageY: number) {
    return new Vector2(
      (pageX - this.screen.left) / this.screen.width,
      (pageY - this.screen.top) / this.screen.height
    );
  }

  getMouseOnCircle(pageX: number, pageY: number) {
    const { left, top, width, height } = this.screen;
    return new Vector2(
      (pageX - width * 0.5 - left) / (width * 0.5),
      (height + 2 * (top - pageY)) / width // screen.width intentional
    );
  }

  update() {
    this._eye.subVectors(this.position, this.target);

    if (!this.noRotate) {
      this.rotateCamera();
    }

    if (!this.noZoom) {
      this.zoomCamera();
    }

    if (!this.noPan) {
      this.panCamera();
    }

    this.position.addVectors(this.target, this._eye);

    this.checkDistances();
    if (this.lastPosition.distanceToSquared(this.position) > EPS) {
      this.lastPosition.copy(this.position);
    }
  }

  rotateCamera() {
    const axis = new Vector3();
    const quaternion = new Quaternion();
    const eyeDirection = new Vector3();
    const cameraUpDirection = new Vector3();
    const cameraSidewaysDirection = new Vector3();
    const moveDirection = new Vector3(
      this._moveCurr.x - this._movePrev.x,
      this._moveCurr.y - this._movePrev.y,
      0
    );
    let angle = moveDirection.length();

    if (angle) {
      this._eye.copy(this.position).sub(this.target);

      eyeDirection.copy(this._eye).normalize();
      cameraUpDirection.copy(this.up).normalize();
      cameraSidewaysDirection
        .crossVectors(cameraUpDirection, eyeDirection)
        .normalize();

      cameraUpDirection.setLength(this._moveCurr.y - this._movePrev.y);
      cameraSidewaysDirection.setLength(this._moveCurr.x - this._movePrev.x);

      moveDirection.copy(cameraUpDirection.add(cameraSidewaysDirection));

      axis.crossVectors(moveDirection, this._eye).normalize();

      angle *= this.rotateSpeed;
      quaternion.setFromAxisAngle(axis, angle);

      this._eye.applyQuaternion(quaternion);
      this.up.applyQuaternion(quaternion);

      this._lastAxis.copy(axis);
      this._lastAngle = angle;
    } else if (!this.staticMoving && this._lastAngle) {
      this._lastAngle *= Math.sqrt(1.0 - this.dynamicDampingFactor);
      this._eye.copy(this.position).sub(this.target);
      quaternion.setFromAxisAngle(this._lastAxis, this._lastAngle);
      this._eye.applyQuaternion(quaternion);
      this.up.applyQuaternion(quaternion);
    }

    this._movePrev.copy(this._moveCurr);
  }

  zoomCamera() {
    let factor;

    if (this._state === STATE.TOUCH_ZOOM_PAN) {
      // touch
    } else {
      factor = 1.0 + (this._zoomEnd.y - this._zoomStart.y) * this.zoomSpeed;

      if (factor !== 1.0 && factor > 0.0) {
        this._eye.multiplyScalar(factor);
      }

      if (this.staticMoving) {
        this._zoomStart.copy(this._zoomEnd);
      } else {
        this._zoomStart.y +=
          (this._zoomEnd.y - this._zoomStart.y) * this.dynamicDampingFactor;
      }
    }
  }

  panCamera() {
    const mouseChange = this._panEnd.clone().sub(this._panStart);

    if (!mouseChange.lengthSq()) {
      return;
    }

    const cameraUp = this.up.clone();
    const pan = new Vector3();

    mouseChange.multiplyScalar(this._eye.length() * this.panSpeed);

    pan.copy(this._eye).cross(this.up).setLength(mouseChange.x);
    pan.add(cameraUp.setLength(mouseChange.y));

    this.position.add(pan);
    this.target.add(pan);

    if (this.staticMoving) {
      this._panStart.copy(this._panEnd);
    } else {
      this._panStart.add(
        mouseChange
          .subVectors(this._panEnd, this._panStart)
          .multiplyScalar(this.dynamicDampingFactor)
      );
    }
  }

  checkDistances() {
    if (this.noZoom || this.noPan) {
      return;
    }

    if (this._eye.lengthSq() > this.maxDistance * this.maxDistance) {
      this.position.addVectors(
        this.target,
        this._eye.setLength(this.maxDistance)
      );
      this._zoomStart.copy(this._zoomEnd);
    }

    if (this._eye.lengthSq() < this.minDistance * this.minDistance) {
      this.position.addVectors(
        this.target,
        this._eye.setLength(this.minDistance)
      );
      this._zoomStart.copy(this._zoomEnd);
    }
  }

  mousedown = (event: MouseEvent) => {
    if (!this.enabled) return;

    event.preventDefault();
    event.stopPropagation();

    if (this._state === STATE.NONE) {
      switch (event.button) {
        case MouseButtons.LEFT:
          this._state = STATE.ROTATE;
          break;
        case MouseButtons.MIDDLE:
          this._state = STATE.ZOOM;
          break;
        case MouseButtons.RIGHT:
          this._state = STATE.PAN;
          break;
        default:
          this._state = STATE.NONE;
      }
    }

    if (this._state === STATE.ROTATE && !this.noRotate) {
      this._moveCurr.copy(this.getMouseOnCircle(event.pageX, event.pageY));
      this._movePrev.copy(this._moveCurr);
    } else if (this._state === STATE.ZOOM && !this.noZoom) {
      this._zoomStart.copy(this.getMouseOnScreen(event.pageX, event.pageY));
      this._zoomEnd.copy(this._zoomStart);
    } else if (this._state === STATE.PAN && !this.noPan) {
      this._panStart.copy(this.getMouseOnScreen(event.pageX, event.pageY));
      this._panEnd.copy(this._panStart);
    }

    document.addEventListener("mousemove", this.mousemove, false);
    document.addEventListener("mouseup", this.mouseup, false);
  };

  mousemove = (event: MouseEvent) => {
    if (this.enabled === false) return;

    event.preventDefault();
    event.stopPropagation();

    if (this._state === STATE.ROTATE && !this.noRotate) {
      this._movePrev.copy(this._moveCurr);
      this._moveCurr.copy(this.getMouseOnCircle(event.pageX, event.pageY));
    } else if (this._state === STATE.ZOOM && !this.noZoom) {
      this._zoomEnd.copy(this.getMouseOnScreen(event.pageX, event.pageY));
    } else if (this._state === STATE.PAN && !this.noPan) {
      this._panEnd.copy(this.getMouseOnScreen(event.pageX, event.pageY));
    }
  };

  mouseup = (event: MouseEvent) => {
    if (this.enabled === false) return;

    event.preventDefault();
    event.stopPropagation();

    this._state = STATE.NONE;

    document.removeEventListener("mousemove", this.mousemove);
    document.removeEventListener("mouseup", this.mouseup);
  };

  mousewheel = (event: WheelEvent) => {
    if (this.enabled === false) return;

    if (this.noZoom === true) return;

    event.preventDefault();
    event.stopPropagation();

    switch (event.deltaMode) {
      case 2:
        // Zoom in pages
        this._zoomStart.y -= event.deltaY * 0.025;
        break;

      case 1:
        // Zoom in lines
        this._zoomStart.y -= event.deltaY * 0.01;
        break;

      default:
        // undefined, 0, assume pixels
        this._zoomStart.y -= event.deltaY * 0.00025;
        break;
    }
  };

  contextmenu = (event) => {
    if (!this.enabled) return;
    event.preventDefault();
  };

  dispose() {
    this.el.removeEventListener("contextmenu", this.contextmenu, false);
    this.el.removeEventListener("mousedown", this.mousedown, false);
    this.el.removeEventListener("wheel", this.mousewheel, false);

    // this.el.removeEventListener("touchstart", this.touchstart, false);
    // this.el.removeEventListener("touchend", this.touchend, false);
    // this.el.removeEventListener("touchmove", this.touchmove, false);

    document.removeEventListener("mousemove", this.mousemove, false);
    document.removeEventListener("mouseup", this.mouseup, false);

    // window.removeEventListener("keydown", this.keydown, false);
    // window.removeEventListener("keyup", this.keyup, false);
  }
}

export default TrackballControl;
export { TrackballControl };
