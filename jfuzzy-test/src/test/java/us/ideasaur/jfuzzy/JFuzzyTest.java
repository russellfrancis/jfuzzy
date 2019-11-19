package us.ideasaur.jfuzzy;
/*
 * JNI bindings for the libfuzzy shared library.
 * Copyright (C) 2019 Russell Francis
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
import static org.junit.Assert.assertEquals;

import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import org.junit.Test;

public class JFuzzyTest {
  @Test(expected = RuntimeException.class)
  public void testCompare_InvalidSigs_ThrowsException() {
    JFuzzy.compare("A", "B");
  }

  @Test
  public void testCompare_Identical_Returns100() {
    String sig1 = "3:a6/E:asE";
    assertEquals(100, JFuzzy.compare(sig1, sig1));
  }

  @Test
  public void testHash_Buffer() {
    byte[] bytes = "Hello, world!".getBytes(StandardCharsets.UTF_8);
    assertEquals("3:a6/E:asE", JFuzzy.hash(bytes));
  }

  @Test(expected = NullPointerException.class)
  public void testHash_Null_ThrowsException() {
    JFuzzy.hash((byte[]) null);
  }

  @Test
  public void testHash_File() throws IOException {
    File file = File.createTempFile("jfuzzy", "data");
    file.deleteOnExit();

    try (OutputStream outs = new FileOutputStream(file)) {
      outs.write("Hello, world!".getBytes(StandardCharsets.UTF_8));
    }

    assertEquals("3:a6/E:asE", JFuzzy.hash(file));
  }

  @Test(expected = NullPointerException.class)
  public void testHash_NullFile_ThrowsException() {
    JFuzzy.hash((File) null);
  }

  @Test
  public void testHash_ValidInput() throws IOException {
    try (ByteArrayInputStream ins = new ByteArrayInputStream("Hello, world!".getBytes(StandardCharsets.UTF_8))) {
      final String hash = JFuzzy.hash(ins);
      assertEquals("3:a6/E:asE", hash);
    }
  }
}
