import * as d3 from "d3";
import Emitter from "./utils/Emitter";

function pointBuilder(x, y, color = "#000000") {
  return { x, y, color };
}

function clamp(value, min = 0, max = 1) {
  return Math.max(min, Math.min(max, value));
}

function sortPoints(pointsArray) {
  pointsArray.sort((a, b) => a.x - b.x);
  /* eslint-disable no-param-reassign */
  pointsArray.forEach((point, index) => {
    point.index = index;
    point.fixedX = index === 0 || index + 1 === pointsArray.length;
  });
  /* eslint-enable no-param-reassign */
  return pointsArray;
}

export function getCanvasSize(ctx, margin = 0) {
  let { width, height } = ctx.canvas;
  width -= 2 * margin;
  height -= 2 * margin;

  return { width, height, margin };
}

function getCanvasCoordinates(ctx, point, margin) {
  const { width, height } = getCanvasSize(ctx, margin);
  let { x, y } = point;
  const { color } = point;
  x = Math.floor(x * width + margin + 0.5);
  y = Math.floor((1 - y) * height + margin + 0.5);
  return { x, y, color };
}

function drawControlPoint(ctx, point, radius, color, selected) {
  const { x, y } = point;
  if (selected) {
    ctx.beginPath();
    ctx.fillStyle = "#00c0ff";
    ctx.arc(x, y, radius + 2, 0, 2 * Math.PI, false);
    ctx.fill();
  }
  ctx.beginPath();
  ctx.fillStyle = color;
  ctx.arc(x, y, radius, 0, 2 * Math.PI, false);
  ctx.fill();
}

function getNormalizePosition(event, ctx, margin) {
  const { width, height } = getCanvasSize(ctx, margin);
  const rect = event.target.getBoundingClientRect();

  return {
    x: (event.clientX - rect.left - margin) / width,
    y: 1 - (event.clientY - rect.top - margin) / height,
    epsilon: {
      x: (2 * margin) / width,
      y: (2 * margin) / height,
    },
  };
}

function findPoint(position, pointList) {
  const pointsFound = pointList.filter(
    (point) =>
      point.x + position.epsilon.x > position.x &&
      point.x - position.epsilon.x < position.x &&
      point.y + position.epsilon.y > position.y &&
      point.y - position.epsilon.y < position.y
  );
  return pointsFound[0];
}

// ----------------------------------------------------------------------------
// LinearPieceWiseEditor
// ----------------------------------------------------------------------------

export const CHANGE_TOPIC = "LinearPieceWiseEditor.change";
const EDIT_MODE_TOPIC = "LinearPieceWiseEditor.edit.mode";

export default class LinearPieceWiseEditor extends Emitter {
  canvas: HTMLCanvasElement;
  ctx: CanvasRenderingContext2D;
  radius: number;
  controlPoints: any;
  activeIndex: number;
  historagm: number[];
  activeControlPoint: any;
  stroke: number;
  color: string;
  activePointColor: string;
  fillColor: string;

  constructor(canvas: HTMLCanvasElement, style?) {
    super();
    this.canvas = canvas;
    this.activeIndex = -1;
    this.radius = 0;
    this.historagm = [];
    this.stroke = 2;
    this.color = "#33333";
    this.activePointColor = "#EE3333";
    this.fillColor = "#EEEEEE";
    this.ctx = canvas.getContext("2d") as CanvasRenderingContext2D;
    this.resetControlPoints();
    this.setStyle(style);
    this.setContainer(canvas);
  }

  onMouseDown = (event) => {
    const click = getNormalizePosition(event, this.ctx, this.radius);
    const controlPoint = findPoint(click, this.controlPoints);
    this.activeControlPoint = controlPoint;
    if (this.activeControlPoint) {
      this.activeIndex = controlPoint.index;
      this.render();
    } else {
      this.activeIndex = -1;
      this.render();
    }
    this.canvas.addEventListener("mousemove", this.onMouseMove);
    this.emit(EDIT_MODE_TOPIC, true);
  };

  onMouseMove = (event) => {
    if (this.activeControlPoint) {
      const newPosition = getNormalizePosition(event, this.ctx, this.radius);
      if (!this.activeControlPoint.fixedX) {
        this.activeControlPoint.x = clamp(newPosition.x);
      }
      this.activeControlPoint.y = clamp(newPosition.y);
      sortPoints(this.controlPoints);
      this.activeIndex = this.activeControlPoint.index;
      this.render();
      this.emit(CHANGE_TOPIC, this.controlPoints);
    }
  };

  onMouseUp = () => {
    this.activeControlPoint = null;
    if (this.canvas) {
      this.canvas.removeEventListener("mousemove", this.onMouseMove);
    }
    this.emit(EDIT_MODE_TOPIC, false);
  };

  onMouseLeave = this.onMouseUp;

  onClick = (event) => {
    // Remove point
    if (event.metaKey || event.ctrlKey) {
      const click = getNormalizePosition(event, this.ctx, this.radius);
      const controlPoint = findPoint(click, this.controlPoints);
      if (controlPoint && !controlPoint.fixedX) {
        this.controlPoints.splice(controlPoint.index, 1);
        // fix indexes after deletion
        for (let i = 0; i < this.controlPoints.length; i += 1) {
          this.controlPoints[i].index = i;
        }
        this.activeIndex = -1;
      }
      this.render();
    }
  };

