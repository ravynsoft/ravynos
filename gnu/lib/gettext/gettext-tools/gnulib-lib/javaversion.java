/* Show the Java version.
 * Copyright (C) 2006, 2009-2023 Free Software Foundation, Inc.
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * This program shows the Java version.
 *
 * This program _must_ be compiled with
 *   javac -d . -target 1.1 javaversion.java
 * since its purpose is to show the version of _any_ Java implementation.
 *
 * @author Bruno Haible
 */
public class javaversion {
  public static void main (String[] args) {
    System.out.println(System.getProperty("java.specification.version"));
  }
}
