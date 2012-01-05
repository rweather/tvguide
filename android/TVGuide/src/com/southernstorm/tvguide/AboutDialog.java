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

import android.app.Dialog;
import android.content.Context;
import android.text.SpannableString;
import android.text.method.LinkMovementMethod;
import android.view.Window;
import android.widget.TextView;

public class AboutDialog extends Dialog {

    public AboutDialog(Context context) {
        super(context);
        
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.about_dialog);
        
        TextView linkView = (TextView)findViewById(R.id.about_links);
        CharSequence seq = linkView.getText();
        if (seq instanceof SpannableString)
            return;
        String str = seq.toString();
        
        RichTextFormatter formatter = new RichTextFormatter(context);
        int posn = 0;
        while (posn < str.length()) {
            int next = str.indexOf("[[", posn);
            if (next >= 0) {
                if (posn < next)
                    formatter.append(str.substring(posn, next));
                int divider = str.indexOf("][", next);
                int terminator = str.indexOf("]]", divider);
                formatter.addLink(str.substring(divider + 2, terminator), str.substring(next + 2, divider));
                posn = terminator + 2;
            } else {
                formatter.append(str.substring(posn));
                break;
            }
        }
        linkView.setText(formatter.toSpannableString());
        
        linkView.setMovementMethod(LinkMovementMethod.getInstance());
    }
}
