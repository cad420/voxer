import React, { Component } from 'react'
import LinearPieceWise from './LinearPiesewise'
import equals from 'mout/src/lang/deepEquals';

export default class PieceWiseFunctionEditorWidget extends Component {
  constructor(props) {
    super(props);
    this.state = {
      height: props.height,
      width: props.width,
      activePoint: -1,
    };
  }

  componentDidMount() {
    const canvas = this.canvas;
    this.editor = new LinearPieceWise(canvas);

    this.editor.setControlPoints(this.props.value);
    this.editor.render();
    this.editor.onChange(this.updatePoints);
    this.editor.onEditModeChange(this.props.onEditModeChange);
  }

  componentWillReceiveProps(newProps) {
    const newState = {};
    if (!equals(newProps.value, this.props.value)) {
      this.editor.setControlPoints(newProps.value, this.editor.activeIndex);
      if (this.state.activePoint >= newProps.value.length) {
        newState.activePoint = -1;
      }
    }
    if (newProps.width !== this.props.width) {
      newState.width = newProps.width;
    }
    if (newProps.height !== this.props.height) {
      newState.height = newProps.height;
    }
    this.setState(newState);
  }

  componentDidUpdate(prevProps, prevState) {
    if (
      this.state.width !== prevState.width ||
      this.state.height !== prevState.height
    ) {
      this.editor.render();
    }
  }

  componentWillUnmount() {
    if (this.sizeSubscription) {
      this.sizeSubscription.unsubscribe();
      this.sizeSubscription = null;
      this.editor.destroy();
      this.editor = null;
    }
  }

  updatePoints = (newPoints, envelope) => {
    const activePoint = this.editor.activeIndex;
    this.setState({ activePoint });
    const dataPoints = this.props.value.map((pt) => ({
      x: pt.x,
      y: pt.y,
      color: pt.color
    }));
    const newDataPoints = newPoints.map((pt) => ({
      x: pt.x,
      y: pt.y,
      color: pt.color
    }));
    this.oldPoints = dataPoints;
    if (this.props.onChange) {
      this.props.onChange(this.props.label, newDataPoints);
    }
  }

  updateActivePointDataValue = (e) => {
    if (this.state.activePoint === -1) {
      return;
    }
    const value = parseFloat(e.target.value);
    const points = this.props.value.map((pt) => ({
      x: pt.x,
      y: pt.y,
      color: pt.color
    }));
    points[this.state.activePoint].x =
      (value - this.props.rangeMin) /
      (this.props.rangeMax - this.props.rangeMin);
    this.editor.setControlPoints(points, this.state.activePoint);
  }

  updateActivePointOpacity = (e) => {
    if (this.state.activePoint === -1) {
      return;
    }
    const value = parseFloat(e.target.value);
    const points = this.props.value.map((pt) => ({
      x: pt.x,
      y: pt.y,
      color: pt.color
    }));
    points[this.state.activePoint].y = value;
    this.editor.setControlPoints(points, this.state.activePoint);
  }

  updateActivePointColor = (e) => {
    if (this.state.activePoint === -1) {
      return;
    }
    const points = this.props.value.map((pt) => ({
      x: pt.x,
      y: pt.y,
      color: pt.color
    }));
    points[this.state.activePoint].color = e.target.value;
    this.editor.setControlPoints(points, this.state.activePoint);
  }

  removePoint = (e) => {
    if (this.state.activePoint === -1) {
      return;
    }
    const points = this.props.value.map((pt) => ({
      x: pt.x,
      y: pt.y,
      color: pt.color
    }));
    points.splice(this.state.activePoint, 1);
    this.editor.setActivePoint(-1);
    this.editor.setControlPoints(points);
  }

  render() {
    const activePointDataValue =
      (this.state.activePoint !== -1
        ? this.props.value[this.state.activePoint].x
        : 0.5) *
        (this.props.rangeMax - this.props.rangeMin) +
      this.props.rangeMin;
    const activePointOpacity =
      this.state.activePoint !== -1
        ? this.props.value[this.state.activePoint].y
        : 0.5;
    return (
      <div
        style={{ userSelect: 'none' }}
        ref={(c) => {
          this.rootContainer = c;
        }}
      >
        <canvas
          height={this.state.height}
          ref={(c) => {
            this.canvas = c;
          }}
        />
        {this.props.hidePointControl ? null : (
          <div>
            <div>
              <input type="color" name="" id="" onChange={this.updateActivePointColor} />
              &nbsp;&nbsp;&nbsp;
              <button onClick={this.removePoint}>Delete</button>
            </div>
            <br/>
            <div>
              <div>
                <label>Data</label>&nbsp;&nbsp;
                <input
                  type="number"
                  step="any"
                  min={this.props.rangeMin}
                  max={this.props.rangeMax}
                  value={activePointDataValue.toFixed(2)}
                  onChange={this.updateActivePointDataValue}
                />
              </div>
              <br/>
              <div>
                <label>Opacity</label>&nbsp;&nbsp;
                <input
                  type="number"
                  step={0.01}
                  min={0}
                  max={1}
                  value={Math.floor(100 * activePointOpacity) / 100}
                  onChange={this.updateActivePointOpacity}
                />
              </div>
              <br/>
            </div>
            <div>
              
            </div>
          </div>
        )}
      </div>
    );
  }
}
