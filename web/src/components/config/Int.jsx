import React from 'react'
import Number from './Number'

function checkInt(value) {
  if (isNaN(parseInt(value, 10)) && value.indexOf('.') === -1) {
    return false
  }
  return true
}

export default ({ label, value, onChange }) => (
  <Number
    label={label}
    value={value}
    validator={checkInt}
    onChange={onChange}
  />
)
