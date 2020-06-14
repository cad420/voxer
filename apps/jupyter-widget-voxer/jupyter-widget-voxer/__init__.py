 
from .voxer import ExampleWidget
from ._version import __version__, version_info

def _jupyter_nbextension_paths():
    return [{
        'section': 'notebook',
        'src': 'static',
        'dest': 'jupyter-widget-voxer',
        'require': 'jupyter-widget-voxer/extension'
    }]
