export default {
  Dataset: {
    common: {
      params: [
        { label: 'source', type: 'Select', options: ['tooth', 'bucky', 'heptane', 'magnetic'] }
      ],
      ports: {
        outputs: [
          { name: 'dataset', label: 'out' }
        ]
      }
    },
    type: [
      { name: 'Default Dataset' },
      { name: 'Multivariate Scalar' },
      { name: 'Time-varying Scalar' },
    ]
  },
  Processing: {
    common: {
      params: [
        
      ],
      ports: {
        outputs: [
          { name: 'transferfunction', label: 'out' }
        ]
      }
    },
    type: [
    ]
  },
  TransferFunction: {
    common: {
      params: [
        { label: 'tfcn', type: 'TransferFunction', rangeMax: 100, rangeMin: 0, default: [
          { x: 0, y: 0, color: '#00008e' },
          { x: 1/6, y: 1/6, color: '#0000ff' },
          { x: 2/6, y: 2/6, color: '#00ffff' },
          { x: 3/6, y: 3/6, color: '#80ff80' },
          { x: 4/6, y: 4/6, color: '#ffff00' },
          { x: 5/6, y: 5/6, color: '#ff00ff' },
          { x: 1, y: 1, color: '#800000' }
        ] }
      ],
      ports: {
        outputs: [
          { name: 'transferfunction', label: 'out' }
        ]
      }
    },
    type: [
      {
        name: 'Linear Piecewise',
        type: 'linear',
      }
    ]
  },
  Volume: {
    common: {
      params: [
        { label: 'voxelRange', type: 'Vec2f' },
        { label: 'gradientShadingEnabled', type: 'Switch', default: false },
        { label: 'preIntegration', type: 'Switch', default: false },
        { label: 'singleShade', type: 'Switch', default: true },
        { label: 'adaptiveSampling', type: 'Switch', default: true },
        { label: 'adaptiveScalar', type: 'Float', default: 15 },
        { label: 'adaptiveMaxSamplingRate', type: 'Float', default: 2 },
        { label: 'samplingRate', type: 'Float', default: 0.125, min: 0.01, max: 20 },
        { label: 'specular', type: 'Vec3f', default: [0.3, 0.3, 0.3] },
        { label: 'volumeClippingBoxLower', type: 'Vec3f' },
        { label: 'volumeClippingBoxUpper', type: 'Vec3f' }
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
        name: 'Structured Volume',
        type: 'structured',
        params: [
          { label: 'dimensions', type: 'Vec3i' },
          { label: 'voxelType', type: 'Select', values: ['unchar', 'short', 'ushort', ''] },
          { label: 'gridOrigin', type: 'Vec3f' },
          { label: 'gridSpacing', type: 'Vec3f' },
        ],
      },
      {
        name: 'AMR Volume',
        type: 'AMR',
        params: [
          { label: 'gridOrigin', type: 'Vec3f' },
          { label: 'gridSpacing', type: 'Vec3f' },
          { label: 'amrMethod', type: 'select' },
          { label: 'voxelType', type: 'select' },
          { label: 'brickInfo', type: '_data' },
          { label: 'brickData', type: '_data' },
        ],
      },
      {
        name: 'Unstructured Volume',
        type: 'unstructured',
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
        inputs: [{ name: 'geometry', label: 'in' }]
      }
    },
    type: [
      {
        name: 'Triangle Mesh',
        type: 'triangle',
        params: [
          { label: 'vertex', type: 'Vec3f(a)[]' },
          { label: 'vertex.normal', type: 'Vec3f(a)[]' },
          { label: 'vertex.color', type: 'vec4f(a)[]' },
          { label: 'vertex.texcoord', type: 'vec2f[]' },
          { label: 'index', type: 'vec3i(a)[]' },
        ]
      },
      {
        name: 'Spheres',
        type: 'sphere',
        params: [
          { label: 'radius', type: 'Float' },
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
    type: [{ name: 'Default Model' }]
  },
  /* Lights: {
    common: {
      ports: {
        outputs: [{ name: 'light', label: 'out' }]
      },
      params: [
        { label: 'color', type: 'Color', default: [255, 255, 255] },
        { label: 'intensity', type: 'float', default: 1 },
        { label: 'isVisible', type: 'Switch', default: true },
      ]
    },
    type: [
      {
        name: 'Directional Light',
        type: 'directional',
        params: [
          { label: 'direction', type: 'Vec3f(a)' },
          { label: 'angularDiameter', type: 'Float' }
        ]
      },
      {
        name: 'Point Light',
        type: 'point',
        params: [
          { label: 'poition', type: 'Vec3f(a)' },
          { label: 'radius', type: 'Float' },
        ]
      },
      {
        name: 'Spot Light',
        type: 'spot',
        params: [
          { label: 'poition', type: 'Vec3f' },
          { label: 'direction', type: 'Vec3f' },
          { label: 'openingAngle', type: 'Float' },
          { label: 'penumbraAngle', type: 'Float' },
          { label: 'radius', type: 'Float' }
        ]
      },
      { name: 'Quad Light' },
      { name: 'HDRI Light' },
      { name: 'Ambient Light' },
    ]
  }, */
  /* Camera: {
    common: {
      params: [
        { label: 'pos', type: 'Vec3f' },
        { label: 'dir', type: 'Vec3f' },
        { label: 'up', type: 'Vec3f', default: [0, 1, 0], max: 1, min: -1 },
        { label: 'nearClip', type: 'Float', min: 0.0, max: 500, default: 0.0 },
      ],
      ports: {
        outputs: [{ name: 'camera', label: 'out' }]
      }
    },
    type: [
      {
        name: 'Perspective Camera',
        type: 'perspective',
        params: [
          { label: 'stereoMode', type: 'Select', options: [
            { label: 'no stereo', value: 0 },
            { label: 'left eye', value: 1 },
            { label: 'right eye', value: 2 },
            { label: 'side-by-side', value: 3 }
          ]},
          { label: 'fovy', type: 'Float' },
          { label: 'apertureRadius', type: 'Float' },
          { label: 'foucsDistance', type: 'Float' },
          { label: 'architectural', type: 'Float' },
          { label: 'interpupillartDistance', type: 'Float' },
        ]
      },
      {
        name: 'Orthographic Camera',
        type: 'orthographic',
        params: [
          { label: 'height', type: 'Float' },
          { label: 'aspect', type: 'Float' }
        ]
      }
    ]
  }, */
  Renderer: {
    common: {
      ports: {
        inputs: [
          { name: 'model', label: 'in' },
          // { name: 'lights', label: 'in' },
          // { name: 'camera', label: 'in' }
        ],
        outputs: [
          { name: 'image', label: 'out' }
        ]
      },
      params: [
        { label: 'epsilon', type: 'Float' },
        { label: 'spp', type: 'Int' },
        { label: 'maxDepth', type: 'Int' },
        { label: 'minContribution', type: 'Float' },
        { label: 'varianceThreshold', type: 'Float' },
      ]
    },
    type: [
      {
        name: 'SciVis Renderer',
        type: 'scivis',
        params: [
          { label: 'shadowsEnabled', type: 'Switch' },
          { label: 'aoSamples', type: 'Int' },
          { label: 'aoDistance', type: 'Float' },
          { label: 'aoTransparencyEnabled', type: 'Switch' },
          { label: 'oneSidedLighting', type: 'Switch' },
          { label: 'bgColor', type: 'Vec3f' },
          // { name: 'maxDepthTexture', type: '_texture' },
        ]
      }
    ]
  },
  Display: {
    common: {
      ports: {
        inputs: [
          { name: 'image', label: 'in' },
        ]
      },
      params: [
        { label: 'size', type: 'Vec2i', max: 1024, min: 0, default: [64, 64] }
      ]
    },
    type: [
      {
        name: 'Image Display',
        node: 'display',
        params: []
      },
      {
        name: 'Animation Display',
        node: 'display',
        params: [
          { label: 'frame', type: 'Int' }
        ]
      }
    ]
  },
  /* Transition: {
    ports: []
  } */
}