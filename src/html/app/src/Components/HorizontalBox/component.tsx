import * as React from "react";
import "./style.css"

export interface HorizontalBoxArgs {
}

class HorizontalBoxComponent extends React.Component<HorizontalBoxArgs, HorizontalBoxArgs> {
    constructor(props: HorizontalBoxArgs){
        super(props);
        this.state = props;
    }
}