import {
  DOMWidgetModel,
  DOMWidgetView,
  ISerializers,
} from "@jupyter-widgets/base";
import Pickr from "@simonwep/pickr";
import { ModuleName, ModuleVersion } from "./meta";
import LinearPieceWiseEditor from "./LinearPisewide";
import TrackballControl from "./utils/TrackballControl";
import "./style.css";
import Vector3 from "./utils/Vector3";

class TransferFunctionModel extends DOMWidgetModel {
  static model_name = "TransferFunctionModel";
  static model_module = ModuleName;
  static model_module_version = ModuleVersion;
  static view_name = "TransferFunctionView";
  static view_module = ModuleName;
  static view_module_version = ModuleVersion;
  static serializers: ISerializers = {
    ...DOMWidgetModel.serializers,
  };

  defaults() {
    return {
      ...super.defaults(),
      _model_name: TransferFunctionModel.model_name,
      _model_module: TransferFunctionModel.model_module,
      _model_module_version: TransferFunctionModel.model_module_version,
      _view_name: TransferFunctionModel.view_name,
      _view_module: TransferFunctionModel.view_module,
      _view_module_version: TransferFunctionModel.view_module_version,
    };
  }
}

class TransferFunctionView extends DOMWidgetView {
  canvas: HTMLCanvasElement;
  editor: LinearPieceWiseEditor;
  interacting: boolean;
  pickr: Pickr | null;

  constructor(options) {
    super(options);
    this.interacting = false;
    this.canvas = document.createElement("canvas");
    this.pickr = null;

    this.editor = new LinearPieceWiseEditor(this.canvas);
    this.editor.onChange((points) => {
      this.interacting = true;
      this.save(points);
      this.interacting = false;
    });
    this.editor.onEditModeChange(() => {
      if (!this.pickr) return;

      const { activeIndex, controlPoints } = this.editor;
      if (activeIndex === -1) {
        this.pickr.disable();
      } else {
        this.pickr.enable();
        this.pickr.setColor(controlPoints[activeIndex].color);
        this.pickr.applyColor(true);
      }
    });
  }

  render() {
    const el = this.el as HTMLElement;
    el.classList.add("ipyvoxer-tfcn");
    const pickrEl = document.createElement("div");

    el.appendChild(this.canvas);
    el.appendChild(pickrEl);

    this.pickr = Pickr.create({
      el: pickrEl,
      theme: "monolith", // or 'monolith', or 'nano'
      components: {
        hue: true,
        preview: true,
        opacity: true,
        interaction: {
          hex: true,
          rgba: true,
          input: true,
        },
      },
    });
    this.pickr.on("change", (hsva: Pickr.HSVaColor) => {
      if (!this.pickr) {
        return;
      }

      this.interacting = true;
      this.pickr.applyColor(true);

      const { activeIndex, controlPoints } = this.editor;
      if (activeIndex === -1) {
        return;
      }
      const point = controlPoints[activeIndex];

      const hex = hsva.toHEXA();
      const rgb = `#${hex[0]}${hex[1]}${hex[2]}`;
      if (rgb === point.color) {
        return;
      }

      point.color = rgb;

      this.editor.render();
      this.save(controlPoints);
      this.interacting = false;
    });

    this.value_changed();
    this.model.on("change:points", this.value_changed, this);
  }

  value_changed() {
    if (this.interacting) return;

    const points = this.model.get("points");
    this.editor.setControlPoints(points);
    this.editor.render();
  }

  save(points) {
    this.model.set(
      "points",
      points.map((point) => ({
        x: point.x,
        y: point.y,
        color: point.color,
      }))
    );
    this.model.save_changes();
  }
}

class RendererModel extends DOMWidgetModel {
  static model_name = "RendererModel";
  static model_module = ModuleName;
  static model_module_version = ModuleVersion;
  static view_name = "RendererView";
  static view_module = ModuleName;
  static view_module_version = ModuleVersion;
  static serializers: ISerializers = {
    ...DOMWidgetModel.serializers,
  };

  defaults() {
    return {
      ...super.defaults(),
      _model_name: RendererModel.model_name,
      _model_module: RendererModel.model_module,
      _model_module_version: RendererModel.model_module_version,
      _view_name: RendererModel.view_name,
      _view_module: RendererModel.view_module,
      _view_module_version: RendererModel.view_module_version,
    };
  }
}

class RendererView extends DOMWidgetView {
  image: HTMLImageElement;
  controller: TrackballControl | null;
  tickId: number;
  count: 0;
  save: {
    pos: Vector3;
    up: Vector3;
    target: Vector3;
  };

  constructor(options) {
    super(options);
    this.image = document.createElement("img");
    this.controller = null;
    this.tickId = 0;
    this.count = 0;
    this.save = {
      pos: new Vector3(),
      up: new Vector3(),
      target: new Vector3(),
    };
    this.model.on("msg:custom", (data) => {
      if (!this.controller) {
        return;
      }
      this.controller.position.copyArray(data.pos);
      this.controller.target.copyArray(data.target);
      this.controller.up.copyArray(data.up);
    });
  }

  render() {
    const el = this.el as HTMLElement;
    el.classList.add("ipyvoxer-image");
    el.appendChild(this.image);

    const pos = this.model.get("pos");
    const target = this.model.get("target");
    const up = this.model.get("up");
    this.save.pos.copyArray(pos);
    this.save.target.copyArray(target);
    this.save.up.copyArray(up);

    this.image_changed();
    this.image.addEventListener("load", () => {
      if (!this.controller) {
        (window as any).C = this.controller = new TrackballControl(this.image);
        this.controller.position.copyArray(pos);
        this.controller.target.copyArray(target);
        this.controller.up.copyArray(up);
        this.controller.staticMoving = true;
        this.tickId = window.requestAnimationFrame(this.tick);
      }
    });
    this.model.on("change:image", this.image_changed, this);
  }

  image_changed() {
    const arrayBuffer = this.model.get("image");
    const blob = new Blob([arrayBuffer]);
    const url = URL.createObjectURL(blob);
    this.image.src = url;
  }

  tick = () => {
    if (this.controller) {
      this.controller.update();
      if (this.count <= 2) {
        this.count += 1;
        this.tickId = window.requestAnimationFrame(this.tick);
        return;
      }

      this.count = 0;

      const { position, target, up } = this.controller;
      if (
        !position.equals(this.save.pos) ||
        !target.equals(this.save.target) ||
        !up.equals(this.save.up)
      ) {
        this.save.pos.copy(position);
        this.save.target.copy(target);
        this.save.up.copy(up);
        this.model.set({
          pos: position.toArray(),
          target: target.toArray(),
          up: up.toArray(),
        });
        this.model.save_changes();
      }
    }
    this.tickId = window.requestAnimationFrame(this.tick);
  };
}

export default {
  TransferFunctionModel,
  TransferFunctionView,
  RendererModel,
  RendererView,
};
