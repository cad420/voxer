 
from .voxer import *
from .meta import __version__, version_info, module_name

def _jupyter_nbextension_paths():
    return [{
        'section': 'notebook',
        'src': 'static',
        'dest': module_name,
        'require': module_name + '/extension'
    }]
