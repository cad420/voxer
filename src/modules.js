export default {
  Dataset: {
    common: {
      params: [
        { label: 'Source', type: 'select',  }
      ],
      ports: {
        outputs: [
          { name: 'dataset', label: 'out' }
        ]
      }
    },
    type: [
      { name: 'default' }
    ]
  },
  TransferFunction: {
    common: {
      params: [
        { label: 'TF', type: 'transfer' }
      ],
      ports: {
        outputs: [
          { name: 'transferfunction', label: 'out' }
        ]
      }
    },
    type: [
      { name: 'default' }
    ]
  },
  Volume: {
    common: {
      params: [
        { label: 'preIntegration', type: 'Switch' },
        { label: 'adaptiveSampling', type: 'Switch' },
        { label: 'adaptiveScalar', type: 'input[number]' },
        { label: 'adaptiveMaxSamplingRate', type: 'input[number]' },
        { label: 'samplingRate', type: 'input[number]' },
        { label: 'specular', type: 'vec3f' },
        { label: 'volumeClippingBoxLower', type: 'vec3f' },
        { label: 'volumeClippingBoxUpper', type: 'vec3f' }
      ],
      ports: {
        inputs: [
          { name: 'transferfunction', label: 'in' },
          { name: 'dataset', label: 'in' }
        ],
        outputs: [
          { name: 'volume', label: 'out' }
        ]
      }
    },
    type: [
      {
        name: 'Structured',
        params: [
          { label: 'dimensions', type: 'vec3i' },
          { label: 'voxelType', type: 'select' },
          { label: 'gridOrigin', type: 'vec3f' },
          { label: 'gridSpacing', type: 'vec3f' },
        ],
      },
      {
        name: 'Adaptive Mesh Refinement',
        params: [
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
        params: [
          { label: 'vertices', type: 'vec3if[]' },
          { label: 'field', type: 'intices' },
          { label: 'hexMethod', type: 'select' },
        ]
      },
    ]
  },
  Geometry: {
    common: {
      ports: {
        inputs: [{ name: 'geometry', label: 'out' }]
      }
    },
    type: [
      {
        name: 'Triangle Mesh',
        params: [
          { label: 'vertex', type: 'vec3f(a)[]' },
          { label: 'vertex.normal', type: 'vec3f(a)[]' },
          { label: 'vertex.color', type: 'vec4f(a)[]' },
          { label: 'vertex.texcoord', type: 'vec2f[]' },
          { label: 'index', type: 'vec3i(a)[]' },
        ]
      },
      {
        name: 'Spheres',
        params: [
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
      ports: {
        inputs: [
          { name: 'geometry', label: 'in' },
          { name: 'volume', label: 'in' }
        ],
        outputs: [
          { name: 'model', label: 'out' }
        ]
      }
    },
    type: [{ name: 'default' }]
  },
  Lights: {
    common: {
      ports: {
        outputs: [{ name: 'light', label: 'out' }]
      }
    },
    type: [
      {
        name: 'Directional Light',
        params: [
          { label: 'direction', type: 'vec3f(a)' },
          { label: 'angularDiameter', type: 'float' }
        ]
      },
      {
        name: 'Point Light',
        params: [
          { label: 'poition', type: 'vec3f(a)' },
          { label: 'radius', type: 'float' },
        ]
      },
      {
        name: 'Spot Light',
        params: [
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
      params: [
        { label: 'pos', type: 'vec3f(a)' },
        { label: 'dir', type: 'vec3f(a)' },
        { label: 'up', type: 'vec3f(a)' },
        { label: 'nearClip', type: 'float' },
        { label: 'imageStart', type: 'vec2f' },
        { label: 'imageEnd', type: 'vec2f' },
      ],
      ports: {
        outputs: [{ name: 'camera', label: 'out' }]
      }
    },
    type: [
      {
        name: 'Perspective Camera',
        params: [
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
        params: [
          { label: 'height', type: 'float' },
          { label: 'aspect', type: 'float' }
        ]
      }
    ]
  },
  Renderer: {
    common: {
      ports: {
        inputs: [
          { name: 'model', label: 'in' },
          { name: 'lights', label: 'in' },
          { name: 'camera', label: 'in' }
        ],
        outputs: [
          { name: 'renderer', label: 'out' }
        ]
      },
      params: [
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
        params: [
          { name: 'shadowsEnabled', type: 'Switch' },
          { name: 'aoSamples', type: 'int' },
          { name: 'aoDistance', type: 'float' },
          { name: 'aoTransparencyEnabled', type: 'Switch' },
          { name: 'oneSidedLighting', type: 'Switch' },
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