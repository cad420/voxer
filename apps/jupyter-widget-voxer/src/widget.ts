import {
  DOMWidgetModel,
  DOMWidgetView,
  ISerializers,
} from "@jupyter-widgets/base";
import { ModuleName, ModuleVersion } from "./meta";

export class VoxerModel extends DOMWidgetModel {
  defaults() {
    return {
      ...super.defaults(),
      _model_name: VoxerModel.model_name,
      _model_module: VoxerModel.model_module,
      _model_module_version: VoxerModel.model_module_version,
      _view_name: VoxerModel.view_name,
      _view_module: VoxerModel.view_module,
      _view_module_version: VoxerModel.view_module_version,
      value: "Hello World",
    };
  }

  static serializers: ISerializers = {
    ...DOMWidgetModel.serializers,
    // Add any extra serializers here
  };

  static model_name = "VoxerModel";
  static model_module = ModuleName;
  static model_module_version = ModuleVersion;
  static view_name = "VoxerView"; // Set to null if no view
  static view_module = ModuleName; // Set to null if no view
  static view_module_version = ModuleVersion;
}

export class VoxerView extends DOMWidgetView {
  render() {
    this.el.classList.add("custom-widget");

    this.value_changed();
    this.model.on("change:value", this.value_changed, this);
  }

  value_changed() {
    this.el.textContent = this.model.get("value");
  }
}

export default {
  VoxerModel,
  VoxerView,
};
