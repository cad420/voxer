import {
  DOMWidgetModel,
  DOMWidgetView,
  ISerializers,
} from "@jupyter-widgets/base";
import { ModuleName, ModuleVersion } from "./meta";
import LinearPieceWiseEditor from "./LinearPisewide";
import Pickr from "@simonwep/pickr";
import "./style.css";

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

class RendererModel extends DOMWidgetModel {}

class RendererView extends DOMWidgetView {}

export default {
  TransferFunctionModel,
  TransferFunctionView,
  RendererModel,
  RendererView,
};
