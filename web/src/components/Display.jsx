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
    this.server =  window.location.hostname + ':3000/'
  }

  componentDidMount() {
    this.ws = new WebSocket('ws://' + this.server)
    this.ws.binaryType = 'arraybuffer'
    this.ws.onmessage = msg => {
      const bytes = new Uint8Array(msg.data)
      if (typeof msg.data === 'object') {
        const blob = new Blob([bytes.buffer], { type: 'image/jpeg' })
        this.setState({ data: URL.createObjectURL(blob) })
      } else {
        const data = JSON.parse(msg.data)
        if (data.type === 'config') {
          this.setState({ url: this.server + data.value })
        } else if (data.type === 'error') {
          window.alert(data.value)
        }
      }
    }
    this.loopId = window.requestAnimationFrame(this.loop)
    this.ws.onopen = this.renderImage
    this.image.addEventListener('mousedown', this.setInteracting)
    this.image.addEventListener('mouseup', this.unsetInteracting)
    window.addEventListener('mouseup', this.unsetInteracting)
  }

  componentDidUpdate() {
    const imageSize = this.image.getBoundingClientRect().height
    const { data } = this.state
    if (!data || this.controls || imageSize.height === 0 ) return
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
    this.controls.addEventListener('change', this.updateCamera)
  }

  componentWillUnmount() {
    this.ws.close()
    this.controls && this.controls.removeEventListener('change', this.renderImage)
    this.image.removeEventListener('mousedown', this.setInteracting)
    this.image.removeEventListener('mouseup', this.unsetInteracting)
    window.removeEventListener('mouseup', this.unsetInteracting)
    window.cancelAnimationFrame(this.loopId)
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
    const { model } = this.props
    const dir = new Vector3()
    const camera = this.controls.object
    camera.getWorldDirection(dir)
    const links = model.ports.image.links
    const linkKeys = Object.keys(links)
    const renderer = links[linkKeys[0]].sourcePort.parent
    const pos = camera.position
    const up = camera.up
    renderer.extras.values.pos = [pos.x, pos.y, pos.z]
    renderer.extras.values.up = [up.x, up.y, up.z]
    renderer.extras.values.dir = [ dir.x, dir.y, dir.z ]
    this.renderImage()
  }

  renderImage = () => {
    this.sendRequest('render')
  }

  generate = () => {
    this.sendRequest('generate')
  }

  sendRequest = (operation) => {
    const { model } = this.props
    const { url } = this.state
    const params = model.extras.values
    if (!model.extras.status) {
      this.setState({ data: null, url: '' })
      this.controls = null
      return
    }
    if (url.length) {
      this.setState({ url: '' })
    }
    this.ws.send(JSON.stringify({ operation, params }))
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