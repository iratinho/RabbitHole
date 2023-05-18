import { v4 as uuidv4 } from 'uuid';
import * as React from "react";
import './style.css';
import { useMemo, useState, useEffect } from 'react';

export interface ToggleButtonArgs {
    checkedLabel?: string;
    uncheckedLabel?: string;
}

const Component = (props: ToggleButtonArgs) => {
  const [checkedLabel] = useState(props.checkedLabel)
  const [uncheckedLabel] = useState(props.uncheckedLabel)

  const UUID = useMemo(() =>{ return uuidv4(); }, []);
  const toggleLabelStr = useMemo(() => { return `toggle-label-container-${UUID}` }, [UUID]);

  // Generates a new css style for :before and :after for the labels to show diferent text for diferent Component instances
  useEffect(() => {
    const generateDynamicStyle = () => {
      const dynamicStyles = 
      `.${toggleLabelStr}::after { content: '${checkedLabel}'; } 
       .${toggleLabelStr}::before { content: '${uncheckedLabel}'; }`;
          return dynamicStyles;
    };

    const styleElement = document.createElement('style');
    styleElement.innerHTML = generateDynamicStyle();
    document.head.appendChild(styleElement);

    return () => {
      const styleElement = document.querySelector(`style[title="${toggleLabelStr}"]`);
      if (styleElement) {
        document.head.removeChild(styleElement);
      }
    };
  }, [checkedLabel, toggleLabelStr, uncheckedLabel]);

  return (
    <div className={`toggle-button-container slide ${toggleLabelStr}`}>
      <input id={`f-${UUID}`} type="checkbox" />
      <label htmlFor={`f-${UUID}`} className='toggle-button-label'>
        <div className='toggle-button-cover slide'></div>
      </label>
    </div>
  );
}

export default Component;