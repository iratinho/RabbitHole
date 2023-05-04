import React from 'react'

const Sidebar = () => {
    return <div className='sidebar'>
        <div className='sidebar-block'>Settings</div>
        <div className='sidebar-block sidebar-block-background'>
            <div className=''>Floor Grid</div>
            <div className='sidebar-block-entry-child'>
                <div>Visible</div>
                <div>
                    <div className="toggle slide">
                        <input id="a" type="checkbox" />
                            <label htmlFor="a">
                            <div className="card slide"></div>    
                        </label>
                    </div>
                </div>
            </div>

            <div className='sidebar-block-entry-child'>
                <div>Draw Axis</div>
                <div>
                    <div className="toggle slide">
                        <input id="b" type="checkbox" />
                            <label htmlFor="b">
                            <div className="card slide"></div>    
                        </label>
                    </div>
                </div>
            </div>

        </div>

    </div>
}

export default Sidebar;