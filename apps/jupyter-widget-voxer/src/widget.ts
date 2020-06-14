import {
  DOMWidgetModel,
  DOMWidgetView,
  ISerializers,
} from "@jupyter-widgets/base";
import { name, version } from "./version";

export class ExampleModel extends DOMWidgetModel {
  defaults() {
    return {
      ...super.defaults(),
      _model_name: ExampleModel.model_name,
      _model_module: ExampleModel.model_module,
      _model_module_version: ExampleModel.model_module_version,
      _view_name: ExampleModel.view_name,
      _view_module: ExampleModel.view_module,
      _view_module_version: ExampleModel.view_module_version,
      value: "Hello World",
    };
  }

  static serializers: ISerializers = {
    ...DOMWidgetModel.serializers,
    // Add any extra serializers here
  };

  static model_name = "ExampleModel";
  static model_module = name;
  static model_module_version = version;
  static view_name = "ExampleView"; // Set to null if no view
  static view_module = name; // Set to null if no view
  static view_module_version = version;
}

export class ExampleView extends DOMWidgetView {
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
  ExampleModel,
  ExampleView,
};
