define(function () { 'use strict';

    if (window.require) {
        window.require.config({
            map: {
                "*": {
                    "jupyter-widget-voxer": "nbextensions/jupyter-widget-voxer/index",
                },
            },
        });
    }
    // Export the required load_ipython_extention
    var extension = {
        load_ipython_extension: function () { },
    };

    return extension;

});
