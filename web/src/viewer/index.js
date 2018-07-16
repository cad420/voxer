import LinearPieceWise from '../components/config/LinearPiesewise';
import Trackball from '../components/trackball'
import { PerspectiveCamera, Vector3 } from 'three'
import axios from 'axios'

const canvas = document.getElementById('tf');

const image = document.createElement('img');
image.setAttribute('src', 'http://127.0.0.1:3000/20dc5d18-6e43-4379-91ad-75af8107f8b9?width=128&height=128');
const initialPosition = [149, 149, 149];
let controls = null;

function updateCamera() {
  controls.updateCount++;
  if (controls.updateCount < 5) return;
  controls.updateCount = 0;
  const dir = new Vector3()
  const camera = controls.object
  camera.getWorldDirection(dir)
  const pos = camera.position
  const up = camera.up;
  getImage(pos, up, dir);
}

function getImage(pos, up, dir) {
  let url = 'http://127.0.0.1:3000/20dc5d18-6e43-4379-91ad-75af8107f8b9';
  url += `?pos.x=${pos.x}&pos.y=${pos.y}&pos.z=${pos.z}&`;
  url += `dir.x=${dir.x}&dir.y=${dir.y}&dir.z=${dir.z}&`;
  url += `up.x=${up.x}&up.y=${up.y}&up.z=${up.z}&`;
  url += 'width=128&height=128';
  axios.get(url, {
    responseType: 'arraybuffer'
  }).then((res) => {
    const bytes = new Uint8Array(res.data);
    const blob = new Blob([bytes.buffer], { type: 'image/jpeg' });
    image.setAttribute('src', URL.createObjectURL(blob));
  });
}

image.onload = () => {
  const container = document.getElementById('container');
  container.appendChild(image);
  const camera = new PerspectiveCamera(60, 1, 0.01, 10000);
  camera.position.x = initialPosition[0];
  camera.position.y = initialPosition[1];
  camera.position.z = initialPosition[2];
  controls = new Trackball(camera, image);
  controls.rotateSpeed = 3.0;
  controls.zoomSpeed = 4;
  controls.panSpeed = 2.8;
  controls.staticMoving = true;
  controls.dynamicDampingFactor = 0.3;
  controls.updateCount = 0;
  controls.addEventListener('change', updateCamera);
  loop();
}

function loop() {
  controls && controls.update();
  window.requestAnimationFrame(loop);
}

const editor = new LinearPieceWise(canvas);
const initialPoints = [
  { x: 0, y: 0, color: '#00008e' },
  { x: 1/6, y: 1/6, color: '#0000ff' },
  { x: 2/6, y: 2/6, color: '#00ffff' },
  { x: 3/6, y: 3/6, color: '#80ff80' },
  { x: 4/6, y: 4/6, color: '#ffff00' },
  { x: 5/6, y: 5/6, color: '#ff00ff' },
  { x: 1, y: 1, color: '#800000' }
];

editor.setControlPoints(initialPoints);
editor.render();
