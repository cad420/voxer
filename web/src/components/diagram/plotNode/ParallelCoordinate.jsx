import * as d3 from 'd3';
import React from 'react';
import './ParallelCoordinate.css';

window.d3 = d3

export default class extends React.Component {
  componentDidMount() {
    const margin = { top: 30, right: 10, bottom: 10, left: 10 };
    const width = 960 - margin.left - margin.right;
    const height = 500 - margin.top - margin.bottom;

    const x = d3.scalePoint().range([0, width]).padding(1);
    const y = {};
    const dragging = {};

    const line = d3.line();
    const axis = d3.axisLeft();

    let background;
    let foreground;

    const svg = d3.select(this.ref).append('svg')
      .attr('width', width + margin.left + margin.right)
      .attr('height', height + margin.top + margin.bottom)
      .style('background', 'white')
      .append('g')
      .attr('transform', `translate(${margin.left},${margin.top})`);

    d3.csv('/assets/cars.csv').then(cars => {
      // Extract the list of dimensions and create a scale for each.
      const dimensions = d3.keys(cars[0]).filter((d) => (
        d != 'name' &&
        (
          y[d] = d3.scaleLinear()
            .domain(d3.extent(cars, p => +p[d]))
            .range([height, 0])
        )
      ));
      x.domain(dimensions);

      // Add grey background lines for context.
      background = svg.append('g')
        .attr('class', 'background')
        .selectAll('path')
        .data(cars)
        .enter()
        .append('path')
        .attr('d', path);

      // Add blue foreground lines for focus.
      foreground = svg.append('g')
        .attr('class', 'foreground')
        .selectAll('path')
        .data(cars)
        .enter().append('path')
        .attr('d', path);

      // Add a group element for each dimension.
      const g = svg.selectAll('.dimension')
        .data(dimensions)
        .enter()
        .append('g')
        .attr('class', 'dimension')
        .attr('transform', d => `translate(${x(d)})`);

      // Add an axis and title.
      g.append('g')
        .attr('class', 'axis')
        .each(function (d) { d3.select(this).call(axis.scale(y[d])); })
        .append('text')
        .style('text-anchor', 'middle')
        .attr('y', -9)
        .text(d => d);

      // Add and store a brush for each axis.
      g.append('g')
        .attr('class', 'brush')
        .each(function (d) {
          d3.select(this).call(y[d].brush = d3.brushY()
            .extent([[-10, 0], [10, height]])
            .on('brush', brush)
            .on('end', brush)
          );
        })
        .selectAll('rect')
        .attr('x', -8)
        .attr('width', 16);

      // Returns the path for a given data point.
      function path(d) {
        return line(dimensions.map(p => [x(p), y[p](d[p])]));
      }

      // Handles a brush event, toggling the display of foreground lines.
      function brush() {
        const actives = [];
        svg.selectAll('.brush')
          .filter(function (d) {
            y[d].brushSelectionValue = d3.brushSelection(this);
            return d3.brushSelection(this);
          })
          .each(function (d) {
            actives.push({
              dimension: d,
              extent: d3.brushSelection(this).map(y[d].invert)
            })
          });

        foreground.style('display', d => (
          actives.every(active => (
            active.extent[1] <= d[active.dimension] &&
              d[active.dimension] <= active.extent[0]
          )) ? null : 'none'
        ));
      }


    });
  }

  render() {
    return <div className='para-coord' ref={ref => this.ref = ref}></div>
  }
}
