export default {
  Dataset: {
    common: {
      panel: [
        { label: 'Source', type: 'select',  }
      ],
      ports: [
        { isInput: false, name: 'dataset', label: 'out' }
      ]
    },
    type: [
      { name: 'default' }
    ]
  },
  TransferFunction: {
    common: {
      panel: [
        { label: 'TF', type: 'transfer' }
      ],
      ports: [
        { isInput: false, name: 'transferfunction', label: 'out' }
      ]
    },
    type: [
      { name: 'default' }
    ]
  },
  Volume: {
    common: {
      panel: [
        { label: 'preIntegration', type: 'switch' },
        { label: 'adaptiveSampling', type: 'switch' },
        { label: 'adaptiveScalar', type: 'input[number]' },
        { label: 'adaptiveMaxSamplingRate', type: 'input[number]' },
        { label: 'samplingRate', type: 'input[number]' },
        { label: 'specular', type: 'vec3f' },
        { label: 'volumeClippingBoxLower', type: 'vec3f' },
        { label: 'volumeClippingBoxUpper', type: 'vec3f' }
      ],
      ports: [
        { isInput: true, name: 'transferfunction', label: 'in' },
        { isInput: true, name: 'dataset', label: 'in' },
        { isInput: false, name: 'volume', label: 'out' }
      ],
    },
    type: [
      {
        name: 'Structured',
        panel: [
          { label: 'dimensions', type: 'vec3i' },
          { label: 'voxelType', type: 'select' },
          { label: 'gridOrigin', type: 'vec3f' },
          { label: 'gridSpacing', type: 'vec3f' },
        ],
      },
      {
        name: 'Adaptive Mesh Refinement',
        panel: [
          { label: 'gridOrigin', type: 'vec3f' },
          { label: 'gridSpacing', type: 'vec3f' },
          { label: 'amrMethod', type: 'select' },
          { label: 'voxelType', type: 'select' },
          { label: 'brickInfo', type: '_data' },
          { label: 'brickData', type: '_data' },
        ],
      },
      {
        name: 'Unstructured',
        panel: [
          { label: 'vertices', type: 'vec3if[]' },
          { label: 'field', type: 'intices' },
          { label: 'hexMethod', type: 'select' },
        ]
      },
    ]
  },
  Geometry: {
    common: {
      ports: [
        { isInput: false, name: 'geometry', label: 'out' }
      ],
      panel: []
    },
    type: [
      {
        name: 'Triangle Mesh',
        panel: [
          { label: 'vertex', type: 'vec3f(a)[]' },
          { label: 'vertex.normal', type: 'vec3f(a)[]' },
          { label: 'vertex.color', type: 'vec4f(a)[]' },
          { label: 'vertex.texcoord', type: 'vec2f[]' },
          { label: 'index', type: 'vec3i(a)[]' },
        ]
      },
      {
        name: 'Spheres',
        panel: [
          { label: 'radius', type: 'float' },
          { label: 'spheres', type: '_data' },
          { label: 'bytes_per_sphere', type: 'int' },
          { label: 'offset_center', type: 'int' },
          { label: 'offset_radius', type: 'int' },
          { label: 'color', type: 'vec4f[]' },
          { label: 'texcoord', type: 'vec2f[]' },
        ]
      },
      /* {
        name: 'Cylinders',
      },
      {
        name: 'Streamlines'
      },
      {
        name: 'Isosurfaces',
      },
      {
        name: 'Slices'
      } */
    ],
  },
  Model: {
    common: {
      ports: [
        { isInput: true, name: 'geometry', label: 'in' },
        { isInput: true, name: 'volume', label: 'in' },
        { isInput: false, name: 'model', label: 'out' },
      ]
    },
    type: [{ name: 'default' }]
  },
  Lights: {
    common: {
      ports: [
        { isInput: false, name: 'light', label: 'out' }
      ]
    },
    type: [
      {
        name: 'Directional Light',
        panel: [
          { label: 'direction', type: 'vec3f(a)' },
          { label: 'angularDiameter', type: 'float' }
        ]
      },
      {
        name: 'Point Light',
        panel: [
          { label: 'poition', type: 'vec3f(a)' },
          { label: 'radius', type: 'float' },
        ]
      },
      {
        name: 'Spot Light',
        panel: [
          { label: 'poition', type: 'vec3f(a)' },
          { label: 'direction', type: 'vec3f(a)' },
          { label: 'openingAngle', type: 'float' },
          { label: 'penumbraAngle', type: 'float' },
          { label: 'radius', type: 'float' }
        ]
      },
      { name: 'Quad Light' },
      { name: 'HDRI Light' },
      { name: 'Ambient  Light' },
    ]
  },
  Camera: {
    common: {
      panel: [
        { label: 'pos', type: 'vec3f(a)' },
        { label: 'dir', type: 'vec3f(a)' },
        { label: 'up', type: 'vec3f(a)' },
        { label: 'nearClip', type: 'float' },
        { label: 'imageStart', type: 'vec2f' },
        { label: 'imageEnd', type: 'vec2f' },
      ],
      ports: [
        { isInput: false, name: 'camera', label: 'out' }
      ]
    },
    type: [
      {
        name: 'Perspective Camera',
        panel: [
          { label: 'fovy', type: 'float' },
          { label: 'aspect', type: 'float' },
          { label: 'apertureRadius', type: 'float' },
          { label: 'foucsDistance', type: 'float' },
          { label: 'architectural', type: 'float' },
          { label: 'stereoMode', type: 'float' },
          { label: 'interpupillartDistance', type: 'float' },
        ]
      },
      {
        name: 'Orthographic Camera',
        panel: [
          { label: 'height', type: 'float' },
          { label: 'aspect', type: 'float' }
        ]
      }
    ]
  },
  Renderer: {
    common: {
      ports: [
        { isInput: true, name: 'model', label: 'in' },
        { isInput: true, name: 'lights', label: 'in' },
        { isInput: true, name: 'camera', label: 'in' },
        { isInput: false, name: 'renderer', label: 'out' }
      ],
      panel: [
        { name: 'epsilon', type: 'float' },
        { name: 'spp', type: 'int' },
        { name: 'maxDepth', type: 'int' },
        { name: 'minContribution', type: 'float' },
        { name: 'varianceThreshold', type: 'float' },
      ]
    },
    type: [
      {
        name: 'SciVis',
        panel: [
          { name: 'shadowsEnabled', type: 'switch' },
          { name: 'aoSamples', type: 'int' },
          { name: 'aoDistance', type: 'float' },
          { name: 'aoTransparencyEnabled', type: 'switch' },
          { name: 'oneSidedLighting', type: 'switch' },
          { name: 'bgColor', type: 'vec3f' },
          // { name: 'maxDepthTexture', type: '_texture' },
        ]
      }
    ]
  },
  /* Transition: {
    ports: []
  } */
}