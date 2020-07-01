import base from "@jupyter-widgets/base";
import widget, { ModuleVersion } from "./index";

export default {
  id: "jupyter.extensions.ipyvoxer",
  requires: [base.IJupyterWidgetRegistry],
  activate: (app: any, widgets: any) => {
    widgets.registerWidget({
      name: "ipyvoxer",
      version: ModuleVersion,
      exports: widget,
    });
  },
  autoStart: true,
};
