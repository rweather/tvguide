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

import java.util.Map;
import java.util.TreeMap;

public class IconFactory {

    private Map<String, Integer> resources;
    private static IconFactory factory;
    
    private IconFactory() {
        resources = new TreeMap<String, Integer>();
        resources.put("abc2.png", R.drawable.abc2);
        resources.put("abc3.png", R.drawable.abc3);
        resources.put("abc-news-24.png", R.drawable.abc_news_24);
        resources.put("abc.png", R.drawable.abc);
        resources.put("dig-radio.png", R.drawable.dig_radio);
        resources.put("eleven.png", R.drawable.eleven);
        resources.put("gem.png", R.drawable.gem);
        resources.put("go.png", R.drawable.go);
        resources.put("nbn.png", R.drawable.nbn);
        resources.put("nine.png", R.drawable.nine);
        resources.put("one.png", R.drawable.one);
        resources.put("prime.png", R.drawable.prime);
        resources.put("sbs.png", R.drawable.sbs);
        resources.put("sbs-two.png", R.drawable.sbs_two);
        resources.put("seven-mate.png", R.drawable.seven_mate);
        resources.put("seven.png", R.drawable.seven);
        resources.put("seven-two.png", R.drawable.seven_two);
        resources.put("southern-cross-television.png", R.drawable.southern_cross_television);
        resources.put("southern-cross-ten.png", R.drawable.southern_cross_ten);
        resources.put("ten.png", R.drawable.ten);
        resources.put("win.png", R.drawable.win);
    }

    public static IconFactory getInstance() {
        if (factory == null)
            factory = new IconFactory();
        return factory;
    }

    public int getChannelIconResource(String name) {
        if (resources.containsKey(name))
            return resources.get(name);
        else
            return 0;
    }
}
