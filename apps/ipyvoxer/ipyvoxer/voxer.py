from ipywidgets import (Widget, DOMWidget, widget_serialization, Image)
from traitlets import HasTraits, Unicode, Dict, List, Float, Instance
from io import BytesIO
from PIL import Image
from ._meta import module_name
import matplotlib.pyplot as plt
import pyvoxer


class TransferFunction(DOMWidget):
    _model_name = Unicode("TransferFunctionModel").tag(sync=True)
    _model_module = Unicode(module_name).tag(sync=True)
    _model_module_version = Unicode('^0.1.0').tag(sync=True)

    _view_name = Unicode('TransferFunctionView').tag(sync=True)
    _view_module = Unicode(module_name).tag(sync=True)
    _view_module_version = Unicode('^0.1.0').tag(sync=True)

    control_points = List([dict(x=0, y=0, color='#333333'), dict(
        x=1, y=1, color='#333333')]).tag(sync=True)


class Renderer(HasTraits):
    renderer = Instance(pyvoxer.RenderingContext)

    def __init__(self):
        self.renderer = pyvoxer.RenderingContext(
            pyvoxer.RenderingContext.Type.OSPRay)

    def render(self, scene, datasets):
        self.renderer.render(scene, datasets)
        raw_image = self.renderer.get_colors()
        jpeg = pyvoxer.Image.encode(
            raw_image, pyvoxer.Image.Format.JPEG, pyvoxer.Image.Quality.HIGH)
        image = Image.open(BytesIO(bytes(jpeg.data)))
        ax = plt.axes([0, 0, 1, 1], frameon=False)
        ax.get_xaxis().set_visible(False)
        ax.get_yaxis().set_visible(False)
        plt.autoscale(tight=True)
        plt.imshow(image)
