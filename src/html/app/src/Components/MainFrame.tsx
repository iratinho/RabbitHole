import React from 'react'
import ToolbarComponent from './Toolbar';
import Sidebar from './Sidebar';

const MainFrameComponent = () => {
    return <div className="container">
                <Sidebar></Sidebar>
                {/* <ToolbarComponent></ToolbarComponent> */}
            </div>
}

export default MainFrameComponent;
