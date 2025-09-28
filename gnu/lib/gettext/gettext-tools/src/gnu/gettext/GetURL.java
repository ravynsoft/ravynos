/* Fetch an URL's contents.
 * Copyright (C) 2001, 2008, 2020 Free Software Foundation, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

package gnu.gettext;

import java.io.*;
import java.net.*;

/**
 * Fetch an URL's contents and emit it to standard output.
 * Exit code: 0 = success
 *            1 = failure
 *            2 = timeout
 * @author Bruno Haible
 */
public class GetURL {
  // Use a separate thread to signal a timeout error if the URL cannot
  // be accessed and completely read within a given amount of time.
  private static long timeout = 30*1000; // 30 seconds
  private boolean done;
  private Thread timeoutThread;
  public void fetch (String s) {
    URL url;
    try {
      url = new URL(s);
    } catch (MalformedURLException e) {
      System.exit(1);
      return;
    }
    done = false;
    timeoutThread =
      new Thread() {
        public void run () {
          try {
            sleep(timeout);
            if (!done) {
              System.exit(2);
            }
          } catch (InterruptedException e) {
          }
        }
      };
    timeoutThread.start();
    try {
      URLConnection connection = url.openConnection();
      // Override the User-Agent string, so as to not reveal the Java version.
      connection.setRequestProperty("User-Agent", "urlget");
      InputStream istream = new BufferedInputStream(connection.getInputStream());
      OutputStream ostream = new BufferedOutputStream(System.out);
      for (;;) {
        int b = istream.read();
        if (b < 0) break;
        ostream.write(b);
      }
      ostream.close();
      System.out.flush();
      istream.close();
    } catch (IOException e) {
      //e.printStackTrace();
      System.exit(1);
    }
    done = true;
  }
  public static void main (String[] args) {
    if (args.length != 1)
      System.exit(1);
    (new GetURL()).fetch(args[0]);
    System.exit(0);
  }
}
