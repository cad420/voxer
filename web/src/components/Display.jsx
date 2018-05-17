import React, { Component } from 'react'
import { PerspectiveCamera, Vector3 } from 'three'
import Trackball from './trackball'

export default class extends Component {
  constructor(props) {
    super(props)
    this.state = {
      data: null,
      url: '',
      interacting: false
    }
    this.ws = null
    this.camera = {}
  }

  componentDidMount() {
    this.props.node.extras.values.camera = {
      type: 'perspective',
      pos: new Vector3(10, 50, 200),
      up: new Vector3(0, 1, 0),
      dir: new Vector3(-10, -50, -200)
    }
    this.ws = new WebSocket('ws://' + window.location.hostname + ':3000/')
    this.ws.binaryType = 'arraybuffer';
    this.ws.onmessage = msg => {
      const bytes = new Uint8Array(msg.data);
      if (typeof msg.data === 'object') {
        const blob = new Blob([bytes.buffer], { type: 'image/jpeg' });
        this.setState({
          data: URL.createObjectURL(blob)
        })
      } else {
        this.setState({ url: window.location.hostname + ':3000/' + msg.data })
      }
    };
    this.ws.onopen = this.update
    const loop = () => {
      window.requestAnimationFrame(loop)
      this.controls && this.controls.update()
    }
    loop()
    this.image.addEventListener('mousedown', this.setInteracting)
    this.image.addEventListener('mouseup', this.unsetInteracting)
    window.addEventListener('mouseup', this.unsetInteracting)
  }

  setInteracting = () => {
    this.setState({ interacting: true })
  }

  unsetInteracting = () => {
    const { interacting } = this.state
    if (!interacting) return
    this.setState({ interacting: false })
  }

  componentWillUnmount() {
    this.ws.close()
    clearInterval(this.interval)
    this.image.removeEventListener('mousedown', this.setInteracting)
    this.image.removeEventListener('mouseup', this.unsetInteracting)
    window.removeEventListener('mouseup', this.unsetInteracting)
  }

  update = () => {
    const { node } = this.props
    const data = node.extras.values
    if (!Object.keys(node.ports.image.links).length) return
    const dataset = data.image.model ? (
      data.image.model.volume ?
        (
          data.image.model.volume.dataset ?
            data.image.model.volume.dataset.source :
            undefined
        ) : undefined
    ) : undefined;
    if (!data.image.model.volume.transferfunction) return
    const tfcn = data.image.model.volume.transferfunction.tfcn
    this.renderImage(dataset, null, null, null, null, data.camera, null, data.size, tfcn)
  }

  componentDidUpdate() {
    const data = this.props.node.extras.values
    if (data.camera && !this.controls && this.image.getBoundingClientRect().height) {
      this.camera = new PerspectiveCamera(60, 1, 0.01, 10000)
      this.camera.position.x = 10
      this.camera.position.y = 50
      this.camera.position.z = 200
      window.controls = this.controls = new Trackball(this.camera, this.image)
      this.controls.rotateSpeed = 3.0;
      this.controls.zoomSpeed = 1.2;
      this.controls.panSpeed = 2.8;
      this.controls.staticMoving = true;
      this.controls.dynamicDampingFactor = 0.3;
      this.controls.addEventListener('change', () => {
        const data = this.props.node.extras.values
        data.camera.pos = this.camera.position
        data.camera.up = this.camera.up
        this.camera.getWorldDirection(data.camera.dir)
        const dataset = data.image.model ? (
          data.image.model.volume ?
            (
              data.image.model.volume.dataset ?
                data.image.model.volume.dataset.source :
                undefined
            ) : undefined
        ) : undefined;
        if (!data.image.model.volume.transferfunction) return
        const tfcn = data.image.model.volume.transferfunction.tfcn
        this.renderImage(dataset, null, null, null, null, data.camera, null, data.size, tfcn)
      })
    }
  }

  renderImage = (dataset, transferFunction, volume, geometries, model, camera, lights, image, tfcn) => {
    if (!dataset || /* !transferFunction || !volume || !geometries || !model || */ !camera || /* !lights || */ !image || !tfcn) return
    if (!camera.type || !camera.pos || !camera.up) return
    this.sendRequest('render', dataset, camera, image, tfcn)
  }

  sendRequest = (op, dataset, camera, image, tfcn) => {
    this.ws.send(JSON.stringify({
      operation: op,
      params: {
        dataset,
        image: {
          width: image[0] || 1024,
          height: image[1] || 768
        },
        tfcn: {
          colors: tfcn.map(p => p.color),
          opacities: tfcn.map(p => p.y)
        },
        camera: {
          type: camera.type,
          pos: {
            x: camera.pos.x || 0,
            y: camera.pos.y || 0,
            z: camera.pos.z || 1
          },
          up: {
            x: camera.up.x || 0,
            y: camera.up.y || 1,
            z: camera.up.z || 0
          },
          dir: {
            x: camera.dir.x || 0,
            y: camera.dir.y || 0,
            z: camera.dir.z || -1
          }
        }
      }
    }))
  }

  generate = () => {
    const data = this.props.node.extras.values;
    const dataset = data.image.model ? (
      data.image.model.volume ?
        (
          data.image.model.volume.dataset ?
            data.image.model.volume.dataset.source :
            undefined
        ) : undefined
    ) : undefined;
    const camera = data.camera;
    const image = data.size;
    if (!dataset || /* !transferFunction || !volume || !geometries || !model || */ !camera || /* !lights || */ !image) return
    if (!camera.type || !camera.pos || !camera.up) return
    const tfcn = data.image.model.volume.transferfunction.tfcn
    this.sendRequest('generate', dataset, camera, image, tfcn)
  }

  render() {
    const { data, url, interacting } = this.state
    return (
      <div>
        <div>
          <img
            ref={img => this.image = img} className={'rendered-image' + (interacting ? ' interacting' : '')}
            src={data}
            alt=""
          />
        </div>
        <div className="center generate"><button onClick={this.generate}>Generate Configure</button></div>
        {url.length > 0 && <div className="center image-url" onMouseDown={e => e.stopPropagation()}>{url}</div>}
      </div>
    )
  }
}