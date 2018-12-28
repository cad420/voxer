import * as d3 from 'd3';
import React from 'react';
import axios from 'axios';

export default class extends React.Component {
  constructor(props) {
    super(props);
    const margin = { top: 20, right: 20, bottom: 30, left: 40 };
    const width = 600 - margin.left - margin.right;
    const height = 600 - margin.top - margin.bottom;

    const x = d3.scaleLinear()
      .domain(([0, 255]))
      .range([margin.left, width - margin.right]);

    const y = d3.scaleLinear()
      .domain(([0, 255]))
      .range([height - margin.bottom, margin.top]);

    this.state = {
      width,
      height,
      margin,
      x, y,
      data: [],
      value: []
    };
  }

  componentDidMount() { 
    this.getData();
  }

  getData = () => {
    axios.get('http://localhost:3000/scatter/lsabel-TCf-08/lsabel-Pf-08/').then((res) => {
      const data = res.data;
      data.x = 'Pf';
      data.y = 'Tcf';
      this.setState({ data });
    });
  }

  brushed = () => {
    const { data, x, y } = this.state;
    let value = [];
    if (d3.event.selection) {
      const [[x0, y0], [x1, y1]] = d3.event.selection;
      value = data.filter(d => x0 <= x(d.x) && x(d.x) < x1 && y0 <= y(d.y) && y(d.y) < y1);
    }
    window.values = value;
    this.setState({ value });
  }

  render() {
    const { width, height, margin, x, y, data } = this.state;
    return (
      <div style={{ position: 'relative', background: 'white' }}>
        <canvas
          width={width}
          height={height}
          style={{ position: 'absolute', left: 0, top: 0, zIndex: 1 }}
          ref={
            c => {
              if (!c) return;
              const ctx = c.getContext('2d');
              data.forEach((point) => {
                ctx.fillStyle = `rgba(101,115,255, ${Math.floor(point.v * 255)})`;
                const px = x(point.x);
                const py = y(point.y);
                ctx.beginPath();
                ctx.arc(px, py, 1, 0, 2 * Math.PI,true);
                ctx.fill();
              });
            }
          }
        />
        <svg
          width={width}
          height={height}
          style={{ position: 'relative', zIndex: 2 }}
        >
          <g
            transform={`translate(${margin.left}, 0)`}
            ref={g => d3.select(g).call(d3.axisLeft(y))}
          />
          <g
            transform={`translate(0, ${height - margin.bottom})`}
            ref={g => d3.select(g).call(d3.axisBottom(x))}
          />
          <g ref={
            g => d3.select(g)
              .call(
                d3.brush()
                  .extent([[margin.left, margin.top], [width - margin.right, height - margin.bottom]])
                  .on("start brush end", this.brushed)
              )
          }/>
        </svg>
      </div>
    )
  }
}