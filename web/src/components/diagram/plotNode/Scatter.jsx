import * as d3 from 'd3';
import React from 'react';

window.d3 = d3

export default class extends React.Component {
  componentDidMount() {
    var margin = { top: 20, right: 20, bottom: 30, left: 40 },
      width = 960 - margin.left - margin.right,
      height = 500 - margin.top - margin.bottom;

    var x = d3.scaleLinear()
      .range([0, width]);

    var y = d3.scaleLinear()
      .range([height, 0]);

    var color = d3.schemeCategory10;
    const species = Object.create(null);
    var xAxis = d3.axisBottom()
      .scale(x);

    var yAxis = d3.axisLeft()
      .scale(y);

    var svg = d3.select(this.ref).append("svg")
      .attr("width", width + margin.left + margin.right)
      .attr("height", height + margin.top + margin.bottom)
      .style('background', 'white')
      .append("g")
      .attr("transform", "translate(" + margin.left + "," + margin.top + ")");

    d3.tsv("/assets/data.tsv").then((data) => {
      data.forEach(function (d) {
        d.sepalLength = +d.sepalLength;
        d.sepalWidth = +d.sepalWidth;
      });

      x.domain(d3.extent(data, function (d) { return d.sepalWidth; })).nice();
      y.domain(d3.extent(data, function (d) { return d.sepalLength; })).nice();

      svg.append("g")
        .attr("class", "x axis")
        .attr("transform", "translate(0," + height + ")")
        .call(xAxis)
        .append("text")
        .attr("class", "label")
        .attr("x", width)
        .attr("y", -6)
        .style("text-anchor", "end")
        .text("Sepal Width (cm)");

      svg.append("g")
        .attr("class", "y axis")
        .call(yAxis)
        .append("text")
        .attr("class", "label")
        .attr("transform", "rotate(-90)")
        .attr("y", 6)
        .attr("dy", ".71em")
        .style("text-anchor", "end")
        .text("Sepal Length (cm)")

      svg.selectAll(".dot")
        .data(data)
        .enter().append("circle")
        .attr("class", "dot")
        .attr("r", 3.5)
        .attr("cx", function (d) { return x(d.sepalWidth); })
        .attr("cy", function (d) { return y(d.sepalLength); })
        .style("fill", function (d) {
          const len = Object.keys(species).length;
          species[d.species] = color[len === 0 ? 0 : len - 1];
          return species[d.species]; 
        });

      var legend = svg.selectAll(".legend")
        .data(Object.keys(species))
        .enter().append("g")
        .attr("class", "legend")
        .attr("transform", function (d, i) { return "translate(0," + i * 20 + ")"; });

      legend.append("rect")
        .attr("x", width - 18)
        .attr("width", 18)
        .attr("height", 18)
        .style("fill", (d) => species[d]);

      legend.append("text")
        .attr("x", width - 24)
        .attr("y", 9)
        .attr("dy", ".35em")
        .style("text-anchor", "end")
        .text(function (d) { return d; });

    });
  }

  render() {
    return <div ref={ref => this.ref = ref}></div>
  }
}