from ._meta import module_name
from ipywidgets import (Widget, DOMWidget, widget_serialization)
from traitlets import Unicode, Dict, List, Float, Instance, Bytes, observe, Bool
import pyvoxer
from pyvoxer import Dataset
import copy
from IPython.display import Image, display

class TransferFunction(DOMWidget):
    _model_name = Unicode("TransferFunctionModel").tag(sync=True)
    _model_module = Unicode(module_name).tag(sync=True)
    _model_module_version = Unicode('^0.1.0').tag(sync=True)

    _view_name = Unicode('TransferFunctionView').tag(sync=True)
    _view_module = Unicode(module_name).tag(sync=True)
    _view_module_version = Unicode('^0.1.0').tag(sync=True)

    points = List([dict(x=0, y=0, color='#333333'), dict(
        x=1, y=1, color='#333333')]).tag(sync=True)

    histogram = List().tag(sync=True)

    def get(self):
        opacities = list()
        colors = list()
        value = pyvoxer.TransferFunction()
        for point in self.points:
            p = pyvoxer.ControlPoint()
            p.x = point["x"]
            p.y = point["y"]
            p.color = point['color']
            # result.append(p)
        value.opacities = opacities
        value.colors = colors
        return value

    def update(self, index, **kw):
        clone = copy.deepcopy(self.points)
        if 'x' in kw:
            clone[index]['x'] = kw['x']
        if 'y' in kw:
            clone[index]['y'] = kw['y']
        if 'color' in kw:
            clone[index]['color'] = kw['color']
        self.points = clone

    def add(self, x, y, color):
        clone = self.points.copy()
        clone.append(dict(x=x, y=y, color=color))
        self.points = clone

    def remove(self, index):
        clone = self.points.copy()
        del clone[index]
        self.points = clone

    def set_histogram(self, histogram):
        self.histogram = histogram


class Renderer(DOMWidget):
    _model_name = Unicode("RendererModel").tag(sync=True)
    _model_module = Unicode(module_name).tag(sync=True)
    _model_module_version = Unicode('^0.1.0').tag(sync=True)

    _view_name = Unicode('RendererView').tag(sync=True)
    _view_module = Unicode(module_name).tag(sync=True)
    _view_module_version = Unicode('^0.1.0').tag(sync=True)

    image = Bytes().tag(sync=True)

    renderer = Instance(pyvoxer.VolumeRenderer)
    camera = pyvoxer.Camera()

    pos = List([0, 0, 1]).tag(sync=True)
    target = List([0, 0, 0]).tag(sync=True)
    up = List([0, 1, 0]).tag(sync=True)
    canRender = Bool(True).tag(sync=True)

    def __init__(self, backend):
        super(Renderer, self).__init__()
        self.renderer = pyvoxer.VolumeRenderer(backend)

    def add_volume(self, volume):
        self.renderer.add_volume(volume)

    def set_camera(self, **kw):
        self.canRender = False
        if 'pos' in kw:
            self.pos = kw['pos']
        if 'target' in kw:
            self.target = kw['target']
        if 'up' in kw:
            self.up = kw['up']
        if 'width' in kw:
            self.camera.width = kw['width']
        if 'height' in kw:
            self.camera.height = kw['height']
        self.canRender = True
        self.send(content=dict(pos=self.pos,
                               up=self.up, target=self.target))
        self.render()

    def render(self):
        if self.camera.width == 0:
            return
        if self.camera.height == 0:
            return
        self.camera.pos = self.pos
        self.camera.target = self.target
        self.camera.up = self.up
        self.renderer.set_camera(self.camera)
        self.renderer.render()
        raw_image = self.renderer.get_colors()
        jpeg = pyvoxer.Image.encode(
            raw_image, pyvoxer.Image.Format.JPEG, pyvoxer.Image.Quality.HIGH)
        self.image = bytes(jpeg.data)

    @observe('pos')
    @observe('up')
    @observe('target')
    def _observe_camera(self, change):
        if not self.canRender:
            return
        self.render()

class Volume(pyvoxer.Volume):
    def __init__(self):
        super(Volume, self).__init__()

    def set_tfcn(self, tfcn):
        data = pyvoxer.TransferFunction()

        for point in tfcn.points:
            h = point['color'][1:]
            color = tuple(int(h[i:i+2], 16) for i in (0, 2, 4))
            data.add_point(point['x'], point['y'], [color[0] / 255, color[1] / 255, color[2] / 255])
        self.set_transfer_function(data)
