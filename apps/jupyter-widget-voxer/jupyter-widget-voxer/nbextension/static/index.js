define(['exports', '@jupyter-widgets/base'], function (exports, base) { 'use strict';

  var name = "jupyter-widget-voxer";
  var version = "1.0.0";
  var description = "";
  var main = "index.js";
  var directories = {
  	example: "example"
  };
  var scripts = {
  	test: "echo \"Error: no test specified\" && exit 1",
  	build: "rollup -c"
  };
  var keywords = [
  ];
  var author = "";
  var license = "ISC";
  var dependencies = {
  	"@jupyter-widgets/base": "^3.0.0",
  	"@rollup/plugin-commonjs": "^13.0.0",
  	"@rollup/plugin-json": "^4.1.0",
  	"@rollup/plugin-node-resolve": "^8.0.1",
  	"@rollup/plugin-replace": "^2.3.3",
  	"@rollup/plugin-typescript": "^4.1.2",
  	rollup: "^2.15.0",
  	tslib: "^2.0.0",
  	typescript: "^3.9.5"
  };
  var data = {
  	name: name,
  	version: version,
  	description: description,
  	main: main,
  	directories: directories,
  	scripts: scripts,
  	keywords: keywords,
  	author: author,
  	license: license,
  	dependencies: dependencies
  };

  /**
   * The _model_module_version/_view_module_version this package implements.
   *
   * The html widget manager assumes that this is the same as the npm package
   * version number.
   */
  var MODULE_VERSION = data.version;
  /*
   * The current package name.
   */
  var MODULE_NAME = data.name;

  /*! *****************************************************************************
  Copyright (c) Microsoft Corporation.

  Permission to use, copy, modify, and/or distribute this software for any
  purpose with or without fee is hereby granted.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
  REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
  AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
  INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
  LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
  OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
  PERFORMANCE OF THIS SOFTWARE.
  ***************************************************************************** */
  /* global Reflect, Promise */

  var extendStatics = function(d, b) {
      extendStatics = Object.setPrototypeOf ||
          ({ __proto__: [] } instanceof Array && function (d, b) { d.__proto__ = b; }) ||
          function (d, b) { for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p]; };
      return extendStatics(d, b);
  };

  function __extends(d, b) {
      extendStatics(d, b);
      function __() { this.constructor = d; }
      d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
  }

  var __assign = function() {
      __assign = Object.assign || function __assign(t) {
          for (var s, i = 1, n = arguments.length; i < n; i++) {
              s = arguments[i];
              for (var p in s) if (Object.prototype.hasOwnProperty.call(s, p)) t[p] = s[p];
          }
          return t;
      };
      return __assign.apply(this, arguments);
  };

  var ExampleModel = /** @class */ (function (_super) {
      __extends(ExampleModel, _super);
      function ExampleModel() {
          return _super !== null && _super.apply(this, arguments) || this;
      }
      ExampleModel.prototype.defaults = function () {
          return __assign(__assign({}, _super.prototype.defaults.call(this)), { _model_name: ExampleModel.model_name, _model_module: ExampleModel.model_module, _model_module_version: ExampleModel.model_module_version, _view_name: ExampleModel.view_name, _view_module: ExampleModel.view_module, _view_module_version: ExampleModel.view_module_version, value: "Hello World" });
      };
      ExampleModel.serializers = __assign({}, base.DOMWidgetModel.serializers);
      ExampleModel.model_name = "ExampleModel";
      ExampleModel.model_module = MODULE_NAME;
      ExampleModel.model_module_version = MODULE_VERSION;
      ExampleModel.view_name = "ExampleView"; // Set to null if no view
      ExampleModel.view_module = MODULE_NAME; // Set to null if no view
      ExampleModel.view_module_version = MODULE_VERSION;
      return ExampleModel;
  }(base.DOMWidgetModel));
  var ExampleView = /** @class */ (function (_super) {
      __extends(ExampleView, _super);
      function ExampleView() {
          return _super !== null && _super.apply(this, arguments) || this;
      }
      ExampleView.prototype.render = function () {
          this.el.classList.add("custom-widget");
          this.value_changed();
          this.model.on("change:value", this.value_changed, this);
      };
      ExampleView.prototype.value_changed = function () {
          this.el.textContent = this.model.get("value");
      };
      return ExampleView;
  }(base.DOMWidgetView));

  // Entry point for the notebook bundle containing custom model definitions.
  //
  // Setup notebook base URL
  //
  // Some static assets may be required by the custom widget javascript. The base
  // url for the notebook is not known at build time and is therefore computed
  // dynamically.
  window.__webpack_public_path__ =
      document.querySelector("body").getAttribute("data-base-url") +
          "nbextensions/jupyter-widget-voxer";

  exports.ExampleModel = ExampleModel;
  exports.ExampleView = ExampleView;
  exports.MODULE_NAME = MODULE_NAME;
  exports.MODULE_VERSION = MODULE_VERSION;

  Object.defineProperty(exports, '__esModule', { value: true });

});
