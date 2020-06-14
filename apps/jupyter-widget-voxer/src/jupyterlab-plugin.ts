import base from "@jupyter-widgets/base";
import widget, { version } from "./index";

export default {
  id: "jupyter.extensions.jupyter-widget-voxer",
  requires: [base.IJupyterWidgetRegistry],
  activate: (app: any, widgets: any) => {
    widgets.registerWidget({
      name: "jupyter-widget-voxer",
      version,
      exports: widget,
    });
  },
  autoStart: true,
};
