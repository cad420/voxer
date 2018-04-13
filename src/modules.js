export default {
  Dataset: {
    common: {
      params: [
        { label: 'Source', type: 'Select', options: ['tooth', 'bucky', 'heptane', 'magnetic'] }
      ],
      ports: {
        outputs: [
          { name: 'dataset', label: 'out' }
        ]
      }
    },
    type: [
      { name: 'Default Dataset' }
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
      { name: 'Default Transfer Function' }
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
        { label: 'samplingRate', type: 'Float', default: 0.125 },
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
        params: [
          { label: 'dimensions', type: 'Vec3i' },
          { label: 'voxelType', type: 'Select', values: ['unchar', 'short', 'ushort', ''] },
          { label: 'gridOrigin', type: 'Vec3f' },
          { label: 'gridSpacing', type: 'Vec3f' },
        ],
      },
      {
        name: 'AMR Volume',
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
          { label: 'vertex', type: 'Vec3f(a)[]' },
          { label: 'vertex.normal', type: 'Vec3f(a)[]' },
          { label: 'vertex.color', type: 'vec4f(a)[]' },
          { label: 'vertex.texcoord', type: 'vec2f[]' },
          { label: 'index', type: 'vec3i(a)[]' },
        ]
      },
      {
        name: 'Spheres',
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
  Lights: {
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
        params: [
          { label: 'direction', type: 'Vec3f(a)' },
          { label: 'angularDiameter', type: 'Float' }
        ]
      },
      {
        name: 'Point Light',
        params: [
          { label: 'poition', type: 'Vec3f(a)' },
          { label: 'radius', type: 'Float' },
        ]
      },
      {
        name: 'Spot Light',
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
  },
  Camera: {
    common: {
      params: [
        { label: 'pos', type: 'Vec3f' },
        { label: 'dir', type: 'Vec3f' },
        { label: 'up', type: 'Vec3f' },
        { label: 'nearClip', type: 'Float' },
        { label: 'imageStart', type: 'Vec2f' },
        { label: 'imageEnd', type: 'Vec2f' },
      ],
      ports: {
        outputs: [{ name: 'camera', label: 'out' }]
      }
    },
    type: [
      {
        name: 'Perspective Camera',
        params: [
          { label: 'fovy', type: 'Float' },
          { label: 'aspect', type: 'Float' },
          { label: 'apertureRadius', type: 'Float' },
          { label: 'foucsDistance', type: 'Float' },
          { label: 'architectural', type: 'Float' },
          { label: 'stereoMode', type: 'Float' },
          { label: 'interpupillartDistance', type: 'Float' },
        ]
      },
      {
        name: 'Orthographic Camera',
        params: [
          { label: 'height', type: 'Float' },
          { label: 'aspect', type: 'Float' }
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
        { name: 'epsilon', type: 'Float' },
        { name: 'spp', type: 'Int' },
        { name: 'maxDepth', type: 'Int' },
        { name: 'minContribution', type: 'Float' },
        { name: 'varianceThreshold', type: 'Float' },
      ]
    },
    type: [
      {
        name: 'SciVis Renderer',
        params: [
          { name: 'shadowsEnabled', type: 'Switch' },
          { name: 'aoSamples', type: 'Int' },
          { name: 'aoDistance', type: 'Float' },
          { name: 'aoTransparencyEnabled', type: 'Switch' },
          { name: 'oneSidedLighting', type: 'Switch' },
          { name: 'bgColor', type: 'Vec3f' },
          // { name: 'maxDepthTexture', type: '_texture' },
        ]
      }
    ]
  },
  /* Transition: {
    ports: []
  } */
}