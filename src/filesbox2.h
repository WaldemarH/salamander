#pragma once

struct ViewMode
{
    public: enum Value
    {
        brief,          //Several columns of data; only horizontal scrollbar; bottom items are always fully visible.
        detailed,       //One column of data; both scrollbars are displayed; the last row may not be fully visible.
        icons,          //Large icons from left to right and then top to bottom; only vertical scrollbar.
        thumbnails,     //Views from left to right and then top to bottom; only vertical scrolling.
        tiles           //Large (48x48) icons from left to right and then top to bottom; vertical scroll only.
    };
};

