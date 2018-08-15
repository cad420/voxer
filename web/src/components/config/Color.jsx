import React, { Component } from 'react'
import Pickr from 'pickr-widget';
import 'pickr-widget/dist/pickr.min.css'

export default class extends Component {
  componentDidMount() {
    const { value = '#ffffff' } = this.props;
    this.pickr = Pickr.create({
      el: this.ref,
      appendToBody: true,
      default: value.substring(1),
      components: {
        preview: true,
        hue: true,
        interaction: {
          hex: true,
          rgba: true,
          hsva: true,
          input: true,
          save: true
        }
      },
      onSave: (hsva) => {
        const { value } = this.props;
        const hex = hsva.toHEX();
        const color = `#${hex[0]}${hex[1]}${hex[2]}`;
        if (color === value) return;
        this.handleChange(hsva.toHEX());
      }
    });
  }

  componentWillReceiveProps(nextProps) {
    if ('value' in nextProps) {
      this.pickr.setColor(nextProps.value);
    }
  }

  handleChange = (color) => {
    const { onChange, label } = this.props
    const value = `#${color[0]}${color[1]}${color[2]}`
    onChange && onChange(label, value)
  }

  render() {
    const { label, value } = this.props
    return (
      <div>
        {label && <div>{label}</div>}
        <div ref={ref => this.ref = ref}></div>
      </div>
    )
  }
}