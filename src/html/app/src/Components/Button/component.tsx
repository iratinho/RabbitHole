import * as React from "react";
import "./style.css"

export interface ButtonArgs {
  text?: string;
  width?: number;
  height?: number
}

interface ButtonState extends ButtonArgs {}

class Button extends React.Component<ButtonArgs, ButtonState> {
  constructor(props: ButtonArgs){
    super(props);
    this.state = {
      text: props.text,
      width: props.width,
      height: props.height,
    };
  }

  setWidth(value: number) {
    this.setState({width: value});
  }

  getWidth(): number {
    return this.state.width as number;
  }

  setHeight(value: number) {
    this.setState({height: value});
  }

  getHeight(): number {
    return this.state.height as number;
  }

  setText(value: string) {
    this.setState({text: value});
  }

  getText(): string {
    return this.state.text as string;
  }

  render() {    
    const buttonStyle = {
      width: `${this.getWidth()}px`,
      height: `${this.getHeight()}px`
    }

    console.log("rendering")
    
    return <button style={buttonStyle}>{this.getText()}</button>;
  }
}

export default Button;
