"""
TODO: Add module docstring
"""

from ipywidgets import DOMWidget
from traitlets import Unicode

class ExampleWidget(DOMWidget):
    """TODO: Add docstring here
    """
    _model_name = Unicode('ExampleModel').tag(sync=True)
    _view_name = Unicode('ExampleView').tag(sync=True)

    value = Unicode('Hello World').tag(sync=True)
