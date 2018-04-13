export default {
  nodes: [
    {
      name: 'Dataset 1',
      color: '#ddd',
      position: [300, 100],
      ports: [
        { isInput: false, name: 'Out1', label: 'Out' },
      ]
    },
    {
      name: 'Transfer Function 1',
      color: '#ddd',
      position: [200, 300],
      ports: [
        { isInput: false, name: 'Out2', label: 'Out' },
      ]
    }
  ],
  links: []
}
