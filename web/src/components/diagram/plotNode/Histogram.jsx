function histogramChart() {
  var margin = { top: 0, right: 0, bottom: 20, left: 0 },
    width = 960,
    height = 500;

  var histogram = d3.histogram(),
    x = d3.scaleOrdinal(),
    y = d3.scaleLinear(),
    xAxis = d3.svg.axis().scaleBottom(x).tickSize(6, 0);

  function chart(selection) {
    selection.each(function (data) {

      // Compute the histogram.
      data = histogram(data);

      // Update the x-scale.
      x.domain(data.map(function (d) { return d.x; }))
        .rangeRoundBands([0, width - margin.left - margin.right], .1);

      // Update the y-scale.
      y.domain([0, d3.max(data, function (d) { return d.y; })])
        .range([height - margin.top - margin.bottom, 0]);

      // Select the svg element, if it exists.
      var svg = d3.select(this).selectAll("svg").data([data]);

      // Otherwise, create the skeletal chart.
      var gEnter = svg.enter().append("svg").append("g");
      gEnter.append("g").attr("class", "bars");
      gEnter.append("g").attr("class", "x axis");

      // Update the outer dimensions.
      svg.attr("width", width)
        .attr("height", height);

      // Update the inner dimensions.
      var g = svg.select("g")
        .attr("transform", "translate(" + margin.left + "," + margin.top + ")");

      // Update the bars.
      var bar = svg.select(".bars").selectAll(".bar").data(data);
      bar.enter().append("rect");
      bar.exit().remove();
      bar.attr("width", x.rangeBand())
        .attr("x", function (d) { return x(d.x); })
        .attr("y", function (d) { return y(d.y); })
        .attr("height", function (d) { return y.range()[0] - y(d.y); })
        .order();

      // Update the x-axis.
      g.select(".x.axis")
        .attr("transform", "translate(0," + y.range()[0] + ")")
        .call(xAxis);
    });
  }

  chart.margin = function (_) {
    if (!arguments.length) return margin;
    margin = _;
    return chart;
  };

  chart.width = function (_) {
    if (!arguments.length) return width;
    width = _;
    return chart;
  };

  chart.height = function (_) {
    if (!arguments.length) return height;
    height = _;
    return chart;
  };

  // Expose the histogram's value, range and bins method.
  // d3.rebind(chart, histogram, "value", "range", "bins");

  // Expose the x-axis' tickFormat method.
  // d3.rebind(chart, xAxis, "tickFormat");

  return chart;
}

import * as d3 from 'd3';
import React from 'react';

window.d3 = d3

export default class extends React.Component {
  componentDidMount() {
    d3.select(this.ref)
      .datum(irwinHallDistribution(10000, 10))
      .call(histogramChart()
        .bins(d3.scaleLinear().ticks(20))
        .tickFormat(d3.format(".02f")));

    function irwinHallDistribution(n, m) {
      var distribution = [];
      for (var i = 0; i < n; i++) {
        for (var s = 0, j = 0; j < m; j++) {
          s += Math.random();
        }
        distribution.push(s / m);
      }
      return distribution;
    }
  }

  render() {
    return <div ref={ref => this.ref = ref}></div>
  }
}