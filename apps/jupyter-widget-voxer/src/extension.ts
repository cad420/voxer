if (window.require) {
  (window.require as any).config({
    map: {
      "*": {
        "jupyter-widget-voxer": "nbextensions/jupyter-widget-voxer/index",
      },
    },
  });
}

// Export the required load_ipython_extention
export default {
  load_ipython_extension: function () {},
};
