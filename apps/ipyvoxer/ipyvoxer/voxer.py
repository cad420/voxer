from ipywidgets import (Widget, DOMWidget, widget_serialization)
from traitlets import HasTraits, Unicode, Dict, List, Float, Instance, Bytes, observe, Bool
from ._meta import module_name
import pyvoxer
import copy


class TransferFunction(DOMWidget):
    _model_name = Unicode("TransferFunctionModel").tag(sync=True)
    _model_module = Unicode(module_name).tag(sync=True)
    _model_module_version = Unicode('^0.1.0').tag(sync=True)

    _view_name = Unicode('TransferFunctionView').tag(sync=True)
    _view_module = Unicode(module_name).tag(sync=True)
    _view_module_version = Unicode('^0.1.0').tag(sync=True)

    points = List([dict(x=0, y=0, color='#333333'), dict(
        x=1, y=1, color='#333333')]).tag(sync=True)

    def get(self):
        result = list()
        value = pyvoxer.TransferFunction()
        for point in self.points:
            p = pyvoxer.ControlPoint()
            p.x = point["x"]
            p.y = point["y"]
            p.color = point['color']
            result.append(p)
        value = result
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


OSPRayRenderer = pyvoxer.RenderingContext(pyvoxer.RenderingContext.Type.OSPRay)
OpenGLRenderer = pyvoxer.RenderingContext(pyvoxer.RenderingContext.Type.OpenGL)


class Renderer(DOMWidget):
    _model_name = Unicode("RendererModel").tag(sync=True)
    _model_module = Unicode(module_name).tag(sync=True)
    _model_module_version = Unicode('^0.1.0').tag(sync=True)

    _view_name = Unicode('RendererView').tag(sync=True)
    _view_module = Unicode(module_name).tag(sync=True)
    _view_module_version = Unicode('^0.1.0').tag(sync=True)

    backend = Unicode('OSPRay').tag(sync=True)
    image = Bytes().tag(sync=True)

    datasets = Instance(pyvoxer.DatasetStore).tag(sync=False)
    scene = Instance(pyvoxer.Scene).tag(sync=False)
    pos = List([0, 0, 1]).tag(sync=True)
    target = List([0, 0, 0]).tag(sync=True)
    up = List([0, 1, 0]).tag(sync=True)
    canRender = Bool(True).tag(sync=True)

    def __init__(self, backend):
        super(Renderer, self).__init__()
        self.backend = backend
        self.scene = pyvoxer.Scene()
        self.datasets = pyvoxer.DatasetStore()

    def addDataset(self, name, variable='default', timestep=0):
        dataset = pyvoxer.SceneDataset()
        dataset.name = name
        dataset.variable = variable
        dataset.timestep = timestep
        self.scene.datasets = [dataset]

    def addVolume(self, dataset, tfcn, render=True):
        volume = pyvoxer.Volume()
        volume.dataset = dataset
        volume.tfcn = tfcn
        volume.render = render
        self.scene.volumes = [volume]

    def addTfcn(self, tfcn):
        self.scene.tfcns = [tfcn]

    def setCamera(self, **kw):
        self.canRender = False
        if 'pos' in kw:
            self.pos = kw['pos']
        if 'target' in kw:
            self.target = kw['target']
        if 'up' in kw:
            self.up = kw['up']
        if 'width' in kw:
            self.scene.camera.width = kw['width']
        if 'height' in kw:
            self.scene.camera.height = kw['height']
        self.canRender = True
        self.send(content=dict(pos=self.pos,
                               up=self.up, target=self.target))
        self.render()

    def render(self):
        if self.scene.camera.width == 0:
            return
        if self.scene.camera.height == 0:
            return
        self.scene.camera.pos = self.pos
        self.scene.camera.target = self.target
        self.scene.camera.up = self.up
        raw_image = pyvoxer.Image()
        if (self.backend == 'OSPRay'):
            OSPRayRenderer.render(self.scene, self.datasets)
            raw_image = OSPRayRenderer.get_colors()
        else:
            OpenGLRenderer.render(self.scene, self.datasets)
            raw_image = OpenGLRenderer.get_colors()
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
