import React, { useRef, createRef } from 'react'
import Button from './Button/component';
import ToggleButton from './ToggleButton/component';

const Sidebar = () => {
    const componentRef = useRef<Button>(null);
                
return <div className='sidebar'>
                <Button ref={componentRef} width={100} height={100}></Button>
                <button onClick={() => {componentRef.current?.setWidth(2); componentRef.current?.setHeight(2)} }>Set Width</button>

        <div className='sidebar-block'>Settings</div>
        <div className='sidebar-block sidebar-block-background'>
            <div className=''>Floor Grid</div>
            <div className='sidebar-block-entry-child'>
                <div>Visible</div>
                <div>
                    <ToggleButton checkedLabel='Hello' uncheckedLabel='Bye'></ToggleButton>
                </div>
            </div>

            <div className='sidebar-block-entry-child'>
                <div>Draw Axis</div>
                <div>
                    <ToggleButton checkedLabel='123' uncheckedLabel='321'></ToggleButton>
                </div>
            </div>

        </div>

    </div>
}

export default Sidebar;