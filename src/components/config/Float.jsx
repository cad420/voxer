import React from 'react'
import Number from './Number'

function checkFloat(value) {
  if (isNaN(parseFloat(value))) {
    return false
  }
  return true
}

export default ({ label, value }) => (
  <Number
    label={label}
    value={value}
    validator={checkFloat}
  />
)
