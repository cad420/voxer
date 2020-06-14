def _jupyter_nbextension_paths():
    return [{
        'section': 'notebook',
        'src': 'nbextension/static',
        'dest': 'jupyter-widget-voxer',
        'require': 'jupyter-widget-voxer/extension'
    }]
