import React, { Component } from 'react'
import { message } from 'antd';
import { PerspectiveCamera, Vector3 } from 'three'
import Trackball from './trackball'
import ConfigContext from '../store/config';

function verifyVolume(params) {
  if (params.type === 'structured') {
    const tfcnParams = params.transferfunction;
    const datasetParams = params.dataset;
    if (!tfcnParams || !datasetParams) return false;
    return true;
  } else if (params.type === 'diff') {
    if (!verifyVolume(params.first)) return false;
    if (!verifyVolume(params.second)) return false;
  } else if (params.type === 'transform') {
    if (!verifyVolume(params.volume)) return false;
  } else if (params.type === 'clipping') {
    if (!verifyVolume(params.volume1)) return false;
  }
}

function verify(params) {
  if (!params.width || !params.height) return false;
  const renderParams = params.image;
  if (!renderParams) return false;
  const volumesParams = renderParams.volumes;
  if (!volumesParams || volumesParams.length === 0) return false;
  for (let i = 0; i < volumesParams.length; i++) {
    const volumeParams = volumesParams[i];
    if (!volumeParams) return false;
    if (!verifyVolume(volumeParams)) return false;
  }
  return true;
}

class Display extends Component {
  constructor(props) {
    super(props)
    this.state = {
      data: null,
      url: '',
      interacting: false
    };
    this.ws = null;
    this.timeoutId = null;
  }

  componentDidMount() {
    this.connect();
    this.image.addEventListener('mousedown', this.setInteracting)
    this.image.addEventListener('mouseup', this.unsetInteracting)
    window.addEventListener('mouseup', this.unsetInteracting)
  }

  componentDidUpdate() {
    const imageSize = this.image.getBoundingClientRect()
    const { data } = this.state
    if (!data || this.controls || imageSize.height === 0) return
    const camera = new PerspectiveCamera(60, 1, 0.01, 10000)
    const links = this.props.model.ports.image.links
    const linkKeys = Object.keys(links)
    const renderer = links[linkKeys[0]].sourcePort.parent
    camera.position.x = renderer.extras.values.pos[0]
    camera.position.y = renderer.extras.values.pos[1]
    camera.position.z = renderer.extras.values.pos[2]
    this.controls = new Trackball(camera, this.image)
    this.controls.rotateSpeed = 3.0;
    this.controls.zoomSpeed = 1.2;
    this.controls.panSpeed = 2.8;
    this.controls.staticMoving = true;
    this.controls.dynamicDampingFactor = 0.3;
    this.controls.updateCount = 0;
    this.controls.addEventListener('change', this.updateCamera)
  }

  componentWillReceiveProps(nextProps) {
    if ('server' in nextProps) {
      this.connect();
    }
  }

  componentWillUnmount() {
    this.ws.close()
    this.controls && this.controls.removeEventListener('change', this.renderImage)
    this.image.removeEventListener('mousedown', this.setInteracting)
    this.image.removeEventListener('mouseup', this.unsetInteracting)
    window.removeEventListener('mouseup', this.unsetInteracting)
    window.cancelAnimationFrame(this.loopId)
  }

  connect = () => {
    const { server } = this.props;
    if (this.ws) this.ws.close();
    if (this.loopId) window.cancelAnimationFrame(this.loopId);
    this.ws = new WebSocket('ws://' + server)
    this.ws.binaryType = 'arraybuffer'
    this.ws.onmessage = msg => {
      const bytes = new Uint8Array(msg.data)
      if (typeof msg.data === 'object') {
        const blob = new Blob([bytes.buffer], { type: 'image/jpeg' })
        this.setState({ data: URL.createObjectURL(blob) })
      } else {
        const data = JSON.parse(msg.data)
        if (data.type === 'config') {
          this.setState({ url: `http://${server}/${data.value}` })
        } else if (data.type === 'error') {
          message.error(data.value)
        }
      }
    }
    this.loopId = window.requestAnimationFrame(this.loop)
    this.ws.onopen = this.renderImage
  }

  setInteracting = () => {
    if (this.state.interacting) return
    this.setState({ interacting: true })
  }

  unsetInteracting = () => {
    if (!this.state.interacting) return
    this.setState({ interacting: false })
  }

  loop = () => {
    this.controls && this.controls.update()
    this.loopId = window.requestAnimationFrame(this.loop)
  }

  updateCamera = () => {
    this.controls.updateCount++;
    if (this.controls.updateCount < 5) return;
    this.controls.updateCount = 0;
    const { model } = this.props
    const dir = new Vector3()
    const camera = this.controls.object
    camera.getWorldDirection(dir)
    const links = model.ports.image.links
    const linkKeys = Object.keys(links)
    const renderer = links[linkKeys[0]].sourcePort.parent
    const pos = camera.position
    const up = camera.up
    renderer.extras.values.pos = [
      parseFloat(pos.x.toFixed(2)),
      parseFloat(pos.y.toFixed(2)),
      parseFloat(pos.z.toFixed(2))
    ];
    renderer.extras.values.up = [
      parseFloat(up.x.toFixed(2)),
      parseFloat(up.y.toFixed(2)),
      parseFloat(up.z.toFixed(2))
    ]
    renderer.extras.values.dir = [
      parseFloat(dir.x.toFixed(2)),
      parseFloat(dir.y.toFixed(2)),
      parseFloat(dir.z.toFixed(2))
    ]
    this.renderImage()
  }

  renderImage = () => {
    this.sendRequest('render')
  }

  generate = () => {
    this.sendRequest('generate', false)
  }

  sendRequestForHighQuality = () => this.sendRequest('render', false)

  sendRequest = (operation, interacting = true) => {
    const { model } = this.props
    const { url } = this.state
    const params = model.extras.values
    if (!verify(params)) {
      this.setState({ data: null, url: '' })
      this.controls = null
      return
    }
    return
    if (url.length) {
      this.setState({ url: '' })
    }
    if (interacting) {
      clearTimeout(this.timeoutId)
      this.ws.send(JSON.stringify({
        operation, params: {
          ...params,
          width: 64, height: Math.ceil(64 / params.width * params.height)
        }
      }))
      this.timeoutId = setTimeout(this.sendRequestForHighQuality, 100);
    } else {
      this.ws.send(JSON.stringify({ operation, params }))
    }
  }

  render() {
    const { data, url, interacting } = this.state
    return (
      <div>
        <div>
          <img
            ref={img => this.image = img}
            className={'rendered-image' + (interacting ? ' interacting' : '')}
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

export default class extends React.Component {
  renderImage = () => {
    this.ref.renderImage();
  }

  render() {
    return (
      <ConfigContext.Consumer>
        {({ server }) => (
          <Display
            {...this.props}
            server={server}
            ref={ref => this.ref = ref}
          />
        )}
      </ConfigContext.Consumer>
    );
  }
}