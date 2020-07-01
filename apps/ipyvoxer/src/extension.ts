if (window.require) {
  (window.require as any).config({
    map: {
      "*": {
        ipyvoxer: "nbextensions/ipyvoxer/index",
      },
    },
  });
}

// Export the required load_ipython_extention
export default {
  load_ipython_extension: function () {},
};
