/*
 * Copyright (C) 2011,2012  Southern Storm Software, Pty Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

package com.southernstorm.tvguide;

import android.content.Context;
import android.util.AttributeSet;
import android.view.View;
import android.view.ViewGroup;

/**
 * Special layout that organizes children into two columns, where the first column is
 * defined by the first child and all other children go into the second column.
 * The last child in each column will be stretched to fill the maximum height.
 */
public class TwoColumnLayout extends ViewGroup {

    private static final int PADDING = 3;
    
    public TwoColumnLayout(Context context) {
        super(context);
    }

    public TwoColumnLayout(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    public TwoColumnLayout(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
    }

    @Override
    protected void onLayout(boolean changed, int left, int top, int right, int bottom) {
        int width = right - left;
        int height = bottom - top;
        int count = getChildCount();
        int leftColumnWidth = 0;
        int rightColumnTop = 0;
        for (int index = 0; index < count; ++index) {
            View child = getChildAt(index);
            if (child.getVisibility() == GONE)
                continue;
            if (index == 0) {
                leftColumnWidth = child.getMeasuredWidth();
                child.layout(0, 0, leftColumnWidth, height);
                leftColumnWidth += PADDING;
            } else if (index < (count - 1)) {
                int rightColumnBottom = rightColumnTop + child.getMeasuredHeight();
                child.layout(leftColumnWidth, rightColumnTop, width, rightColumnBottom);
                rightColumnTop = rightColumnBottom;
            } else {
                child.layout(leftColumnWidth, rightColumnTop, width, height);
            }
        }
    }

    private int createChildSpec(int parentSpec, int childSize) {
        if (childSize == LayoutParams.MATCH_PARENT) {
            return parentSpec;
        } else if (childSize == LayoutParams.WRAP_CONTENT) {
            if (MeasureSpec.getMode(parentSpec) == MeasureSpec.UNSPECIFIED)
                return parentSpec;
            return MeasureSpec.makeMeasureSpec(MeasureSpec.getSize(parentSpec), MeasureSpec.AT_MOST);
        } else {
            return MeasureSpec.makeMeasureSpec(childSize, MeasureSpec.EXACTLY);
        }
    }

    @Override
    protected void onMeasure(int widthMeasureSpec, int heightMeasureSpec) {
        int count = getChildCount();
        int leftColumnWidth = 0;
        int rightColumnWidth = 0;
        int leftColumnHeight = 0;
        int rightColumnHeight = 0;
        
        int widthSpec = widthMeasureSpec;
        
        for (int index = 0; index < count; ++index) {
            View child = getChildAt(index);
            if (child.getVisibility() == GONE)
                continue;
            
            ViewGroup.LayoutParams params = child.getLayoutParams();
            int childWidthSpec = createChildSpec(widthSpec, params.width);
            int childHeightSpec = createChildSpec(heightMeasureSpec, params.height);

            child.measure(childWidthSpec, childHeightSpec);
            if (index == 0) {
                leftColumnWidth = child.getMeasuredWidth();
                leftColumnHeight = child.getMeasuredHeight();

                // Reduce the parent width spec for the right column.
                widthSpec = MeasureSpec.makeMeasureSpec(MeasureSpec.getSize(widthSpec) - leftColumnWidth - PADDING, MeasureSpec.getMode(widthSpec));
            } else {
                rightColumnWidth = Math.max(rightColumnWidth, child.getMeasuredWidth());
                rightColumnHeight = rightColumnHeight + child.getMeasuredHeight();
            }
        }
        
        int width = leftColumnWidth + rightColumnWidth + PADDING;
        int height = Math.max(leftColumnHeight, rightColumnHeight);
        
        setMeasuredDimension(resolveSize(width, widthMeasureSpec), resolveSize(height, heightMeasureSpec));
    }
}
