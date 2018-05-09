import React, { Component } from 'react'

export default class extends Component {
  constructor(props) {
    super(props)
    this.state = {
      data: null,
      url: ''
    }
    this.ws = null
  }

  componentDidMount() {
    this.ws = new WebSocket('ws://127.0.0.1:3000/')
    this.ws.binaryType = 'arraybuffer';
    this.ws.onmessage = msg => {
      console.timeEnd('trip')
      const bytes = new Uint8Array(msg.data);
      if (typeof msg.data === 'object') {
        this.setState({
          data: this.binaryToDataURL([bytes.buffer])
        })
      } else {
        this.setState({ url: window.location.href + msg.data })
      }
    };
  }

  componentWillUnmount() {
    this.ws.close()
  }

  binaryToDataURL(inputArray) {
    const blob = new Blob(inputArray, { type: 'image/jpeg' });
    return URL.createObjectURL(blob);
  }

  renderImage = (dataset, transferFunction, volume, geometries, model, camera, lights, image) => {
    console.log(dataset, camera, image)
    if (!dataset || /* !transferFunction || !volume || !geometries || !model || */ !camera || /* !lights || */ !image) return
    if (!camera.type || !camera.pos || !camera.up) return
    this.ws.send(JSON.stringify({
      operation: 'render',
      params: {
        dataset,
        image: {
          width: image[0] || 1024,
          height: image[1] || 768
        },
        camera: {
          type: camera.type,
          position: {
            x: camera.pos[0] || 100,
            y: camera.pos[1] || 90,
            z: camera.pos[2] || 200
          },
          up: {
            x: camera.up[0] || 0,
            y: camera.up[1] || 1,
            z: camera.up[2] || 0
          }
        }
      }
    }))
  }

  generate = () => {
    const data = this.props.node.extras.values;
    if (data.image && data.image.camera) {
      const dataset = data.image.model ? (
        data.image.model.volume ?
          (
            data.image.model.volume.dataset ?
              data.image.model.volume.dataset.source :
              undefined
          ) : undefined
      ) : undefined;
      const camera = data.image.camera;
      const image = data.size;
      if (!dataset || /* !transferFunction || !volume || !geometries || !model || */ !camera || /* !lights || */ !image) return
      if (!camera.type || !camera.pos || !camera.up) return
      this.ws.send(JSON.stringify({
        operation: 'generate',
        params: {
          dataset,
          image: {
            width: image[0] || 1024,
            height: image[1] || 768
          },
          camera: {
            type: camera.type,
            position: {
              x: camera.pos[0] || 100,
              y: camera.pos[1] || 90,
              z: camera.pos[2] || 200
            },
            up: {
              x: camera.up[0] || 0,
              y: camera.up[1] || 1,
              z: camera.up[2] || 0
            }
          }
        }
      }))
    }
  }

  render() {
    const { data, url } = this.state
    return (
      <div>
        <div>
          <img style={{ width: '100%' }} src={data} alt="" />
        </div>
        <div className="center generate"><button onClick={this.generate}>Generate Configure</button></div>
        {url.length > 0 && <div className="center">{url}</div>}
      </div>
    )
  }
}