  onDblClick = (event) => {
    const point = getNormalizePosition(event, this.ctx, this.radius);
    const sanitizedPoint = {
      x: clamp(point.x),
      y: clamp(point.y),
      color: "#000000",
      index: 0,
    };
    this.controlPoints.push(sanitizedPoint);
    sortPoints(this.controlPoints);
    this.activeIndex = sanitizedPoint.index;
    this.render();
    this.emit(EDIT_MODE_TOPIC, false);
    this.emit(CHANGE_TOPIC, this.controlPoints);
  };

  resetControlPoints() {
    this.controlPoints = [pointBuilder(0, 0), pointBuilder(1, 1)];
    sortPoints(this.controlPoints);
  }

  // Sets the control points to the new list of points.  The input should be a list
  // of objects with members x and y (i.e. { x: 0.0, y: 1.0 }).  The valid range for
  // x and y is [0,1] with 0 being the left/bottom edge of the canvas and 1 being
  // the top/right edge.
  // The second parameter specifies (in the list passed in) which point should be
  // active after setting the control points.  Pass -1 for no point should be active
  setControlPoints(points, activeIndex = -1) {
    this.controlPoints = points.map((pt) => pointBuilder(pt.x, pt.y, pt.color));
    let activePoint = null;
    if (activeIndex !== -1) {
      activePoint = this.controlPoints[activeIndex];
    }
    sortPoints(this.controlPoints);
    if (activeIndex !== -1) {
      for (let i = 0; i < this.controlPoints.length; i += 1) {
        if (activePoint === this.controlPoints[i]) {
          this.activeIndex = i;
        }
      }
    } else {
      this.activeIndex = -1;
    }
    if (this.activeControlPoint) {
      this.controlPoints.forEach((pt, index) => {
        if (
          pt.x === this.activeControlPoint.x &&
          pt.y === this.activeControlPoint.y &&
          index === this.activeIndex
        ) {
          this.activeControlPoint = pt;
        }
      });
    }
  }

  setStyle({
    radius = 6,
    stroke = 2,
    color = "#000000",
    activePointColor = "#EE3333",
    fillColor = "#EEEEEE",
  } = {}) {
    this.radius = radius;
    this.stroke = stroke;
    this.color = color;
    this.activePointColor = activePointColor;
    this.fillColor = fillColor;
  }

  setContainer(canvas) {
    if (this.canvas) {
      this.canvas.removeEventListener("click", this.onClick);
      this.canvas.removeEventListener("dblclick", this.onDblClick);
      this.canvas.removeEventListener("mousedown", this.onMouseDown);
      this.canvas.removeEventListener("mouseleave", this.onMouseLeave);
      this.canvas.removeEventListener("mousemove", this.onMouseMove);
      this.canvas.removeEventListener("mouseup", this.onMouseMove);
    }

    if (canvas) {
      this.canvas = canvas;
      this.ctx = canvas.getContext("2d");

      this.canvas.addEventListener("click", this.onClick);
      this.canvas.addEventListener("dblclick", this.onDblClick);
      this.canvas.addEventListener("mousedown", this.onMouseDown);
      this.canvas.addEventListener("mouseleave", this.onMouseLeave);
      this.canvas.addEventListener("mouseup", this.onMouseUp);
    }
  }

  setActivePoint(index) {
    this.activeIndex = index;
    this.render();
  }

  clearActivePoint() {
    this.setActivePoint(-1);
  }

  setHistogram(historagm) {
    this.historagm = historagm;
  }

  render() {
    const { width, height, margin } = getCanvasSize(this.ctx, this.radius);
    this.ctx.fillStyle = this.fillColor;
    this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
    this.ctx.fillRect(margin, margin, width, height);

    if (this.historagm.length > 0) {
      const max = d3.max(this.historagm);
      const stepX = width / this.historagm.length;
      const stepY = height / max;
      this.ctx.fillStyle = "gray";
      this.ctx.beginPath();
      this.ctx.moveTo(margin, height + margin);
      this.historagm.forEach((data, i) => {
        const startX = i * stepX;
        const startY = data * stepY;
        this.ctx.lineTo(margin + startX, margin + height - startY);
      });
      this.ctx.lineTo(width + margin, height + margin);
      this.ctx.closePath();
      this.ctx.fill();
      this.ctx.fillStyle = this.fillColor;
    }

    const linearPath: any = [];
    this.controlPoints.forEach((point) => {
      linearPath.push(getCanvasCoordinates(this.ctx, point, this.radius));
    });

    // Draw path
    this.ctx.beginPath();
    this.ctx.lineWidth = this.stroke;
    linearPath.forEach((point, idx) => {
      if (idx === 0) {
        this.ctx.moveTo(point.x, point.y);
      } else {
        this.ctx.lineTo(point.x, point.y);
      }
    });
    this.ctx.stroke();

    // Draw control points
    linearPath.forEach((point, index) => {
      drawControlPoint(
        this.ctx,
        point,
        this.radius,
        point.color,
        this.activeIndex === index
      );
    });
  }

  onChange(callback) {
    return callback ? this.on(CHANGE_TOPIC, callback) : null;
  }

  onEditModeChange(callback) {
    return callback ? this.on(EDIT_MODE_TOPIC, callback) : null;
  }

  destroy() {
    this.off();
    this.setContainer(null);
  }
}
