/*
 * Copyright (C) 2011  Southern Storm Software, Pty Ltd.
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

import java.util.ArrayList;
import java.util.List;

import android.content.Context;
import android.graphics.drawable.Drawable;
import android.graphics.Typeface;
import android.text.SpannableString;
import android.text.style.ForegroundColorSpan;
import android.text.style.ImageSpan;
import android.text.style.StrikethroughSpan;
import android.text.style.StyleSpan;
import android.text.style.UnderlineSpan;

public class RichTextFormatter {

    private class Style {
        int textStyle;
        int color;
        int start;
        int end;
        int resourceId;
    };

    private Context context;
    private StringBuilder builder;
    private List<Style> styles;
    private int textStyle;
    private int color;
    private int startSpan;
    private boolean needEndParagraph;
    private boolean needEndLine;

    private static final int STRIKE_THROUGH = 0x00010000;
    private static final int UNDERLINE      = 0x00020000;

    public RichTextFormatter(Context context) {
        this.context = context;
        this.builder = new StringBuilder();
        this.styles = new ArrayList<Style>();
        this.textStyle = 0;
        this.color = 0;
        this.startSpan = 0;
        this.needEndParagraph = false;
        this.needEndLine = false;
    }

    /**
     * Appends a text string to this formatter.
     *
     * @param str the string to append
     * @return this formatter object
     */
    public RichTextFormatter append(String str) {
        if (needEndParagraph || needEndLine) {
            builder.append('\n');
            needEndParagraph = false;
            needEndLine = false;
        }
        if (str != null)
            builder.append(str);
        return this;
    }

    /**
     * Appends a newline to this formatter.
     *
     * @return this formatter object
     */
    public RichTextFormatter nl() {
        needEndLine = true;
        return this;
    }

    /**
     * Appends a paragraph separator to this formatter.
     *
     * @return this formatter object
     */
    public RichTextFormatter endParagraph() {
        needEndParagraph = true;
        return this;
    }

    /**
     * Sets the following text to be bold or not.
     *
     * @param bold true to enable bold, false to disable bold
     */
    public void setBold(boolean bold) {
        int newStyle = textStyle;
        if (bold)
            newStyle |= Typeface.BOLD;
        else
            newStyle &= ~Typeface.BOLD;
        if (newStyle != textStyle) {
            flushStyle();
            textStyle = newStyle;
        }
    }

    /**
     * Sets the following text to be italic or not.
     *
     * @param italic true to enable italic, false to disable italic
     */
    public void setItalic(boolean italic) {
        int newStyle = textStyle;
        if (italic)
            newStyle |= Typeface.ITALIC;
        else
            newStyle &= ~Typeface.ITALIC;
        if (newStyle != textStyle) {
            flushStyle();
            textStyle = newStyle;
        }
    }

    /**
     * Sets the following text to be strike-through or not.
     *
     * @param strike true to enable strike-through, false to disable
     */
    public void setStrikeThrough(boolean strike) {
        int newStyle = textStyle;
        if (strike)
            newStyle |= STRIKE_THROUGH;
        else
            newStyle &= ~STRIKE_THROUGH;
        if (newStyle != textStyle) {
            flushStyle();
            textStyle = newStyle;
        }
    }

    /**
     * Sets the following text to be underlined or not.
     *
     * @param underline true to enable underline, false to disable
     */
    public void setUnderline(boolean underline) {
        int newStyle = textStyle;
        if (underline)
            newStyle |= UNDERLINE;
        else
            newStyle &= ~UNDERLINE;
        if (newStyle != textStyle) {
            flushStyle();
            textStyle = newStyle;
        }
    }

    /**
     * Sets the text foreground color for the following text.
     *
     * @param color the foreground color to set, or 0 for the default color
     */
    public void setColor(int color) {
        if (this.color != color) {
            flushStyle();
            this.color = color;
        }
    }
    
    /**
     * Adds an image resource to the output at this position.
     * 
     * @param resourceId the identifier of the image resource
     */
    public void addImage(int resourceId) {
        flushStyle();
        builder.append(' ');
        int length = builder.length();
        Style style = new Style();
        style.textStyle = 0;
        style.color = 0;
        style.start = startSpan;
        style.end = length;
        style.resourceId = resourceId;
        styles.add(style);
        startSpan = length;
    }

    /**
     * Converts this formatter to a SpannableString suitable for
     * setting on a TextView.
     *
     * @return the spannable sring
     */
    public SpannableString toSpannableString() {
        flushStyle();
        SpannableString str = new SpannableString(builder.toString());
        for (Style style: styles) {
            if (style.resourceId != 0) {
                Drawable drawable = context.getResources().getDrawable(style.resourceId);
                drawable.setBounds(0, 0, 16, 16);
                str.setSpan(new ImageSpan(drawable, ImageSpan.ALIGN_BASELINE), style.start, style.end, 0);
                continue;
            }
            int textStyle = style.textStyle;
            if ((textStyle & 0xFFFF) != 0)
                str.setSpan(new StyleSpan(textStyle & 0xFFFF), style.start, style.end, 0);
            if ((textStyle & STRIKE_THROUGH) != 0)
                str.setSpan(new StrikethroughSpan(), style.start, style.end, 0);
            if ((textStyle & UNDERLINE) != 0)
                str.setSpan(new UnderlineSpan(), style.start, style.end, 0);
            if (style.color != 0)
                str.setSpan(new ForegroundColorSpan(style.color), style.start, style.end, 0);
        }
        return str;
    }

    private void flushStyle() {
        int length = builder.length();
        if (startSpan < length) {
            Style style = new Style();
            style.textStyle = textStyle;
            style.color = color;
            style.start = startSpan;
            style.end = length;
            style.resourceId = 0;
            styles.add(style);
            startSpan = length;
        }
    }
}
