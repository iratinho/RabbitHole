import Component, { ToggleButtonArgs } from './component';
import React from 'react';

const ToggleButton = (props: ToggleButtonArgs) => {  
  console.log("Hello")
  return React.createElement(Component, {
    checkedLabel: props.checkedLabel,
    uncheckedLabel: props.uncheckedLabel,
  });
};

export default ToggleButton;
  