"""
TODO: Add module docstring
"""
import ipywidgets as widgets
from traitlets import Unicode
from ._meta import module_name

class VoxerWidget(widgets.DOMWidget):
    """TODO: Add docstring here
    """
    _model_name = Unicode('VoxerModel').tag(sync=True)
    _view_name = Unicode('VoxerView').tag(sync=True)
    _view_module = Unicode(module_name).tag(sync=True)
    _model_module = Unicode(module_name).tag(sync=True)
    _view_module_version = Unicode('^0.1.0').tag(sync=True)
    _model_module_version = Unicode('^0.1.0').tag(sync=True)
    value = Unicode('Hello World').tag(sync=True)
