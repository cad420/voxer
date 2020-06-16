import {
  DOMWidgetModel,
  DOMWidgetView,
  ISerializers,
} from "@jupyter-widgets/base";
import { ModuleName, ModuleVersion } from "./meta";
import LinearPieceWiseEditor from "./LinearPisewide";

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
      value: {
        data: [
          {
            x: 0,
            y: 0,
            color: "#333333",
          },
          {
            x: 1,
            y: 1,
            color: "#333333",
          },
        ],
      },
    };
  }
}

class TransferFunctionView extends DOMWidgetView {
  canvas: HTMLCanvasElement;
  editor: LinearPieceWiseEditor;

  constructor(options) {
    super(options);
    this.canvas = document.createElement("canvas");
    this.editor = new LinearPieceWiseEditor(this.canvas);
  }

  render() {
    const el = this.el as HTMLElement;
    el.classList.add("custom-widget");
    el.appendChild(this.canvas);

    this.value_changed();
    this.model.on("change:control_points", this.value_changed, this);
  }

  value_changed() {
    const points = this.model.get("control_points");
    this.editor.setControlPoints(points);
    this.editor.render();
  }
}

export default {
  TransferFunctionModel,
  TransferFunctionView,
};
