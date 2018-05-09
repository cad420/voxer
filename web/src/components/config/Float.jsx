import React from 'react'
import Number from './Number'

function checkFloat(value) {
  if (isNaN(parseFloat(value))) {
    return false
  }
  return true
}

export default ({ label, value, onChange, max, min }) => (
  <Number
    label={label}
    value={value}
    max={max} min={min}    
    validator={checkFloat}
    onChange={onChange}
  />
)
