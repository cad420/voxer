from ipywidgets import (Widget, DOMWidget, widget_serialization)
from traitlets import HasTraits, Unicode, Dict, List, Float, Instance
from ._meta import module_name


class TransferFunction(DOMWidget):
    _model_name = Unicode("TransferFunctionModel").tag(sync=True)
    _model_module = Unicode(module_name).tag(sync=True)
    _model_module_version = Unicode('^0.1.0').tag(sync=True)

    _view_name = Unicode('TransferFunctionView').tag(sync=True)
    _view_module = Unicode(module_name).tag(sync=True)
    _view_module_version = Unicode('^0.1.0').tag(sync=True)

    control_points = List([dict(x=0, y=0, color='#333333'), dict(
        x=1, y=1, color='#333333')]).tag(sync=True)


class Camera(HasTraits):
    position = List([0, 0, 3])
    target = List([0, 0, 0])
    up = List([0, 1, 0])


class Renderer(HasTraits):
    camera = Instance(Camera, default_value=Camera())

    def render(self):
        print('render...')
